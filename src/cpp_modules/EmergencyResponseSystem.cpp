#include "../../include/EmergencyResponseSystem.h"

#include <iostream>
#include <limits>

using namespace std;
EmergencyResponseSystem::EmergencyResponseSystem() {
}

void EmergencyResponseSystem::initialize(
    int numberOfLocations
) {

    initializeGraph(
        &cityGraph,
        numberOfLocations
    );
}
initializeGraph()
shortestDistance()
shortestPath()
void EmergencyResponseSystem::addRoad(
    int source,
    int destination,
    double distance
) {

    ::addRoad(
        &cityGraph,
        source,
        destination,
        distance
    );
}
void EmergencyResponseSystem::addAmbulance(
    const Ambulance& ambulance
) {

    ambulances.push_back(ambulance);
}
void EmergencyResponseSystem::addHospital(
    const Hospital& hospital
) {

    hospitals.push_back(hospital);
}
void EmergencyResponseSystem::addEmergency(
    Emergency emergency
) {

    emergencyPriorityQueue.push(emergency);

    cout << "\nEmergency E"
         << emergency.emergencyID
         << " added successfully.";

    cout << "\nSeverity: "
         << emergency.severity
         << endl;
}
int EmergencyResponseSystem::findSuitableAmbulance(
    const Emergency& emergency
) {

    int selectedAmbulanceID = -1;

    double minimumDistance =
        numeric_limits<double>::max();


    for (const Ambulance& ambulance : ambulances) {

        // --------------------------------
        // Rule 1: Must be available
        // --------------------------------

        if (!ambulance.isAvailable()) {
            continue;
        }


        // --------------------------------
        // Rule 2: Must have capability
        // --------------------------------

        if (!ambulance.hasCapability(
                emergency.requiredCapability)) {

            continue;
        }


        // --------------------------------
        // Rule 3: Find actual route
        // distance using Dijkstra
        // --------------------------------

        double distance =
            shortestDistance(
                &cityGraph,
                ambulance.getLocation(),
                emergency.locationID
            );


        // Ignore unreachable locations

        if (distance >= INF) {
            continue;
        }


        // --------------------------------
        // Rule 4: Select nearest among
        // SUITABLE ambulances
        // --------------------------------

        if (distance < minimumDistance) {

            minimumDistance = distance;

            selectedAmbulanceID =
                ambulance.getID();
        }
    }


    return selectedAmbulanceID;
}
int EmergencyResponseSystem::findSuitableHospital(
    const Emergency& emergency
) {

    int selectedHospitalID = -1;

    double minimumDistance =
        numeric_limits<double>::max();


    for (const Hospital& hospital : hospitals) {

        // Hospital must be available

        if (!hospital.isAvailable()) {
            continue;
        }


        // Hospital must contain required facility

        if (!hospital.hasFacility(
                emergency.requiredFacility)) {

            continue;
        }


        // Calculate route distance

        double distance =
            shortestDistance(
                &cityGraph,
                emergency.locationID,
                hospital.getLocation()
            );


        if (distance >= INF) {
            continue;
        }


        if (distance < minimumDistance) {

            minimumDistance = distance;

            selectedHospitalID =
                hospital.getID();
        }
    }


    return selectedHospitalID;
}
void EmergencyResponseSystem::dispatchAmbulance(
    int ambulanceID,
    int emergencyID
) {

    for (Ambulance& ambulance : ambulances) {

        if (ambulance.getID() == ambulanceID) {

            ambulance.assignEmergency(
                emergencyID
            );

            cout << "\nAmbulance A"
                 << ambulanceID
                 << " dispatched for Emergency E"
                 << emergencyID
                 << endl;

            return;
        }
    }
}
void EmergencyResponseSystem::processNextEmergency() {

    if (emergencyPriorityQueue.empty()) {

        cout << "\nNo emergency requests waiting.\n";

        return;
    }


    // Highest severity emergency

    Emergency emergency =
        emergencyPriorityQueue.top();

    emergencyPriorityQueue.pop();


    cout << "\n====================================\n";
    cout << "PROCESSING EMERGENCY E"
         << emergency.emergencyID << endl;
    cout << "====================================\n";


    cout << "Location: "
         << emergency.locationID
         << endl;

    cout << "Severity: "
         << emergency.severity
         << endl;

    cout << "Required Ambulance Capability: "
         << emergency.requiredCapability
         << endl;

    cout << "Required Hospital Facility: "
         << emergency.requiredFacility
         << endl;


    // ==========================================
    // STEP 1 - Select suitable ambulance
    // ==========================================

    int ambulanceID =
        findSuitableAmbulance(emergency);


    if (ambulanceID == -1) {

        cout << "\nNo suitable ambulance available.\n";

        return;
    }


    emergency.assignedAmbulanceID =
        ambulanceID;


    cout << "\nSelected Ambulance: A"
         << ambulanceID
         << endl;


    // ==========================================
    // STEP 2 - Dispatch ambulance
    // ==========================================

    dispatchAmbulance(
        ambulanceID,
        emergency.emergencyID
    );


    // ==========================================
    // STEP 3 - Select suitable hospital
    // ==========================================

    int hospitalID =
        findSuitableHospital(emergency);


    if (hospitalID == -1) {

        cout << "\nNo suitable hospital available.\n";

        return;
    }


    emergency.assignedHospitalID =
        hospitalID;


    cout << "Selected Hospital: H"
         << hospitalID
         << endl;


    cout << "\nEmergency E"
         << emergency.emergencyID
         << " successfully assigned.\n";
}
void EmergencyResponseSystem::completeEmergency(
    int emergencyID
) {

    for (Ambulance& ambulance : ambulances) {

        if (ambulance.getAssignedEmergency()
            == emergencyID) {

            ambulance.clearAssignment();

            cout << "\nEmergency E"
                 << emergencyID
                 << " completed.";

            cout << "\nAmbulance A"
                 << ambulance.getID()
                 << " is available again.\n";

            return;
        }
    }


    cout << "\nEmergency not found.\n";
}
void EmergencyResponseSystem::displaySystemStatus() const {

    cout << "\n====================================\n";
    cout << "      EMERGENCY RESPONSE SYSTEM\n";
    cout << "====================================\n";


    cout << "\nAMBULANCES\n";
    cout << "------------------------------------\n";

    for (const Ambulance& ambulance : ambulances) {

        cout << "A"
             << ambulance.getID();

        cout << " | Location: "
             << ambulance.getLocation();

        cout << " | ";

        if (ambulance.isAvailable())
            cout << "AVAILABLE";
        else
            cout << "BUSY";

        cout << endl;
    }


    cout << "\nHOSPITALS\n";
    cout << "------------------------------------\n";

    for (const Hospital& hospital : hospitals) {

        cout << "H"
             << hospital.getID();

        cout << " | "
             << hospital.getName();

        cout << " | Location: "
             << hospital.getLocation();

        cout << " | ";

        if (hospital.isAvailable())
            cout << "AVAILABLE";
        else
            cout << "UNAVAILABLE";

        cout << endl;
    }


    cout << "\nWaiting Emergencies: "
         << emergencyPriorityQueue.size()
         << endl;
}
