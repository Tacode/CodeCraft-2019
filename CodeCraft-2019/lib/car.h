#ifndef CAR_H
#define CAR_H
#include <iostream>
#include <vector>
#include <queue>
#include <math.h>
using namespace std;
class Car
{
public:
    Car();
    Car(int id, int src, int dst, int speed, int planTime, bool priority, bool preset);
    Car(const Car & C);

public:
    int id;
    int src;
    int dst;
    double speed;
    int planTime;
    bool priority;
    bool preset;

    int status;
    int startTime;
    int costTime;
    int direction;

    int runSpeed;
    int availiableSpeed; //
    int preRoadID;
    int nextRoadID;
    bool isForwad;
    double blockDistance;
    vector<int> histroyRoadsID;
    queue<int> roadsIDQueue;
    int index = 0; // road index
    int runningRoadID;
    int curCross;
    int lengthToCross;
    bool managePreset = false; // 标志在预置车辆中需要进行调度的10%
};

#endif // CAR_H
