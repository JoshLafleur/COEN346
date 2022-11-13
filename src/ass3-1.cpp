/**
 * @file ass3-1.cpp
 * @brief  Source code of the Programming Assignment 3-1 COEN346 Fall 2022
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
#include <fstream>
#include <vector>
#include <unistd.h>

/******************************************************************************
 *                             T Y P E D E F S
 ******************************************************************************/

typedef struct {
  actions_E command;
  string id;
  unsigned int val;
} command_S;

/******************************************************************************
 *                       P U B L I C  F U N C T I O N S
 ******************************************************************************/

void* func(void* arg) {
  thread_data_S* data = (thread_data_S*) arg;
  vector<command_S> commands;
  unsigned int start_time = time_msec;

  fstream cmnd_file = fstream("commands.txt");
  string tmp;

  while (cmnd_file >> tmp) {
    command_S cmnd;
    if (tmp == "Store" ) {
        cmnd.command = STORE;
    } else if (tmp == "Release") {
        cmnd.command = RELEASE;
    } else if (tmp == "Lookup") {
        cmnd.command = LOOKUP;
    }

    cmnd_file >> cmnd.id;
    
    if (cmnd.command == STORE) {
      cmnd_file >> tmp;
      cmnd.val = stoi(tmp);
    }

    commands.push_back(cmnd);

    tmp.clear();
  }

  while (start_time + data->run_time > time_msec) {
next:
    unsigned int random_cmnd = rand() % commands.size();
    unsigned int random_time = rand() % 1001;

    switch(commands[random_cmnd].command) {
      case STORE:
        pthread_mutex_lock(&out.lock);
        *out.out << "Clock: " << time_msec << ", Process " << data->pid <<
                    ", Store: Variable " << commands[random_cmnd].id << ", Value: " <<
                    commands[random_cmnd].val << endl;
        pthread_mutex_unlock(&out.lock);
        break;
      case RELEASE:
        pthread_mutex_lock(&out.lock);
        *out.out << "Clock: " << time_msec << ", Process " << data->pid <<
                    ", Release: Variable " << commands[random_cmnd].id << endl;
        pthread_mutex_unlock(&out.lock);
        break;
      case LOOKUP:
        {
          unsigned int ret = Lookup(commands[random_cmnd].id);
          usleep(random_time * 1000);
          pthread_mutex_lock(&out.lock);
          *out.out << "Clock: " << time_msec << ", Process " << data->pid <<
                      ", Lookup: Variable " << commands[random_cmnd].id << ", Value" <<
                      ret << endl;
          pthread_mutex_unlock(&out.lock);
          goto next;
          break;
        }
      case WAITING:
        break;
    }

    usleep(random_time * 1000);
  }

  data->complete = true;
  return 0;
}
