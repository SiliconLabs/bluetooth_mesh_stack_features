#include "utils/timer.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>

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

void timerThread(void *arg) {
    struct threadArgument *Arguments = (struct threadArgument *)arg;
    usleep(Arguments->microseconds);
    pthread_mutex_lock(&timerThreadMutex);
    if(!timerQueue) timerQueue = createQueue(10);
    enqueue(timerQueue, Arguments->handle);
    for(int index = 0; index < timerQueue->capacity; index++) {
        printf("Queue item %d\r\n", timerQueue->array[index]);
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