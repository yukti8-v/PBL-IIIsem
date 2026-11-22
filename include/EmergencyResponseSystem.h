#ifndef EMERGENCY_RESPONSE_SYSTEM_H
#define EMERGENCY_RESPONSE_SYSTEM_H

#include <vector>
#include <queue>

#include "Ambulance.h"
#include "Hospital.h"

extern "C" {
#include "common.h"
#include "emergency.h"
#include "graph.h"
}


// --------------------------------------------------
// Priority queue comparison
// Higher severity gets higher priority
// --------------------------------------------------

struct EmergencyCompare {

    bool operator()(
        const Emergency& a,
        const Emergency& b
    ) const {

        return a.severity < b.severity;
    }
};


class EmergencyResponseSystem {

private:

    Graph cityGraph;

    std::vector<Ambulance> ambulances;

    std::vector<Hospital> hospitals;

    std::priority_queue<
        Emergency,
        std::vector<Emergency>,
        EmergencyCompare
    > emergencyPriorityQueue;


public:

    EmergencyResponseSystem();

    // -------------------------
    // Initialization
    // -------------------------

    void initialize(int numberOfLocations);

    void addRoad(
        int source,
        int destination,
        double distance
    );


    // -------------------------
    // Ambulance / Hospital
    // -------------------------

    void addAmbulance(
        const Ambulance& ambulance
    );

    void addHospital(
        const Hospital& hospital
    );


    // -------------------------
    // Emergency handling
    // -------------------------

    void addEmergency(
        Emergency emergency
    );

    void processNextEmergency();


    // -------------------------
    // Selection
    // -------------------------

    int findSuitableAmbulance(
        const Emergency& emergency
    );

    int findSuitableHospital(
        const Emergency& emergency
    );


    // -------------------------
    // Dispatch
    // -------------------------

    void dispatchAmbulance(
        int ambulanceID,
        int emergencyID
    );


    // -------------------------
    // Complete Emergency
    // -------------------------

    void completeEmergency(
        int emergencyID
    );


    // -------------------------
    // Display
    // -------------------------

    void displaySystemStatus() const;
};

#endif