#ifndef SOLUTION_H
#define SOLUTION_H
#include <iostream>
#include <map>
#include <set>
#include <vector>
#include <queue>
#include <functional>
#include <algorithm>
#include <numeric>
#include <assert.h>
#include <fstream>
#include <limits.h>
#include "road.h"
#include "car.h"
#include "cross.h"
#include "minheap.h"
using namespace std;


class Edge
{
public:
    Edge();
    Edge(int roadID, int src, int dst, double weight, bool isForwad);
    Edge(const Edge &E);
    bool operator <(const Edge & e) const {
        return this->roadID < e.roadID;
    }
public:
    int roadID;
    int src;
    int dst;
    double weight;
    bool isForward;
};


class Solver {
public:
    Solver();
    Solver(map<int,Car*> &, map<int,Road*> &, map<int,Cross*> &, map<int, Car*> &);
    vector<int> search_path(int startPoint, int target, double carSpeed, double alpha=1.0, double beta=1.0, bool isTail=false);
    void manage_once(vector<int> &newRunCars, bool isTail);
    void manage_all();
    void route(bool isTail);
    void route_one_car(int carID, double, double, bool);
    void run_new_cars(vector<int> &newRunCars);
    void run_one_car(int carID);
    void analyse();

public:
    map<int, Car*> carsMap;
    map<int, Road*> roadsMap;
    map<int, Cross*> crossesMap;
    map<int, Car*> presetMap;
    map<int, Car*> manageCarsMap;
    map<int, vector<int>> carsPlanTimeMap;
    map<int, vector<int>> managePlanTimeMap;    // 需要自行调度的车辆计划出发时间
    map<int, vector<int>> presetStartTimeMap;   // 预置车辆实际出发时间
    set<int> runningCars;
    set<int> waitingRouterCars;

    int timeSlice;
    int runQueLength;

    //build graph
    set<int> vertexes;
    set<Edge> edges;
    map<int, set<Edge>> adjacents;

    int roadSumArea_;
    double roadsAvgCost;
    int T;          // T 所有车辆调度时间
    int T_pri;      // 优先车辆调度时间 T_pri = prioEndTime - prioEarlyTime
    int prioEarlyTime;   // 优先车辆最早计划出发时间
    int prioEndTime = 0;    // 
    double a;


};

//bool is_contains(priority_queue<Tuple, vector<Tuple>, greater<Tuple>> distQueue, int dst);
//void change_queue(priority_queue<Tuple, vector<Tuple>, greater<Tuple>> &distQueue, Tuple);
#endif // Solver_H
