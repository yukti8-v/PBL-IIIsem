#include "astar.h"

#include <math.h>
#include <stddef.h>

#define EARTH_RADIUS_KM 6371.0
#define PI 3.14159265358979323846

/* Convert degrees to radians */
static double toRadians(double degrees)
{
    return degrees * PI / 180.0;
}

/* Calculate straight-line distance between two coordinates */
static double haversine(
    const AStarCoordinate *a,
    const AStarCoordinate *b)
{
    double lat1 = toRadians(a->lat);
    double lat2 = toRadians(b->lat);

    double dLat = toRadians(b->lat - a->lat);
    double dLng = toRadians(b->lng - a->lng);

    double sinLat = sin(dLat / 2.0);
    double sinLng = sin(dLng / 2.0);

    double value =
        sinLat * sinLat +
        cos(lat1) * cos(lat2) * sinLng * sinLng;

    if (value > 1.0) value = 1.0;
    if (value < 0.0) value = 0.0;

    return 2.0 * EARTH_RADIUS_KM *
           asin(sqrt(value));
}

/*
 * Calculate a safe scale factor.
 *
 * For every road:
 * scale <= road distance / straight-line distance
 *
 * This prevents the scaled heuristic from exceeding
 * the actual cost of any individual edge.
 */
static double calculateHeuristicScale(
    const Graph *graph,
    const AStarCoordinate coordinates[])
{
    double scale = 1.0;
    int foundValidEdge = 0;

    for (int i = 0; i < graph->numberOfLocations; i++) {
        if (!coordinates[i].valid) continue;

        for (int j = 0; j < graph->numberOfLocations; j++) {
            double roadDistance =
                graph->adjacencyMatrix[i][j];

            if (i == j || roadDistance == INF ||
                !coordinates[j].valid) {
                continue;
            }

            double straightDistance =
                haversine(&coordinates[i], &coordinates[j]);

            if (straightDistance > 0.000001) {
                double ratio = roadDistance / straightDistance;

                if (ratio < scale) {
                    scale = ratio;
                }

                foundValidEdge = 1;
            }
        }
    }

    if (!foundValidEdge) return 0.0;

    /*
     * A small safety margin protects against
     * floating-point rounding.
     */
    scale *= 0.99;

    if (scale < 0.0) scale = 0.0;
    if (scale > 1.0) scale = 1.0;

    return scale;
}

/* Estimate distance from a node to the destination */
static double heuristic(
    int node,
    int destination,
    const AStarCoordinate coordinates[],
    double scale)
{
    if (scale == 0.0 ||
        !coordinates[node].valid ||
        !coordinates[destination].valid) {
        return 0.0;
    }

    return scale * haversine(
        &coordinates[node],
        &coordinates[destination]);
}

/* Find the unvisited node having the lowest f-score */
static int findLowestScore(
    const double fScore[],
    const int closed[],
    int n)
{
    double minimum = INF;
    int minimumIndex = -1;

    for (int i = 0; i < n; i++) {
        if (!closed[i] && fScore[i] < minimum) {
            minimum = fScore[i];
            minimumIndex = i;
        }
    }

    return minimumIndex;
}

int astarShortestPath(
    const Graph *graph,
    const AStarCoordinate coordinates[],
    int source,
    int destination,
    int path[],
    int *pathLength,
    double *totalDistance,
    int *nodesVisited)
{
    if (nodesVisited != NULL) {
        *nodesVisited = 0;
    }

    if (graph == NULL || path == NULL ||
        pathLength == NULL || totalDistance == NULL ||
        nodesVisited == NULL) {
        return RESULT_INVALID_INPUT;
    }

    int n = graph->numberOfLocations;

    if (n <= 0 || n > MAX_LOCATIONS ||
        source < 0 || destination < 0 ||
        source >= n || destination >= n) {
        return RESULT_INVALID_INPUT;
    }

    double gScore[MAX_LOCATIONS];
    double fScore[MAX_LOCATIONS];

    int parent[MAX_LOCATIONS];
    int closed[MAX_LOCATIONS];

    for (int i = 0; i < n; i++) {
        gScore[i] = INF;
        fScore[i] = INF;
        parent[i] = -1;
        closed[i] = 0;
    }

    double scale = 0.0;

    if (coordinates != NULL) {
        scale = calculateHeuristicScale(graph, coordinates);
    }

    gScore[source] = 0.0;

    fScore[source] = coordinates != NULL
        ? heuristic(source, destination, coordinates, scale)
        : 0.0;

    while (1) {
        int current = findLowestScore(fScore, closed, n);

        if (current == -1) break;

        closed[current] = 1;
        (*nodesVisited)++;

        if (current == destination) {
            break;
        }

        for (int neighbour = 0; neighbour < n; neighbour++) {
            double edge =
                graph->adjacencyMatrix[current][neighbour];

            if (edge == INF || edge < 0 ||
                closed[neighbour]) {
                continue;
            }

            double tentativeG =
                gScore[current] + edge;

            if (tentativeG < gScore[neighbour]) {
                parent[neighbour] = current;
                gScore[neighbour] = tentativeG;

                double h = coordinates != NULL
                    ? heuristic(neighbour, destination,
                                coordinates, scale)
                    : 0.0;

                fScore[neighbour] = tentativeG + h;
            }
        }
    }

    if (gScore[destination] == INF) {
        return RESULT_ROUTE_NOT_FOUND;
    }

    *totalDistance = gScore[destination];

    /* Reconstruct path backwards */
    int reversePath[MAX_LOCATIONS];
    int count = 0;
    int current = destination;

    while (current != -1 && count < MAX_LOCATIONS) {
        reversePath[count++] = current;
        current = parent[current];
    }

    *pathLength = count;

    for (int i = 0; i < count; i++) {
        path[i] = reversePath[count - i - 1];
    }

    return RESULT_SUCCESS;
}