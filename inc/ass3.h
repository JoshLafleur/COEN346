/**
 * @file ass3.h
 * @brief  Header file of the Programming Assignment 3 COEN346 Fall 2022
 * @author Joshua Lafleur 40189389, Eden Bouskila 40170349, Ahmed Enani 26721281
 * @version 1.0
 * @date 2022-11-12
 *
 * @note We certify that this work is our own work and meet's the faculties
 * certification of originality.
 */

#pragma once

/******************************************************************************
 *                             I N C L U D E S
 ******************************************************************************/

#include <fstream>
#include <iostream>
#include <pthread.h>
#include <vector>

using namespace std;


/******************************************************************************
 *                              D E F I N E S
 ******************************************************************************/

#define UNUSED(x) (void)(x)

/******************************************************************************
 *                             T Y P E D E F S
 ******************************************************************************/

typedef struct {
    bool complete;
    unsigned int run_time;
    unsigned int pid;
} thread_data_S;

typedef enum {
  WAITING = 0x00,
  STORE,
  RELEASE,
  LOOKUP,
} actions_E;

typedef struct output_S {
#if defined(TEST) /**< Allows the developper to choose output stream at        \
                   * compile time helpful when debugging */
  ostream *out = &cout;
#else  /**< Normal operation */
  ofstream *out = new ofstream("Output.txt");
#endif /**< TEST */
  pthread_mutex_t lock;
} output_S;

/******************************************************************************
 *                              E X T E R N S
 ******************************************************************************/

extern output_S out;
extern unsigned int time_msec;

/******************************************************************************
 *            P U B L I C  F U N C T I O N  P R O T O T Y P E S
 ******************************************************************************/

void Store(unsigned int variableId, unsigned int value);
void Release(unsigned int variableId);
unsigned int Lookup(unsigned int variableId);

