#include <stdio.h>
#include <string.h>

#include "../../include/emergency.h"


Emergency createEmergency(
    int id,
    const char location[],
    const char type[],
    int severity,
    const char support[],
    const char status[]
)
{
    Emergency e;

    e.emergencyID = id;

    strcpy(e.location, location);
    strcpy(e.emergencyType, type);

    e.severity = severity;

    strcpy(e.requiredSupport, support);
    strcpy(e.status, status);

    return e;
}


const char* getSeverityName(int severity)
{
    switch (severity)
    {
        case 4:
            return "Critical";

        case 3:
            return "High";

        case 2:
            return "Medium";

        case 1:
            return "Low";

        default:
            return "Unknown";
    }
}


void displayEmergency(Emergency e)
{
    printf("\n==============================\n");

    printf("Emergency ID     : %d\n", e.emergencyID);
    printf("Location         : %s\n", e.location);
    printf("Emergency Type   : %s\n", e.emergencyType);

    printf(
        "Severity         : %d (%s)\n",
        e.severity,
        getSeverityName(e.severity)
    );

    printf("Required Support : %s\n", e.requiredSupport);
    printf("Status : %s\n", e.status);

    printf("\n");
}