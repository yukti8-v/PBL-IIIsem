#include <stdio.h>
#include "../include/emergency.h"
#include "../include/linkedlist.h"
#include "../include/search_sort.h"

int main()
{
    EmergencyNode* head = NULL;

    Emergency e1 = createEmergency(
        101, "Clock Tower", "Accident",
        2, "Basic Support", "Pending"
    );

    Emergency e2 = createEmergency(
        102, "Rajpur Road", "Heart Attack",
        4, "Cardiac Support", "Pending"
    );

    Emergency e3 = createEmergency(
        103, "ISBT", "Fire Accident",
        3, "Trauma Support", "Pending"
    );

    Emergency e4 = createEmergency(
        104, "Prem Nagar", "Minor Injury",
        1, "First Aid", "Pending"
    );

    addEmergency(&head, e1);
    addEmergency(&head, e2);
    addEmergency(&head, e3);
    addEmergency(&head, e4);

    printf("\nBEFORE SORTING:\n");
    displayEmergencies(head);

    sortBySeverity(head);

    printf("\nAFTER SORTING:\n");
    displayEmergencies(head);

    printf("\nSEARCHING CRITICAL EMERGENCIES:\n");
    searchBySeverity(head, 4);

    freeEmergencyList(&head);

    return 0;
}