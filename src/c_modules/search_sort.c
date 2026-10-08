#include <stdio.h>
#include "../../include/search_sort.h"

/* Sort linked list using Bubble Sort */
void sortBySeverity(EmergencyNode* head)
{
    if (head == NULL)
        return;

    EmergencyNode* current;
    EmergencyNode* last = NULL;
    int swapped;

    do
    {
        swapped = 0;
        current = head;

        while (current->next != last)
        {
            if (current->data.severity <
                current->next->data.severity)
            {
                Emergency temp = current->data;
                current->data = current->next->data;
                current->next->data = temp;

                swapped = 1;
            }

            current = current->next;
        }

        last = current;

    } while (swapped);
}

/* Search emergencies by severity */
void searchBySeverity(EmergencyNode* head, int severity)
{
    EmergencyNode* current = head;
    int found = 0;

    printf("\n===== EMERGENCIES WITH SEVERITY %d =====\n",
           severity);

    while (current != NULL)
    {
        if (current->data.severity == severity)
        {
            displayEmergency(current->data);
            found = 1;
        }

        current = current->next;
    }

    if (!found)
        printf("No emergencies found.\n");
}