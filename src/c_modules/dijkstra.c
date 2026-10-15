#include "dijkstra.h"
#include <stddef.h>

/* Finds the unvisited location having minimum distance */
static int findMinimumDistance(
    double distance[],
    int visited[],
    int n)
{
    double minimum = INF;
    int minimumIndex = -1;

    for (int i = 0; i < n; i++)
    {
        if (!visited[i] && distance[i] < minimum)
        {
            minimum = distance[i];
            minimumIndex = i;
        }
    }

    return minimumIndex;
}


/* Returns only the shortest distance */
double shortestDistance(
    const Graph *graph,
    int source,
    int destination)
{
    int path[MAX_LOCATIONS];
    int pathLength = 0;
    double totalDistance = 0;

    int result = shortestPath(
        graph,
        source,
        destination,
        path,
        &pathLength,
        &totalDistance
    );

    if (result != RESULT_SUCCESS)
        return -1;

    return totalDistance;
}


/* Dijkstra algorithm + route reconstruction */
int shortestPath(
    const Graph *graph,
    int source,
    int destination,
    int path[],
    int *pathLength,
    double *totalDistance)
{
    if (graph == NULL ||
        path == NULL ||
        pathLength == NULL ||
        totalDistance == NULL)
    {
        return RESULT_INVALID_INPUT;
    }

    int n = graph->numberOfLocations;

    if (source < 0 ||
        destination < 0 ||
        source >= n ||
        destination >= n)
    {
        return RESULT_INVALID_INPUT;
    }

    double distance[MAX_LOCATIONS];
    int visited[MAX_LOCATIONS];
    int parent[MAX_LOCATIONS];

    /* Initialisation */
    for (int i = 0; i < n; i++)
    {
        distance[i] = INF;
        visited[i] = 0;
        parent[i] = -1;
    }

    distance[source] = 0.0;


    /* Main Dijkstra loop */
    for (int count = 0; count < n; count++)
    {
        int current =
            findMinimumDistance(
                distance,
                visited,
                n
            );

        if (current == -1)
            break;

        visited[current] = 1;

        if (current == destination)
            break;


        /* Check all neighbouring locations */
        for (int neighbour = 0;
             neighbour < n;
             neighbour++)
        {
            double edge =
                graph->adjacencyMatrix
                [current][neighbour];

            if (!visited[neighbour] &&
                edge != INF &&
                distance[current] != INF)
            {
                /*
                 * RELAXATION STEP
                 */
                double newDistance =
                    distance[current] + edge;

                if (newDistance < distance[neighbour])
                {
                    distance[neighbour] =
                        newDistance;

                    parent[neighbour] =
                        current;
                }
            }
        }
    }


    /* Destination cannot be reached */
    if (distance[destination] == INF)
    {
        return RESULT_ROUTE_NOT_FOUND;
    }

    *totalDistance = distance[destination];


    /* Reconstruct shortest route */
    int reversePath[MAX_LOCATIONS];
    int count = 0;

    int current = destination;

    while (current != -1)
    {
        reversePath[count++] = current;
        current = parent[current];
    }


    /* Reverse route */
    *pathLength = count;

    for (int i = 0; i < count; i++)
    {
        path[i] =
            reversePath[count - i - 1];
    }

    return RESULT_SUCCESS;
}