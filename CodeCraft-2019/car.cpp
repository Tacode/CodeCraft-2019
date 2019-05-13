#include "lib/car.h"

Car::Car()
{

}

Car::Car(int id, int src, int dst, int speed, int planTime, bool priority, bool preset)
{
    this->id = id;
    this->src = src;
    this->dst = dst;
    this->speed = (double)speed;
    this->planTime = planTime;
    this->priority = priority;
    this->preset = preset;

    startTime = 0;
    costTime = 0;
    runSpeed = 10;
    availiableSpeed = 0;

    runningRoadID = -1;
    status = -1;
    curCross = -1;
    lengthToCross = -1;
}

Car::Car(const Car &C)
{
    this->id = C.id;
    this->src = C.src;
    this->dst = C.dst;
    this->speed = C.speed;
    this->planTime = C.planTime;
    this->startTime = C.startTime;
    this->status = C.status;
    this->runSpeed = C.runSpeed;
    this->availiableSpeed = C.availiableSpeed;
    this->runningRoadID = C.runningRoadID;
    this->histroyRoadsID = C.histroyRoadsID;
    this->isForwad = C.isForwad;
}
