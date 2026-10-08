#include <stdio.h>

#include "../include/emergency.h"
#include "../include/queue.h"


int main()
{
    EmergencyQueue queue;

    /* Initialize queue */
    initializeQueue(&queue);


    /* Create sample emergencies */

    Emergency e1 = createEmergency(
        101,
        "Clock Tower",
        "Road Accident",
        4,
        "Trauma Support",
        "Pending"
    );

    Emergency e2 = createEmergency(
        102,
        "Rajpur Road",
        "Medical Emergency",
        2,
        "Basic Life Support",
        "Pending"
    );

    Emergency e3 = createEmergency(
        103,
        "ISBT",
        "Fire Accident",
        3,
        "Fire and Trauma Support",
        "Pending"
    );


    /* Add emergencies to queue */

    enqueueEmergency(&queue, e1);
    enqueueEmergency(&queue, e2);
    enqueueEmergency(&queue, e3);


    printf("\nAFTER ENQUEUE:\n");

    displayEmergencyQueue(&queue);


    /* Remove first emergency */

    Emergency removed;

    printf("\nDEQUEUE OPERATION:\n");

    if (dequeueEmergency(&queue, &removed))
    {
        printf(
            "\nEmergency %d removed from queue.\n",
            removed.emergencyID
        );

        printf("Removed Emergency:\n");

        displayEmergency(removed);
    }
    else
    {
        printf("\nQueue is empty!\n");
    }


    printf("\nQUEUE AFTER DEQUEUE:\n");

    displayEmergencyQueue(&queue);

