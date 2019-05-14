#include "lib/judge.h"

Judgement::Judgement()
{

}

Judgement::Judgement(map<int, Car*> &carsMap, map<int, Road*> &roadsMap, map<int, Cross*> &crossesMap)
{
    this->carsMap = carsMap;
    this->roadsMap = roadsMap;
    this->crossesMap = crossesMap;
    
    this->timeSlice = INT_MAX;
    for (auto &item:this->carsMap)
    {
        Car *car = item.second;
        if (car->startTime < this->timeSlice)
            this->timeSlice = car->startTime;
    }
    this->timeSlice -= 1;
    this->totalTimeSlice = 0;
    this->priorityTimeSlice = 0;
    this->priorityFirstTime = INT_MAX;
    this->priorityLastTime = 0;
    this->finishCnt = 0;
    this->garageCnt = 0;
    this->runingCnt = 0;
    this->init();
    //this->driveCarInitList(false);
    // driveJustCurrentRoad();
}

void Judgement::init()
{
    //初始化garage,其中garage[0]存非优先车辆，garage[1]存优先车辆
    for (auto &item:this->carsMap)
    {
        Car *car = item.second;
        if (car->priority)
            this->garage[1][car->src].push_back(car->id);
        else
            this->garage[0][car->src].push_back(car->id);
    }
    // 处理非优先车辆
    for (auto &itemPerCross:this->garage[0])
    {
        int crossID = itemPerCross.first;
        vector<int> carVec = itemPerCross.second;
        vector<int> sortedCarVec;
        map<int, vector<int>> carOnTimeMap;
        sortedCarVec.clear();
        //按出发时间排序 获取字典
        for (int carID:carVec)
            carOnTimeMap[this->carsMap[carID]->startTime].push_back(carID);
        //给同一出发时间的车辆按ID排序，存入排序vector中
        for (auto &item:carOnTimeMap)
        {
            sort(item.second.begin(), item.second.end());
            for (auto carID:item.second)
                sortedCarVec.push_back(carID);
        }
        this->garage[0][crossID] = sortedCarVec;
    }

    // 处理优先车辆
    for (auto &itemPerCross:this->garage[1])
    {
        int crossID = itemPerCross.first;
        vector<int> carVec = itemPerCross.second;
        vector<int> sortedCarVec;
        map<int, vector<int>> carOnTimeMap;
        sortedCarVec.clear();
        //按出发时间排序 获取字典
        for (int carID:carVec)
            carOnTimeMap[this->carsMap[carID]->startTime].push_back(carID);
        //给同一出发时间的车辆按ID排序，存入排序vector中
        for (auto &item:carOnTimeMap)
        {
            sort(item.second.begin(), item.second.end());
            for (auto carID:item.second)
                sortedCarVec.push_back(carID);
        }
        this->garage[1][crossID] = sortedCarVec;
    }

    //初始化transport
    for (auto &item:this->roadsMap)
    {
        Road *road = item.second;
        int start = road->src;
        int end = road->dst;
        int numChannel = road->numChannel;
        vector<vector<int>> tmpVec;
        tmpVec.resize(numChannel);
        this->transport[end][start].second = road->id;
        this->transport[end][start].first = tmpVec;
        if (road->isDuplex)
        {
            this->transport[start][end].second = road->id;
            this->transport[start][end].first = tmpVec;
        }
    }
}

bool Judgement::isFinished()
{
    int cnt = 0;
    this->garageCnt = 0;
    this->runingCnt = 0;
    for (auto &item : this->garage[0])
    {
        this->garageCnt += item.second.size();
    }
    for (auto &item:this->garage[1])
    {
        this->garageCnt += item.second.size();
    }

    for (auto &item1:this->transport)
    {
        int cur_cross = item1.first;
        for (auto &item2:this->transport[cur_cross])
        {
            vector<vector<int>> channelState = item2.second.first;
            for (auto &v:channelState)
                this->runingCnt += v.size();
        }
    }
    cnt = this->garageCnt + this->runingCnt;
    if (cnt == 0)
        return true;
    else
        return false;
    
}

void Judgement::judge()
{
    while(true)
    {
        this->timeSlice += 1;
        for (auto &item:this->carsMap)
        {
            this->carsMap[item.first]->status = -1;
        }
        //车辆的标定
        this->driveJustCurrentRoad();
        this->driveCarInitList(true);//特权车上路
        
        while (true)
        {
            vector<int> waittingCars = this->carsNotToEnd();
            this->carsAcrossRoad();
            vector<int> newWaittingCars = this->carsNotToEnd();
            // cout << "wait: " << waittingCars.size() << "newWait: " << newWaittingCars.size() << endl;
            if (newWaittingCars.size() == 0)
                break;
            else if(newWaittingCars.size() == waittingCars.size())
            {
                cout << "-------------------------------------" << endl;
                for (auto lockedCar : newWaittingCars)
                {
                    printf("Car:%d, CurRoad: %d, CurCross: %d ", lockedCar, this->carsMap[lockedCar]->runningRoadID, this->carsMap[lockedCar]->curCross);
                    printf("Distance: %d, State: %d\n", this->carsMap[lockedCar]->lengthToCross, this->carsMap[lockedCar]->status);
                    
                }
                cout << "dead lock..." << endl;
                exit(-1);
            }
        }
        
        this->driveCarInitList(false);
        if (isFinished())
        {
            cout << this->timeSlice << " running: " <<this->runingCnt <<" garage: "<< this->garageCnt<< " finished: " << this->finishCnt << endl;
            break;
        }
        else
        {
            cout << this->timeSlice << " running: " <<this->runingCnt <<" garage: "<< this->garageCnt<< " finished: " << this->finishCnt << endl;
        }
    }
    this->printResult();
}

void Judgement::driveJustCurrentRoad()
{
    // 0：等待状态，不出路口，还没完成调度; 1:等待状态，会出路口，还没完成调度; 2:终止状态，完成了调度
    for (auto &item1 : this->transport)
    {
        int curCross = item1.first;
        for (auto &item2:this->transport[curCross] )
        {
            // int preCross = item2.first;
            int curRoadID = item2.second.second; //当前道路
            for (auto &channel:item2.second.first) //取各个车道
            {
                int carIndex = 0;
                for (auto &curCarID : channel)
                {
                    //车辆运行速度
                    assert(this->roadsMap[curRoadID]);
                    assert(this->carsMap[curCarID]);
                    int v1 = min(this->roadsMap[curRoadID]->speed, this->carsMap[curCarID]->speed);
                    //该车离路口的距离
                    int s1 = carsMap[curCarID]->lengthToCross;
                    if (s1 < v1) // 该车可能会通过路口
                    {
                        //如果该车为第一辆车或者前车不为终止车辆
                        if (carIndex == 0 || this->carsMap[channel[carIndex-1]]->status != 2)
                            carsMap[curCarID]->status = 1;//标记为等待状态
                        else
                        {
                            carsMap[curCarID]->lengthToCross = carsMap[channel[carIndex - 1]]->lengthToCross + 1;
                            carsMap[curCarID]->status = 2;
                        }

                    }
                    else //该车不会过路口
                    {
                        if(carIndex == 0 || (carsMap[curCarID]->lengthToCross - carsMap[channel[carIndex-1]]->lengthToCross-1) >= v1)
                        {
                            carsMap[curCarID]->lengthToCross = s1 - v1;
                            carsMap[curCarID]->status = 2;
                        }
                        else //前车有阻挡
                        {
                            // 如果前车是终止状态
                            if (carsMap[channel[carIndex-1]]->status == 2)
                            {
                                carsMap[curCarID]->lengthToCross = carsMap[channel[carIndex - 1]]->lengthToCross + 1;
                                carsMap[curCarID]->status = 2;
                            }
                            else
                            {
                                carsMap[curCarID]->status = 0;
                            }
                        }
                    }
                    
                    carIndex++;
                }
            }
        }
    }

}

void Judgement::driveCarInitList(bool priority)
{
    vector<int> delCars;
    delCars.clear();
    for (auto &item : this->transport) // transport为字典，键已经排序
    {
        int curCross = item.first;
        // cout << curCross << " in " << this->garage[1][curCross].size() <<endl;
        
        for (int carID : this->garage[1][curCross])
        {
            assert(this->carsMap[carID]);
            if (this->carsMap[carID]->startTime <= this->timeSlice)
            {
                // cout << "in2" << endl;
                if (this->runToRoad(carID, curCross))
                    delCars.push_back(carID);
                // this->garage[1][curCross].erase(find(this->garage[1][curCross].begin(), this->garage[1][curCross].begin(), carID));
                // cout << "in3" << endl;
            }
        }
        for (auto carID:delCars)
        {
            this->garage[1][curCross].erase(find(this->garage[1][curCross].begin(), this->garage[1][curCross].end(), carID));
        }
       
        delCars.clear();
        if (!priority)
        {
            for (int carID:this->garage[0][curCross])
            {
                assert(this->carsMap[carID]);
                if (this->carsMap[carID]->startTime <= this->timeSlice)
                {
                    if (this->runToRoad(carID, curCross))
                        delCars.push_back(carID);
                }
            }
            for(auto carID:delCars)
            {
                this->garage[0][curCross].erase(find(this->garage[0][curCross].begin(), this->garage[0][curCross].end(), carID));
            }
        }
        delCars.clear();
    }
}

bool Judgement::runToRoad(int carID, int curCross)
{
    int nextRoadID = this->carsMap[carID]->histroyRoadsID[0];
    int nextCross = -1;
    assert(this->roadsMap[nextRoadID]);
    if (this->roadsMap[nextRoadID]->src == curCross)
        nextCross = this->roadsMap[nextRoadID]->dst;
    else
        nextCross = this->roadsMap[nextRoadID]->src;
    if (nextCross == -1)
        throw "nextCross error ...";
    //遍历车道
    int avalibleSpeed = min(this->roadsMap[nextRoadID]->speed, this->carsMap[carID]->speed);
    for (int i = 0; i < this->roadsMap[nextRoadID]->numChannel; i++)
    {
        vector<int> carsInChannel;
        carsInChannel.clear();
        carsInChannel = this->transport[nextCross][curCross].first[i];
        //前面车道没车，或者有空位到达，直接驶过去
        if (carsInChannel.size() == 0 || \
            this->carsMap[carsInChannel.back()]->lengthToCross < (this->roadsMap[nextRoadID]->length - avalibleSpeed))
        {
            this->transport[nextCross][curCross].first[i].push_back(carID);
            int new_s = this->roadsMap[nextRoadID]->length - avalibleSpeed;
            updateCarInfo(new_s, curCross, nextCross, carID);
            return true;
        } // 无法直接驶入，但是依然有空位 
        else 
        {
            if(this->carsMap[carsInChannel.back()]->status == 2)
            {
                if(this->carsMap[carsInChannel.back()]->lengthToCross < (this->roadsMap[nextRoadID]->length-1)) // 前车为终止状态
                {
                    this->transport[nextCross][curCross].first[i].push_back(carID);
                    int new_s = this->carsMap[carsInChannel.back()]->lengthToCross + 1;
                    updateCarInfo(new_s, curCross, nextCross, carID);
                    return true;
                }
                else 
                {
                    if (i == this->roadsMap[nextRoadID]->numChannel-1) // 如果已经遍历了所有车道
                        return false;
                }
            }
            else
            {
                return false;
            }
        }
    }
    return false;
}

void Judgement::driveCarOneDirect(int preCross, int curCross)
{
    vector<int> delCars;
    for (int carID : this->garage[1][preCross])
    {
        assert(this->carsMap[carID]);
        if (this->carsMap[carID]->startTime <= this->timeSlice)
        {
            if (this->runToOneDirectRoad(carID, preCross, curCross))
                delCars.push_back(carID);
            // this->garage[1][curCross].erase(find(this->garage[1][curCross].begin(), this->garage[1][curCross].begin(), carID));
        }
    } 
    for (auto carID:delCars)
    {      
        this->garage[1][preCross].erase(find(this->garage[1][preCross].begin(), this->garage[1][preCross].end(), carID));
    }
    delCars.clear();
    
}

bool Judgement::runToOneDirectRoad(int carID, int curCross, int expectCross)
{
    int nextRoadID = this->carsMap[carID]->histroyRoadsID[0];
    int nextCross = -1;
    assert(this->roadsMap[nextRoadID]);
    if (this->roadsMap[nextRoadID]->src == curCross)
        nextCross = this->roadsMap[nextRoadID]->dst;
    else
        nextCross = this->roadsMap[nextRoadID]->src;
    if (nextCross == -1)
        throw "nextCross error ...";
    if (transport[expectCross][curCross].second != nextRoadID)
        return false;
    //遍历车道
    int avalibleSpeed = min(this->roadsMap[nextRoadID]->speed, this->carsMap[carID]->speed);
    for (int i = 0; i < this->roadsMap[nextRoadID]->numChannel; i++)
    {
        vector<int> carsInChannel;
        carsInChannel.clear();
        carsInChannel = this->transport[nextCross][curCross].first[i];
        //前面车道没车，或者有空位到达，直接驶过去
        if (carsInChannel.size() == 0 || \
            this->carsMap[carsInChannel.back()]->lengthToCross < (this->roadsMap[nextRoadID]->length - avalibleSpeed))
        {
            this->transport[nextCross][curCross].first[i].push_back(carID);
            int new_s = this->roadsMap[nextRoadID]->length - avalibleSpeed;
            updateCarInfo(new_s, curCross, nextCross, carID);
            return true;
        } // 无法直接驶入，但是依然有空位 
        else 
        {
            if(this->carsMap[carsInChannel.back()]->status == 2)
            {
                if(this->carsMap[carsInChannel.back()]->lengthToCross < (this->roadsMap[nextRoadID]->length-1)) // 前车为终止状态
                {
                    this->transport[nextCross][curCross].first[i].push_back(carID);
                    int new_s = this->carsMap[carsInChannel.back()]->lengthToCross + 1;
                    updateCarInfo(new_s, curCross, nextCross, carID);
                    return true;
                }
                else 
                {
                    if (i == this->roadsMap[nextRoadID]->numChannel-1) // 如果已经遍历了所有车道
                        return false;
                }
            }
            else
            {
                return false;
            }
        }

    }
}

void Judgement::updateCarInfo(int new_s, int curCross, int nextCross, int carID)
{
    if (this->carsMap[carID]->runningRoadID != this->transport[nextCross][curCross].second)
        this->carsMap[carID]->roadsIDQueue.pop();
    this->carsMap[carID]->curCross = nextCross;
    this->carsMap[carID]->runningRoadID = this->transport[nextCross][curCross].second;
    this->carsMap[carID]->lengthToCross = new_s;
    this->carsMap[carID]->status = 2; // 终止状态
}

vector<int> Judgement::carsNotToEnd()
{
    vector<int> waittingCars;
    for (auto &item1 : this->transport)
    {
        int curCross = item1.first;
        for (auto &item2:this->transport[curCross])
        {
            int preCross = item2.first;
            for (auto &channel:this->transport[curCross][preCross].first)
            {
                for (auto &carID:channel)
                {
                    if (this->carsMap[carID]->status != 2)
                        waittingCars.push_back(carID);
                }
            }
        }
    }
    return waittingCars;
}

void Judgement::carsAcrossRoad()
{
    for (auto &item1:this->transport)
    {
        int curCross = item1.first;
        map<int, int> tmp; //存放{道路ID，先前路口ID}
        for (auto &item2 : this->transport[curCross])
        {
            tmp[item2.second.second] = item2.first;
        }
        for (auto &tmpItem : tmp)
        {
            int curRoad = tmpItem.first;
            int preCross = tmpItem.second;
            bool flag = true;
            while (flag)
            {
                pair<int, int> priorityCarState(-1, -1);
                priorityCarState = this->highestPriorCar(curCross, preCross);
                int channelID = priorityCarState.first;
                int position = priorityCarState.second;
                // cout << "curRoad" << curRoad << endl;
                if (channelID == -1 && position == -1)
                {
                    // cout << "channel is -1 " << curRoad << endl;
                    flag = false;
                    break;
                }
                assert(position == 0);
                assert(this->transport[curCross][preCross].first[channelID][position]);
                int priorityCar = this->transport[curCross][preCross].first[channelID][position];

                //拿到转向信息[转向，下一路口，下一道路]
                vector<int> turningState = this->turningOfCar(priorityCar, curCross, curRoad);
                int turning = turningState[0];
                int nextRoadID = turningState[2];
                if (this->isConflict(turning, this->carsMap[priorityCar]->priority, curCross, curRoad))
                {
                    // cout << "conflict" << endl;
                    flag = false;
                    break;
                }
                if (this->isMoveToEndState(priorityCar, curCross, preCross, nextRoadID, channelID))
                {
                    //上优先级车辆 单车道单方向
                    this->driveCarOneDirect(preCross, curCross);
                }
                else
                {
                    flag = false;
                    break;
                }
            }
        }
    }
}

pair<int, int> Judgement::highestPriorCar(int curCross, int preCross)
{
    map<int, int> frontCarsMap;
    vector<int> waitRouteCarList;
    int targetCar = -1;
    int channelID = -1;
    int position = -1;
    int channelIndex = 0;
    for (auto &channel:this->transport[curCross][preCross].first) //获取每一个车道
    {
        if (channel.size() != 0)
        {
            int firstCarID = channel[0];
            if (this->carsMap[firstCarID]->status == 1)
            {
                frontCarsMap[firstCarID] = channelIndex;
                waitRouteCarList.push_back(firstCarID);
            }
        }
        channelIndex++;
    }
    map<int, vector<int>> flagMap; // 标记map 其中键1为优先级车辆，0为非优先级车辆
    for (auto carID : waitRouteCarList)
    {
        if (this->carsMap[carID]->priority)
            flagMap[1].push_back(carID);
        else
            flagMap[0].push_back(carID);
    }

    if (waitRouteCarList.size() != 0)
    {
        if (flagMap[1].size() == 1) //只有一辆优先级车辆，那麽它就是最高级车辆
        {
            targetCar = flagMap[1][0];
        }
        else if(flagMap[1].size() > 1) //有多辆优先级车辆
        {
            vector<int> distanceVec;
            for (auto carID:flagMap[1])
                distanceVec.push_back(carsMap[carID]->lengthToCross);
            int pos = distance(begin(distanceVec), min_element(distanceVec.begin(), distanceVec.end()));
            targetCar = flagMap[1][pos];
            distanceVec.clear();
        }
        else //没有优先级车辆 
        {
            if (flagMap[0].size() > 1)
            {
                vector<int> distanceVec;
                for (auto carID:flagMap[0])
                    distanceVec.push_back(carsMap[carID]->lengthToCross);
                int pos = distance(begin(distanceVec), min_element(distanceVec.begin(), distanceVec.end()));
                targetCar = flagMap[0][pos];
                distanceVec.clear();
            }
            else
                targetCar = flagMap[0][0];
        }
    }
    if (targetCar != -1)
    {
        channelID = frontCarsMap[targetCar];
        position = 0;
    }
    return pair<int, int>(channelID, position);
}

//判断车辆转向情况，返回一个vector包含[转向，下一个路口ID，下一条道路ID]
vector<int> Judgement::turningOfCar(int carID, int curCross, int curRoad)
{
    if(this->carsMap[carID]->dst == curCross)
        return vector<int>{0, -1, -1};
    int nextCross = -1;
    int nextRoad = this->carsMap[carID]->roadsIDQueue.front();
    assert(curRoad != nextRoad);
    // this->carsMap[carID]->roadsIDQueue.pop();
    if (this->roadsMap[nextRoad]->dst == curCross)
        nextCross = this->roadsMap[nextRoad]->src;
    else
        nextCross = this->roadsMap[nextRoad]->dst;
    vector<int> roadOnOneCross;
    roadOnOneCross = this->crossesMap[curCross]->roadsID_;
    int index1 = distance(roadOnOneCross.begin(), find(roadOnOneCross.begin(), roadOnOneCross.end(), curRoad));
    int index2 = distance(roadOnOneCross.begin(), find(roadOnOneCross.begin(), roadOnOneCross.end(), nextRoad));
    if (abs(index1 - index2) % 2 == 0) //直行
        return vector<int>{0, nextCross, nextRoad}; 
    else if((index1-index2) == -1 || (index1 == 3 && index2 == 0)) //左转
        return vector<int>{-1, nextCross, nextRoad};
    else //右转
        return vector<int>{1, nextCross, nextRoad};
}

//得到其他道路优先级最高车辆的转向信息，返回vector[carID, turning, 是否优先车辆, 顺时针第idx个路口]
vector<int> Judgement::getTurnInfo(int idx, int curCross, int curRoad)
{
    vector<int> roadOnOneCross;
    roadOnOneCross = this->crossesMap[curCross]->roadsID_;
    int curRoadIndex = (distance(roadOnOneCross.begin(),find(roadOnOneCross.begin(),roadOnOneCross.end(),curRoad)));
    int index = (curRoadIndex + idx) % 4;//还原第idx个道路到路口对道路的索引 0,1,2,3
    assert(index < 4);
    if (this->crossesMap[curCross]->roadsID_[index] != -1) //道路存在
    {
        int preCross = -1;
        for (auto &item:this->transport[curCross])
        {
            //得到该道路的preCross
            if (item.second.second == this->crossesMap[curCross]->roadsID_[index])
            {
                preCross = item.first;
                break;
            }
        }
        if (preCross != -1)
        {
            //取出该条道路的最高优先级车
            pair<int, int> priorityCarState = this->highestPriorCar(curCross, preCross);
            int channelID = priorityCarState.first;
            int position = priorityCarState.second;
            if (channelID == -1 and position == -1)
                return vector<int>{};
            assert(this->transport[curCross][preCross].first[channelID][position]);
            int priorityCar = this->transport[curCross][preCross].first[channelID][position];
            //得到最高优先级车辆的转向信息
            vector<int> turningInfo = this->turningOfCar(priorityCar, curCross,this->crossesMap[curCross]->roadsID_[index]);
            //返回所需的车辆转向信息
            return vector<int>{priorityCar, turningInfo[0], this->carsMap[priorityCar]->priority, idx};
        }

    }
    return vector<int>{};
}


bool Judgement::isConflict(int turning, bool isPriority, int curCross, int curRoad)
{
    map<int, vector<int>> otherCarInfo;
    for (int i = 1; i < 4; i++)
    {
        vector<int> turningInfo = this->getTurnInfo(i, curCross, curRoad);
        if (turningInfo.size() != 0)
        {
            for (int j = 1; j < (int)turningInfo.size(); j++)
                otherCarInfo[turningInfo[0]].push_back(turningInfo[j]);
        }
    }

    for (auto & item:otherCarInfo)
    {
        int carID = item.first;
        if (otherCarInfo.count(carID) > 0)
        {
            // cout << "-->" << item.second[2] << item.second[0] << item.second[1];
            if (isPriority)
            {
                if (turning == 0) //直行
                {
                    return false;
                }
                else if(turning == -1) //左转
                {
                    if (item.second[2] == 3 && item.second[0] == 0 && item.second[1])  //被顺时针第3个道路直行的车阻碍
                        return true;
                }
                else //右转
                {
                    if (item.second[2] == 1 && item.second[0]  == 0 && item.second[1])  // 被顺时针第1个道路的直行车辆阻碍
                        return true;
                    if (item.second[2] == 2 && item.second[0]  == -1 && item.second[1]) // 被顺时针第2个道路的左转车辆阻碍
                        return true;
                }
            }
            else //当前非优先
            {
                if (turning == 0) //直行
                {
                    if (item.second[2] == 1 && item.second[0] == -1 && item.second[1])
                        return true;
                    if (item.second[2] == 3 && item.second[0] == 1 && item.second[1])
                        return true;
                }
                else if(turning == -1) //左转
                {
                    if (item.second[2] == 3 && item.second[0] == 0)  //被顺时针第3个道路直行的车阻碍
                        return true;
                    if (item.second[2] == 2 && item.second[0] == 1 && item.second[1])
                        return true;
                }
                else //右转
                {
                    if (item.second[2] == 1 && item.second[0]  == 0)  // 被顺时针第1个道路的直行车辆阻碍
                        return true;
                    if (item.second[2] == 2 && item.second[0]  == -1) // 被顺时针第2个道路的左转车辆阻碍
                        return true;
                }
            }
            
        }
    }
    return false;
}

//更新后面车辆状态
void Judgement::updateFollowingCars(int curCross, int preCross, int channelID)
{
    int curRoad = this->transport[curCross][preCross].second;
    for (int i = 0; i < (int)this->transport[curCross][preCross].first[channelID].size(); i++)
    {
        int curCar = this->transport[curCross][preCross].first[channelID][i];
        int avaliableSpeed = min(this->roadsMap[curRoad]->speed, this->carsMap[curCar]->speed);
        if (this->carsMap[curCar]->status == 2) //如果处理过了
            continue;
        else if(this->carsMap[curCar]->status == 1)
        {
            if (i == 0) //如果该车目前是首车而且是属于真等待状态，直接跳出，等待优先级排序，后面的车状态不变
                break;
            int preCar = this->transport[curCross][preCross].first[channelID][i-1];
            assert(this->carsMap[preCar]->status == 2);
            this->carsMap[curCar]->lengthToCross = this->carsMap[preCar]->lengthToCross + 1;
            this->carsMap[curCar]->status = 2;
        }
        else //不出路口，还没有被处理
        {
            int preCar;
            if (i != 0)
                preCar = this->transport[curCross][preCross].first[channelID][i-1];
            //没有阻挡
            if (i == 0 || (this->carsMap[curCar]->lengthToCross - this->carsMap[preCar]->lengthToCross) > avaliableSpeed)
            {
                this->carsMap[curCar]->lengthToCross -= avaliableSpeed;
                this->carsMap[curCar]->status = 2;
            }
            else //有阻挡
            {
                assert(this->carsMap[preCar]->status == 2);
                this->carsMap[curCar]->lengthToCross = this->carsMap[preCar]->lengthToCross+1;
                this->carsMap[curCar]->status = 2;
            }
        }
        
    }
}


bool Judgement::isMoveToEndState(int carID, int curCross, int preCross,int nextRoad, int channelID)
{
    if (this->carsMap[carID]->dst == curCross) //如果车辆到达了终点
    {
        this->finishCnt += 1;
        // cout << "Car " << carID << " arrived" << endl;
        this->transport[curCross][preCross].first[channelID].erase(this->transport[curCross][preCross].first[channelID].begin());
        this->updateFollowingCars(curCross, preCross, channelID);
        this->totalTimeSlice += (this->timeSlice - this->carsMap[carID]->planTime);
        if (this->carsMap[carID]->priority)
        {
            this->priorityTimeSlice += (this->timeSlice - this->carsMap[carID]->planTime);
            this->priorityLastTime = this->timeSlice;
        }
        return true;
    }

    int v2 = min(this->roadsMap[nextRoad]->speed, this->carsMap[carID]->speed);
    int s2 = v2 - this->carsMap[carID]->lengthToCross;
    if (s2 <= 0) //下条道路行驶距离为0
    {
        this->carsMap[carID]->lengthToCross = 0;
        this->carsMap[carID]->status = 2;
        this->updateFollowingCars(curCross, preCross, channelID);
        return true;
    }
    int nextCross = -1;
    if (this->roadsMap[nextRoad]->dst == curCross)
        nextCross = this->roadsMap[nextRoad]->src;
    else
        nextCross = this->roadsMap[nextRoad]->dst;
    assert(nextCross != -1);
    int nextRoadLength = this->roadsMap[nextRoad]->length;
    for (int i = 0; i < this->roadsMap[nextRoad]->numChannel; i++)
    {
        //如果前路没车，或者可以到达目的位置
        if (this->transport[nextCross][curCross].first[i].size() == 0 || \
            this->carsMap[this->transport[nextCross][curCross].first[i].back()]->lengthToCross < (nextRoadLength-s2))
        {
            int new_s = nextRoadLength - s2;
            this->updateCarInfo(new_s, curCross, nextCross, carID);
            this->transport[curCross][preCross].first[channelID].erase(this->transport[curCross][preCross].first[channelID].begin());
            this->transport[nextCross][curCross].first[i].push_back(carID);
            this->updateFollowingCars(curCross, preCross, channelID);
            return true;
        } 
        else // 车道有车而且阻碍了当前车辆,阻碍车为终止状态
        {
            if(this->carsMap[this->transport[nextCross][curCross].first[i].back()]->status == 2)
            {
                //还有空位
                if (this->carsMap[this->transport[nextCross][curCross].first[i].back()]->lengthToCross < nextRoadLength - 1)
                {
                    int new_s = this->carsMap[this->transport[nextCross][curCross].first[i].back()]->lengthToCross + 1;
                    this->updateCarInfo(new_s, curCross, nextCross, carID);
                    this->transport[curCross][preCross].first[channelID].erase(this->transport[curCross][preCross].first[channelID].begin());
                    this->transport[nextCross][curCross].first[i].push_back(carID);
                    this->updateFollowingCars(curCross, preCross, channelID);
                    return true;
                } //没有空位
                else
                {
                    if ( i == this->roadsMap[nextRoad]->numChannel-1)
                    {
                        this->carsMap[carID]->lengthToCross = 0;
                        this->carsMap[carID]->status = 2;
                        this->updateFollowingCars(curCross, preCross, channelID);
                        return true;
                    }
                }
            }
            else//阻碍的车不是终止状态
                return false;
        }
    }
}


double controlPrecise(double x)
{
    return floor(x * 100000.00f + 0.5) / 100000.00f;
}

void Judgement::printResult()
{
    int priNum = 0;
    vector<int> speed, priorSpeed;
    vector<int> startTime, priorStartTime;
    set<int> startLocation, priorStartLocation;
    set<int> endLocation, priorEndLocation;
    for(auto &item:this->carsMap)
    {
        int carID = item.first;
        Car *car = item.second;
        if(car->priority && car->planTime < this->priorityFirstTime)
            this->priorityFirstTime = car->planTime;
        if(car->priority)
        {
            priNum++;
            priorSpeed.push_back(car->speed);
            priorStartTime.push_back(car->planTime);
            priorStartLocation.insert(car->src);
            priorEndLocation.insert(car->dst);
        }
        speed.push_back(car->speed);
        startTime.push_back(car->planTime);
        startLocation.insert(car->src);
        endLocation.insert(car->dst);
    }

    int priorityTime = this->priorityLastTime - this->priorityFirstTime;
    cout << "------------------------------------------------------------------" << endl;
    printf("piror: schedule time: %d, all schedule time: %d\n", priorityTime, priorityTimeSlice);
    printf("total: schedule time: %d, all schedule time: %d\n",timeSlice, totalTimeSlice);

    double carRatio, speedRatio,startTimeRatio, startLocRatio, endLocRatio;
    carRatio = controlPrecise(carsMap.size() / (double)priNum);
    assert(priNum != 0);

    double minSpeed = (double)*min_element(speed.begin(), speed.end());
    double maxSpeed = (double)*max_element(speed.begin(), speed.end());
    double minPriorSpeed = (double)*min_element(priorSpeed.begin(), priorSpeed.end());
    double maxPriorSpeed = (double)*max_element(priorSpeed.begin(), priorSpeed.end());
    speedRatio = controlPrecise(maxSpeed / minSpeed) / controlPrecise(maxPriorSpeed / minPriorSpeed);

    double minStartTime = (double)*min_element(startTime.begin(), startTime.end());
    double maxStartTime = (double)*max_element(startTime.begin(), startTime.end());
    double minPriorStartTime = (double)*min_element(priorStartTime.begin(), priorStartTime.end());
    double maxPriorStartTime = (double)*max_element(priorStartTime.begin(), priorStartTime.end());
    startTimeRatio = controlPrecise(maxStartTime / minStartTime) / controlPrecise(maxPriorStartTime / minPriorStartTime);

    startLocRatio = controlPrecise(startLocation.size() / (double)priorStartLocation.size());
    endLocRatio = controlPrecise(endLocation.size() / (double)priorEndLocation.size());

    double a = 0.05 * carRatio + (speedRatio + startTimeRatio + startLocRatio + endLocRatio) * 0.2375;
    double b = 0.80 * carRatio + (speedRatio + startTimeRatio + startLocRatio + endLocRatio) * 0.05;
    cout << "a is " << a <<"  b is " << b << endl;
    cout<< "weighted: schedule time: "<< round(this->timeSlice+a*priorityTime) << endl;
    cout << "weighted: all schedule time: " << setprecision(8)<<round(this->totalTimeSlice + b * this->priorityTimeSlice) << endl;
}