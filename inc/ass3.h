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

using namespace std;

/******************************************************************************
 *                              E X T E R N S
 ******************************************************************************/

/******************************************************************************
 *                              D E F I N E S
 ******************************************************************************/

#define UNUSED(x) (void)(x)

/******************************************************************************
 *                             T Y P E D E F S
 ******************************************************************************/

typedef struct {
} thread_data_S;

typedef struct output_S {
#if defined(TEST) /**< Allows the developper to choose output stream at        \
                   * compile time helpful when debugging */
  ostream *out = &cout;
#else  /**< Normal operation */
  ofstream *out = new ofstream("Output.txt");
#endif /**< TEST */
  pthread_mutex_t lock;
} output_S;

