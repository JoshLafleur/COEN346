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
const unsigned int *const quantum = &sched.quantum;
unsigned int time_sec = ~0;

/******************************************************************************
 *          P R I V A T E  F U N C T I O N  P R O T O T Y P E S
 ******************************************************************************/

static void* scheduler(void* arg);
static void *func(void *arg);
static void scdl(int arg);

/******************************************************************************
 *                     P R I V A T E  F U N C T I O N S
 ******************************************************************************/

/**
 * @brief  Scheduling algorithm
 *
 * @param arg UNUSED
 */
static void* scheduler(void* arg) {
  UNUSED(arg);
  static unsigned int last_start_time = 0;
  static vector<user_S> active_users;
  unsigned int user_quantum;

  time_sec++;

  if (last_start_time % sched.quantum == 0) {
    vector<int> user_deletion_index;

    for (size_t user = 0; user < sched.waiting_users.size(); user++) {
      user_S tmp;
      vector<int> proc_deletion_index;

      tmp.name = sched.waiting_users[user].name;

      for (size_t proc = 0; proc < sched.waiting_users[user].procs.size();
           proc++) {
        if (sched.waiting_users[user].procs[proc].data.state == FINISHED) {
          proc_deletion_index.push_back(proc);
          continue;
        }
        if (sched.waiting_users[user].procs[proc].start_time <= time_sec) {
          tmp.procs.push_back(sched.waiting_users[user].procs[proc]);
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

      if (tmp.procs.size() > 0)
        active_users.push_back(tmp);
    }

    while (user_deletion_index.size() > 0) {
      sched.waiting_users.erase(sched.waiting_users.begin() +
                                user_deletion_index.back());
      user_deletion_index.pop_back();
    }

    if (sched.waiting_users.size() == 0)
      exit(0);

    user_quantum = sched.quantum / active_users.size();

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
      pthread_create(&sched.curr_proc->thread, NULL, func,
                     &sched.curr_proc->data);
      sched.curr_proc->run_time_quantum--;
      sched.curr_proc->running = true;
      return NULL;
    } else if (sched.curr_proc->running == false) {
      sched.curr_proc->running = true;
      sched.curr_proc->run_time_quantum--;
      pthread_kill(sched.curr_proc->thread, SIGUSR2);
      return NULL;
    } else if (sched.curr_proc->run_time_quantum != 0) {
      sched.curr_proc->run_time_quantum--;
      return NULL;
    } else {
      sched.curr_proc->running = false;
      pthread_kill(sched.curr_proc->thread, SIGUSR1);
      if (sched.curr_proc++ == sched.curr_user->procs.end()) {
        sched.curr_user++;
        sched.curr_proc = sched.curr_user->procs.begin();
        goto run;
      }
    }
  }
  return NULL;
}

static void thread_sig(int sig) {
  switch (sig) {
  case SIGUSR1:
    pause();
    break;
  case SIGUSR2:
    break;
  }
}

void sched(int arg) {

}

static void *func(void *arg) {
  thread_data_S *data = (thread_data_S *)arg;
  struct sigaction act;

  data->state = RUNNING;

  act.sa_handler = thread_sig;
  act.sa_flags = 0;
  sigemptyset(&act.sa_mask);
  sigaction(SIGUSR1, &act, NULL);
  sigaction(SIGUSR2, &act, NULL);

  while (data->run_time--)
    sleep(1);

  data->state = FINISHED;

  pthread_exit(NULL);
}

/**
 * @brief  Main function of Assignment 2
 *
 * @retval   0
 */
int main() {
  struct sigaction act;
  struct itimerval val;
  cpu_set_t cpu_mask;

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
    }

    sched.waiting_users.push_back(user);

    tmp.clear();
  }

  CPU_ZERO(&cpu_mask);
  CPU_SET(0, &cpu_mask);

  sched_setaffinity(0, sizeof(cpu_mask), &cpu_mask);

  act.sa_handler = scheduler;
  act.sa_flags = 0;
  sigemptyset(&act.sa_mask);
  sigaction(SIGPROF, &act, NULL);

  val.it_value.tv_sec = 1;
  val.it_value.tv_usec = 0;
  val.it_interval = val.it_value;
  setitimer(ITIMER_PROF, &val, NULL);

  while (1)
    ;
  return 0;
}
