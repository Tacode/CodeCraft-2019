#ifndef ROAD_H
#define ROAD_H
#include <map>
#include <set>
#include <vector>
#include <float.h>

#define MAX_WEIGHT 1000000.0    // 定义道路最大权重1百万，防止溢出，未用 DBL_MAX

using namespace std;
class Road
{
public:
    Road();
    Road(int id, int length, int speed, int numChannel, int src, int dst, bool isDuplex);
    void update_slice(set<int> &waittingRouterCars);  
    double forward_weight( );
    double backward_weight();

public:
    int id;
    double length;
    double speed;
    int numChannel;
    int src;
    int dst;
    bool isDuplex;

    int numForwardCar;
    int numBackwardCar;
    int numTotalCar;
    bool reverseForbidden;
    double baseWeight;
    int maxCarNum;
    double area;
    double forwardDensity = 0.0;
    double backwardDensity = 0.0;

    map<int,double> forwardCarTime;
    map<int,double> backwardCarTime;

};

#endif // ROAD_H
