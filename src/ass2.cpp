/**
 * @file ass2.cpp
 * @brief  Source code of the Programming Assignment 2 COEN346 Fall 2022
 * @author Joshua Lafleur 40189389, Eden Bouskila 40170349, Ahmed Enani 26721281
 * @version 1.0
 * @date 2022-09-20
 *
 * @note We certify that this work is our own work and meet's the faculties
 * certification of originality.
 */

/******************************************************************************
 *                             I N C L U D E S
 ******************************************************************************/

#include "ass2.h"
#include <cstdio>
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>

using namespace std;

/******************************************************************************
 *                             T Y P E D E F S
 ******************************************************************************/

typedef struct {
  void *func;
  pthread_t thread;
  unsigned int start_time;
  thread_data_S data;
  unsigned int run_time_quantum;
  bool running;
} proc_S;

typedef struct {
  string name;
  vector<proc_S> procs;
  void* active_index;
} user_S;

typedef struct {
  vector<user_S> waiting_users;
  unsigned int quantum;
  vector<proc_S>::iterator curr_proc;
  vector<user_S>::iterator curr_user;
} scheduler_S;

/******************************************************************************
 *                         P R I V A T E  V A R S
 ******************************************************************************/

static scheduler_S sched;
static pthread_t sched_thread;
const unsigned int *const quantum = &sched.quantum;
unsigned int time_sec = 0;

/******************************************************************************
 *          P R I V A T E  F U N C T I O N  P R O T O T Y P E S
 ******************************************************************************/

static void *scheduler(void *arg);
static void *func(void *arg);
static void scdl(int arg);
static void thread_sig(int sig);

/******************************************************************************
 *                     P R I V A T E  F U N C T I O N S
 ******************************************************************************/

static void thread_sig(int sig) {
  switch (sig) {
  case SIGUSR1:
    pause();
    break;
  case SIGUSR2:
    break;
  }
}

/**
 * @brief  Scheduling algorithm
 *
 * @param arg UNUSED
 */
void *scheduler(void *arg) {
  UNUSED(arg);
  static unsigned int last_start_time = 0;
  static vector<user_S> active_users;
  unsigned int user_quantum;
  cpu_set_t cpu_mask;
  struct sigaction act1;
  struct sigaction act2;
  struct itimerval val;

  CPU_ZERO(&cpu_mask);
  CPU_SET(0, &cpu_mask);

  sched_setaffinity(0, sizeof(cpu_mask), &cpu_mask);
  
  act1.sa_handler = thread_sig;
  act1.sa_flags = 0;
  sigemptyset(&act1.sa_mask);
  sigaction(SIGUSR1, &act1, NULL);
  sigaction(SIGUSR2, &act1, NULL);


  act2.sa_handler = scdl;
  act2.sa_flags = 0;
  sigemptyset(&act2.sa_mask);
  sigaction(SIGPROF, &act2, NULL);

  val.it_value.tv_sec = 1;
  val.it_value.tv_usec = 0;
  val.it_interval = val.it_value;
  setitimer(ITIMER_PROF, &val, NULL);

  sched.curr_user = sched.waiting_users.end();

  while (sched.waiting_users.size() || active_users.size()) {
      vector<int> user_deletion_index;

      if (((time_sec - last_start_time) % sched.quantum == 0) || (active_users.size() == 0)) {
      if (sched.curr_user != sched.waiting_users.end())
          if (sched.curr_proc->running == true) {
            cout << "Time " << time_sec << ", User " << sched.curr_user->name << 
                ", Process " << sched.curr_proc - sched.curr_user->procs.begin() <<
                ", Paused\n";
        sched.curr_proc->running = false;
      }
resched:
      for (size_t user = 0; user < sched.waiting_users.size(); user++) {
        user_S tmp;
        vector<int> proc_deletion_index;

        tmp.name = sched.waiting_users[user].name;

        for (size_t proc = 0; proc < sched.waiting_users[user].procs.size();
             proc++) {
          if (sched.waiting_users[user].procs[proc].start_time <= time_sec) {
            tmp.procs.push_back(sched.waiting_users[user].procs[proc]);
            proc_deletion_index.push_back(proc);
            
          }
        }
        
        if (tmp.procs.size() > 0) {
          if (sched.waiting_users[user].active_index == 0) {
            active_users.push_back(tmp);
            sched.waiting_users[user].active_index = (void*) &active_users[active_users.size() - 1]; 
          } else {
              for (unsigned int i = 0; i < tmp.procs.size(); i++)
                  ((user_S*) sched.waiting_users[user].active_index)->procs.push_back(tmp.procs[i]);
          }
        }
        
        while (proc_deletion_index.size() > 0) {
          sched.waiting_users[user].procs.erase(
              sched.waiting_users[user].procs.begin() +
              proc_deletion_index.back());
          proc_deletion_index.pop_back();
        }

        if (sched.waiting_users[user].procs.size() == 0) {
          user_deletion_index.push_back(user);
        }
      }

      while (user_deletion_index.size() > 0) {
        sched.waiting_users.erase(sched.waiting_users.begin() +
                                  user_deletion_index.back());
        user_deletion_index.pop_back();
      }

      if ((sched.waiting_users.size() == 0) && (active_users.size() == 0))
        exit(0);

      if (active_users.size() != 0) {
        user_quantum = sched.quantum / active_users.size();
        goto cont;
      }

      goto skip;

cont:
      for (unsigned int user = 0; user < active_users.size(); user++) {
        unsigned int proc_quantum =
            user_quantum / active_users[user].procs.size();

        for (unsigned int proc = 0; proc < active_users[user].procs.size();
             proc++) {
          active_users[user].procs[proc].run_time_quantum = proc_quantum;
          active_users[user].procs[proc].running = false;
        }
      }

      sched.curr_user = active_users.begin();
      sched.curr_proc = sched.curr_user->procs.begin();
      goto run;
    } else {
    run:
      if (sched.curr_proc->data.state == UNSTARTED) {
        cout << "Time " << time_sec << ", User " << sched.curr_user->name << 
            ", Process " << sched.curr_proc - sched.curr_user->procs.begin() <<
            ", Started\n";
        cout << "Time " << time_sec << ", User " << sched.curr_user->name << 
            ", Process " << sched.curr_proc - sched.curr_user->procs.begin() <<
            ", Resumed\n";
        last_start_time = time_sec;
        sched.curr_proc->run_time_quantum--;
        sched.curr_proc->running = true;
        pthread_create(&sched.curr_proc->thread, NULL, func,
                       &sched.curr_proc->data);
      } else if (sched.curr_proc->running == false) {
        cout << "Time " << time_sec << ", User " << sched.curr_user->name << 
            ", Process " << sched.curr_proc - sched.curr_user->procs.begin() <<
            ", Resumed\n";
        last_start_time = time_sec;
        sched.curr_proc->running = true;
        sched.curr_proc->run_time_quantum--;
        pthread_kill(sched.curr_proc->thread, SIGUSR2);
      } else if (sched.curr_proc->data.state == FINISHED || sched.curr_proc->data.run_time == 0) {
        cout << "Time " << time_sec << ", User " << sched.curr_user->name << 
            ", Process " << sched.curr_proc - sched.curr_user->procs.begin() <<
            ", Finished\n";
        sched.curr_user->procs.erase(sched.curr_proc--);
        if (sched.curr_user->procs.size() == 0) {
          active_users.erase(sched.curr_user--);
          if (active_users.size() == 0) goto finished;
          sched.curr_proc = sched.curr_user->procs.begin();
        }
        goto done;
      } else if (sched.curr_proc->run_time_quantum != 0) {
        sched.curr_proc->run_time_quantum--;
        pthread_kill(sched.curr_proc->thread, SIGUSR2);
      } else {
        cout << "Time " << time_sec << ", User " << sched.curr_user->name << 
            ", Process " << sched.curr_proc - sched.curr_user->procs.begin() <<
            ", Paused\n";
        sched.curr_proc->running = false;
        //pthread_kill(sched.curr_proc->thread, SIGUSR1);
done:
        if (++sched.curr_proc == sched.curr_user->procs.end()) {
          if (++sched.curr_user == active_users.end())
            goto resched;
          sched.curr_proc = sched.curr_user->procs.begin();
        }
        goto run;
      }
    }
skip:
    pause();
    time_sec++;
  }

finished:
  return NULL;
}

void scdl(int arg) {
    UNUSED(arg);
    pthread_kill(sched_thread, SIGUSR2);
    
}

void *func(void *arg) {
  thread_data_S data = *((thread_data_S *)arg);
  struct sigaction act;
  cpu_set_t cpu_mask;
  
  CPU_ZERO(&cpu_mask);
  CPU_SET(0, &cpu_mask);

  sched_setaffinity(0, sizeof(cpu_mask), &cpu_mask);

  ((thread_data_S*)arg)->state = RUNNING;

  act.sa_handler = thread_sig;
  act.sa_flags = 0;
  sigemptyset(&act.sa_mask);
  sigaction(SIGUSR1, &act, NULL);
  sigaction(SIGUSR2, &act, NULL);

  while (data.run_time) {
      cout << data.run_time << endl;
      data.run_time--;
      pause();
  }
  ((thread_data_S*)arg)->state = FINISHED;
  pthread_kill(sched_thread, SIGUSR2);

  pthread_exit(NULL);
}

/**
 * @brief  Main function of Assignment 2
 *
 * @retval   0
 */
int main() {
  fstream file_in = fstream("Input.txt");
  string tmp;

  file_in >> tmp;

  sched.quantum = stoi(tmp);

  while (file_in >> tmp) {
    user_S user;
    proc_S proc;
    uint8_t num_proc;

    user.name = tmp;
    file_in >> tmp;
    num_proc = stoi(tmp);

    for (unsigned int i = 0; i < num_proc; i++) {
      file_in >> tmp;
      proc.start_time = stoi(tmp);
      file_in >> tmp;
      proc.data.run_time = stoi(tmp);
      proc.func = (void *)func;
      proc.data.state = UNSTARTED;
      user.procs.push_back(proc);
      user.active_index = 0;
    }

    sched.waiting_users.push_back(user);

    tmp.clear();
  }

  pthread_create(&sched_thread, NULL, scheduler,
                 NULL);

  while (1);

  return 0;
}
