#ifndef AMBULANCE_H
#define AMBULANCE_H

#include <string>
#include <vector>

extern "C" {
#include "common.h"
}

class Ambulance {
private:
    int ambulanceID;
    int currentLocationID;
    AmbulanceStatus status;
    std::string type;
    std::vector<std::string> capabilities;
    int assignedEmergencyID;

public:
    Ambulance(
        int id,
        int locationID,
        std::string ambulanceType
    );

    int getID() const;
    int getLocation() const;
    AmbulanceStatus getStatus() const;
    std::string getType() const;

    bool isAvailable() const;

    bool hasCapability(
        const std::string& capability
    ) const;

    void addCapability(
        const std::string& capability
    );

    void setLocation(int locationID);

    void setStatus(
        AmbulanceStatus newStatus
    );

    void assignEmergency(int emergencyID);

    void clearAssignment();

    int getAssignedEmergency() const;
};

#endif