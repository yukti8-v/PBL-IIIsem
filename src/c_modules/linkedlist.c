#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../include/linkedlist.h"


/* Add a new emergency at the end of the linked list */
void addEmergency(EmergencyNode** head, Emergency emergency)
{
    EmergencyNode* newNode =
        (EmergencyNode*)malloc(sizeof(EmergencyNode));

    if (newNode == NULL)
    {
        printf("Memory allocation failed!\n");
        return;
    }

    newNode->data = emergency;
    newNode->next = NULL;

    /* If list is empty */
    if (*head == NULL)
    {
        *head = newNode;
        return;
    }

    /* Otherwise move to the last node */
    EmergencyNode* current = *head;

    while (current->next != NULL)
    {
        current = current->next;
    }

    current->next = newNode;
}


/* Search emergency using Emergency ID */
EmergencyNode* searchEmergency(
    EmergencyNode* head,
    int emergencyID
)
{
    EmergencyNode* current = head;

    while (current != NULL)
    {
        if (current->data.emergencyID == emergencyID)
        {
            return current;
        }

        current = current->next;
    }

    return NULL;
}


/* Delete emergency using Emergency ID */
int deleteEmergency(
    EmergencyNode** head,
    int emergencyID
)
{
    if (*head == NULL)
    {
        return 0;
    }

    EmergencyNode* current = *head;
    EmergencyNode* previous = NULL;

    while (current != NULL)
    {
        if (current->data.emergencyID == emergencyID)
        {
            /* First node is being deleted */
            if (previous == NULL)
            {
                *head = current->next;
            }
            else
            {
                previous->next = current->next;
            }

            free(current);

            return 1;
        }

        previous = current;
        current = current->next;
    }

    return 0;
}


/* Update emergency status */
int updateEmergencyStatus(
    EmergencyNode* head,
    int emergencyID,
    const char newStatus[]
)
{
    EmergencyNode* emergency =
        searchEmergency(head, emergencyID);

    if (emergency == NULL)
    {
        return 0;
    }

    strncpy(
        emergency->data.status,
        newStatus,
        MAX_STATUS - 1
    );

    emergency->data.status[MAX_STATUS - 1] = '\0';

    return 1;
}


/* Display all emergency records */
void displayEmergencies(EmergencyNode* head)
{
    if (head == NULL)
    {
        printf("\nNo emergency records available.\n");
        return;
    }

    EmergencyNode* current = head;

    printf("\n===== ALL EMERGENCY RECORDS =====\n");

    while (current != NULL)
    {
        displayEmergency(current->data);

        current = current->next;
    }
}


/* Free complete linked list */
void freeEmergencyList(EmergencyNode** head)
{
    EmergencyNode* current = *head;

    while (current != NULL)
    {
        EmergencyNode* temp = current;

        current = current->next;

        free(temp);
    }

    *head = NULL;
}
/* Add an emergency record to history */
void addEmergencyHistory(
    EmergencyNode** historyHead,
    Emergency emergency
)
{
    if (historyHead == NULL)
    {
        printf("Invalid history list.\n");
        return;
    }

    /* Create a new history node */
    EmergencyNode* newNode =
        (EmergencyNode*)malloc(sizeof(EmergencyNode));

    if (newNode == NULL)
    {
        printf("Memory allocation failed!\n");
        return;
    }

    newNode->data = emergency;
    newNode->next = NULL;

    /* If history is empty */
    if (*historyHead == NULL)
    {
        *historyHead = newNode;
        return;
    }

    /* Move to the last history node */
    EmergencyNode* temp = *historyHead;

    while (temp->next != NULL)
    {
        temp = temp->next;
    }

    temp->next = newNode;
}


/* Display all emergency history records */
void displayEmergencyHistory(
    EmergencyNode* historyHead
)
{
    if (historyHead == NULL)
    {
        printf("\nEmergency history is empty.\n");
        return;
    }

    printf("\n===== EMERGENCY HISTORY =====\n");

    EmergencyNode* temp = historyHead;

    while (temp != NULL)
    {
        displayEmergency(temp->data);
        temp = temp->next;
    }
}