/*
    huawei CodeCraft-2019
    author: 支涛、唐裕亮、武林璐@西电
*/

#include "lib/solution.h"
using namespace std;

Solver::Solver() {}

Solver::Solver(map<int, Car *> &carsMap, map<int, Road *> &roadsMap,
               map<int, Cross *> &crossesMap, map<int, Car *> &presetMap)
{
    this->carsMap = carsMap;
    this->roadsMap = roadsMap;
    this->crossesMap = crossesMap;
    this->presetMap = presetMap;
    this->timeSlice = 1;
    this->alpha = 1.0;
    this->beta = 1.0;
    this->judge = new Judgement(this->carsMap, this->roadsMap, this->crossesMap);
    // 给gamma向量初始化

    // build graph
    for (auto &item : roadsMap)
    {
        Road *road = item.second;
        this->vertexes.insert(road->src);
        this->vertexes.insert(road->dst);
        Edge edgeForward = Edge(road->id, road->src, road->dst, road->baseWeight, true);
        this->adjacents[road->src].insert(edgeForward);
        this->edges.insert(edgeForward);
        this->laneNum++;
        if (road->isDuplex)
        {
            Edge edgeBackward = Edge(road->id, road->dst, road->src, road->baseWeight, false);
            this->adjacents[road->dst].insert(edgeBackward);
            this->edges.insert(edgeBackward);
            this->laneNum++;
        }
    }
    this->analyse();
    this->get_manage_preset_car();
}

void Solver::get_manage_preset_car()
{
    //1. 直接按照最后的出发时间的10%的用来自己调度
    map<int, set<int>> presetStartTimeMap;
    Car * car;
    for (auto &item: this->presetMap)
    {
        int presetCarID = item.first;
        car = item.second;
        presetStartTimeMap[car->startTime].insert(presetCarID);
    }

    vector<int> presetStartTimeOrdCars;
    for (auto &item : presetStartTimeMap) {
        for (auto &v : item.second) {
            presetStartTimeOrdCars.push_back(v);
        }
    }


    int num1 = (int)this->presetMap.size() * 0.05;
    int num2 = num1;
    
    for (int i = presetStartTimeOrdCars.size() - 1; i > presetStartTimeOrdCars.size() - num1; i--)
    {
        int carID = presetStartTimeOrdCars[i];
        car = this->presetMap[carID];
        if (car->priority)
            continue;
        this->manCarsMap[carID] = car; // 加入manage集合中
        car->managePreset = true; // 标志这些车辆需要重新寻路
        car->histroyRoadsID.clear();
        car->roadsIDQueue = queue<int>();
    }
    for (int j=0,i = presetStartTimeOrdCars.size() - 1; j < num2; i--)
    {
        int carID = presetStartTimeOrdCars[i];
        car = this->presetMap[carID];
        if (car->priority)
        {
            this->manCarsMap[carID] = car;  // 加入manage集合中
            car->startTime = car->planTime;
            j++;
        }
    }


    this->presetStartTimeMap.clear();
    for (auto &item : this->presetMap)
    {
        int carID = item.first;
        car = item.second;
        this->presetStartTimeMap[car->startTime].push_back(carID);
        if (car->startTime > this->lastPresetStartTime)
            this->lastPresetStartTime = car->startTime;
    }
}

void Solver::analyse()
{
    set<int> allCarSrc, allCarDst, prioCarSrc, prioCarDst;
    vector<int> allCarSpeed, prioCarSpeed;
    int allCarEarlyTime = INT_MAX, prioCarEarlyTime = INT_MAX;
    int allCarLastTime = 0, prioCarLastTime = 0;
    Car *car;
    int carID, pritotalCarCnt = 0;
    for (auto &item : this->carsMap)
    {
        carID = item.first;
        car = item.second;
        allCarSpeed.push_back(car->speed);
        allCarSrc.insert(car->src);
        allCarDst.insert(car->dst);
        allCarEarlyTime = car->planTime < allCarEarlyTime ? car->planTime : allCarEarlyTime;
        allCarLastTime = car->planTime > allCarLastTime ? car->planTime : allCarLastTime;
        if (car->priority) {
            pritotalCarCnt += 1;
            prioCarSpeed.push_back(car->speed);
            prioCarSrc.insert(car->src);
            prioCarDst.insert(car->dst);
            prioCarEarlyTime = car->planTime < prioCarEarlyTime ? car->planTime : prioCarEarlyTime;
            prioCarLastTime = car->planTime > prioCarLastTime ? car->planTime : prioCarLastTime;

        } 
       
        if (!car->preset) {
            this->manCarsMap[carID] = car;
            this->manCarsPlanTimeMap[car->planTime].push_back(carID);
            // 把需要调度的车辆 按照priority 和norm的planTime分别存放，先调度priority
            if (car->priority)
                this->manPrioPlanTimeMap[car->planTime].push_back(carID);
            else
                this->manNormPlanTimeMap[car->planTime].push_back(carID);
        }
        this->carsPlanTimeMap[car->planTime].push_back(carID);
    }

    this->prioEarlyTime = prioCarEarlyTime; // 记录下优先车辆最早出发时间用于计算 T_pri
    double a = 0;
    int maxAllCarSpeed = *max_element(allCarSpeed.begin(), allCarSpeed.end());
    int minAllCarSpeed = *min_element(allCarSpeed.begin(), allCarSpeed.end());
    int maxPrioCarSpeed = *max_element(prioCarSpeed.begin(), prioCarSpeed.end());
    int minPrioCarSpeed = *min_element(prioCarSpeed.begin(), prioCarSpeed.end());

    a = (double)allCarSpeed.size() / prioCarSpeed.size() * 0.05 +
        ((double)maxAllCarSpeed / minAllCarSpeed) / ((double)maxPrioCarSpeed / minPrioCarSpeed) * 0.2375 +
        ((double)allCarLastTime / allCarEarlyTime) / ((double)prioCarLastTime / prioCarEarlyTime) * 0.2375 +
        (double)allCarSrc.size() / prioCarSrc.size() * 0.2375 +
        (double)allCarDst.size() / prioCarDst.size() * 0.2375;

    double a1 = (double)allCarSpeed.size() / prioCarSpeed.size() * 0.05;
    double a2 = ((double)maxAllCarSpeed / minAllCarSpeed) / ((double)maxPrioCarSpeed / minPrioCarSpeed) * 0.2375;
    double a3 = ((double)allCarLastTime / allCarEarlyTime) / ((double)prioCarLastTime / prioCarEarlyTime) * 0.2375;
    double a4 = (double)allCarSrc.size() / prioCarSrc.size() * 0.2375;
    double a5 = (double)allCarDst.size() / prioCarDst.size() * 0.2375;
    cout << "a1 = " << a1 << ", a2 = " << a2 << ", a3 = " << a3 << ", a4 = " << a4 << ", a5 = " << a5 << endl;
    this->a = a;
    cout << "a = " << a << endl;
    int priPresetCarCnt = 0;
    for (auto &item : this->presetMap) {
        carID = item.first;
        car = item.second;
        if (car->priority)
            priPresetCarCnt += 1;
        this->presetStartTimeMap[car->startTime].push_back(carID);
        if (car->startTime > this->lastPresetStartTime)
            this->lastPresetStartTime = car->startTime;
    }

    cout << "============= CAR INFO =================" << endl;
    cout << "[totalCarNum]: " << allCarSpeed.size() << endl;
    cout << "[allCarSrc]: " << allCarSrc.size() << " [allCarDst]:" << allCarDst.size() << endl;
    cout << "[allCarSpeed]:" << *max_element(allCarSpeed.begin(), allCarSpeed.end()) << "(MAX), "
         << *min_element(allCarSpeed.begin(), allCarSpeed.end()) << "(MIN), "
         << (float)accumulate(allCarSpeed.begin(), allCarSpeed.end(), 0) / allCarSpeed.size() << "(AVG)" << endl;

    vector<int> roadsSpeed;
    vector<int> roadsLength;
    vector<int> roadsMaxtotalCarNum;
    vector<double> roadsCost;
    vector<int> roadsChannel;
    vector<int> roadsArea;
    int duplexNum = 0;
    for (auto &item : this->roadsMap)
    {
        roadsSpeed.push_back(item.second->speed);
        roadsLength.push_back(item.second->length);
        roadsMaxtotalCarNum.push_back(item.second->maxCarNum);
        roadsCost.push_back(item.second->baseWeight);
        roadsChannel.push_back(item.second->numChannel);

        if (item.second->isDuplex)
        {
            duplexNum++;
            roadsArea.push_back(item.second->length * item.second->numChannel * 2);
        }
        else
        {
            roadsArea.push_back(item.second->length * item.second->numChannel);
        }
    }

    double maxBaseWeight = *max_element(roadsCost.begin(), roadsCost.end());
    vector<double> roadsCostNorm;
    for (double cost : roadsCost)
    {
        roadsCostNorm.push_back(cost / maxBaseWeight);
    }

    for (auto &item : this->roadsMap)
    {
        item.second->baseWeight = 1.0 * (double)item.second->baseWeight / 1; //maxBaseWeight
    }
    this->roadsAvgCost = (double)accumulate(roadsSpeed.begin(), roadsSpeed.end(), 0) / roadsSpeed.size();

    cout << "============= ROAD INFO =================" << endl;
    cout << "[RoadNum]:" << roadsSpeed.size() << "(TOTAL) "
         << duplexNum << "(DUPLEX) " << roadsSpeed.size() - duplexNum << "(SIMPLEX)" << endl;

    cout << "[RoadSpeed]:" << *max_element(roadsSpeed.begin(), roadsSpeed.end()) << "(MAX) "
         << *min_element(roadsSpeed.begin(), roadsSpeed.end()) << "(MIN) "
         << (float)accumulate(roadsSpeed.begin(), roadsSpeed.end(), 0) / roadsSpeed.size() << "(AVG)" << endl;

    cout << "[RoadLen]:" << *max_element(roadsLength.begin(), roadsLength.end()) << "(MAX) "
         << *min_element(roadsLength.begin(), roadsLength.end()) << "(MIN) "
         << (float)accumulate(roadsLength.begin(), roadsLength.end(), 0) / roadsLength.size() << "(AVG)" << endl;

    cout << "[MaxtotalCarNum]:" << *max_element(roadsMaxtotalCarNum.begin(), roadsMaxtotalCarNum.end()) << "(MAX) "
         << *min_element(roadsMaxtotalCarNum.begin(), roadsMaxtotalCarNum.end()) << "(MIN) "
         << (float)accumulate(roadsMaxtotalCarNum.begin(), roadsMaxtotalCarNum.end(), 0) / roadsMaxtotalCarNum.size() << "(AVG)" << endl;

    cout << "[Weight]:" << maxBaseWeight << "(MAX) "
         << *min_element(roadsCost.begin(), roadsCost.end()) << "(MIN) "
         << (float)accumulate(roadsCost.begin(), roadsCost.end(), 0.0) / roadsCost.size() << "(AVG)" << endl;

    cout << "[WeightNorm]:" << *max_element(roadsCostNorm.begin(), roadsCostNorm.end()) << "(MAX) "
         << *min_element(roadsCostNorm.begin(), roadsCostNorm.end()) << "(MIN) "
         << (float)accumulate(roadsCostNorm.begin(), roadsCostNorm.end(), 0.0) / (float)roadsCostNorm.size() << "(AVG)" << endl;

    cout << "[RoadChannel]:" << *max_element(roadsChannel.begin(), roadsChannel.end()) << "(MAX) "
         << *min_element(roadsChannel.begin(), roadsChannel.end()) << "(MIN) "
         << (float)accumulate(roadsChannel.begin(), roadsChannel.end(), 0) / roadsChannel.size() << "(AVG)" << endl;

    cout << "[RoadArea]:" << *max_element(roadsArea.begin(), roadsArea.end()) << "(MAX) "
         << *min_element(roadsArea.begin(), roadsArea.end()) << "(MIN) "
         << (float)accumulate(roadsArea.begin(), roadsArea.end(), 0) / roadsArea.size() << "(AVG) "
         << (float)accumulate(roadsArea.begin(), roadsArea.end(), 0) << "(SUM)" << endl;
    cout << "=========================================" << endl;
    this->roadSumArea_ = accumulate(roadsArea.begin(), roadsArea.end(), 0);
}

void Solver::calc_dist() {
    vector<int> crossesID;
    for (auto &item : this->crossesMap) {
        crossesID.push_back(item.first);
    }
    // initialize
    Road * road;
    for (auto i: crossesID) {
        for (auto j: crossesID) { 
            if (i == j)
                this->distMap[i][j] = 0.0;
            else
                this->distMap[i][j] = DBL_MAX;
        }
        for(auto &edge: this->adjacents[i]) {
            road = this->roadsMap[edge.roadID];
            this->distMap[i][edge.dst] = road->length;
        }
    }
    // Floyd求最短路径
    for(auto k: crossesID) {
        for(auto i: crossesID) {
            for(auto j: crossesID) {    // 防止DBL__MAX溢出，提前判断
                if (this->distMap[i][k] == DBL_MAX || this->distMap[k][j] == DBL_MAX)
                    continue;
                if (this->distMap[i][j] > this->distMap[i][k] + this->distMap[k][j])
                    this->distMap[i][j] = this->distMap[i][k] + this->distMap[k][j];
            }
        }
    }
}

vector<int> Solver::search_path(int startPoint, int target, double carSeed, double alpha, double beta, bool isTail)
{
    MinHeap *distQueue = new MinHeap(512);
    map<int, double> dist2Vtx;
    map<int, Edge> edge2Vtx;

    dist2Vtx[startPoint] = 0.0;
    Tuple tuple(startPoint, dist2Vtx[startPoint]);
    if (distQueue->insert(tuple) == -1)
        cout << "Insert Fail" << endl;

    while (!distQueue->empty())
    {
        int vertex = distQueue->top().vertex;
        distQueue->pop();
        for (auto &edge : this->adjacents[vertex])
        {
            double carWeight = 0.0;
            int src = edge.src;
            int dst = edge.dst;
            Road *road = this->roadsMap[edge.roadID];
            if (edge.isForward)
                carWeight = road->forward_weight();
            else
                carWeight = road->backward_weight();

            double baseWeight = 0.0;
            double gamma = 1.0;
            double v = min(carSeed, road->speed);
            // 结尾车辆较少的时候，不考虑车道数的影响 倾向于选用时最短的路
            if (isTail && this->runningCars.size() < (int)this->roadSumArea_*0.03) {
                baseWeight = (double)road->length / v;
                if (carWeight < 1.0) {
                    if (carWeight <= 0.2)
                        gamma = 10;
                    else if (carWeight <= 0.3)
                        gamma = 14;    
                    else if (carWeight <= 0.4)
                        gamma = 18;
                    else if (carWeight <= 0.5)
                        gamma = 25;
                    else 
                        gamma = 40;
                    carWeight = gamma * carWeight; 
                }
            }
            else {// 考虑路的车道数影响 通道数为[1, 4] -> [1.25, 5]
                baseWeight = (double)road->length / v + 5.0 / road->numChannel;
                if (carWeight < 1.0) {
                    if (carWeight <= 0.2)
                        gamma = 10;
                    else if (carWeight <= 0.3)
                        gamma = 25;    
                    else if (carWeight <= 0.4)
                        gamma = 30;
                    else if (carWeight <= 0.5)
                        gamma = 35;
                    else
                        gamma = 50;
                    carWeight = gamma * carWeight;
                }
            }

            double nextRoadWeight = dist2Vtx[src] + (this->alpha * baseWeight) + (this->beta * carWeight);
            if (dist2Vtx.count(dst) == 0)
                dist2Vtx[dst] = MAX_WEIGHT;
            if (dist2Vtx[dst] > nextRoadWeight) {
                dist2Vtx[dst] = nextRoadWeight;
                edge2Vtx[dst] = edge;
                if (distQueue->find(dst) != -1) {
                    if (distQueue->replace(Tuple(dst, nextRoadWeight)) == -1) {
                        cout << "Replace Fail" << endl;
                        exit(-1);
                    }
                }
                else {
                    if (distQueue->insert(Tuple(dst, nextRoadWeight)) == -1) {
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

void Solver::manage_once(vector<int> &totalNewRunCars, bool isTail)
{

    for (auto &item : this->roadsMap)
    {
        Road *road = this->roadsMap[item.first];
        road->update_slice(this->waitingRouterCars);
    }
    set<int> prioNewCars;
    set<int> normNewCars;
    Car *car;
    // 根据优先级 放在set中按照ID从小到达排序，然后先上路优先级高的
    for (auto carID : totalNewRunCars)
    {
        car = this->carsMap[carID];
        if (car->priority)
            prioNewCars.insert(carID);
        else
            normNewCars.insert(carID);
    }
    // cout << "prio: " << prioNewCars.size() << ", norm: " << normNewCars.size() << endl;
    // 1. 先上路优先级高的
    for (auto carID : prioNewCars)
    {
        this->run_one_car(carID);
    }
    // 2.寻路
    this->route(isTail);
    // 3. 上路普通车辆
    for (auto carID : normNewCars)
    {
        this->run_one_car(carID);
    }

    // 计算loadState, 先清空并初始化；loadState[5] 代表 0.5<=carDensity<0.6的方向数，其他值一样。
    this->loadState.clear();
    for (int i = 0; i <= 9; i++) {
        this->loadState[i] = 0;
    }
    Road * road;
    int k;
    for (auto &item: this->roadsMap) {
        road = item.second;
        k = (int) (road->forwardDensity * 10);
        this->loadState[k] += 1;
        if (road->isDuplex) {
            k = (int) (road->backwardDensity * 10);
            this->loadState[k] += 1;
        }
    }
    int cnt = 0;
    for (auto &item: this->loadState) {
        cnt += item.second;
    }
    assert(cnt == this->laneNum);
}

bool Solver::manage_all() {

    int totalCarNum = this->carsMap.size();
    int totalCarCnt = 0, manCarCnt = 0;
    bool isTail = false, addNewCar = true;
    this->alpha = 1.0;
    this->beta = 1.0;
    this->timeSlice = 0;
    
    // man 是manage调度的简写，指需要调度的非预置车辆，prio norm 分别是优先和普通车辆。
    vector<int> manWaitRunCars;
    vector<int> tmpManWaitRunCars;
    vector<int> manPrioWaitRunCars;
    vector<int> manNormWaitRunCars;
    vector<int> manNewRunCars;
    vector<int> presetNewRunCars;
    vector<int> totalNewRunCars;
    map<int, int> tmpManPlanTimeOrdIdx;
    map<int, int> manPlanTimeOrdIdx;
    map<int, int> manPrioPlanTimeOrdIdx;
    map<int, int> manNormPlanTimeOrdIdx;

    // 把需要调度的优先级车辆和普通车辆按照planTime从小到大分别存放在vector中,
    // manPLanTimeOrdIdx 存储的是到当前planTime为止的最大车辆数在vec中的索引
    // 如果planTime不是连续的话，这里还有问题，没有处理！！！！
    int index = 0;
    for (auto &item : this->manPrioPlanTimeMap) {
        for (auto &v : item.second) {
            manPrioWaitRunCars.push_back(v);
        }
        index += item.second.size();
        manPrioPlanTimeOrdIdx[item.first] = index;
    }
    index = 0;
    for (auto &item: this->manNormPlanTimeMap) {
        for (auto &v: item.second) {
            manNormWaitRunCars.push_back(v);
        }
        index += item.second.size();
        manNormPlanTimeOrdIdx[item.first] = index;
    }
    // 初始化 tmpManWaitRunCars, 和 tmpManPlanTimeOrdIdx, 先调度优先级车辆
    tmpManWaitRunCars.assign(manPrioWaitRunCars.begin(), manPrioWaitRunCars.end());
    tmpManPlanTimeOrdIdx = manPrioPlanTimeOrdIdx;
    this->lastManPrioStartTime = this->lastPresetStartTime;
    this->runQueLength = (int)(this->roadSumArea_ * this->stage1CarDensity);
    cout << "RunQueLength: " << this->runQueLength << endl;
    cout << "Total Car Num: " << totalCarNum << endl;
    bool flags = true;
    while (true) {
  
        // 人工调度优先车辆均已上路，准备人工调度普通车辆，接着上路
        if (tmpManWaitRunCars.size() == manCarCnt && tmpManWaitRunCars.size() == manPrioWaitRunCars.size() ) {
            cout << "############################### switch to norm cars ########" << endl;
            this->lastManPrioStartTime = this->timeSlice-1;
            tmpManWaitRunCars.assign(manNormWaitRunCars.begin(), manNormWaitRunCars.end());
            tmpManPlanTimeOrdIdx = manNormPlanTimeOrdIdx;
            manCarCnt = 0;
        }

        //最后一批预置车辆上路后加大队列长度 动态调整runQueLength
        // 3 密度大于0.6的多于5条，或者出现密度大于0.7 0.8 0.9的意味着可能发生死锁，减小runQueLength
        // if (this->timeSlice > this->lastPresetStartTime) {
        //     if ((this->loadState[6] > 5 || this->loadState[7] > 1 || this->loadState[8] > 0 || 
        //         this->loadState[9] > 0) && this->runQueLength > this->roadSumArea_ * 0.1)
        //     {
        //         this->runQueLength -= 30;
        //     }
        //     // 否则线性增加runQueLength
        //     if (!(this->loadState[6] > 3 || this->loadState[7] > 0 || this->loadState[8] > 0 || 
        //         this->loadState[9] > 0) && this->runQueLength < this->roadSumArea_ * 0.15) 
        //     {
        //         this->runQueLength += 15;
        //     }
        // }
        if (this->timeSlice == this->lastPresetStartTime) {
            this->runQueLength = (int)(this->roadSumArea_ * this->stage2CarDensity);
        }

        // 2 最后一辆人工调度优先车辆上路后，预置车辆上路结束前
        // if (this->timeSlice > this->lastManPrioStartTime && this->timeSlice <= this->lastPresetStartTime) {
        //     if ((this->loadState[6] > 1 || this->loadState[7] > 0 || this->loadState[8] > 0 || 
        //         this->loadState[9] > 0) && this->runQueLength > this->roadSumArea_ * 0.05)
        //     {
        //         this->runQueLength -= 30;
        //     }
        //     // 否则线性增加runQueLength
        //     if (!(this->loadState[6] > 0 || this->loadState[7] > 0 || this->loadState[8] > 0 || 
        //         this->loadState[9] > 0) && this->runQueLength < this->roadSumArea_ * 0.06) 
        //     {
        //         this->runQueLength += 15;
        //     }
        //     if (this->timeSlice == this->lastPresetStartTime)
        //     {
        //         this->lastPresetSetoutNum = totalCarCnt; // 记录最后一批预置车辆上路时，已经出发的车数量
        //         this->runQueLength = (int)(this->roadSumArea_ * 0.06); // 同时增加runQueLength
        //     } 
        // }

        // 1 人工调度优先车辆上路结束前
        // if (this->timeSlice <= this->lastManPrioStartTime) {
        //     if ((this->loadState[6] > 1 || this->loadState[7] > 0 || this->loadState[8] > 0 || 
        //         this->loadState[9] > 0) && this->runQueLength > this->roadSumArea_ * 0.05)
        //     {
        //         this->runQueLength -= 30;
        //     }
        //     // 否则线性增加runQueLength
        //     if (!(this->loadState[6] > 0 || this->loadState[7] > 0 || this->loadState[8] > 0 || 
        //         this->loadState[9] > 0) && this->runQueLength < this->roadSumArea_ * 0.06) 
        //     {
        //         this->runQueLength += 15;
        //     } 
        // }
        
        presetNewRunCars.clear();
        manNewRunCars.clear();
        this->waitingRouterCars.clear();
        if (totalCarCnt == totalCarNum && this->runningCars.size() == 0) {
            cout << "Done" << endl;
            break;
        }
        if (totalCarCnt == totalCarNum) {
            if (!isTail) {
                cout << "########################### switch to tail" << endl;
                this->tailTime = this->timeSlice;
            }
            isTail = true;
        }
        else if(addNewCar) {   
            if (this->presetStartTimeMap.count(this->timeSlice) > 0) // 此刻有预置车辆需要上路
                presetNewRunCars = this->presetStartTimeMap[this->timeSlice];
            int maxManNewRunNum = max((this->runQueLength - (int)this->runningCars.size() - (int)presetNewRunCars.size()), 0);
            int maxEndIndex = min(manCarCnt + maxManNewRunNum, (int)tmpManWaitRunCars.size());
            for (int i = manCarCnt; i < maxEndIndex; i++)
                manNewRunCars.push_back(tmpManWaitRunCars[i]);
            if (manNewRunCars.size() > 0){ 
                // 检测planTime是否早于timeSlice
                int lastCarID = manNewRunCars[manNewRunCars.size() - 1];
                Car *lastCar = this->carsMap[lastCarID];
                if (lastCar->planTime > this->timeSlice) {
                    int lastCarIndex = tmpManPlanTimeOrdIdx[this->timeSlice];
                    manNewRunCars.clear();
                    for (int i = manCarCnt; i < lastCarIndex; i++)
                        manNewRunCars.push_back(tmpManWaitRunCars[i]);
                }
            }
        }
        totalNewRunCars.clear();
        // 把预置车辆和调度车辆合并
        for (auto &v: presetNewRunCars)
            totalNewRunCars.push_back(v);
        for (auto &v: manNewRunCars)
            totalNewRunCars.push_back(v);
        totalCarCnt += totalNewRunCars.size();
        manCarCnt += manNewRunCars.size();
        cout << this->timeSlice << " " << totalCarCnt << " " << this->runningCars.size() << " "
             << manNewRunCars.size() << " " << presetNewRunCars.size() << " | "
             << this->loadState[4] << " " << this->loadState[5] << " " << this->loadState[6] << " "
             << this->loadState[7] << " " << this->loadState[8] << " " << this->loadState[9] << " | " 
             << this->runQueLength << endl;
        
        this->manage_once(totalNewRunCars, isTail);
        this->timeSlice++;
        if (this->timeSlice > 10000)    // 发生几辆车循环的情况，退出。
            return false;
    }
    // for (auto &carID:this->managePresetCar)
    //     this->manCarsMap[carID] = this->carsMap[carID];
    cout << "lastManPrioStartTime: " << this->lastManPrioStartTime << ", lastPrestStartTime: " << this->lastPresetStartTime
        << ", tailTime: " << this->tailTime << endl;
    this->T = this->timeSlice;
    this->T_pri = this->prioEndTime - this->prioEarlyTime;
    cout << "a = " << this->a << ", T = " << this->T << ", T_pri = " << this->T_pri << endl;
    double judgeTime = this->a * this->T_pri + this->T;
    cout << "judgeTime: " << judgeTime << endl;
    return true;
}

void Solver::manage_unlock() {
    bool isDeadLock, manageOver = false;
    this->stageMap[1] = make_pair<double, double>(0.12, 0.15);
    this->stageMap[2] = make_pair<double, double>(0.10, 0.13);
    this->stageMap[3] = make_pair<double, double>(0.09, 0.11);
    this->stageMap[4] = make_pair<double, double>(0.08, 0.1);
    this->stageMap[5] = make_pair<double, double>(0.06, 0.08);

    Car *car;
    int deadLockNum = 0;
    this->stage1CarDensity = 0.14;
    this->stage2CarDensity = 0.17;
    cout << this->stage1CarDensity << "  " << this->stage2CarDensity << endl;
    clock_t start = clock();
    manageOver = this->manage_all();

    while (!manageOver) // 发生死循环的处理
    {
        for (auto &item: this->carsMap) {
            car = item.second;
            car->index = 0;
            if (!car->preset || car->managePreset) {
                car->histroyRoadsID.clear();
            }
        }
        this->alpha = 1.3;
        this->beta = 1.1;
        manageOver = this->manage_all();
    }
    
    for (auto &item: this->carsMap) {
        car = item.second;
        assert (car->histroyRoadsID.size() > 0);
        if (!car->preset || car->managePreset) {
            car->roadsIDQueue = queue<int>();
            for (auto &roadID: car->histroyRoadsID)
                car->roadsIDQueue.push(roadID);
        }
        assert (car->roadsIDQueue.size() > 0);
    }
    isDeadLock = this->judge->judge();
    double duration = (double)(clock()-start)/CLOCKS_PER_SEC;
    cout << "first solution time used: " << duration << " s." << endl; 

    while (isDeadLock && deadLockNum < 5) {
        deadLockNum++;
        // 1. 改变一些参数, 建议直接通过this改变, 目前简单分为两个阶段，预置车辆发完车前为stage1, 发完车后为stage2
        this->stage1CarDensity = this->stageMap[deadLockNum].first;
        this->stage2CarDensity = this->stageMap[deadLockNum].second;


        // 2. 把调度车辆 histroyRoadsID 和 roadsIDQueue 路径清空, 以及所有car->index=0
        for (auto &item: this->carsMap) {
            car = item.second;
            car->index = 0;
            if (!car->preset || car->managePreset) {
                car->histroyRoadsID.clear();
            }
        }
        // 3. 重新规划
        
        cout << this->stage1CarDensity << "  " << this->stage2CarDensity << endl;
        start = clock();
        manageOver = this->manage_all();
        while (!manageOver) // 发生死循环的处理
        {
            for (auto &item: this->carsMap) {
                car = item.second;
                car->index = 0;
                if (!car->preset || car->managePreset) {
                    car->histroyRoadsID.clear();
                }
            }
            this->alpha = 1.2;
            this->beta = 1.1;
            manageOver = this->manage_all();
        }

        // 4. 再次判断是否死锁
        for (auto &item: this->carsMap) {
            car = item.second;
            assert (car->histroyRoadsID.size() > 0);
            car->roadsIDQueue = queue<int>();
            for (auto &roadID: car->histroyRoadsID)
                car->roadsIDQueue.push(roadID);
            assert (car->roadsIDQueue.size() > 0);
        }
        isDeadLock = this->judge->judge();
        duration = (double)(clock()-start)/CLOCKS_PER_SEC;
        cout << deadLockNum+1 << " solution time used: " << duration << " s." << endl; 
    }
}

void Solver::route(bool isTail)
{
    double alpha = 1.0;
    double beta = 1.0;
    if (isTail)
    {
        int runningCarsNum = this->runningCars.size();
        alpha = min((double)this->runQueLength / runningCarsNum * 3, 12.0);
        beta = max((double)runningCarsNum / this->runQueLength * 2, 0.5);
        this->alpha = alpha;
        this->beta = beta;
    }
    set<int> normRouteCars;
    set<int> prioRouteCars;
    for (int carID : this->waitingRouterCars)
    {
        Car *car = this->carsMap[carID];
        if (car->priority) // 先调度优先级高的车辆
            this->route_one_car(carID, alpha, beta, isTail);
        else // 再调度一般车辆
            normRouteCars.insert(carID);
    }

    for (int carID : normRouteCars)
    {
        this->route_one_car(carID, alpha, beta, isTail);
    }
}

void Solver::route_one_car(int carID, double alpha, double beta, bool isTail)
{
    int startPoint, newRoadID;
    double leftTime;
    Road *newRoad = NULL;
    Car *car = this->carsMap[carID];
    Road *runningRoad = this->roadsMap[car->runningRoadID];
    if (car->isForwad)
    {
        startPoint = runningRoad->dst;
        leftTime = runningRoad->forwardCarTime[carID];
    }
    else
    {
        startPoint = runningRoad->src;
        leftTime = runningRoad->backwardCarTime[carID];
    }

    if (startPoint == car->dst)
    {
        car->costTime = this->timeSlice - car->startTime;
        this->runningCars.erase(carID);
        if (car->priority)
            this->prioEndTime = this->timeSlice; // 记录最后一辆优先级车结束时间
        return;
    }
    if (car->preset && !car->managePreset)
    { // 预置车辆
        newRoadID = car->histroyRoadsID[car->index++];
        car->runningRoadID = newRoadID;
        // car->histroyRoadsID.erase(car->histroyRoadsID.begin());
        newRoad = this->roadsMap[newRoadID];
        car->isForwad = newRoad->src == startPoint ? true : false;
        car->availiableSpeed = min(car->speed, newRoad->speed);
        if (car->isForwad)
            newRoad->forwardCarTime[car->id] = (double)(newRoad->length) / (car->availiableSpeed) + leftTime;
        else
            newRoad->backwardCarTime[car->id] = (double)(newRoad->length) / (car->availiableSpeed) + leftTime;
        return;
    }
    runningRoad->reverseForbidden = true;
    vector<int> shortestPath = this->search_path(startPoint, car->dst, car->speed, alpha, beta, isTail = isTail);
    runningRoad->reverseForbidden = false;
    newRoadID = shortestPath[0];
    car->runningRoadID = newRoadID;
    newRoad = this->roadsMap[newRoadID];
    assert(newRoad);
    car->histroyRoadsID.push_back(car->runningRoadID);
    car->isForwad = newRoad->src == startPoint ? true : false;
    car->availiableSpeed = min(car->speed, newRoad->speed);
    if (car->isForwad)
        newRoad->forwardCarTime[car->id] = (double)(newRoad->length) / (car->availiableSpeed) + leftTime;
    else
        newRoad->backwardCarTime[car->id] = (double)(newRoad->length) / (car->availiableSpeed) + leftTime;
}

void Solver::run_new_cars(vector<int> &totalNewRunCars)
{
    set<int> prioNewCars;
    set<int> normNewCars;
    Car *car;
    // 根据优先级 放在set中按照ID从小到达排序，然后先上路优先级高的
    for (auto carID : totalNewRunCars)
    {
        car = this->carsMap[carID];
        if (car->priority)
            prioNewCars.insert(carID);
        else
            normNewCars.insert(carID);
    }
    for (auto carID : prioNewCars)
    {
        this->run_one_car(carID);
    }
    for (auto carID : normNewCars)
    {
        this->run_one_car(carID);
    }
}

void Solver::run_one_car(int carID)
{
    double alpha = 1.0;
    double beta = 1.0;
    int newRoadID;
    Road *newRoad = NULL;
    Car *car = this->carsMap[carID];
    assert(car);
    if (car->preset && !car->managePreset)
    {
        newRoadID = car->histroyRoadsID[car->index++];
        assert(newRoadID);
        // car->histroyRoadsID.erase(car->histroyRoadsID.begin());
        newRoad = this->roadsMap[newRoadID];
        car->runningRoadID = newRoadID;
        assert(newRoad);
        car->isForwad = newRoad->src == car->src ? true : false;
        car->availiableSpeed = min(car->speed, newRoad->speed);
        if (car->isForwad)
            newRoad->forwardCarTime[car->id] = (double)newRoad->length / car->availiableSpeed;
        else
            newRoad->backwardCarTime[car->id] = (double)newRoad->length / car->availiableSpeed;
        this->runningCars.insert(carID);
        return;
    }

    car->startTime = this->timeSlice;
    vector<int> shortestPath = this->search_path(car->src, car->dst, car->speed, alpha, beta);
    car->runningRoadID = shortestPath[0];
    newRoad = this->roadsMap[shortestPath[0]];
    assert(newRoad);
    car->histroyRoadsID.push_back(shortestPath[0]);
    car->isForwad = newRoad->src == car->src ? true : false;
    car->availiableSpeed = min(car->speed, newRoad->speed);
    if (car->isForwad)
        newRoad->forwardCarTime[car->id] = (double)newRoad->length / car->availiableSpeed;
    else
        newRoad->backwardCarTime[car->id] = (double)newRoad->length / car->availiableSpeed;
    this->runningCars.insert(carID);
}

Edge::Edge() {}

Edge::Edge(int roadID, int src, int dst, double weight, bool isForwad)
{
    this->roadID = roadID;
    this->dst = dst;
    this->src = src;
    this->weight = weight;
    this->isForward = isForwad;
}

Edge::Edge(const Edge &E)
{
    this->roadID = E.roadID;
    this->dst = E.dst;
    this->src = E.src;
    this->weight = E.weight;
    this->isForward = E.isForward;
}
