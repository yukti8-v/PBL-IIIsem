#ifndef QUEUE_H
#define QUEUE_H

#include "emergency.h"

/* Node used in Emergency Queue */
typedef struct QueueNode
{
    Emergency data;
    struct QueueNode* next;

} QueueNode;


/* Queue structure */
typedef struct
{
    QueueNode* front;
    QueueNode* rear;

} EmergencyQueue;


/* Initialize an empty queue */
void initializeQueue(EmergencyQueue* queue);

/* Add emergency to rear of queue */
void enqueueEmergency(
    EmergencyQueue* queue,
    Emergency emergency
);


/* Remove emergency from front of queue */
int dequeueEmergency(
    EmergencyQueue* queue,
    Emergency* removedEmergency
);


/* Check whether queue is empty */
int isQueueEmpty(EmergencyQueue* queue);


/* Display waiting emergencies */
void displayEmergencyQueue(EmergencyQueue* queue);


/* Free queue memory */
void freeEmergencyQueue(EmergencyQueue* queue);

#endif