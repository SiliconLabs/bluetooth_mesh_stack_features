/***************************************************************************//**
 * @file
 * @brief Timer for BTmesh NCP-host Switch Example Project.
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

/* Includes */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <unistd.h>

#include "utils/timer.h"

// A structure to represent a queue
struct Queue {
    int front, rear, size;
    unsigned capacity;
    int* array;
};

struct Queue *timerQueue = NULL;

// function to create a queue
// of given capacity.
// It initializes size of queue as 0
struct Queue* createQueue(unsigned capacity)
{
    struct Queue* queue = (struct Queue*)malloc(
        sizeof(struct Queue));
    queue->capacity = capacity;
    queue->front = queue->size = 0;
 
    // This is important, see the enqueue
    queue->rear = capacity - 1;
    queue->array = (int*)malloc(
        queue->capacity * sizeof(int));
    return queue;
}
 
// Queue is full when size becomes
// equal to the capacity
int isFull(struct Queue* queue)
{
    return (queue->size == queue->capacity);
}
 
// Queue is empty when size is 0
int isEmpty(struct Queue* queue)
{
    return (queue->size == 0);
}
 
// Function to add an item to the queue.
// It changes rear and size
void enqueue(struct Queue* queue, int item)
{
    if (isFull(queue))
        return;
    queue->rear = (queue->rear + 1)
                  % queue->capacity;
    queue->array[queue->rear] = item;
    queue->size = queue->size + 1;
    printf("%d enqueued to queue\n", item);
}
 
// Function to remove an item from queue.
// It changes front and size
int dequeue(struct Queue* queue)
{
    if (isEmpty(queue))
        return INT_MIN;
    int item = queue->array[queue->front];
    queue->front = (queue->front + 1)
                   % queue->capacity;
    queue->size = queue->size - 1;
    return item;
}
 
// Function to get front of queue
int front(struct Queue* queue)
{
    if (isEmpty(queue))
        return INT_MIN;
    return queue->array[queue->front];
}
 
// Function to get rear of queue
int rear(struct Queue* queue)
{
    if (isEmpty(queue))
        return INT_MIN;
    return queue->array[queue->rear];
}

//static pthread_t timerThreadId;
static pthread_mutex_t timerThreadMutex;
struct threadArgument {
    uint8_t handle;
    double microseconds;
};

void *timerThread(void *arg) {
    struct threadArgument *Arguments = (struct threadArgument *)arg;
    usleep(Arguments->microseconds);
    pthread_mutex_lock(&timerThreadMutex);
    if(!timerQueue) timerQueue = createQueue(10);
    enqueue(timerQueue, Arguments->handle);
    for(int index = 0; index < timerQueue->capacity; index++) {
        printf("Queue item %d %d\r\n", timerQueue->array[index], timerQueue->size);
    }
    pthread_mutex_unlock(&timerThreadMutex);
    pthread_exit(NULL);
}

int startTimer(double microseconds, uint8_t handle) {
    struct threadArgument *Arguments = (struct threadArgument *)malloc(sizeof(struct threadArgument));
    Arguments->handle = handle;
    Arguments->microseconds = microseconds;

    printf("%d %f timer\n", handle, microseconds);

    pthread_t timerThreadId;
    return pthread_create(&timerThreadId,
                           NULL,
                           timerThread,
                           (void *)Arguments);
}

int isTimerElapsed() {
    pthread_mutex_lock(&timerThreadMutex);
    if(!timerQueue) timerQueue = createQueue(10);
    int result = !isEmpty(timerQueue);
    if(result) printf("elapsed\n");
    pthread_mutex_unlock(&timerThreadMutex);
    return result;
}

int getElapsedTimer() {
    pthread_mutex_lock(&timerThreadMutex);
    if(!timerQueue) timerQueue = createQueue(10);
    int result = dequeue(timerQueue);
    printf("Dequeued %d\r\n", result);
    pthread_mutex_unlock(&timerThreadMutex);
    return result;
}