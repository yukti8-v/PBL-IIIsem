#ifndef COMMON_H
#define COMMON_H

#define MAX_NAME 100
#define MAX_TYPE 50
#define MAX_CAPABILITIES 10
#define MAX_FACILITIES 10

#define MAX_LOCATIONS 100
#define INF 999999


/* Common result/error codes used by all modules */

typedef enum
{
    RESULT_SUCCESS = 1,
    RESULT_FAILURE = 0,

    RESULT_NOT_FOUND = -1,
    RESULT_INVALID_INPUT = -2,

    RESULT_NO_AMBULANCE = -3,
    RESULT_NO_SUITABLE_AMBULANCE = -4,
    RESULT_NO_SUITABLE_HOSPITAL = -5,

    RESULT_ROUTE_NOT_FOUND = -6

} ResultCode;

#endif