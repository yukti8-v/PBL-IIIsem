#include <stdio.h>
#include "../include/emergency.h"

int main()
{
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

    printf("\n EMERGENCY MANAGEMENT TEST \n");

    displayEmergency(e1);
    displayEmergency(e2);

    return 0;
}