/**
 * @file ass1.h
 * @brief  Header file of the Programming Assignment 1 COEN346 Fall 2022
 * @author Joshua Lafleur 40189389
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

#if defined(_WIN32) || defined(_WIN64)
    #include <synchapi.h>
    #include <processthreadsapi.h>
#elif defined(__GNUC__)
    #include <pthread.h>
#endif

#include <vector>
#include <iostream>
#include <fstream>

using namespace std;


/******************************************************************************
 *                              D E F I N E S
 ******************************************************************************/

/**< OS_THREAD */
#if defined(_WIN32) || defined(_WIN64)
    #define OS_THREAD hThreadArray
#elif defined(__GNUC__)
    #define OS_THREAD pthread_t
#endif

/**< OS_MUTEX */
#if defined(_WIN32) || defined(_WIN64)
    #define OS_MUTEX HANDLE
#elif defined(__GNUC__)
    #define OS_MUTEX pthread_mutex_t 
#endif

/**< OS_THREAD_CREATE */
#if defined(_WIN32) || defined(_WIN64)
    #define OS_THREAD_CREATE(th_id, func, arg) CreateThread(NULL, 0, th_id, arg, 0, 0)    
#elif defined(__GNUC__)
    #define OS_THREAD_CREATE(th_id, func, arg) pthread_create(&th_id, NULL, func, &arg)
#endif

/**< OS_THREAD_WAIT */
#if defined(_WIN32) || defined(_WIN64)
    #define OS_THREAD_WAIT(th_handle) WaitForSingleObject(th_handle, INFINTE)    
#elif defined(__GNUC__)
    #define OS_THREAD_WAIT(th_handle) pthread_join(th_handle, NULL)
#endif

/**< OS_MUTEX_INIT */
#if defined(_WIN32) || defined(_WIN64)
    #define OS_MUTEX_INIT(mu_handle) mu_handle = CreateMutex(NULL, FALSE, NULL)    
#elif defined(__GNUC__)
    #define OS_MUTEX_INIT(mu_handle) pthread_mutex_init(&mu_handle, NULL)
#endif

/**< OS_MUTEX_DEINIT */
#if defined(_WIN32) || defined(_WIN64)
    #define OS_MUTEX_DEINIT((mu_handle) CloseHandle(mu_handle, INFINTE)    
#elif defined(__GNUC__)
    #define OS_MUTEX_DEINIT(mu_handle) pthread_mutex_destroy(&mu_handle)
#endif

/**< OS_MUTEX_LOCK */
#if defined(_WIN32) || defined(_WIN64)
    #define OS_MUTEX_LOCK(mu_handle) WaitForSingleOject(mu_handle, INFINTE)    
#elif defined(__GNUC__)
    #define OS_MUTEX_LOCK(mu_handle) pthread_mutex_lock(&mu_handle)
#endif

/**< OS_MUTEX_UNLOCK */
#if defined(_WIN32)
    #define OS_MUTEX_UNLOCK(mu_handle) ReleaseMutex(mu_handle)    
#elif defined(__GNUC__)
    #define OS_MUTEX_UNLOCK(mu_handle) pthread_mutex_unlock(&mu_handle)
#endif


/******************************************************************************
 *                             T Y P E D E F S
 ******************************************************************************/
typedef struct {
  int val;
  OS_MUTEX lock;
} int_mutex_S;

typedef struct {
    int id;
    vector<int> vals;
} thread_data_S;

typedef struct output_S {
#if defined(TEST) /**< Allows the developper to choose output stream at compile time
                  *    helpful when debugging */
    ostream* out = &cout;
#else /**< Normal operation */
    ofstream* out = new ofstream("Output.txt");
#endif /**< TEST */
    OS_MUTEX lock;
} output_S;
