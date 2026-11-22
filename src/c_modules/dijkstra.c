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



/* Dijkstra algorithm with node visit statistics */
int shortestPathWithStats(
    const Graph *graph,
    int source,
    int destination,
    int path[],
    int *pathLength,
    double *totalDistance,
    int *nodesVisited)
{
    if (nodesVisited != NULL)
        *nodesVisited = 0;

    if (graph == NULL ||
        path == NULL ||
        pathLength == NULL ||
        totalDistance == NULL ||
        nodesVisited == NULL)
    {
        return RESULT_INVALID_INPUT;
    }

    int n = graph->numberOfLocations;

    if (n <= 0 || n > MAX_LOCATIONS ||
        source < 0 || destination < 0 ||
        source >= n || destination >= n)
    {
        return RESULT_INVALID_INPUT;
    }

    double distance[MAX_LOCATIONS];
    int visited[MAX_LOCATIONS];
    int parent[MAX_LOCATIONS];

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
        int current = findMinimumDistance(
            distance,
            visited,
            n
        );

        if (current == -1)
            break;

        visited[current] = 1;

        /* Count each settled node exactly once */
        (*nodesVisited)++;

        if (current == destination)
            break;

        for (int neighbour = 0;
             neighbour < n;
             neighbour++)
        {
            double edge =
                graph->adjacencyMatrix[current][neighbour];

            if (!visited[neighbour] &&
                edge != INF &&
                distance[current] != INF)
            {
                double newDistance =
                    distance[current] + edge;

                if (newDistance < distance[neighbour])
                {
                    distance[neighbour] = newDistance;
                    parent[neighbour] = current;
                }
            }
        }
    }

    if (distance[destination] == INF)
        return RESULT_ROUTE_NOT_FOUND;

    *totalDistance = distance[destination];

    int reversePath[MAX_LOCATIONS];
    int count = 0;
    int current = destination;

    while (current != -1 && count < MAX_LOCATIONS)
    {
        reversePath[count++] = current;
        current = parent[current];
    }

    *pathLength = count;

    for (int i = 0; i < count; i++)
    {
        path[i] = reversePath[count - i - 1];
    }

    return RESULT_SUCCESS;
}

/* Original function retained for compatibility */
int shortestPath(
    const Graph *graph,
    int source,
    int destination,
    int path[],
    int *pathLength,
    double *totalDistance)
{
    int nodesVisited = 0;

    return shortestPathWithStats(
        graph,
        source,
        destination,
        path,
        pathLength,
        totalDistance,
        &nodesVisited
    );
}
