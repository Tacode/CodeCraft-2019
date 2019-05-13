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
#include "judge.h"

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
    bool manage_all();
    void manage_unlock();
    void route(bool isTail);
    void route_one_car(int carID, double, double, bool);
    void run_new_cars(vector<int> &newRunCars);
    void run_one_car(int carID);
    void analyse();
    void calc_dist();   // 计算每个点到其他点的最短长度
    void get_manage_preset_car();


public:
    map<int, Car*> carsMap;
    map<int, Road*> roadsMap;
    map<int, Cross*> crossesMap;
    map<int, Car*> presetMap;
    map<int, Car*> manCarsMap;                   // manage 调度车辆
    map<int, vector<int>> carsPlanTimeMap;
    map<int, vector<int>> manCarsPlanTimeMap;
    map<int, vector<int>> manNormPlanTimeMap;    // 需要自行调度的普通车辆计划出发时间
    map<int, vector<int>> manPrioPlanTimeMap;    // 需要进行调度的优先车辆计划出发时间
    map<int, vector<int>> presetStartTimeMap;   // 预置车辆实际出发时间

    set<int> runningCars;
    set<int> waitingRouterCars;
    map<int, int> loadState;

    int timeSlice;
    int runQueLength;

    //build graph
    set<int> vertexes;
    set<Edge> edges;
    map<int, set<Edge>> adjacents;

    map<int, map<int, double>> distMap;    // n*n的一个方阵，存储每个cross到其他cross的最短距离

    int roadSumArea_;
    double roadsAvgCost;
    int T;          // T 所有车辆调度时间
    int T_pri;      // 优先车辆调度时间 T_pri = prioEndTime - prioEarlyTime
    int prioEarlyTime;   // 优先车辆最早计划出发时间
    int prioEndTime = 0;    // 全部优先车辆结束时间
    int lastPresetStartTime = 0;
    int laneNum = 0; // 道路的数量，不是roadsMap里road的数量，双向路计2，单向路计1.
    int lastManPrioStartTime = 0;    // 需要调度的优先车辆最后上车时间
    int lastPresetSetoutNum = 0;     // 记录到最后一批预置车辆上路时，共有多少辆车已经上路
    int tailTime = 0;   //转换到tail的时间片
    double a;
    double alpha, beta;
    Judgement* judge;    // 判题器用于判断是否死锁
    set<int> managePresetCar;
    
    double stage1CarDensity;
    double stage2CarDensity;
    map<int, pair<double, double>> stageMap;

};

//bool is_contains(priority_queue<Tuple, vector<Tuple>, greater<Tuple>> distQueue, int dst);
//void change_queue(priority_queue<Tuple, vector<Tuple>, greater<Tuple>> &distQueue, Tuple);
#endif // Solver_H
