/**
 * @file ass1.cpp
 * @brief  Source code of the Programming Assignment 1 COEN346 Fall 2022
 * @author Joshua Lafleur (josh.lafleur@outlook.com)
 * @version 1.0
 * @date 2022-09-20
 *
 * @note We certify that this work is our own work and meet's the faculties
 * certification of originality.
 */

/******************************************************************************
 *                             I N C L U D E S
 ******************************************************************************/

#include <pthread.h>
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

pthread_mutex_t output_lock;

/******************************************************************************
 *          P R I V A T E  F U N C T I O N  P R O T O T Y P E S
 ******************************************************************************/

void* sort_child(void* arg);

/******************************************************************************
 *                     P R I V A T E  F U N C T I O N S
 ******************************************************************************/

void* sort_child(void* arg) {
    thread_data_S child_data[2];
    pthread_t threads[2];
    thread_data_S* data = (thread_data_S*) arg;
    vector<int> merged_data;
    size_t id, sort_a, sort_b;
    sort_a = 0;
    sort_b = 0;

    pthread_mutex_lock(&thread_count.lock);
    id = ++thread_count.val;
    pthread_mutex_unlock(&thread_count.lock);
    
    pthread_mutex_lock(&output_lock);
    cout << "Thread " << bitset<8>(id) << " started\n";
    pthread_mutex_unlock(&output_lock);
    
    if (data->vals.size() > 2) {
        for (size_t i = 0; i < (data->vals.size() / 2); i++)
            child_data[0].vals.push_back(data->vals[i]);
            
        pthread_create(&threads[0], NULL, sort_child, &child_data[0]);
        
        for (size_t i = (data->vals.size() / 2) + 1; i < data->vals.size(); i++)
            child_data[1].vals.push_back(data->vals[i]);

        pthread_create(&threads[1], NULL, sort_child, &child_data[1]);
        
        for(size_t i = 0; i < 2; i++) 
            pthread_join(threads[i], NULL);
    }

    for (; (sort_a < child_data[0].vals.size()) && (sort_b < child_data[1].vals.size());) {
        if (child_data[0].vals[sort_a] < child_data[1].vals[sort_b]) {
            merged_data.push_back(child_data[0].vals[sort_a++]);
        } else {
            merged_data.push_back(child_data[1].vals[sort_b++]);
        }
    } 
    
    pthread_mutex_lock(&output_lock);
    cout << "Thread " << bitset<8>(id) << " finished: ";
    for (int i : merged_data)
        cout << i << " ";
    cout << "\n";
    pthread_mutex_unlock(&output_lock);

    data->vals = merged_data;
        
    return NULL;
}

int main() {
    thread_data_S child_data[2];
    pthread_t top_threads[2];
    array<int, 8> input_arr;

    pthread_mutex_init(&thread_count.lock, NULL);

    if (input_arr.size() > 2) {
        for (size_t i = 0; i < (input_arr.size() / 2); i++)
            child_data[0].vals.push_back(input_arr[i]);
            
        pthread_create(&top_threads[0], NULL, sort_child, &child_data[0]);
        
        for (size_t i = (input_arr.size() / 2) + 1; i < input_arr.size(); i++)
            child_data[1].vals.push_back(input_arr[i]);

        pthread_create(&top_threads[1], NULL, sort_child, &child_data[1]);
        
        for(int i = 0; i < 2; i++) 
            pthread_join(top_threads[i], NULL);
    }

    return 0; 
}
