/*
    huawei CodeCraft-2019
    author: 支涛、唐裕亮、武林璐@西电
*/

#include "lib/solution.h"

Solver::Solver() {}

Solver::Solver(map<int, Car*> &carsMap, map<int, Road*> &roadsMap,
                   map<int,Cross*> &crossesMap, map<int, Car*> &presetMap) {
    this->carsMap = carsMap;
    this->roadsMap = roadsMap;
    this->crossesMap = crossesMap;
    this->presetMap = presetMap;
    this->timeSlice = 1;
    // build graph
    for (auto &item:roadsMap) {
        Road* road = item.second;
        this->vertexes.insert(road->src);
        this->vertexes.insert(road->dst);
        Edge edgeForward = Edge(road->id, road->src, road->dst, road->baseWeight, true);
        this->adjacents[road->src].insert(edgeForward);
        this->edges.insert(edgeForward);
        if (road->isDuplex) {
            Edge edgeBackward = Edge(road->id, road->dst, road->src, road->baseWeight, false);
            this->adjacents[road->dst].insert(edgeBackward);
            this->edges.insert(edgeBackward);
        }
    }
    this->analyse();

    this->runQueLength = (int)(this->roadSumArea_ * 0.06);
    cout << "RunQueLength: " << this->runQueLength << endl;
}


void Solver::analyse() {
    set<int> allCarSrc, allCarDst, prioCarSrc, prioCarDst;
    vector<int> allCarSpeed, prioCarSpeed;
    int allCarEarlyTime = INT_MAX, prioCarEarlyTime = INT_MAX;
    int allCarLastTime = 0, prioCarLastTime = 0;
    Car * car;
    int carID, pritotalCarCnt = 0;
    for (auto &item: this->carsMap) {
        carID = item.first;
        car = item.second;
        allCarSpeed.push_back(car->speed);
        allCarSrc.insert(car->src);
        allCarDst.insert(car->dst);
        allCarEarlyTime = car->planTime < allCarEarlyTime? car->planTime: allCarEarlyTime;
        allCarLastTime = car->planTime > allCarLastTime? car->planTime: allCarLastTime;
        if (car->priority) {
            pritotalCarCnt += 1;
            prioCarSpeed.push_back(car->speed);
            prioCarSrc.insert(car->src);
            prioCarDst.insert(car->dst);
            prioCarEarlyTime = car->planTime < prioCarEarlyTime?car->planTime: prioCarEarlyTime;
            prioCarLastTime = car->planTime > prioCarLastTime?car->planTime: prioCarLastTime;
        }
        if (!car->preset) {
            this->manageCarsMap[carID] = car;
            this->managePlanTimeMap[car->planTime].push_back(carID);
        }
        this->carsPlanTimeMap[car->planTime].push_back(carID);
    }
    if (this->manageCarsMap.size() + this->presetMap.size() != this->carsMap.size()) {
        cout << "preset and total car num error!" << endl;
        exit(-1);
    }
    this->prioEarlyTime = prioCarEarlyTime; // 记录下优先车辆最早出发时间用于计算 T_pri
    double a = 0;
    int maxAllCarSpeed = *max_element(allCarSpeed.begin(), allCarSpeed.end());
    int minAllCarSpeed = *min_element(allCarSpeed.begin(), allCarSpeed.end());
    int maxPrioCarSpeed = *max_element(prioCarSpeed.begin(), prioCarSpeed.end());
    int minPrioCarSpeed = *min_element(prioCarSpeed.begin(), prioCarSpeed.end());

    a = (double)allCarSpeed.size() / prioCarSpeed.size() * 0.05 + \
        ((double)maxAllCarSpeed/minAllCarSpeed) / ((double)maxPrioCarSpeed/minPrioCarSpeed) * 0.2375 + \
        ((double)allCarLastTime/allCarEarlyTime) / ((double)prioCarLastTime/prioCarEarlyTime) * 0.2375 + \
        (double)allCarSrc.size() / prioCarSrc.size() * 0.2375 + \
        (double)allCarDst.size() / prioCarDst.size() * 0.2375;

    double a1 = (double)allCarSpeed.size() / prioCarSpeed.size() * 0.05;
    double a2 = ((double)maxAllCarSpeed/minAllCarSpeed) / ((double)maxPrioCarSpeed/minPrioCarSpeed) * 0.2375;
    double a3 = ((double)allCarLastTime/allCarEarlyTime) / ((double)prioCarLastTime/prioCarEarlyTime) * 0.2375;
    double a4 = (double)allCarSrc.size() / prioCarSrc.size() * 0.2375;
    double a5 = (double)allCarDst.size() / prioCarDst.size() * 0.2375;
    cout << "a1 = " << a1 << ", a2 = " << a2 << ", a3 = " << a3 << ", a4 = " << a4 << ", a5 = " << a5 << endl;
    this->a = a;
    cout << "a = " << a << endl;
    int priPresetCarCnt = 0;
    for (auto &item: this->presetMap) {
        carID = item.first;
        car = item.second;
        if (car->priority)
            priPresetCarCnt += 1;
        this->presetStartTimeMap[car->startTime].push_back(carID);
    }

    cout << "============= CAR INFO =================" << endl;
    cout << "[totalCarNum]: " << allCarSpeed.size() << endl;
    cout << "[allCarSrc]: " << allCarSrc.size() << " [allCarDst]:" << allCarDst.size() << endl;
    cout << "[allCarSpeed]:" << *max_element(allCarSpeed.begin(),allCarSpeed.end()) << "(MAX), " \
         <<  *min_element(allCarSpeed.begin(),allCarSpeed.end()) << "(MIN), " \
         <<  (float)accumulate(allCarSpeed.begin(),allCarSpeed.end(),0) / allCarSpeed.size() << "(AVG)" << endl;


    vector<int> roadsSpeed;
    vector<int> roadsLength;
    vector<int> roadsMaxtotalCarNum;
    vector<double> roadsCost;
    vector<int> roadsChannel;
    vector<int> roadsArea;
    int duplexNum = 0;
    for (auto &item: this->roadsMap)
    {
        roadsSpeed.push_back(item.second->speed);
        roadsLength.push_back(item.second->length);
        roadsMaxtotalCarNum.push_back(item.second->maxCarNum);
        roadsCost.push_back(item.second->baseWeight);
        roadsChannel.push_back(item.second->numChannel);

        if (item.second->isDuplex) {
            duplexNum++;
            roadsArea.push_back(item.second->length*item.second->numChannel*2);
        }
        else {
            roadsArea.push_back(item.second->length*item.second->numChannel);
        }
    }

    double maxBaseWeight = *max_element(roadsCost.begin(),roadsCost.end());
    vector<double> roadsCostNorm;
    for (double cost:roadsCost) {
        roadsCostNorm.push_back(cost / maxBaseWeight);
    }

    for (auto &item:this->roadsMap) {
        item.second->baseWeight = 1.0*(double)item.second->baseWeight / 1; //maxBaseWeight
    }
    this->roadsAvgCost = (double)accumulate(roadsSpeed.begin(),roadsSpeed.end(),0) / roadsSpeed.size();

    cout << "============= ROAD INFO =================" << endl;
    cout << "[RoadNum]:" << roadsSpeed.size() <<"(TOTAL) "\
         << duplexNum<<"(DUPLEX) " << roadsSpeed.size()-duplexNum << "(SIMPLEX)" <<endl;

    cout << "[RoadSpeed]:" << *max_element(roadsSpeed.begin(),roadsSpeed.end()) << "(MAX) " \
         << *min_element(roadsSpeed.begin(),roadsSpeed.end()) << "(MIN) " \
         << (float)accumulate(roadsSpeed.begin(),roadsSpeed.end(),0) / roadsSpeed.size() << "(AVG)" << endl;

    cout << "[RoadLen]:" << *max_element(roadsLength.begin(),roadsLength.end()) << "(MAX) " \
         << *min_element(roadsLength.begin(),roadsLength.end()) << "(MIN) " \
         << (float)accumulate(roadsLength.begin(),roadsLength.end(),0) / roadsLength.size() << "(AVG)" << endl;

    cout << "[MaxtotalCarNum]:" << *max_element(roadsMaxtotalCarNum.begin(),roadsMaxtotalCarNum.end()) << "(MAX) " \
         << *min_element(roadsMaxtotalCarNum.begin(),roadsMaxtotalCarNum.end()) << "(MIN) " \
         << (float)accumulate(roadsMaxtotalCarNum.begin(),roadsMaxtotalCarNum.end(),0) / roadsMaxtotalCarNum.size() << "(AVG)" << endl;

    cout << "[Weight]:" << maxBaseWeight << "(MAX) " \
         << *min_element(roadsCost.begin(),roadsCost.end()) << "(MIN) " \
         << (float)accumulate(roadsCost.begin(),roadsCost.end(),0.0) / roadsCost.size() << "(AVG)" << endl;

    cout << "[WeightNorm]:" << *max_element(roadsCostNorm.begin(),roadsCostNorm.end()) << "(MAX) " \
         << *min_element(roadsCostNorm.begin(),roadsCostNorm.end()) << "(MIN) " \
         << (float)accumulate(roadsCostNorm.begin(),roadsCostNorm.end(),0.0)/(float)roadsCostNorm.size() << "(AVG)" << endl;

    cout << "[RoadChannel]:" << *max_element(roadsChannel.begin(),roadsChannel.end()) << "(MAX) " \
         << *min_element(roadsChannel.begin(),roadsChannel.end()) << "(MIN) " \
         << (float)accumulate(roadsChannel.begin(),roadsChannel.end(),0) / roadsChannel.size() << "(AVG)" << endl;

    cout << "[RoadArea]:" << *max_element(roadsArea.begin(), roadsArea.end()) << "(MAX) " \
         << *min_element(roadsArea.begin(), roadsArea.end()) << "(MIN) " \
         << (float)accumulate(roadsArea.begin(),roadsArea.end(),0) / roadsArea.size() << "(AVG) " \
         << (float)accumulate(roadsArea.begin(),roadsArea.end(),0) << "(SUM)" << endl;
    cout << "=========================================" << endl;
    this->roadSumArea_ = accumulate(roadsArea.begin(),roadsArea.end(),0);
}

vector<int> Solver::search_path(int startPoint, int target, double carSeed, double alpha, double beta, bool isTail) {
    MinHeap* distQueue = new MinHeap(380);
    map<int, double> dist2Vtx;
    map<int, Edge> edge2Vtx;

    dist2Vtx[startPoint] = 0.0;
    Tuple tuple(startPoint, dist2Vtx[startPoint]);
    if (distQueue->insert(tuple) == -1)
        cout << "Insert Fail" << endl;

    while (!distQueue->empty()) {
        int vertex = distQueue->top().vertex;
        distQueue->pop();
        for(auto &edge:this->adjacents[vertex]) {
            double carWeight = 0.0;
            int src = edge.src;
            int dst = edge.dst;

            if (edge.isForward)
                carWeight = this->roadsMap[edge.roadID]->forward_weight();
            else
                carWeight = this->roadsMap[edge.roadID]->backward_weight();

            Road* road = this->roadsMap[edge.roadID];
            double baseWeight = 0.0;
            double v = min(carSeed, road->speed);
            baseWeight = (double)road->length / v;

            double gamma = 1.0;
            if (carWeight <= 0.2)
                gamma = 5;
            else if (carWeight <= 0.3)
                gamma = 10;
            else if (carWeight <= 0.4)
                gamma = 15;
            else if (carWeight <= 0.5)
                gamma = 20;
            else
                gamma = 30;
            if (carWeight < 1000)
                carWeight *= gamma;
            
            double nextRoadWeight = dist2Vtx[src] + (alpha * baseWeight) + (beta * carWeight);
            if (dist2Vtx.count(dst) == 0)
                dist2Vtx[dst] = FLT_MAX;
            if (dist2Vtx[dst] > nextRoadWeight) {
                dist2Vtx[dst] = nextRoadWeight;
                edge2Vtx[dst] = edge;
                if (distQueue->find(dst) != -1) {
                    if(distQueue->replace(Tuple(dst, dist2Vtx[dst])) == -1) {
                        cout << "Replace Fail" << endl;
                        exit(-1);
                    }
                } else {
                     if (distQueue->insert(Tuple(dst, dist2Vtx[dst])) == -1) {
                         cout << "Insert Fail" << endl;
                         exit(-1);
                     }
                }
            }
        }
        if (edge2Vtx.count(target) == 1)
            break;
    }

    vector<int> shortestPath;
    int tmpVtx = target;
    shortestPath.clear();
    while (tmpVtx != startPoint)
    {
        Edge edge = edge2Vtx[tmpVtx];
        shortestPath.push_back(edge.roadID);
        tmpVtx = edge.src;
    }

    reverse(shortestPath.begin(), shortestPath.end());
    delete distQueue;
    return shortestPath;
}

void Solver::manage_once(vector<int> &manageNewRunCars, bool isTail) {
    for (auto &item:this->roadsMap) {
        Road* road = this->roadsMap[item.first];
        road->update_slice(this->waitingRouterCars);
    }
    
    this->route(isTail);
    this->run_new_cars(manageNewRunCars);
}

void Solver::manage_all() {
    int totalCarNum = this->carsMap.size();
    int totalCarCnt = 0, manageCarCnt = 0;
    bool isTail = false;
    vector<int> waitingRunCars;
    vector<int> manageNewRunCars;
    vector<int> presetNewRunCars;
    vector<int> totalNewRunCars;
    map<int, int> planTimesOrdIndex;

    int index = 0;
    for(auto &item: this->managePlanTimeMap) {
        for (auto v: item.second) {
            waitingRunCars.push_back(v);
        }
        index += item.second.size();
        planTimesOrdIndex[item.first] = index;
    }

    cout << "Total Car Num: " << totalCarNum << endl;
    while(true) {
        if (totalCarCnt == totalCarNum && this->runningCars.size() == 0) {
            cout << "Done" << endl;
            break;
        }
        this->waitingRouterCars.clear();
        if (totalCarCnt == totalCarNum) {
            presetNewRunCars.clear();
            manageNewRunCars.clear();
            isTail = true;
        }
        else {
            presetNewRunCars.clear();
            if (this->presetStartTimeMap.count(this->timeSlice) > 0) // 此刻需要发预置车辆
                presetNewRunCars = this->presetStartTimeMap[this->timeSlice];
            int maxManageNewRunNum = max((this->runQueLength - (int)this->runningCars.size() - (int)presetNewRunCars.size()), 0) ;
            manageNewRunCars.clear();
            int maxEndIndex = min(manageCarCnt+maxManageNewRunNum, (int)waitingRunCars.size());
            for (int i = manageCarCnt; i < maxEndIndex; i++)
                manageNewRunCars.push_back(waitingRunCars[i]);
            if (manageNewRunCars.size() > 0) {   // 检测planTime是否早于timeSlice
                int lastCarID = manageNewRunCars[manageNewRunCars.size()-1];
                Car* lastCar = this->carsMap[lastCarID];
                if (lastCar->planTime > this->timeSlice) {
                    int lastCarIndex =  planTimesOrdIndex[this->timeSlice];
                    manageNewRunCars.clear();
                    for (int i=manageCarCnt; i<lastCarIndex; i++)
                        manageNewRunCars.push_back(waitingRunCars[i]);
                }
            }
        }
        totalNewRunCars.clear();
        // 把预置车辆和调度车辆合并
        for (auto v: presetNewRunCars)
            totalNewRunCars.push_back(v);
        for (auto v: manageNewRunCars)
            totalNewRunCars.push_back(v);
        totalCarCnt += totalNewRunCars.size();
        manageCarCnt += manageNewRunCars.size();
        cout << this->timeSlice << " " << totalCarCnt << " " << this->runningCars.size() << " " 
            << manageNewRunCars.size() << " " << presetNewRunCars.size() << endl;
        this->manage_once(totalNewRunCars, isTail);
        this->timeSlice++;
    }

    this->T = this->timeSlice;
    this->T_pri = this->prioEndTime - this->prioEarlyTime;
    cout << "a = " << this->a << ", T = " << this->T << ", T_pri = " << this->T_pri << endl;
    double judgeTime = this->a * this->T_pri + this->T;
    cout << "judgeTime: " << judgeTime << endl;
}

void Solver::route(bool isTail) {
    double alpha = 1.0;
    double beta = 1.0;
    if (isTail) {
        int runningCarsNum = this->runningCars.size();
        alpha = min((double)this->runQueLength / runningCarsNum * 2, 12.0);
        beta = max((double)runningCarsNum / this->runQueLength * 3, 0.5) * 2;
    }
    set<int> normRouteCars;
    set<int> prioRouteCars;
    for(int carID: this->waitingRouterCars) {
        Car *car = this->carsMap[carID];
        if (car->priority)  // 先调度优先级高的车辆
            this->route_one_car(carID, alpha, beta, isTail);
        else                // 再调度一般车辆
            normRouteCars.insert(carID);
    }

    for(int carID: normRouteCars) {
        this->route_one_car(carID, alpha, beta, isTail);
    }
}


void Solver::route_one_car(int carID, double alpha, double beta, bool isTail) {
    int startPoint, newRoadID;
    double leftTime;
    Road * newRoad = NULL;
    Car *car = this->carsMap[carID];
    Road* runningRoad = this->roadsMap[car->runningRoadID];
    if (car->isForwad) {
        startPoint = runningRoad->dst;
        leftTime = runningRoad->forwardCarTime[carID];
    } else {
        startPoint = runningRoad->src;
        leftTime = runningRoad->backwardCarTime[carID];
    }

    if (startPoint == car->dst) {
        car->costTime = this->timeSlice - car->startTime;
        this->runningCars.erase(carID);
        if (car->priority)
            this->prioEndTime = this->timeSlice;    // 记录最后一辆优先级车结束时间
        return;
    }
    if (car->preset) {  // 预置车辆
        newRoadID = car->histroyRoadsID[0];
        car->runningRoadID = newRoadID;
        car->histroyRoadsID.erase(car->histroyRoadsID.begin());
        newRoad = this->roadsMap[newRoadID];
        car->isForwad = newRoad->src == startPoint?true:false;
        car->availiableSpeed = min(car->speed, newRoad->speed);
        if (car->isForwad)
            newRoad->forwardCarTime[car->id] = (double)(newRoad->length) / (car->availiableSpeed) + leftTime;
        else
            newRoad->backwardCarTime[car->id] =(double)(newRoad->length) / (car->availiableSpeed) + leftTime;
        return;
    }
    runningRoad->reverseForbidden = true;
    vector<int> shortestPath = this->search_path(startPoint, car->dst, car->speed, alpha, beta, isTail=isTail);
    runningRoad->reverseForbidden = false;
    newRoadID = shortestPath[0];
    car->runningRoadID = newRoadID;
    newRoad = this->roadsMap[newRoadID];
    assert(newRoad);
    car->histroyRoadsID.push_back(car->runningRoadID);
    car->isForwad = newRoad->src == startPoint?true:false;
    car->availiableSpeed = min(car->speed, newRoad->speed);
    if (car->isForwad)
        newRoad->forwardCarTime[car->id] = (double)(newRoad->length) / (car->availiableSpeed) + leftTime;
    else
        newRoad->backwardCarTime[car->id] =(double)(newRoad->length) / (car->availiableSpeed) + leftTime;
}

void Solver::run_new_cars(vector<int> &totalNewRunCars) {
    set<int> prioNewCars;
    set<int> normNewCars;
    Car * car;
    // 根据优先级 放在set中按照ID从小到大排序，然后先上路优先级高的
    for (auto carID: totalNewRunCars) {
        car = this->carsMap[carID];
        if (car->priority)
            prioNewCars.insert(carID);
        else
            normNewCars.insert(carID);
    }
    for (auto carID: prioNewCars) {
        this->run_one_car(carID);
    }
    for (auto carID: normNewCars) {
        this->run_one_car(carID);
    }
}

void Solver::run_one_car(int carID) {
    double alpha = 1.0;
    double beta = 1.0;
    int newRoadID;
    Road * newRoad = NULL;
    Car *car = this->carsMap[carID];
    assert(car);
    if (car->preset) {
        newRoadID = car->histroyRoadsID[0];
        assert(newRoadID);
        car->histroyRoadsID.erase(car->histroyRoadsID.begin());
        newRoad = this->roadsMap[newRoadID];
        car->runningRoadID = newRoadID;
        assert(newRoad);
        car->isForwad = newRoad->src == car->src?true:false;
        car->availiableSpeed = min(car->speed, newRoad->speed);
        if (car->isForwad)
            newRoad->forwardCarTime[car->id] = (double)newRoad->length/car->availiableSpeed;
        else
            newRoad->backwardCarTime[car->id] =(double)newRoad->length/car->availiableSpeed;
        this->runningCars.insert(carID);
        return;
    }
    car->startTime = this->timeSlice;
    vector<int> shortestPath = this->search_path(car->src, car->dst, car->speed, alpha, beta);
    car->runningRoadID = shortestPath[0];
    newRoad = this->roadsMap[shortestPath[0]];
    assert(newRoad);
    car->histroyRoadsID.push_back(shortestPath[0]);
    car->isForwad = newRoad->src == car->src?true:false;
    car->availiableSpeed = min(car->speed, newRoad->speed);
    if (car->isForwad)
        newRoad->forwardCarTime[car->id] = (double)newRoad->length/car->availiableSpeed;
    else
        newRoad->backwardCarTime[car->id] =(double)newRoad->length/car->availiableSpeed;
    this->runningCars.insert(carID);
}


Edge::Edge() {}

Edge::Edge(int roadID, int src, int dst, double weight, bool isForwad) {
    this->roadID = roadID;
    this->dst = dst;
    this->src = src;
    this->weight = weight;
    this->isForward = isForwad;
}

Edge::Edge(const Edge &E) {
    this->roadID = E.roadID;
    this->dst = E.dst;
    this->src = E.src;
    this->weight = E.weight;
    this->isForward = E.isForward;
}


