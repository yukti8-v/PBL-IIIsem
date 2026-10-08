#include <stdio.h>
#include <stdlib.h>

#include "../../include/queue.h"


/* Initialize the queue */
void initializeQueue(EmergencyQueue* queue)
{
    queue->front = NULL;
    queue->rear = NULL;
}


/* Check if queue is empty */
int isQueueEmpty(EmergencyQueue* queue)
{
    return queue->front == NULL;
}


/* Add emergency at the rear of queue */
void enqueueEmergency(
    EmergencyQueue* queue,
    Emergency emergency
)
{
    QueueNode* newNode =
        (QueueNode*)malloc(sizeof(QueueNode));

    if (newNode == NULL)
    {
        printf("Memory allocation failed!\n");
        return;
    }

    newNode->data = emergency;
    newNode->next = NULL;

    /* If queue is empty */
    if (queue->rear == NULL)
    {
        queue->front = newNode;
        queue->rear = newNode;
        return;
    }

    /* Add new node after rear */
    queue->rear->next = newNode;
    queue->rear = newNode;
}


/* Remove emergency from front of queue */
int dequeueEmergency(
    EmergencyQueue* queue,
    Emergency* removedEmergency
)
{
    if (isQueueEmpty(queue))
    {
        return 0;
    }

    QueueNode* temp = queue->front;

    /* Copy removed emergency */
    if (removedEmergency != NULL)
    {
        *removedEmergency = temp->data;
    }

    queue->front = queue->front->next;

    /* Queue became empty */
    if (queue->front == NULL)
    {
        queue->rear = NULL;
    }

    free(temp);

    return 1;
}


/* Display all waiting emergencies */
void displayEmergencyQueue(EmergencyQueue* queue)
{
    if (isQueueEmpty(queue))
    {
        printf("\nEmergency queue is empty.\n");
        return;
    }

    QueueNode* current = queue->front;

    printf("\n===== EMERGENCY WAITING QUEUE =====\n");

    while (current != NULL)
    {
        displayEmergency(current->data);

        current = current->next;
    }
}


/* Free complete queue */
void freeEmergencyQueue(EmergencyQueue* queue)
{
    Emergency removedEmergency;

    while (!isQueueEmpty(queue))
    {
        dequeueEmergency(queue, &removedEmergency);
    }
}