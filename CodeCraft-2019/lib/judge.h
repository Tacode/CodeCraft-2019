#ifndef JUDGE_H
#define JUDGE_H
#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <assert.h>
#include <limits.h>
#include <math.h>
#include <iomanip>
#include "road.h"
#include "cross.h"
#include "car.h"
using namespace std;
typedef map<int, map<int, pair<vector<vector<int>>, int>>> RoadState;
class Judgement
{
  public:
    Judgement();
    Judgement(map<int, Car *> &, map<int, Road *> &, map<int, Cross *> &);
    void init();
    bool isFinished();
    bool judge(); // 判断是否死锁
    void driveJustCurrentRoad();
    void driveCarInitList(bool);//驱动全图上路
    bool runToRoad(int, int); 
    void driveCarOneDirect(int preCross, int curCross); //驱动单道路单方向
    bool runToOneDirectRoad(int carID, int curCross, int expectCross);
    void updateCarInfo(int new_s, int curCross, int nextCross, int carID);
    vector<int> carsNotToEnd();
    void carsAcrossRoad();
    void updateFollowingCars(int curCross, int preCross, int channelID);
    vector<int> getTurnInfo(int idx, int curCross, int curRoad);
    bool isConflict(int, bool, int, int);
    bool isMoveToEndState(int carID, int curCross, int preCross,int nextRoad, int channelID);
    pair<int, int> highestPriorCar(int, int);
    vector<int> turningOfCar(int, int, int);
    void printResult();

  public:
    map<int, Car *> carsMap;
    map<int, Road *> roadsMap;
    map<int, Cross *> crossesMap;
    map<int, map<int, vector<int>>>  garage;
    RoadState transport;
    int timeSlice;
    int totalTimeSlice;
    int priorityTimeSlice;
    int priorityFirstTime;
    int priorityLastTime;
    int finishCnt;
    int garageCnt;
    int runingCnt;

};

#endif // judge.h