#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <utility>
#include <cmath>

#include "RoutingSystem.h"

using namespace std;

struct Road {
    int from;
    int to;
    double km;
};

int main(int argc, char* argv[]) {
    // Usage:
    // routing_cli.exe source destination [blockedA blockedB] [blockedA blockedB] ...
    //
    // Any number of blocked road pairs can follow the destination.
    // Output: found distance originalDistance pathLength pathIDs...

    if (argc < 3) {
        cerr << "Invalid arguments\n";
        return 1;
    }

    if ((argc % 2) == 0) {
        cerr << "Blocked roads must be given as pairs\n";
        return 1;
    }

    int source = 0;
    int destination = 0;
    vector<pair<int, int> > blockedRoads;

    try {
        source = stoi(argv[1]);
        destination = stoi(argv[2]);

        for (int i = 3; i < argc; i = i + 2) {
            int blockedA = stoi(argv[i]);
            int blockedB = stoi(argv[i + 1]);
            blockedRoads.push_back(make_pair(blockedA, blockedB));
        }
    } catch (...) {
        cerr << "Invalid location ID\n";
        return 1;
    }

    ifstream file("data/roads.csv");

    if (file.is_open() == false) {
        cerr << "Cannot open data/roads.csv\n";
        return 1;
    }

    vector<Road> roads;
    string line;
    getline(file, line); // Skip header

    int maxID = -1;

    while (getline(file, line)) {
        if (line.empty()) {
            continue;
        }

        stringstream ss(line);
        string a;
        string b;
        string d;

        getline(ss, a, ',');
        getline(ss, b, ',');
        getline(ss, d, ',');

        try {
            Road r = {stoi(a), stoi(b), stod(d)};

            if (r.from < 0 || r.to < 0 ||
                r.from >= MAX_LOCATIONS ||
                r.to >= MAX_LOCATIONS ||
                std::isfinite(r.km) == false ||
                r.km <= 0 || r.km >= INF) {
                continue;
            }

            roads.push_back(r);

            if (r.from > maxID) {
                maxID = r.from;
            }

            if (r.to > maxID) {
                maxID = r.to;
            }

        } catch (...) {
            continue;
        }
    }

    if (roads.empty()) {
        cerr << "No valid roads found\n";
        return 1;
    }

    if (source < 0 || destination < 0 ||
        source >= MAX_LOCATIONS ||
        destination >= MAX_LOCATIONS ||
        source > maxID || destination > maxID) {
        cerr << "Invalid source or destination\n";
        return 1;
    }

    // Use the full ID range supported by our graph.
    Graph graph;
    initializeGraph(&graph, maxID + 1);

    for (size_t i = 0; i < roads.size(); i = i + 1) {
        const Road& r = roads[i];

        // Keep shortest distance for duplicate roads
        if (r.km < graph.adjacencyMatrix[r.from][r.to]) {
            addRoad(&graph, r.from, r.to, r.km);
        }
    }

    RoutingSystem routing(&graph);

    // Calculate original route first
    RouteResult original =
        routing.getShortestRoute(source, destination);

    // Check that every blocked road exists before removing any of them
    for (size_t i = 0; i < blockedRoads.size(); i = i + 1) {
        int blockedA = blockedRoads[i].first;
        int blockedB = blockedRoads[i].second;

        if (blockedA < 0 || blockedB < 0 ||
            blockedA > maxID || blockedB > maxID ||
            blockedA == blockedB ||
            graph.adjacencyMatrix[blockedA][blockedB] == INF) {
            cerr << "Selected road does not exist\n";
            return 1;
        }
    }

    // Simulate the road closures
    for (size_t i = 0; i < blockedRoads.size(); i = i + 1) {
        int blockedA = blockedRoads[i].first;
        int blockedB = blockedRoads[i].second;

        // Skip a road that was already removed (duplicate pair)
        if (graph.adjacencyMatrix[blockedA][blockedB] != INF) {
            removeRoad(&graph, blockedA, blockedB);
        }
    }

    RouteResult result =
        routing.getShortestRoute(source, destination);

    // Simple machine-readable output:
    // found distance originalDistance pathLength pathIDs...
    cout << (result.found ? 1 : 0) << " ";

    if (result.found) {
        cout << result.distance << " ";
    } else {
        cout << -1 << " ";
    }

    cout << (original.found ? original.distance : -1)
         << " ";

    cout << result.path.size();

    for (int id : result.path) {
        cout << " " << id;
    }

    cout << "\n";
    return 0;
}