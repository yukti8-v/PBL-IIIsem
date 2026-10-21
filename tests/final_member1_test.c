
#include <stdio.h>
#include <string.h>

#include "../include/emergency.h"
#include "../include/linkedlist.h"
#include "../include/queue.h"
#include "../include/search_sort.h"

int main()
{
    EmergencyNode* head = NULL;
    EmergencyNode* historyHead = NULL;
    EmergencyQueue queue;

    printf("\n===== MEMBER 1 FINAL INTEGRATION TEST =====\n");

    /* STEP 1: Initialize queue */
    initializeQueue(&queue);

    /* STEP 2: Create emergency records */
    Emergency e1 = createEmergency(
        101, "Clock Tower", "Road Accident",
        2, "Trauma Support", "Pending"
    );

    Emergency e2 = createEmergency(
        102, "Rajpur Road", "Heart Attack",
        4, "Cardiac Support", "Pending"
    );

    Emergency e3 = createEmergency(
        103, "ISBT", "Fire Accident",
        3, "Fire and Trauma Support", "Pending"
    );

    Emergency e4 = createEmergency(
        104, "Prem Nagar", "Minor Injury",
        1, "First Aid", "Pending"
    );

    /* STEP 3: Add to linked list */
    addEmergency(&head, e1);
    addEmergency(&head, e2);
    addEmergency(&head, e3);
    addEmergency(&head, e4);

    printf("\n===== INITIAL EMERGENCY LIST =====\n");
    displayEmergencies(head);

    /* STEP 4: Search emergency */
    printf("\n===== SEARCH EMERGENCY 102 =====\n");

    EmergencyNode* found = searchEmergency(head, 102);

    if (found != NULL)
    {
        displayEmergency(found->data);
    }
    else
    {
        printf("Emergency not found!\n");
    }

    /* STEP 5: Sort by severity */
    printf("\n===== SORTING BY SEVERITY =====\n");

    sortBySeverity(head);
    displayEmergencies(head);

    /* STEP 6: Search critical emergencies */
    printf("\n===== CRITICAL EMERGENCIES =====\n");
    searchBySeverity(head, 4);

    /* STEP 7: Add emergencies to queue */
    printf("\n===== ADDING TO WAITING QUEUE =====\n");

    enqueueEmergency(&queue, e1);
    enqueueEmergency(&queue, e2);
    enqueueEmergency(&queue, e3);
    enqueueEmergency(&queue, e4);

    displayEmergencyQueue(&queue);

    /* STEP 8: Dequeue first emergency */
    printf("\n===== PROCESSING QUEUE =====\n");

    Emergency processed;

    if (dequeueEmergency(&queue, &processed))
    {
        printf("\nDequeued Emergency ID: %d\n",
               processed.emergencyID);
    }

    displayEmergencyQueue(&queue);

    /* STEP 9: Update emergency 101 */
    printf("\n===== UPDATING EMERGENCY STATUS =====\n");

    if (updateEmergencyStatus(head, 101, "Completed"))
    {
        printf("Emergency 101 marked Completed.\n");
    }

    /* STEP 10: Move emergency 101 to history */
    EmergencyNode* completed = searchEmergency(head, 101);

    if (completed != NULL)
    {
        addEmergencyHistory(&historyHead, completed->data);
        deleteEmergency(&head, 101);
    }

    /* STEP 11: Display active emergencies */
    printf("\n===== REMAINING ACTIVE EMERGENCIES =====\n");
    displayEmergencies(head);

    /* STEP 12: Display history */
    printf("\n===== COMPLETED EMERGENCY HISTORY =====\n");
    displayEmergencyHistory(historyHead);

    /* STEP 13: Free allocated memory */
    freeEmergencyList(&head);
    freeEmergencyList(&historyHead);
    freeEmergencyQueue(&queue);

    printf("\n===== MEMBER 1 FINAL TEST FINISHED =====\n");

    return 0;
}
