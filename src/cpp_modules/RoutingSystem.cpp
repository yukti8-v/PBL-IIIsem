#include "RoutingSystem.h"


/*
    Constructor

    Stores the address of the city graph
    inside the RoutingSystem object.
*/
RoutingSystem::RoutingSystem(
    const Graph *cityGraph
)
{
    graph = cityGraph;
}


/*
    Finds shortest route between
    source and destination.
*/
RouteResult RoutingSystem::getShortestRoute(
    int source,
    int destination
)
{
    RouteResult result;

    /*
        Default values.

        These values mean that a route
        has not been found yet.
    */
    result.distance = -1;
    result.found = false;
    result.path.clear();


    /*
        If graph does not exist,
        immediately return failure result.
    */
    if (graph == nullptr)
    {
        return result;
    }


    /*
        Temporary array used by our
        C Dijkstra function.
    */
    int path[MAX_LOCATIONS];

    int pathLength = 0;

    double totalDistance = 0;


    /*
        Call shortestPath() from dijkstra.c
    */
    int status = shortestPath(
        graph,
        source,
        destination,
        path,
        &pathLength,
        &totalDistance
    );


    /*
        Dijkstra could not find a route.
    */
    if (status != RESULT_SUCCESS)
    {
        return result;
    }


    /*
        Route successfully found.
    */
    result.found = true;

    result.distance = totalDistance;


    /*
        Convert the C array path[]
        into a C++ vector.
    */
    for (int i = 0; i < pathLength; i++)
    {
        result.path.push_back(path[i]);
    }


    return result;
}


/*
    Calculates shortest distances from
    multiple source locations to one
    destination.

    This is useful when several ambulances
    need to be compared.
*/
std::vector<double>
RoutingSystem::calculateDistances(
    const std::vector<int>& sourceLocations,
    int destination
)
{
    std::vector<double> distances;


    /*
        Safety check.
    */
    if (graph == nullptr)
    {
        return distances;
    }


    /*
        Run shortestDistance() for every
        source location.
    */
    for (int source : sourceLocations)
    {
        double distance = shortestDistance(
            graph,
            source,
            destination
        );


        /*
            Add calculated distance
            to our vector.
        */
        distances.push_back(distance);
    }


    return distances;
}


/*
    Finds the nearest source location
    to the destination.
*/
int RoutingSystem::findNearestSource(
    const std::vector<int>& sourceLocations,
    int destination
)
{
    /*
        Cannot calculate anything if:

        1. graph does not exist
        2. source list is empty
    */
    if (graph == nullptr ||
        sourceLocations.empty())
    {
        return -1;
    }


    /*
        Calculate all distances.
    */
    std::vector<double> distances =
        calculateDistances(
            sourceLocations,
            destination
        );


    int nearestIndex = -1;

    double minimumDistance = INF;


    /*
        Compare all calculated distances.
    */
    for (int i = 0;
         i < static_cast<int>(distances.size());
         i++)
    {
        /*
            distance >= 0 means that
            a valid route exists.

            Then check whether this distance
            is smaller than the current
            minimum distance.
        */
        if (distances[i] >= 0 &&
            distances[i] < minimumDistance)
        {
            minimumDistance = distances[i];

            nearestIndex = i;
        }
    }


    return nearestIndex;
}