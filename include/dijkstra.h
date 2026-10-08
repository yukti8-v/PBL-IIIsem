#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include "graph.h"

double shortestDistance(
    const Graph *graph,
    int source,
    int destination
);

int shortestPath(
    const Graph *graph,
    int source,
    int destination,
    int path[],
    int *pathLength,
    double *totalDistance
);

#endif