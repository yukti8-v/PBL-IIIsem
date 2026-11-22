
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <utility>
#include <cmath>
#include <chrono>
#include <iomanip>

#include "RoutingSystem.h"

using namespace std;

struct Road {
    int from;
    int to;
    double km;
};

int main(int argc, char* argv[]) {

    if (argc < 3) {
        cerr << "Usage: routing_cli.exe source destination "
             << "[blockedA blockedB] [--algo dijkstra|astar] [--stats]\n";
        return 1;
    }

    int source = 0;
    int destination = 0;

    bool showStats = false;
    string algorithm = "dijkstra";

    vector<pair<int, int> > blockedRoads;

    try {
        source = stoi(argv[1]);
        destination = stoi(argv[2]);

        int i = 3;

        while (i < argc) {

            string argument = argv[i];

            if (argument == "--stats") {
                showStats = true;
                i++;
            }
            else if (argument == "--algo") {

                if (i + 1 >= argc) {
                    cerr << "Missing algorithm name\n";
                    return 1;
                }

                algorithm = argv[i + 1];

                if (algorithm != "dijkstra" &&
                    algorithm != "astar") {
                    cerr << "Algorithm must be dijkstra or astar\n";
                    return 1;
                }

                i += 2;
            }
            else {
                if (i + 1 >= argc) {
                    cerr << "Blocked roads must be given as pairs\n";
                    return 1;
                }

                int blockedA = stoi(argv[i]);
                int blockedB = stoi(argv[i + 1]);

                blockedRoads.push_back(
                    make_pair(blockedA, blockedB)
                );

                i += 2;
            }
        }
    }
    catch (...) {
        cerr << "Invalid command-line arguments\n";
        return 1;
    }

    // Load road data.
    ifstream file("data/roads.csv");

    if (!file.is_open()) {
        cerr << "Cannot open data/roads.csv\n";
        return 1;
    }

    vector<Road> roads;
    string line;

    getline(file, line);

    int maxID = -1;

    while (getline(file, line)) {

        if (line.empty()) {
            continue;
        }

        stringstream ss(line);

        string a, b, d;

        getline(ss, a, ',');
        getline(ss, b, ',');
        getline(ss, d, ',');

        try {
            Road r = {stoi(a), stoi(b), stod(d)};

            if (r.from < 0 || r.to < 0 ||
                r.from >= MAX_LOCATIONS ||
                r.to >= MAX_LOCATIONS ||
                !isfinite(r.km) ||
                r.km <= 0 || r.km >= INF) {
                continue;
            }

            roads.push_back(r);

            if (r.from > maxID) maxID = r.from;
            if (r.to > maxID) maxID = r.to;
        }
        catch (...) {
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

    // Create graph.
    Graph graph;

    initializeGraph(&graph, maxID + 1);

    for (size_t i = 0; i < roads.size(); i++) {

        const Road& r = roads[i];

        if (r.km < graph.adjacencyMatrix[r.from][r.to]) {
            addRoad(&graph, r.from, r.to, r.km);
        }
    }

    // Load coordinates for A*.
    AStarCoordinate coordinates[MAX_LOCATIONS] = {};

    ifstream coordinateFile("data/coordinates.csv");

    if (coordinateFile.is_open()) {

        getline(coordinateFile, line);

        while (getline(coordinateFile, line)) {

            if (line.empty()) continue;

            stringstream ss(line);

            string idText, latText, lngText;

            getline(ss, idText, ',');
            getline(ss, latText, ',');
            getline(ss, lngText, ',');

            try {
                int id = stoi(idText);
                double lat = stod(latText);
                double lng = stod(lngText);

                if (id >= 0 && id <= maxID &&
                    id < MAX_LOCATIONS &&
                    isfinite(lat) && isfinite(lng) &&
                    lat >= -90 && lat <= 90 &&
                    lng >= -180 && lng <= 180) {

                    coordinates[id].lat = lat;
                    coordinates[id].lng = lng;
                    coordinates[id].valid = 1;
                }
            }
            catch (...) {
                continue;
            }
        }
    }

    // Calculate original route before road closures.
    RoutingSystem routing(&graph);

    RouteResult original =
        routing.getShortestRoute(source, destination);

    // Validate all blocked roads.
    for (size_t i = 0; i < blockedRoads.size(); i++) {

        int a = blockedRoads[i].first;
        int b = blockedRoads[i].second;

        if (a < 0 || b < 0 ||
            a > maxID || b > maxID ||
            a == b ||
            graph.adjacencyMatrix[a][b] == INF) {

            cerr << "Selected road does not exist\n";
            return 1;
        }
    }

    // Remove blocked roads.
    for (size_t i = 0; i < blockedRoads.size(); i++) {

        int a = blockedRoads[i].first;
        int b = blockedRoads[i].second;

        if (graph.adjacencyMatrix[a][b] != INF) {
            removeRoad(&graph, a, b);
        }
    }

    // Run the selected algorithm and measure its execution time.
    int nodesVisited = 0;

    auto startTime = chrono::high_resolution_clock::now();

    RouteResult result;

    if (algorithm == "astar") {

        result = routing.getAStarRoute(
            source,
            destination,
            coordinates,
            &nodesVisited
        );
    }
    else {

        result = routing.getShortestRoute(
    source,
    destination,
    &nodesVisited
);

        // Count visited nodes separately for Dijkstra.
        // This will be implemented in the next step.
    }

    auto endTime = chrono::high_resolution_clock::now();

    double elapsedMicroseconds =
        chrono::duration<double, micro>(
            endTime - startTime
        ).count();

    // Preserve the original machine-readable first line.
    cout << (result.found ? 1 : 0) << " ";

    cout << (result.found ? result.distance : -1) << " ";

    cout << (original.found ? original.distance : -1) << " ";

    cout << result.path.size();

    for (int id : result.path) {
        cout << " " << id;
    }

    cout << "\n";

    // Optional statistics on a separate second line.
    if (showStats) {

        cout << "algorithm=" << algorithm
             << " nodes_visited=" << nodesVisited
             << " time_us=" << fixed << setprecision(2)
             << elapsedMicroseconds << "\n";
    }

    return 0;
}
