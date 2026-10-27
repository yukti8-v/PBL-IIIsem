#include "../../include/Ambulance.h"

Ambulance::Ambulance(
    int id,
    int locationID,
    std::string ambulanceType
) {
    ambulanceID = id;
    currentLocationID = locationID;
    type = ambulanceType;

    status = AMBULANCE_AVAILABLE;
    assignedEmergencyID = -1;
}

int Ambulance::getID() const {
    return ambulanceID;
}

int Ambulance::getLocation() const {
    return currentLocationID;
}

AmbulanceStatus Ambulance::getStatus() const {
    return status;
}

std::string Ambulance::getType() const {
    return type;
}

bool Ambulance::isAvailable() const {
    return status == AMBULANCE_AVAILABLE;
}

bool Ambulance::hasCapability(
    const std::string& capability
) const {

    for (const std::string& item : capabilities) {
        if (item == capability) {
            return true;
        }
    }

    return false;
}

void Ambulance::addCapability(
    const std::string& capability
) {
    capabilities.push_back(capability);
}

void Ambulance::setLocation(int locationID) {
    currentLocationID = locationID;
}

void Ambulance::setStatus(
    AmbulanceStatus newStatus
) {
    status = newStatus;
}

void Ambulance::assignEmergency(int emergencyID) {

    assignedEmergencyID = emergencyID;

    status = AMBULANCE_ASSIGNED;
}

void Ambulance::clearAssignment() {

    assignedEmergencyID = -1;

    status = AMBULANCE_AVAILABLE;
}

int Ambulance::getAssignedEmergency() const {
    return assignedEmergencyID;
}