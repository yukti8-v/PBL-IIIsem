#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include "emergency.h"

/*
    Node of the linked list.

    Each node stores:
    1. One Emergency record
    2. Address of the next node
*/

typedef struct EmergencyNode
{
    Emergency data;
    struct EmergencyNode* next;

} EmergencyNode;


/* Add a new emergency to the linked list */
void addEmergency(EmergencyNode** head, Emergency emergency);


/* Search an emergency using Emergency ID */
EmergencyNode* searchEmergency(
    EmergencyNode* head,
    int emergencyID
);


/* Delete an emergency using Emergency ID */
int deleteEmergency(
    EmergencyNode** head,
    int emergencyID
);


/* Update status of an emergency */
int updateEmergencyStatus(
    EmergencyNode* head,
    int emergencyID,
    const char newStatus[]
);


/* Display all emergencies */
void displayEmergencies(EmergencyNode* head);


/* Free all dynamically allocated memory */
void freeEmergencyList(EmergencyNode** head);
/*
    Add a completed emergency record
    to the emergency history list.
*/
void addEmergencyHistory(
    EmergencyNode** historyHead,
    Emergency emergency
);

/*
    Display all stored emergency
    history records.
*/
void displayEmergencyHistory(
    EmergencyNode* historyHead
);

#endif