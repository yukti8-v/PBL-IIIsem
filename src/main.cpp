#include <iostream>
#include <cstring>

#include "../include/EmergencyResponseSystem.h"

using namespace std;


int main() {

    EmergencyResponseSystem system;


    // ==========================================
    // 1. INITIALIZE CITY GRAPH
    // ==========================================

    system.initialize(10);


    // ==========================================
    // 2. ADD ROADS
    // ==========================================

    system.addRoad(0, 1, 2.5);
    system.addRoad(1, 2, 4.0);
    system.addRoad(2, 3, 3.2);
    system.addRoad(3, 6, 2.8);

    system.addRoad(0, 4, 5.1);
    system.addRoad(1, 5, 3.7);

    system.addRoad(6, 8, 4.2);
    system.addRoad(7, 8, 2.9);

    system.addRoad(1, 7, 4.5);
    system.addRoad(5, 9, 3.0);
    system.addRoad(9, 8, 5.0);


    // ==========================================
    // 3. CREATE AMBULANCES
    // ==========================================

    Ambulance a1(
        1,
        4,
        "Basic"
    );

    a1.addCapability("Oxygen");


    Ambulance a2(
        2,
        5,
        "Advanced"
    );

    a2.addCapability("Trauma");
    a2.addCapability("Oxygen");
    a2.addCapability("Cardiac");


    Ambulance a3(
        3,
        3,
        "Advanced"
    );

    a3.addCapability("Trauma");
    a3.addCapability("ICU");
    a3.addCapability("Oxygen");

    a3.setStatus(
        AMBULANCE_UNAVAILABLE
    );


    Ambulance a4(
        4,
        9,
        "Cardiac"
    );

    a4.addCapability("Cardiac");
    a4.addCapability("Oxygen");


    system.addAmbulance(a1);
    system.addAmbulance(a2);
    system.addAmbulance(a3);
    system.addAmbulance(a4);


    // ==========================================
    // 4. CREATE HOSPITALS
    // ==========================================

    Hospital h1(
        1,
        "City Hospital",
        6
    );

    h1.addFacility("Emergency");
    h1.addFacility("ICU");
    h1.addFacility("Cardiology");


    Hospital h2(
        2,
        "General Hospital",
        7
    );

    h2.addFacility("Emergency");
    h2.addFacility("General Surgery");


    Hospital h3(
        3,
        "Trauma Centre",
        8
    );

    h3.addFacility("Trauma");
    h3.addFacility("ICU");
    h3.addFacility("Emergency Surgery");


    system.addHospital(h1);
    system.addHospital(h2);
    system.addHospital(h3);


    // ==========================================
    // 5. CREATE EMERGENCIES
    // ==========================================

    Emergency e1{};

    e1.emergencyID = 101;
    e1.locationID = 0;

    strcpy(
        e1.emergencyType,
        "Accident"
    );

    e1.severity = CRITICAL;

    strcpy(
        e1.requiredCapability,
        "Trauma"
    );

    strcpy(
        e1.requiredFacility,
        "Trauma"
    );

    e1.status = WAITING;

    e1.assignedAmbulanceID = -1;
    e1.assignedHospitalID = -1;


    Emergency e2{};

    e2.emergencyID = 102;
    e2.locationID = 2;

    strcpy(
        e2.emergencyType,
        "Breathing Problem"
    );

    e2.severity = HIGH;

    strcpy(
        e2.requiredCapability,
        "Oxygen"
    );

    strcpy(
        e2.requiredFacility,
        "Emergency"
    );

    e2.status = WAITING;

    e2.assignedAmbulanceID = -1;
    e2.assignedHospitalID = -1;


    // ==========================================
    // 6. ADD TO PRIORITY QUEUE
    // ==========================================

    system.addEmergency(e2);
    system.addEmergency(e1);


    // ==========================================
    // 7. DISPLAY SYSTEM
    // ==========================================

    system.displaySystemStatus();


    // ==========================================
    // 8. PROCESS
    // ==========================================

    system.processNextEmergency();

    system.processNextEmergency();


    // ==========================================
    // 9. FINAL STATUS
    // ==========================================

    system.displaySystemStatus();


    return 0;
}
