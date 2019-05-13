#include <iostream>
#include "lib/road.h"
using namespace  std;
Road::Road()
{

}

Road::Road(int id, int length, int speed, int numChannel, int src, int dst, bool isDuplex)
{
    this->id = id;
    this->length = (double)length;
    this->speed = (double)speed;
    this->numChannel = numChannel;
    this->src = src;
    this->dst = dst;

    this->isDuplex = isDuplex;
    this->forwardDensity = 0.0;
    this->numForwardCar = 0;
    this->numTotalCar = 0;
    this->reverseForbidden = false;

    this->baseWeight = (double) this->length / (this->numChannel);
    this->maxCarNum = (int) this->length * this->numChannel * 0.6;
    this->area = (double) this->length * this->numChannel;
    if (this->isDuplex)
    {
        this->numBackwardCar = 0;
        this->backwardDensity = 0.0;
    }

}


void Road::update_slice(set<int> &waittingRouterCars)
{

    vector<int> delKeys;
    for (auto &kv:this->forwardCarTime)
    {
        this->forwardCarTime[kv.first] -= 1.0;
        if (this->forwardCarTime[kv.first] < 0)
            delKeys.push_back(kv.first);
        else if(this->forwardCarTime[kv.first] < 1.0)
            waittingRouterCars.insert(kv.first);
    }
    for (int i=0; i< (int)delKeys.size(); i++)
        this->forwardCarTime.erase(delKeys[i]);

    if (!this->isDuplex)
        return;

    delKeys.clear();
    for (auto &kv:this->backwardCarTime)
    {
        this->backwardCarTime[kv.first] -= 1;
        if (this->backwardCarTime[kv.first] < 0)
            delKeys.push_back(kv.first);
        else if(this->backwardCarTime[kv.first] < 1)
            waittingRouterCars.insert(kv.first);
    }
    for (int i=0; i<(int)delKeys.size(); i++)
        this->backwardCarTime.erase(delKeys[i]);

    return;
}

double Road::forward_weight()
{
    double carWeight = 0.0;
    if (this->reverseForbidden)
        return MAX_WEIGHT;
    int carNum = this->forwardCarTime.size();
    double carDensity = (double)carNum / (this->area);
    this->forwardDensity = carDensity;

    if (carNum > this->maxCarNum)
        return 100.0 + carDensity * 1000.0;

    // double beta = 1.0;
    // if (carDensity <= 0.2)
    //     beta = 2;
    // else if(carDensity <= 0.3)
    //     beta = 5;
    // else if(carDensity <= 0.4)
    //     beta = 8;
    // else if(carDensity <= 0.5)
    //     beta = 10;
    // else
    //     beta = 15;
    carWeight = carDensity * 1;

    return carWeight;
}

double Road::backward_weight()
{
    double carWeight = 0.0;
    if (this->reverseForbidden || !this->isDuplex)
        return MAX_WEIGHT;
    int carNum = this->backwardCarTime.size();
    double carDensity  = (double)carNum / (this->area);
    this->backwardDensity = carDensity;

    if (carNum > this->maxCarNum)
        return 100.0 + carDensity*1000.0;

    // double beta = 1.0;
    // if (carDensity <= 0.2)
    //     beta = 2;
    // else if(carDensity <= 0.3)
    //     beta = 5;
    // else if(carDensity <= 0.4)
    //     beta = 8;
    // else if(carDensity <= 0.5)
    //     beta = 10;
    // else
    //     beta = 15;
    carWeight = carDensity * 1;
    return carWeight;
}
