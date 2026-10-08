#ifndef SEARCH_SORT_H
#define SEARCH_SORT_H

#include "linkedlist.h"

/* Sort emergencies by severity (4 to 1) */
void sortBySeverity(EmergencyNode* head);

/* Search emergencies by severity */
void searchBySeverity(EmergencyNode* head, int severity);

#endif