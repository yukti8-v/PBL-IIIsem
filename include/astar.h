#ifndef ASTAR_H
#define ASTAR_H

#include "graph.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    double lat;
    double lng;
    int valid;
} AStarCoordinate;

/*
 * Finds the shortest path using A*.
 *
 * coordinates: array indexed by location ID
 * nodesVisited: number of nodes expanded by A*
 */
int astarShortestPath(
    const Graph *graph,
    const AStarCoordinate coordinates[],
    int source,
    int destination,
    int path[],
    int *pathLength,
    double *totalDistance,
    int *nodesVisited
);

#ifdef __cplusplus
}
#endif

#endif