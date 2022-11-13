/**
 * @file ass3.cpp
 * @brief  Source code of the Programming Assignment 3 COEN346 Fall 2022
 * @author Joshua Lafleur 40189389, Eden Bouskila 40170349, Ahmed Enani 26721281
 * @version 1.0
 * @date 2022-11-12
 *
 * @note We certify that this work is our own work and meet's the faculties
 * certification of originality.
 */

/******************************************************************************
 *                             I N C L U D E S
 ******************************************************************************/

#include "ass3.h"
#include "ass3-1.h"
#include <cstdlib>
#include <fstream>
#include <pthread.h>
#include <string>
#include <unistd.h>

/******************************************************************************
 *                             T Y P E D E F S
 ******************************************************************************/

typedef struct {
    string var_id;
    unsigned int val;
} page_S;

typedef struct {
    page_S page;
    unsigned int timestamp;
} main_mem_S;

typedef struct {
  pthread_t thread;
  unsigned int start_time;
  bool running;
  thread_data_S arg;
  void* next_proc;
} proc_S;

typedef struct {
  proc_S* running_proc;
} cpu_S;

/******************************************************************************
 *                           P U B L I C  V A R S
 ******************************************************************************/

output_S out;
unsigned int time_msec;

/******************************************************************************
 *                         P R I V A T E  V A R S
 ******************************************************************************/

static main_mem_S* mem;
static cpu_S* cpus;
static proc_S* procs;
static unsigned int cpu_count;
static unsigned int mem_count;
static unsigned int procs_count;

static pthread_t proc_timebase;
static pthread_t mem_manager;
static pthread_t proc_scheduler;

static actions_E mem_state;
static unsigned int mem_return;
static string mem_arg;
static pthread_mutex_t mem_lock;
static pthread_mutex_t critical_lock;

/******************************************************************************
 *          P R I V A T E  F U N C T I O N  P R O T O T Y P E S
 ******************************************************************************/

void* manager(void* arg);
void* scheduler(void* arg);
void* timebase(void* arg);

/******************************************************************************
 *                     P R I V A T E  F U N C T I O N S
 ******************************************************************************/

void* manager(void* arg) {
  UNUSED(arg);
  
  while (true) {
    pthread_mutex_lock(&critical_lock);
    fstream disk = fstream("vm.txt", fstream::app);
    switch (mem_state) {
      case STORE:
        {
          unsigned int i = 0;
          for (; i < mem_count; i++) {
            if (mem[i].page.var_id == "") {
              mem[i].page.var_id = mem_arg;
              mem[i].page.val = mem_return;
              mem[i].timestamp = time_msec;
            }
          }
          if (i == mem_count) {
            disk << mem_arg << " " << mem_return << endl;
          }
        }
        break;
      case RELEASE:
        for (unsigned int i = 0; i < mem_count; i++) {
          if (mem[i].page.var_id == mem_arg) {
            disk << mem[i].page.var_id << " " << mem[i].page.val << endl;
            continue;
          }
        }
        break;
      case LOOKUP:
        {
          mem_return = -1;
          unsigned int i = 0;
          for (; i < mem_count; i++) {
            if (mem[i].page.var_id == mem_arg) {
              disk << mem[i].page.var_id << " " << mem[i].page.val << endl;
              continue;
            }
          }
          if (i == mem_count) {
            fstream buff_file = fstream("~vm.txt");
            string tmp;
            while (disk >> tmp) {
              if (tmp == mem_arg) {
                disk >> tmp;
                main_mem_S* oldest;
                for (unsigned int i = 0; i < mem_count; i++) {
                  if (!oldest) oldest = &mem[i];
                  else if (oldest->timestamp > mem[i].timestamp) oldest = &mem[i];
                }
                pthread_mutex_lock(&out.lock);
                *out.out << "Clock: " << time_msec << ", Memory Manager, SWAP: Variable " << oldest->page.var_id <<
                              " with Variable " << tmp << endl;
                pthread_mutex_unlock(&out.lock);
                buff_file << oldest->page.var_id << " " << oldest->page.val << endl;
                oldest->page.var_id = mem_arg;
                oldest->page.val = stoi(tmp);
                oldest->timestamp = time_msec;
                mem_return = oldest->page.val;
              } else {
                buff_file << tmp;
                disk >> tmp;
                buff_file << " " << tmp << "\n";
              }
              tmp.clear();
            }
            buff_file.close();
            buff_file.open("~vm.txt");
            disk.clear();
            while (buff_file >> tmp) {
              disk << tmp;
              tmp.clear();
            }
            disk.close();
          } else mem_return = mem[i].page.val;
          break;
        }
      case WAITING:
        break;
    }
    mem_state = WAITING;
    disk.close();
  }
  return 0;
}

void* scheduler(void* arg) { 
  UNUSED(arg);
  proc_S* first_proc;

  /**< organize entered processus into linked list since FIFO scheduling */
  for (unsigned int i = 0; i < procs_count; i++) {
    if (!first_proc) {
      first_proc = &procs[i];
    } else {
      if (procs[i].start_time < first_proc->start_time) {
        procs[i].next_proc = first_proc;
        first_proc = &procs[i];
        continue;
      }
      proc_S* tmp = first_proc;

      while(procs[i].start_time >= tmp->start_time && tmp->next_proc != nullptr) {
        tmp = (proc_S*) tmp->next_proc;
      }

      procs[i].next_proc = tmp->next_proc;
      tmp->next_proc = &procs[i];
    }
  }

  /**< Scheduler FIFO */
  while(true) {
    if (!first_proc) {
      for (unsigned int i = 0; i < cpu_count; i++)
        if (!cpus[i].running_proc)
          goto cont;
      goto finish;
    }
cont:
    for (unsigned int i = 0; i < cpu_count; i++) {
      if (cpus[i].running_proc) {
        if (cpus[i].running_proc->arg.complete) {
          cpus[i].running_proc = NULL;
          pthread_mutex_lock(&out.lock);
          *out.out << "Clock: " << time_msec << ", Process " << cpus[i].running_proc->arg.pid
                      << ": Finished.\n";
          pthread_mutex_unlock(&out.lock);
        }
      }
      if (!cpus[i].running_proc && (first_proc->start_time * 1000 >= time_msec)) {
        cpus[i].running_proc = first_proc;
        first_proc->running = true;
        pthread_create(&first_proc->thread, NULL, func, &first_proc->arg);
        first_proc = (proc_S*) first_proc->next_proc;
        pthread_mutex_lock(&out.lock);
        *out.out << "Clock: " << time_msec << ", Process " << cpus[i].running_proc->arg.pid
                    << ": Started.\n";
        pthread_mutex_unlock(&out.lock);

      }
    }
  }
finish:
  return 0;
}

void* timebase(void* arg) {
  UNUSED(arg);
  usleep(1000);
  time_msec++;
  return 0;
}

/**
 * @brief  Read's config files and starts system threads
 *
 * @retval   
 */
int main() {
  string tmp;

  /**< Read in Memory configuration */
  fstream mem_file = fstream("memconfig.txt");
  mem_file >> tmp;

  mem_count = stoi(tmp);
  mem = (main_mem_S*) malloc(mem_count * sizeof(main_mem_S));

  /**< Red in CPU and Process config */
  fstream proc_file = fstream("processes.txt");
  proc_file >> tmp;

  cpu_count = stoi(tmp);
  cpus = (cpu_S*) malloc(cpu_count * sizeof(cpu_S));

  proc_file >> tmp;
  procs_count = stoi(tmp);
  procs = (proc_S*) malloc(procs_count * sizeof(proc_S)); 

  for (unsigned int i = 0; i < procs_count; i++) { 
    proc_file >> tmp;
    procs[i].start_time = stoi(tmp);
    proc_file >> tmp;
    procs[i].arg.run_time = stoi(tmp);
    procs[i].arg.pid = i;
    procs[i].arg.complete = false;
    procs[i].running = false;
  }

#if not defined(TEST) /**< Closes the file so the data is saved (outside of testing) */
  out.out->close();
  delete out.out;
#endif /**< !TEST */

  pthread_create(&proc_scheduler, NULL, scheduler, NULL);
  pthread_create(&mem_manager, NULL, manager, NULL);
  pthread_create(&proc_timebase, NULL, timebase, NULL);

  pthread_join(proc_scheduler, NULL);
  pthread_cancel(mem_manager);
  pthread_cancel(proc_timebase);

  free(mem);
  free(cpus);
  free(procs);
  return 0;
}

/******************************************************************************
 *                       P U B L I C  F U N C T I O N S
 ******************************************************************************/

void Store(string variableId, unsigned int value) {
  pthread_mutex_lock(&mem_lock);
  mem_arg = variableId;
  mem_return = value;
  mem_state = RELEASE;
  pthread_mutex_unlock(&critical_lock);
  while (mem_state == RELEASE);
  
  mem_arg.clear();
  mem_return = 0;
  pthread_mutex_unlock(&mem_lock);  
  
}

void Release(string variableId) {
  pthread_mutex_lock(&mem_lock);
  mem_arg = variableId;
  mem_state = RELEASE;
  pthread_mutex_unlock(&critical_lock);
  while (mem_state == RELEASE);

  mem_arg.clear();
  pthread_mutex_unlock(&mem_lock);  
}

unsigned int Lookup(string variableId) {
  pthread_mutex_lock(&mem_lock);
  mem_arg = variableId;
  mem_state = LOOKUP;
  pthread_mutex_unlock(&critical_lock);
  while (mem_state == LOOKUP);

  unsigned int tmp = mem_return;
  mem_arg.clear();
  mem_return = 0;
  pthread_mutex_unlock(&mem_lock);

  return tmp;
}
