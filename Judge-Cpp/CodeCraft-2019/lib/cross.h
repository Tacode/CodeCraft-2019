#ifndef CROSS_H
#define CROSS_H
#include <iostream>
#include <vector>
#include <math.h>

using namespace std;
class Cross
{
public:
    Cross();
    Cross(int id, int north, int east, int south, int west);
    Cross(int id, vector<int> &roadIDs);

public:
    int id_;

    vector<int> roadsID_;
    vector<int> connCrossesID_;
    vector<int> outCrossesID_;
    vector<int> outRoadsID_;
    vector<int> inRoadsID_;

    int outArea_;
    int inArea_;
    int outCarNum_;
    int inCarNum_;
    int outDensity_;
    int inDensity_;
};

#endif // CROSS_H
