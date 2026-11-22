#include <stdio.h>

#include "../include/emergency.h"
#include "../include/linkedlist.h"


int main()
{
    /* Empty linked list */
    EmergencyNode* head = NULL;


    /* Create emergencies */

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


    /* Add emergencies */

    addEmergency(&head, e1);
    addEmergency(&head, e2);
    addEmergency(&head, e3);


    printf("\nAFTER ADDING EMERGENCIES:\n");

    displayEmergencies(head);


    /* Search test */

    printf("\nSEARCHING FOR EMERGENCY 102...\n");

    EmergencyNode* found =
        searchEmergency(head, 102);

    if (found != NULL)
    {
        printf("Emergency 102 found successfully!\n");

        displayEmergency(found->data);
    }
    else
    {
        printf("Emergency not found!\n");
    }


    /* Update test */

    printf("\nUPDATING EMERGENCY 102...\n");

    if (updateEmergencyStatus(head, 102, "Assigned"))
    {
        printf("Status updated successfully!\n");
    }
    else
    {
        printf("Emergency not found!\n");
    }


    /* Delete test */

    printf("\nDELETING EMERGENCY 101...\n");

    if (deleteEmergency(&head, 101))
    {
        printf("Emergency deleted successfully!\n");
    }
    else
    {
        printf("Emergency not found!\n");
    }


    printf("\nFINAL EMERGENCY LIST:\n");

    displayEmergencies(head);


    /* Release memory */

    freeEmergencyList(&head);

    return 0;
}