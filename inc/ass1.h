/**
 * @file ass1.h
 * @brief  Header file of the Programming Assignment 1 COEN346 Fall 2022
 * @author Joshua Lafleur (josh.lafleur@outlook.com)
 * @version 1.0
 * @date 2022-09-20
 *
 * @note We certify that this work is our own work and meet's the faculties
 * certification of originality.
 */

#pragma once

/******************************************************************************
 *                             I N C L U D E S
 ******************************************************************************/

#include <pthread.h>
#include <vector>
#include <iostream>

using namespace std;

/******************************************************************************
 *                             T Y P E D E F S
 ******************************************************************************/

typedef struct {
  int val;
  pthread_mutex_t lock;
} int_mutex_S;

typedef struct {
    int id;
    vector<int> vals;
} thread_data_S;

