
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <iomanip>
#include <cmath>
#include <limits>

#include "RoutingSystem.h"

using namespace std;

// Store location information
struct Location {
    int id;
    string name;
};

// Store road information
struct Road {
    int source;
    int destination;
    double distance;
};

// Remove extra whitespace
string trim(const string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");

    if (start == string::npos)
        return "";

    size_t end = str.find_last_not_of(" \t\r\n");

    return str.substr(start, end - start + 1);
}

// Read locations.csv
bool loadLocations(const string& filename,
                   vector<Location>& locations) {

    ifstream file(filename);

    if (!file.is_open()) {
        cout << "ERROR: Cannot open " << filename << endl;
        return false;
    }

    string line;
    getline(file, line); // Skip header

    while (getline(file, line)) {

        if (trim(line).empty())
            continue;

        stringstream ss(line);
        string idText, name;

        getline(ss, idText, ',');
        getline(ss, name, ',');

        try {
            int id = stoi(trim(idText));

            if (id < 0 || id >= MAX_LOCATIONS) {
                cout << "Skipping invalid location ID: "
                     << id << endl;
                continue;
            }

            if (trim(name).empty()) {
                cout << "Skipping unnamed location: "
                     << id << endl;
                continue;
            }

            locations.push_back({id, trim(name)});
        }
        catch (...) {
            cout << "Skipping invalid location row: "
                 << line << endl;
        }
    }

    return !locations.empty();
}

// Read roads.csv
bool loadRoads(const string& filename,
               vector<Road>& roads) {

    ifstream file(filename);

    if (!file.is_open()) {
        cout << "ERROR: Cannot open " << filename << endl;
        return false;
    }

    string line;
    getline(file, line); // Skip header

    while (getline(file, line)) {

        if (trim(line).empty())
            continue;

        stringstream ss(line);
        string sourceText;
        string destinationText;
        string distanceText;

        getline(ss, sourceText, ',');
        getline(ss, destinationText, ',');
        getline(ss, distanceText, ',');

        try {
            Road road;

            road.source = stoi(trim(sourceText));
            road.destination = stoi(trim(destinationText));
            road.distance = stod(trim(distanceText));

            if (road.source < 0 ||
                road.destination < 0 ||
                road.source >= MAX_LOCATIONS ||
                road.destination >= MAX_LOCATIONS ||
                !std::isfinite(road.distance) ||
                road.distance <= 0 ||
                road.distance >= INF) {

                cout << "Skipping invalid road: "
                     << line << endl;
                continue;
            }

            roads.push_back(road);
        }
        catch (...) {
            cout << "Skipping invalid road row: "
                 << line << endl;
        }
    }

    return !roads.empty();
}

// Find location name using ID
string getLocationName(const vector<Location>& locations,
                       int id) {

    for (const auto& loc : locations) {
        if (loc.id == id)
            return loc.name;
    }

    return "Unknown Location";
}

// Check whether location ID exists
bool locationExists(const vector<Location>& locations,
                    int id) {

    for (const auto& loc : locations) {
        if (loc.id == id)
            return true;
    }

    return false;
}

int main() {

    cout << "\n========================================\n";
    cout << "  DEHRADUN EMERGENCY ROUTING SYSTEM\n";
    cout << "========================================\n";

    vector<Location> locations;
    vector<Road> roads;

    // STEP 1: Load dataset
    if (!loadLocations("data/locations.csv", locations)) {
        cout << "Failed to load locations.csv\n";
        return 1;
    }

    if (!loadRoads("data/roads.csv", roads)) {
        cout << "Failed to load roads.csv\n";
        return 1;
    }

    cout << "\nLocations loaded: "
         << locations.size() << endl;

    cout << "Roads loaded: "
         << roads.size() << endl;

    // STEP 2: Determine graph size
    int maxID = -1;

    for (const auto& loc : locations) {
        if (loc.id > maxID)
            maxID = loc.id;
    }

    if (maxID < 0 || maxID >= MAX_LOCATIONS) {
        cout << "ERROR: Invalid graph size.\n";
        return 1;
    }

    // STEP 3: Initialize graph
    Graph cityGraph;
    initializeGraph(&cityGraph, maxID + 1);

    // STEP 4: Add roads to graph
    int roadsAdded = 0;

    for (const auto& road : roads) {

        if (!locationExists(locations, road.source) ||
            !locationExists(locations, road.destination)) {

            cout << "Skipping road with unknown location ID: "
                 << road.source << " -> "
                 << road.destination << endl;
            continue;
        }

        // Keep minimum distance for duplicate roads
        if (road.distance <
            cityGraph.adjacencyMatrix[road.source][road.destination]) {

            addRoad(&cityGraph,
                    road.source,
                    road.destination,
                    road.distance);

            roadsAdded++;
        }
    }

    cout << "Roads added to graph: "
         << roadsAdded << endl;

    // STEP 5: Create RoutingSystem
    RoutingSystem routing(&cityGraph);

    // STEP 6: Show available locations
    cout << "\nAVAILABLE LOCATIONS:\n";
    cout << "----------------------------------------\n";

    for (const auto& loc : locations) {
        cout << setw(3) << loc.id
             << " - " << loc.name << endl;
    }

    // STEP 7: Take source and destination
    int source, destination;

    cout << "\nEnter source location ID: ";

    if (!(cin >> source)) {
        cout << "ERROR: Invalid source input.\n";
        return 1;
    }

    cout << "Enter destination location ID: ";

    if (!(cin >> destination)) {
        cout << "ERROR: Invalid destination input.\n";
        return 1;
    }

    if (!locationExists(locations, source) ||
        !locationExists(locations, destination)) {

        cout << "ERROR: Location ID not found.\n";
        return 1;
    }

    // STEP 8: Run Dijkstra through RoutingSystem
    RouteResult result =
        routing.getShortestRoute(source, destination);

    if (!result.found) {
        cout << "\nNo route found between these locations.\n";
        return 0;
    }

    // STEP 9: Display shortest route
    cout << "\n========================================\n";
    cout << "           SHORTEST ROUTE\n";
    cout << "========================================\n";

    cout << fixed << setprecision(2);

    cout << "\nSource      : "
         << getLocationName(locations, source) << endl;

    cout << "Destination : "
         << getLocationName(locations, destination) << endl;

    cout << "Distance    : "
         << result.distance << " km\n";

    cout << "\nRoute:\n\n";

    for (size_t i = 0; i < result.path.size(); i++) {

        int id = result.path[i];

        cout << getLocationName(locations, id)
             << " [" << id << "]";

        if (i + 1 < result.path.size()) {
            cout << "\n       |\n       v\n";
        }
    }

    cout << "\n\n========================================\n";
    cout << "       TEST COMPLETED SUCCESSFULLY\n";
    cout << "========================================\n";

    return 0;
}
