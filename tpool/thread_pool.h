/* @file threadpool.h
 * @brief Threadpool library that contains the functions prototypes and data
 * structures needed to use this thread pool
 *
 *
 *
 */

#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

#define MAX_CONNECTIONS 10



/*
 * @brief struct that defines a job in the queue
 */
typedef struct job_t
{
    int            socket;
    struct job_t * next;
} job_t;

/*
 * @brief struct that defines the threadpool
 */
typedef struct threadpool_t
{
    pthread_t *     threads;      // array of threads
    int             pool_size;    // number of threads
    pthread_mutex_t lock;         // lock for the threadpool
    pthread_mutex_t file_lock;    // lock for the file
    int             shutdown;     // flag to indicate if the threadpool should shutdown
    int             queue_size;   // number of jobs in queue
    job_t *         head;         // head of queue
    job_t *         tail;         // tail of queue
    pthread_cond_t  not_empty;    // condition variable for queue not empty
    pthread_cond_t  empty;        // condition variable for queue empty
    FILE *          data_base;    // file to write to
    // If you're using a data base this can also be placed in the threadpool, Use mutex locks when modifying any data 
} threadpool_t;

/**
 * @brief  Initialize threadpool
 * Initializes a threadpool. This function will not return until all
 * threads have initialized successfully.
 *
 * @param  int pool_size number of threads to be created in the threadpool
 * @return threadpool created threadpool on success
 * @return NULL on error
 */
threadpool_t * thpool_init(int pool_size);

/**
 * @brief Add work to the job queue
 *
 * @param  threadpool threadpool to which the work will be added
 * @param  socket socket for the client connection
 * @return SUCCESS_CODE on success
 * @return FAIL_CODE on error
 */
int enqueue_job(threadpool_t * tpool, int socket);

/**
 * @brief Destroy the threadpool
 *
 * @param threadpool the threadpool to destroy
 * @return true on success
 * @return false on error
 */
int thpool_destroy(threadpool_t * tpool);

/**
 * @brief controls the individual thread
 *
 * @param void* arg that is the the job for the thread to do
 * @return void
 */
void * thread_function(void * arg);

#endif /* THREAD_POOL_H */
