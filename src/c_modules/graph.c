#include <stdio.h>
#include "graph.h"

void initializeGraph(
    Graph *graph,
    int numberOfLocations
)
{
    if (graph == NULL ||
        numberOfLocations <= 0 ||
        numberOfLocations > MAX_LOCATIONS)
        return;

    graph->numberOfLocations =
        numberOfLocations;

    for (int i = 0; i < numberOfLocations; i++)
    {
        for (int j = 0; j < numberOfLocations; j++)
        {
            if (i == j)
                graph->adjacencyMatrix[i][j] = 0;
            else
                graph->adjacencyMatrix[i][j] = INF;
        }
    }
}
void addRoad(
    Graph *graph,
    int source,
    int destination,
    double distance
)
{
    if (graph == NULL ||
        source < 0 || destination < 0 ||
        source >= graph->numberOfLocations ||
        destination >= graph->numberOfLocations ||
        distance < 0)
        return;

    graph->adjacencyMatrix
        [source][destination] = distance;

    graph->adjacencyMatrix
        [destination][source] = distance;
}
void removeRoad(
    Graph *graph,
    int source,
    int destination)
{
    if (graph == NULL)
        return;

    if (source < 0 ||
        destination < 0 ||
        source >= graph->numberOfLocations ||
        destination >= graph->numberOfLocations)
        return;

    graph->adjacencyMatrix[source][destination] = INF;
    graph->adjacencyMatrix[destination][source] = INF;
}


void displayGraph(const Graph *graph)
{
    if (graph == NULL)
        return;

    printf("\nAdjacency Matrix:\n\n");

    for (int i = 0; i < graph->numberOfLocations; i++)
    {
        for (int j = 0; j < graph->numberOfLocations; j++)
        {
            if (graph->adjacencyMatrix[i][j] == INF)
                printf("INF\t");
            else
                printf("%.1lf\t",
                       graph->adjacencyMatrix[i][j]);
        }

        printf("\n");
    }
}