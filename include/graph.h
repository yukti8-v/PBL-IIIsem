#ifndef GRAPH_H
#define GRAPH_H

#include "common.h"

typedef struct {
    int numberOfLocations;

    double adjacencyMatrix
        [MAX_LOCATIONS][MAX_LOCATIONS];

} Graph;

void initializeGraph(
    Graph *graph,
    int numberOfLocations
);

void addRoad(
    Graph *graph,
    int source,
    int destination,
    double distance
);

void removeRoad(
    Graph *graph,
    int source,
    int destination
);

void displayGraph(
    const Graph *graph
);

#endif