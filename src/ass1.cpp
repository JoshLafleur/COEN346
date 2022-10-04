/**
 * @file ass1.cpp
 * @brief  Source code of the Programming Assignment 1 COEN346 Fall 2022
 * @author Joshua Lafleur 40189389 
 * @version 1.0
 * @date 2022-09-20
 *
 * @note We certify that this work is our own work and meet's the faculties
 * certification of originality.
 */

/******************************************************************************
 *                             I N C L U D E S
 ******************************************************************************/

#include <string.h>
#include <array>
#include <bitset>

#include "ass1.h"

using namespace std;

/******************************************************************************
 *                         P R I V A T E  V A R S
 ******************************************************************************/

int_mutex_S thread_count = {
    .val = 0,
};

output_S output;

/******************************************************************************
 *          P R I V A T E  F U N C T I O N  P R O T O T Y P E S
 ******************************************************************************/

void* sort_child(void* arg);

/******************************************************************************
 *                     P R I V A T E  F U N C T I O N S
 ******************************************************************************/

/**
 * @brief  Sort's the input vector by delegating half of the input vector to each
 *         of 2 children threads. If the input vector is <= 2, the thread sorts
 *         and returns.
 *
 * @param[in|out] arg Type: thread_data_S. Contains the thread id and vector of
 *                input and output values.
 *
 * @retval   NULL
 */
void* sort_child(void* arg) {
    thread_data_S child_data[2];
    OS_THREAD threads[2];
    vector<int> merged_data;
    size_t id;

    thread_data_S* data = (thread_data_S*) arg;

    /**< Lock the thread-count, retreive count for this thread and unlock count */
    OS_MUTEX_LOCK(thread_count.lock);
    id = ++thread_count.val;
    OS_MUTEX_UNLOCK(thread_count.lock);
    
    /**< Lock the output, output start notification, and unlock output */
    OS_MUTEX_LOCK(output.lock);
#if defined(TEST) /**< Allows for easier debugging and testing */
    *output.out << "Thread " << bitset<3>(id) << " started ";
    for (int i : data->vals)
        cout << i << " ";
    *output.out << "\n";
#else /**< Normal operation */ 
    *output.out << "Thread " << bitset<3>(id) << " started\n";
#endif /**< TEST */
    OS_MUTEX_UNLOCK(output.lock);
    
    if (data->vals.size() > 2) { /**< Split the vector if greater than 2 */
        for (size_t i = 0; i < (data->vals.size() / 2); i++)
            child_data[0].vals.push_back(data->vals[i]);
            
        OS_THREAD_CREATE(threads[0], sort_child, child_data[0]);
        
        for (size_t i = (data->vals.size() / 2); i < data->vals.size(); i++)
            child_data[1].vals.push_back(data->vals[i]);

        OS_THREAD_CREATE(threads[1], sort_child, child_data[1]);
        
        /**< Allows both threads to start executing before joining */
        for(size_t i = 0; i < 2; i++) 
            OS_THREAD_WAIT(threads[i]);
        
        /**< Merge data returned from child threads */
        for (size_t sort_a = 0, sort_b = 0; (sort_a < child_data[0].vals.size()) ||
                (sort_b < child_data[1].vals.size()); ) {
            if (sort_a == child_data[0].vals.size()) {
                merged_data.push_back(child_data[1].vals[sort_b++]);
                continue;
            } else if (sort_b == child_data[1].vals.size()) {
                merged_data.push_back(child_data[0].vals[sort_a++]);
                continue;
            }

            if (child_data[0].vals[sort_a] <= child_data[1].vals[sort_b]) {
                merged_data.push_back(child_data[0].vals[sort_a++]);
            } else {
                merged_data.push_back(child_data[1].vals[sort_b++]);
            }
        } 
    } else if (data->vals.size() == 2){ /**< Merge data if size is 2. 
                                          Prevents un-necessary threading */
        if (data->vals[0] <= data->vals[1]) {
            merged_data = data->vals;
        } else {
            merged_data.push_back(data->vals[1]);
            merged_data.push_back(data->vals[0]);
        }
    } else { /**< Size of vector is 1, therefore no merging needed */
        merged_data = data->vals;
    }
    
    /**< Lock the output, output finished notification, and unlock output */
    OS_MUTEX_LOCK(output.lock);
    *output.out << "Thread " << bitset<3>(id) << " finished: ";
    for (int i : merged_data)
        *output.out << i << " ";
    *output.out << "\n";
    OS_MUTEX_UNLOCK(output.lock);

    /**< Return information to parent process through the data struct */
    data->vals = merged_data;
        
    return NULL;
}

/**
 * @brief  Main function. Creates the top level thread, reads input, and closes file.
 *
 * @retval 0 -> Success
 */
int main() {
    thread_data_S child_data;
    OS_THREAD top_thread;

#if defined(TEST) /**< Allows for easier debugging and testing */ 
    array<int, 8> input_arr = {3304, 8221, 26849, 14038, 1509, 6367, 7856, 21362 };
#else /**< Normal operation */
    /**< Open input file and read, converting to int and pushing onto vector */
    vector<int> input_arr;
    fstream file_in = fstream("Input.txt");
    string tmp;

    while (!file_in.eof()) {
        file_in >> tmp;
        if (tmp.size())
            child_data.vals.push_back(stoi(tmp));
        tmp.clear();
    }
#endif /**< TEST */

    /**< Init the mutex's */
    OS_MUTEX_INIT(thread_count.lock);
    OS_MUTEX_INIT(output.lock);

#if defined(TEST) /**< Converts from array to vector if testing */
    for (size_t i = 0; i < input_arr.size(); i++)
        child_data.vals.push_back(input_arr[i]);
#endif /**< TEST */

    /**< Creates and joins to the top level thread */
    OS_THREAD_CREATE(top_thread, sort_child, child_data);
    OS_THREAD_WAIT(top_thread);

#if not defined(TEST) /**< Closes the file so the data is saved (outside of testing) */
    output.out->close();
    delete output.out;
#endif /**< !TEST */

    OS_MUTEX_DEINIT(thread_count.lock);
    OS_MUTEX_DEINIT(output.lock);
    
    return 0; 
}
