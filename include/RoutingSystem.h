#ifndef ROUTING_SYSTEM_H
#define ROUTING_SYSTEM_H

#include <vector>

/*
    graph.h and dijkstra.h are C files.

    extern "C" allows our C++ RoutingSystem
    to use functions written in C.
*/
extern "C"
{
    #include "graph.h"
    #include "dijkstra.h"
}


/*
    Stores the result of a routing request.

    Example:
    distance = 7.0
    path = [0, 1, 3]
    found = true
*/
struct RouteResult
{
    double distance;
    std::vector<int> path;
    bool found;
};


/*
    RoutingSystem provides an easy C++ interface
    for using our Graph and Dijkstra modules.
*/
class RoutingSystem
{
private:

    /*
        Pointer to the city road graph.
    */
    const Graph *graph;


public:

    /*
        Constructor

        Connects this RoutingSystem object
        with the city graph.
    */
    RoutingSystem(const Graph *cityGraph);


    /*
        Find shortest route from source
        location to destination location.

        Returns:
        - distance
        - path
        - whether route was found
    */
    RouteResult getShortestRoute(
        int source,
        int destination
    );


    /*
        Calculate distances from multiple
        source locations to one destination.

        Example:

        Ambulance locations:
        0, 2, 4

        Emergency location:
        7

        This function calculates:

        0 -> 7
        2 -> 7
        4 -> 7
    */
    std::vector<double> calculateDistances(
        const std::vector<int>& sourceLocations,
        int destination
    );


    /*
        Find which source location is
        nearest to the destination.

        IMPORTANT:
        It returns the INDEX in the vector.

        Returns -1 if no route exists.
    */
    int findNearestSource(
        const std::vector<int>& sourceLocations,
        int destination
    );
};

#endif