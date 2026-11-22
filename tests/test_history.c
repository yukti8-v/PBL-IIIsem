#include <stdio.h>
#include "../include/emergency.h"
#include "../include/linkedlist.h"

int main()
{
    EmergencyNode* head = NULL;
    EmergencyNode* historyHead = NULL;

    /* Create three emergency records */
    Emergency e1 = createEmergency(
        101, "Clock Tower", "Road Accident",
        4, "Trauma Support", "Pending"
    );

    Emergency e2 = createEmergency(
        102, "Rajpur Road", "Heart Attack",
        4, "Cardiac Support", "Pending"
    );

    Emergency e3 = createEmergency(
        103, "ISBT", "Minor Injury",
        2, "Basic Life Support", "Pending"
    );

    /* Add emergencies to active list */
    addEmergency(&head, e1);
    addEmergency(&head, e2);
    addEmergency(&head, e3);

    printf("\n===== INITIAL ACTIVE EMERGENCIES =====\n");
    displayEmergencies(head);

    /* Complete emergency 101 */
    /* Complete emergency 101 */
int updated = updateEmergencyStatus(head, 101, "Completed");

if (updated == 1)
{
    printf("\nSUCCESS: Emergency 101 status updated!\n");
}
else
{
    printf("\nERROR: Emergency 101 not found!\n");
}

/* Verify status after updating */
EmergencyNode* check = searchEmergency(head, 101);

if (check != NULL)
{
    printf("Emergency 101 current status: %s\n",
           check->data.status);
}

    EmergencyNode* completed = searchEmergency(head, 101);

    if (completed != NULL)
    {
        /* Save a copy in history */
        addEmergencyHistory(&historyHead, completed->data);

        /* Remove from active list */
        deleteEmergency(&head, 101);
    }

    printf("\n===== ACTIVE EMERGENCIES AFTER COMPLETION =====\n");
    displayEmergencies(head);

    printf("\n===== COMPLETED EMERGENCY HISTORY =====\n");
    displayEmergencyHistory(historyHead);

    /* Free dynamically allocated memory */
    freeEmergencyList(&head);
    freeEmergencyList(&historyHead);

    return 0;
}