
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include "RoutingSystem.h"

using namespace std;

const double EARTH_RADIUS = 6371.0;
const double PI = 3.14159265358979323846;

double toRadians(double degrees) {
    return degrees * PI / 180.0;
}

double haversine(AStarCoordinate a, AStarCoordinate b) {
    double lat1 = toRadians(a.lat);
    double lat2 = toRadians(b.lat);
    double dLat = toRadians(b.lat - a.lat);
    double dLng = toRadians(b.lng - a.lng);

    double value =
        pow(sin(dLat / 2), 2) +
        cos(lat1) * cos(lat2) *
        pow(sin(dLng / 2), 2);

    value = max(0.0, min(1.0, value));

    return 2 * EARTH_RADIUS * asin(sqrt(value));
}

int main() {
    Graph graph;
    Road_unused:
    initializeGraph(&graph, 20);

    // Load roads.
    ifstream roadsFile("data/roads.csv");
    if (!roadsFile) {
        cerr << "Cannot open roads.csv\n";
        return 1;
    }

    string line;
    getline(roadsFile, line);

    while (getline(roadsFile, line)) {
        stringstream ss(line);
        string a, b, d;

        getline(ss, a, ',');
        getline(ss, b, ',');
        getline(ss, d, ',');

        if (!a.empty() && !b.empty() && !d.empty()) {
            addRoad(&graph, stoi(a), stoi(b), stod(d));
        }
    }

    // Load coordinates.
    AStarCoordinate coordinates[MAX_LOCATIONS] = {};

    ifstream coordFile("data/coordinates.csv");
    if (!coordFile) {
        cerr << "Cannot open coordinates.csv\n";
        return 1;
    }

    getline(coordFile, line);

    while (getline(coordFile, line)) {
        stringstream ss(line);
        string id, lat, lng;

        getline(ss, id, ',');
        getline(ss, lat, ',');
        getline(ss, lng, ',');

        if (!id.empty() && !lat.empty() && !lng.empty()) {
            int index = stoi(id);
            coordinates[index].lat = stod(lat);
            coordinates[index].lng = stod(lng);
            coordinates[index].valid = 1;
        }
    }

    // Calculate the same safe heuristic scale as astar.c.
    double scale = 1.0;

    for (int i = 0; i < 20; i++) {
        for (int j = 0; j < 20; j++) {
            double edge = graph.adjacencyMatrix[i][j];

            if (i != j && edge != INF) {
                double straight =
                    haversine(coordinates[i], coordinates[j]);

                if (straight > 0.000001) {
                    scale = min(scale, edge / straight);
                }
            }
        }
    }

    scale *= 0.99;
    scale = max(0.0, min(1.0, scale));

    RoutingSystem routing(&graph);

    int routeTests = 0;
    int heuristicTests = 0;
    bool passed = true;

    for (int destination = 0; destination < 20; destination++) {
        for (int source = 0; source < 20; source++) {

            RouteResult dijkstra =
                routing.getShortestRoute(source, destination);

            RouteResult astar =
                routing.getAStarRoute(
                    source, destination, coordinates
                );

            routeTests++;

            if (dijkstra.found != astar.found ||
                (dijkstra.found &&
                 fabs(dijkstra.distance - astar.distance) > 0.000001)) {

                cout << "ROUTE MISMATCH: "
                     << source << " -> " << destination << "\n";

                passed = false;
            }

            // Check heuristic admissibility:
            // h(source) must never exceed the true shortest distance.
            if (coordinates[source].valid &&
                coordinates[destination].valid) {

                double h = scale *
                    haversine(coordinates[source],
                              coordinates[destination]);

                double actual = dijkstra.found
                    ? dijkstra.distance
                    : INF;

                heuristicTests++;

                if (h > actual + 0.000001) {
                    cout << "HEURISTIC VIOLATION: "
                         << source << " -> " << destination
                         << " h=" << h
                         << " actual=" << actual << "\n";

                    passed = false;
                }
            }
        }
    }

    cout << "\nRoute tests: " << routeTests << "\n";
    cout << "Heuristic tests: " << heuristicTests << "\n";

    if (passed) {
        cout << "ALL TESTS PASSED\n";
        return 0;
    }

    cout << "SOME TESTS FAILED\n";
    return 1;
}
