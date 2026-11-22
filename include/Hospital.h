#ifndef HOSPITAL_H
#define HOSPITAL_H

#include <string>
#include <vector>

extern "C" {
#include "common.h"
}

class Hospital {
private:
    int hospitalID;
    std::string hospitalName;
    int locationID;
    HospitalStatus status;

    std::vector<std::string> facilities;

public:
    Hospital(
        int id,
        std::string name,
        int locationID
    );

    int getID() const;

    std::string getName() const;

    int getLocation() const;

    HospitalStatus getStatus() const;

    bool isAvailable() const;

    bool hasFacility(
        const std::string& facility
    ) const;

    void addFacility(
        const std::string& facility
    );

    void setStatus(
        HospitalStatus newStatus
    );
};

#endif