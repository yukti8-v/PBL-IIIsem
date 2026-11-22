#ifndef EMERGENCY_H
#define EMERGENCY_H

#define MAX_LOCATION 50
#define MAX_TYPE 50
#define MAX_SUPPORT 100
#define MAX_STATUS 30

/*
Severity Levels:
4 = Critical
3 = High
2 = Medium
1 = Low
*/

typedef struct
{
    int emergencyID;
    char location[MAX_LOCATION];
    char emergencyType[MAX_TYPE];
    int severity;
    char requiredSupport[MAX_SUPPORT];
    char status[MAX_STATUS];

} Emergency;


/* Function declarations */

Emergency createEmergency(
    int id,
    const char location[],
    const char type[],
    int severity,
    const char support[],
    const char status[]
);

void displayEmergency(Emergency emergency);

const char* getSeverityName(int severity);

#endif