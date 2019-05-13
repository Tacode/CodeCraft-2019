import sys
import os
import json
import math
from collections import defaultdict, OrderedDict, deque, Counter


def get_prec_float(x):
    x *= 1000000
    x = int(x)  # 截取掉从第7位往后
    if x % 10 == 5: # 是5的话手动加1  round(2.122235, 5) = 2.12223， round(2.122236, 5) = 2.12224
        x += 1
    x /= 1000000
    return round(x, 5)

class Analyse(object):
    def __init__(self, carsDict, roadsDict, crossesDict, presetDict):
        self.carsDict = carsDict
        self.roadsDict = roadsDict
        self.crossesDict = crossesDict
        self.presetDict = presetDict

        self.carRatio = 0
        self.speedRatio = 0
        self.startTimeRatio = 0
        self.endTimeRatio = 0
        self.startLocRatio = 0
        self.endLocRatio = 0


    def analyseCar(self):
        allCarSpeed = []
        prioCarSpeed = []
        presetPrioCnt, prioCnt = 0, 0
        allCarSrc = set()
        allCarDst = set()
        prioCarSrc = set()
        prioCarDst = set()
        allCarEarlyTime, prioCarEarlyTime = float('inf'), float('inf')
        allCarLastTime, prioCarLastTime = 0, 0
        manCarsDict = {k: v for (k, v) in self.carsDict.items() if k not in self.presetDict}
        assert len(manCarsDict) + len(self.presetDict) == len(self.carsDict)
        carsPlanTimeDict = defaultdict(lambda: [])
        manPrioPlanTimeDict = defaultdict(lambda: [])
        manNormPlanTimeDict = defaultdict(lambda: [])
        presetStartTimeDict = defaultdict(lambda: [])
        presetPrioStartTimeDict = defaultdict(lambda: [])
        prioCarsDict = {}
        manPrioCarsDict = {}
        manNormCarsDict = {}

        for carID, car in self.carsDict.items():
            allCarSpeed.append(car.speed)
            allCarSrc.add(car.src)
            allCarDst.add(car.dst)
            if car.planTime < allCarEarlyTime:
                allCarEarlyTime = car.planTime
            if car.planTime > allCarLastTime:
                allCarLastTime = car.planTime
            if car.priority:
                prioCnt += 1
                prioCarSrc.add(car.src)
                prioCarDst.add(car.dst)
                prioCarSpeed.append(car.speed)
                if car.planTime < prioCarEarlyTime:
                    prioCarEarlyTime = car.planTime
                if car.planTime > prioCarLastTime:
                    prioCarLastTime = car.planTime
        for carID, car in self.presetDict.items():
            presetStartTimeDict[car.startTime].append(carID)
            if car.priority:
                presetPrioStartTimeDict[car.startTime].append(carID)
                presetPrioCnt += 1
        for carID, car in manCarsDict.items():
            carsPlanTimeDict[car.planTime].append(carID)
            if car.priority:
                manPrioCarsDict[carID] = car
                manPrioPlanTimeDict[car.planTime].append(carID)
            else:
                manNormCarsDict[carID] = car
        self.carRatio = get_prec_float(len(allCarSpeed) / prioCnt)
        self.speedRatio = get_prec_float(max(allCarSpeed) / min(allCarSpeed)) / get_prec_float(max(prioCarSpeed) / min(prioCarSpeed))
        self.speedRatio = get_prec_float(self.speedRatio)

        self.startTimeRatio = get_prec_float(allCarLastTime / allCarEarlyTime) / get_prec_float(prioCarLastTime / prioCarEarlyTime)
        self.startTimeRatio = get_prec_float(self.startTimeRatio)

        self.startLocRatio = len(set(allCarSrc)) / len(set(prioCarSrc))
        self.startLocRatio  = get_prec_float(self.startLocRatio )

        self.endLocRatio = len(set(allCarDst)) / len(set(prioCarDst))
        self.endLocRatio = get_prec_float(self.endLocRatio)
        print('################ Cars Info: num {0} ################'.format(len(allCarSpeed)))
        print('[Priority]: {0}, norm: {1}'.format(prioCnt, len(allCarSpeed)-prioCnt))
        print('[Preset]: {0}, priority {1}, norm {2}'.format(len(self.presetDict), presetPrioCnt, len(self.presetDict)-presetPrioCnt))
        print('[Manage]: {0}, priority {1}, norm {2}'.format(len(manCarsDict), len(manPrioCarsDict), len(manNormCarsDict)))
        print('[Speed], min:{0}, max:{1}, avg: {2:.2f}'.format(min(allCarSpeed), max(allCarSpeed), sum(allCarSpeed)/len(allCarSpeed)))
        allCarSpeedCnt = Counter(allCarSpeed)
        print('[CarsSpeed Distribution]: \n', allCarSpeedCnt)
        planTimeCnt = dict(map(lambda x: (x[0], len(x[1])), carsPlanTimeDict.items()))
        print('[AllCarPlanTime Distribution]: %d\n'%len(planTimeCnt), sorted(planTimeCnt.items()))
        manPrioPlanTimeCnt = dict(map(lambda x: (x[0], len(x[1])), manPrioPlanTimeDict.items()))
        print('[ManagePriorityPlanTime Distribution]: %d\n'%len(manPrioPlanTimeCnt), sorted(manPrioPlanTimeCnt.items()))
        presetStartTimeCnt = dict(map(lambda x: (x[0], len(x[1])), presetStartTimeDict.items()))
        print('[PresetCarStartTime Distribution]: %d\n'%len(presetStartTimeCnt), sorted(presetStartTimeCnt.items()))
        presetPrioStartTimeCnt = dict(map(lambda x: (x[0], len(x[1])), presetPrioStartTimeDict.items()))
        print('[PresetPriorityStartTime Distribution]: %d\n'%len(presetPrioStartTimeCnt), sorted(presetPrioStartTimeCnt.items()))

    def analyseRoad(self):
        roadsSpeed = []
        roadsLength = []
        roadsMaxCarNum = []
        roadsCost = []
        roadsChannel = []
        roadsArea = []
        simplexCnt = 0
    
        for roadID, road in self.roadsDict.items():
            roadsSpeed.append(road.speed)
            roadsLength.append(road.length)
            roadsCost.append(road.baseWeight)
            roadsMaxCarNum.append(road.maxCarNum)
            roadsChannel.append(road.numChannel)
            if road.isDuplex:
                roadsArea.append(road.area * 2)
            else:
                roadsArea.append(road.area * 1)
                simplexCnt += 1
        roadsCostNorm = []
        maxRoadCost = max(roadsCost)
        for roadID, road in self.roadsDict.items():
            road.baseWeight = road.baseWeight / maxRoadCost  # 归一化
            roadsCostNorm.append(road.baseWeight)

        print('################ Roads Info: num {0} ################'.format(len(roadsSpeed)))
        print('[Duplex]: {0}, Simplex: {1}'.format(len(roadsSpeed)-simplexCnt, simplexCnt))
        print('[Speed]: min {0}, max {1}, avg {2:.2f}'.format(min(roadsSpeed), max(roadsSpeed), sum(roadsSpeed)/len(roadsSpeed)))
        print('[Length]: min {0}, max {1}, avg {2:.2f}'.format(min(roadsLength), max(roadsLength), sum(roadsLength)/len(roadsLength)))
        print('[MaxtotalCarNum]: min {0}, max {1}, avg {2:.2f}'.format(min(roadsMaxCarNum), max(roadsMaxCarNum), sum(roadsMaxCarNum)/len(roadsMaxCarNum)))
        print('[Cost]: min {0:.2f}, max {1:.2f}, avg {2:.2f}'.format(min(roadsCost), max(roadsCost), sum(roadsCost)/len(roadsCost)))
        print('[CostNorm]: min {0:.2f}, max {1:.2f}, avg {2:.2f}'.format(min(roadsCostNorm), max(roadsCostNorm), sum(roadsCostNorm)/len(roadsCostNorm)))
        print('[Channel]: min {0}, max {1}, avg {2:.2f}'.format(min(roadsChannel), max(roadsChannel), sum(roadsChannel)/len(roadsChannel)))
        print('[Area]: min {0}, max {1}, avg {2:.2f}, total {3}'.format(min(roadsArea), max(roadsArea), sum(roadsArea)/len(roadsArea), sum(roadsArea)))
        roadsLengthCnt = Counter(roadsLength)
        roadsSpeedCnt = Counter(roadsSpeed)
        print('[Length Distribution]: ', roadsLengthCnt)
        print('[Speed Distribution]: ', roadsSpeedCnt)

    def analyseCross(self):
        crossCntForRoad = {}
        for x in range(1, 5):
            crossCntForRoad[x] = []
        for crossID, cross in self.crossesDict.items():
            crossCntForRoad[len(cross.avaliableRoadIDs)].append(crossID)
        print('#################### Cross Info: num {0} ####################'.format(len(self.crossesDict)))
        print("[ConnectRoads]: one: {}, two: {} tree: {} four: {}".format(len(crossCntForRoad[1]), len(crossCntForRoad[2]), len(crossCntForRoad[3]), len(crossCntForRoad[4])))

    def getCoefficient(self):
        a = 0.05 * self.carRatio + (self.speedRatio + self.startTimeRatio + self.startLocRatio + self.endLocRatio) * 0.2375
        b = 0.8 * self.carRatio + (self.speedRatio + self.startTimeRatio + self.startLocRatio + self.endLocRatio) * 0.05
        print('###################### Coefficient #######################')
        print("[a]: {}".format(a))
        print("[b]: {}".format(b))

class Car():
    def __init__(self, id, src, dst, speed, planTime, priority, preset):
        self.id = id
        self.src = src
        self.dst = dst
        self.speed = speed   # 最大车速
        self.planTime = planTime
        self.priority = priority
        self.preset = preset

        self.startTime = 0
        self.runSpeed = 0   # 车辆行驶速度
        self.histroyRoadsID = []  # 历史路径

        self.status = -1  # 车辆运行状态
        self.dstCross = -1  # 车辆面朝运行的路口
        self.curRoadID = -1  # 车辆运行的当前道路ID
        self.lenToDstCross = -1  # 车辆到对面路口的距离  
        self.routeIndex = 0

class Road():
    def __init__(self, id, length, speed, numChannel, src, dst, isDuplex):
        self.id = id
        self.length = length
        self.speed = speed
        self.numChannel = numChannel
        self.src = src
        self.dst = dst
        self.isDuplex = isDuplex
        self.numForwardCar = 0  # 正方向车辆总数
        # 记录每条车道上的车所在的位置 用[position, car]表示，放在deque中 如 [[1, 1001], [2, 1002], [3, 1003]...]
        # 每次车道进入一辆车 deq.appendleft(item), 驶离该车道 deq.pop()即可
        # self.channelsForward = DefaultOrderedDict(lambda: deque())  
        self.forwardCarTime = {}    # 用于存储forward方向上的车辆在该道路上的剩余行驶时间
        self.forwardDensity = 0  
        self.area = length * numChannel
        if isDuplex:
            self.numBackwardCar = 0
            # self.channelsBackward = DefaultOrderedDict(lambda: deque())
            self.backwardCarTime = {}
            self.backwardDensity = 0
        self.numTotalCar = 0
        self.reverseForbidden = False  # 用于寻路时双向路判断禁止调头
        # 路的基础花费是耗时，每条路的耗时不同，大概在[1.25, 5]之间，除以max归一化[0.25, 1]
        # 不归一化更好，暂时没有归一化
        self.baseWeight = 1 / (self.speed * self.numChannel)    
        self.maxCarNum = int(self.length * self.numChannel * 0.6)   # 该道路可承载的最大车辆数    

class Cross():
    def __init__(self, id, *roadIDs):
        self.id = id
        self.coordinate = (math.ceil(id/8), (id-1)%8 + 1) # cross 所在的位置坐标 用于计算A* 预估函数。其中8可修改
        self.roadIDs = roadIDs
        self.reachCrossIDs = [] # 从该路口出发到达的路口ID
        self.reachRoadIDs = []  # 从该路口可到达的路ID
        self.comeRoadIDs = []  # 可到达该路口的路径ID
        avaliableRoadIDs = list(filter(lambda x: x!= -1, roadIDs))   # 去掉无用的roadID 并排序
        avaliableRoadIDs.sort()
        self.avaliableRoadIDs = avaliableRoadIDs
        self.connCrossesID = [] # 与其相连的路口ID
        self.outCrossesID = [] # 从该路口出去可到达的路口ID
        self.outRoadsID = []  # 从该路口可出去的路ID
        self.inRoadsID = []   # 可进入该路口的路径ID

def read_car(car_path):
    f = open(car_path, 'r')
    f.readline()
    carsDict = OrderedDict()
    for line in f.readlines():
        line = line.replace('(', '[')
        line = line.replace(')', ']')
        item = json.loads(line)
        car = Car(*item)
        carsDict[item[0]] = car
    f.close()
    return carsDict

def read_road(road_path):
    f = open(road_path, 'r')
    f.readline()
    roadsDict = OrderedDict()
    for line in f.readlines():
        line = line.replace('(', '[')
        line = line.replace(')', ']')
        item = json.loads(line)
        road = Road(*item)
        roadsDict[item[0]] = road
    f.close()
    return roadsDict

def read_cross(cross_path, roadsDict):
    f = open(cross_path, 'r')
    f.readline()
    crossesDict = OrderedDict()
    for line in f.readlines():
        line = line.replace('(', '[')
        line = line.replace(')', ']')
        item = json.loads(line)
        cross = Cross(*item)
        connCrossesID = set()
        outArea, inArea = 0, 0
        for roadID in cross.avaliableRoadIDs:    # 把从每个路口出发可以直接到达的下一个路口ID, 路ID记录下来
            road = roadsDict[roadID]
            connCrossesID.add(road.src)
            connCrossesID.add(road.dst)
            if road.isDuplex:
                cross.outRoadsID.append(roadID)
                cross.inRoadsID.append(roadID)
                outArea += road.length * road.numChannel
                inArea += road.length * road.numChannel
                if road.src == cross.id:
                    cross.outCrossesID.append(road.dst)
                else:
                    cross.outCrossesID.append(road.src)
            elif road.src == cross.id:
                cross.outRoadsID.append(roadID)
                cross.outCrossesID.append(road.dst)
                outArea += road.length * road.numChannel
            else:
                cross.inRoadsID.append(roadID)
                inArea += road.length * road.numChannel
        cross.outArea, cross.inArea = outArea, inArea
        cross.connCrossesID = list(connCrossesID)
        crossesDict[item[0]] = cross
    f.close()
    return crossesDict

def read_preset_answer(preset_answer_path, carsDict):
    f = open(preset_answer_path)
    f.readline()
    presetDict = OrderedDict()
    for line in f.readlines():
        line = line.strip()[1:-1].split(',')
        item = list(map(int, line))
        car = carsDict[item[0]]
        car.startTime = item[1]
        car.histroyRoadsID = item[2:]
        presetDict[item[0]] = car
    f.close()
    return presetDict


def main():
    rootPath = './'
    carPath = os.path.join(rootPath, 'car.txt')
    roadPath = os.path.join(rootPath, 'road.txt')
    crossPath = os.path.join(rootPath, 'cross.txt')
    presetAnswerPath = os.path.join(rootPath, 'presetAnswer.txt')

    carsDict = read_car(carPath)
    roadsDict = read_road(roadPath)
    crossesDict = read_cross(crossPath, roadsDict)
    presetDict = read_preset_answer(presetAnswerPath, carsDict)
    analyse = Analyse(carsDict, roadsDict, crossesDict, presetDict)
    analyse.analyseCar()
    print(" ")
    analyse.analyseRoad()
    print(" ")
    analyse.analyseCross()
    print(" ")
    analyse.getCoefficient()

if __name__ == "__main__":
    main()
