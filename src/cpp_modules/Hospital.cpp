#include "../../include/Hospital.h"

Hospital::Hospital(
    int id,
    std::string name,
    int locationID
) {
    hospitalID = id;
    hospitalName = name;
    this->locationID = locationID;

    status = HOSPITAL_AVAILABLE;
}

int Hospital::getID() const {
    return hospitalID;
}

std::string Hospital::getName() const {
    return hospitalName;
}

int Hospital::getLocation() const {
    return locationID;
}

HospitalStatus Hospital::getStatus() const {
    return status;
}

bool Hospital::isAvailable() const {
    return status == HOSPITAL_AVAILABLE;
}

bool Hospital::hasFacility(
    const std::string& facility
) const {

    for (const std::string& item : facilities) {

        if (item == facility) {
            return true;
        }
    }

    return false;
}

void Hospital::addFacility(
    const std::string& facility
) {
    facilities.push_back(facility);
}

void Hospital::setStatus(
    HospitalStatus newStatus
) {
    status = newStatus;
}