import itertools
from collections import OrderedDict, defaultdict, deque
from utils import DefaultOrderedDict
from container import Car, Road, Cross, Location, Graph, MinPQ, Edge
import math

class Solver():
    def __init__(self, carsOrdDict, roadsOrdDict, crossOrdDict):
        self.carsOrdDict = carsOrdDict
        self.roadsOrdDict = roadsOrdDict
        self.crossOrdDict = crossOrdDict

        self.runningCars = set()
        self.waittingRouterCars = set()
        self.carsPlanTimeDict = defaultdict(lambda: [])
        self.maxRunCarNum = 1024    # 道路中最多可以同时在路上的车辆数
        self.timeSlice = 1
        self.forwardCrowdedRoad = set()
        self.backwardCrowdedRoad = set()
        self.build_graph()
        self.analyse()
        # self.get_road_adjacency()

    def build_graph(self):
        self.vertexes = set()
        self.edges = set()
        self.adjacents = defaultdict(lambda: set())
        # add edges
        for road in self.roadsOrdDict.values():
            self.vertexes.add(road.src)
            self.vertexes.add(road.dst)
            edgeForward = Edge(road.id, road.src, road.dst, road.baseWeight, isForward=True)
            self.adjacents[road.src].add(edgeForward)
            self.edges.add(edgeForward)
            if road.isDuplex:
                edgeBackward = Edge(road.id, road.dst, road.src, road.baseWeight, isForward=False)
                self.adjacents[road.dst].add(edgeBackward)
                self.edges.add(edgeBackward)

    # 分析 road, cross, cars planTime 获取尽可能多的全局时间空间信息
    # 如数量，道路里程数，平均速度，源和目的地分布 出发时间分布等
    def analyse(self):
        carsSpeed = []
        carsSrc = set()
        carsDst = set()
        carsBlockDistance = []
        for carID, car in self.carsOrdDict.items():
            self.carsPlanTimeDict[car.planTime].append(carID)
            carsSpeed.append(car.speed)
            carsSrc.add(car.src)
            carsDst.add(car.dst)
            srcCross, dstCross = self.crossOrdDict[car.src], self.crossOrdDict[car.dst]
            blockDistance = abs(srcCross.coordinate[0]-dstCross.coordinate[0]) + abs(srcCross.coordinate[1] - dstCross.coordinate[1])
            carsBlockDistance.append(blockDistance)
        print('############ cars info: num {0} ############'.format(len(carsSpeed)))
        print('src_num {0}, dst_num {1}; speed, min:{2}, max:{3}, avg: {4:.2f}'.format(
            len(carsSrc), len(carsDst), min(carsSpeed), max(carsSpeed), sum(carsSpeed)/len(carsSpeed)))
        print('block distance: min {0}, max {1}, avg {2}'.format(min(carsBlockDistance), max(carsBlockDistance), sum(carsBlockDistance)/len(carsBlockDistance)))
        for k, v in self.carsPlanTimeDict.items():
            print('  ', k, len(v))

        roadsSpeed = []
        roadsLength = []
        roadsMaxCarNum = []
        roadsCost = []
        roadsChannel = []
        simplexCnt = 0
        for roadID, road in self.roadsOrdDict.items():
            roadsSpeed.append(road.speed)
            roadsLength.append(road.length)
            roadsCost.append(road.baseWeight)
            roadsMaxCarNum.append(road.maxCarNum)
            roadsChannel.append(road.numChannel)
            if not road.isDuplex:
                simplexCnt += 1
        maxRoadCost = max(roadsCost)
        roadsCostNorm = []
        for roadID, road in self.roadsOrdDict.items():
            road.baseWeight = road.baseWeight / maxRoadCost # 路的基础cost 归一化
            road.normalize()
            roadsCostNorm.append(road.baseWeight)

        print('############ roads info: num {0} ############'.format(len(roadsSpeed)))
        print('Duplex: {0}, Simplex: {1}'.format(len(roadsSpeed)-simplexCnt, simplexCnt))
        print('speed: min {0}, max {1}, avg {2:.2f}'.format(min(roadsSpeed), max(roadsSpeed), sum(roadsSpeed)/len(roadsSpeed)))
        print('length: min {0}, max {1}, avg {2:.2f}'.format(min(roadsLength), max(roadsLength), sum(roadsLength)/len(roadsLength)))
        print('maxCarNum: min {0}, max {1}, avg {2:.2f}'.format(min(roadsMaxCarNum), max(roadsMaxCarNum), sum(roadsMaxCarNum)/len(roadsMaxCarNum)))
        print('Cost: min {0:.2f}, max {1:.2f}, avg {2:.2f}'.format(min(roadsCost), max(roadsCost), sum(roadsCost)/len(roadsCost)))
        print('Cost norm: min {0:.2f}, max {1:.2f}, avg {2:.2f}'.format(min(roadsCostNorm), max(roadsCostNorm), sum(roadsCostNorm)/len(roadsCostNorm)))
        print('channel: min {0}, max {1}, avg {2:.2f}'.format(min(roadsChannel), max(roadsChannel),sum(roadsChannel)/len(roadsChannel)))

        print('############ cross info: num {0} ############'.format(len(self.crossOrdDict)))
    # A*算法寻找到从startPoint 到 target的最短路径，权重最小的路
    # blockDistance作为启发距离, blockDistance = 0 时就退化为 Dijkstra算法
    def search_path(self, startPoint, target):
        distQueue = MinPQ() # 存储F, F = G + H
        dist2Vtx = defaultdict(lambda: float('inf')) # 存储G
        edge2Vtx = {}
        dist2Vtx[startPoint] = 0
        distQueue.insert(startPoint, dist2Vtx[startPoint])  # 初始只有一个点，不加H(start)也行，队列插入0即可，
        targetCross = self.crossOrdDict[target]

        def _relax(edge):
            src, dst = edge.src, edge.dst
            if edge.isForward:
                weight = self.roadsOrdDict[edge.roadID].forward_weight
            else:
                weight = self.roadsOrdDict[edge.roadID].backward_weight
            dstCross = self.crossOrdDict[dst]
            blockDistance = abs(targetCross.coordinate[0] - dstCross.coordinate[0]) + \
                            abs(targetCross.coordinate[1] - dstCross.coordinate[1])
            blockDistance *= 0.2  # blockDistance 每次寻路最多相差2 可给其乘上一个权重
            if dist2Vtx[dst] > dist2Vtx[src] + weight:
                dist2Vtx[dst] = dist2Vtx[src] + weight
                edge2Vtx[dst] = edge
                if distQueue.contains(dst):
                    distQueue.change_dist(dst, dist2Vtx[dst] + blockDistance)
                else:
                    distQueue.insert(dst, dist2Vtx[dst] + blockDistance)
        findTarget = False
        while not distQueue.is_empty():
            vertex, _ = distQueue.del_min()
            for edge in self.adjacents[vertex]:
                _relax(edge)
            if target in edge2Vtx:  # 已经找到目的地，立即停止
                # print('find target ', target, ' queue size:', distQueue.size())
                # print(distQueue.queue)
                break

        shortestPath = []
        tmpVtx = target
        while tmpVtx != startPoint:
            edge = edge2Vtx[tmpVtx]
            shortestPath.append(edge.roadID)
            tmpVtx = edge.src
        shortestPath.reverse()
        return shortestPath

    def manage_once(self, newRunCars):
        # 每次调用该函数时重新初始化等待寻路车辆集合， 在road.update_slice()会更新该集合
        self.waittingRouterCars = set()

        # 1. 更新每条道路上的车辆数
        for roadID, road in self.roadsOrdDict.items():
            road.update_slice(self.waittingRouterCars)
        # 2. 为即将到达路口的车辆规划新路径
        # 如何避免重新规划路径时双向路的调头？
        # 以及路口是否会发生冲突死锁？
        # if len(self.forwardCrowdedRoad) != 0:
        #     print("foward:",self.forwardCrowdedRoad)
        # if len(self.backwardCrowdedRoad) != 0:
        #     print("backward:",self.backwardCrowdedRoad)

        for carID in self.waittingRouterCars:
            car = self.carsOrdDict[carID]
            runningRoad = self.roadsOrdDict[car.runningRoadID]
            if car.isForward:
                startPoint = runningRoad.dst # 当前即将到达的路口
                leftTime = runningRoad.forwardCarTime[carID]    # 该路段走完需要的时间
            else:
                startPoint = runningRoad.src
                leftTime = runningRoad.backwardCarTime[carID]
            if startPoint == car.dst:   # 即将到达终点
                self.runningCars.discard(carID)
                continue

            runningRoad.reverseForbidden= True # 设置禁止调头标志
            shortestPath = self.search_path(startPoint, car.dst)
            runningRoad.reverseForbidden = False # 寻路后取消禁止调头标志
            newRoadID = shortestPath[0]
            car.runningRoadID = newRoadID
            newRoad = self.roadsOrdDict[newRoadID]
            car.histroyRoadsID.append(newRoadID)
            car.isForward = True if newRoad.src == startPoint else False
            car.availableSpeed = min(car.speed, newRoad.speed)
            if car.isForward:
                newRoad.forwardCarTime[car.id] = newRoad.length / car.availableSpeed + leftTime
            else:
                newRoad.backwardCarTime[car.id] = newRoad.length / car.availableSpeed + leftTime

        # 3. 为在当前timeSlice新上路的车规划路径
        for carID in newRunCars:
            car = self.carsOrdDict[carID]
            car.startTime = self.timeSlice
            shortestPath = self.search_path(car.src, car.dst)
            newRoadID = shortestPath[0]
            newRoad = self.roadsOrdDict[newRoadID]
            car.runningRoadID = newRoadID
            car.histroyRoadsID.append(newRoadID)
            car.isForward = True if newRoad.src == car.src else False
            car.availableSpeed = min(car.speed, newRoad.speed)
            if car.isForward:
                newRoad.forwardCarTime[car.id] = newRoad.length / car.availableSpeed
            else:
                newRoad.backwardCarTime[car.id] = newRoad.length / car.availableSpeed
            self.runningCars.add(carID) # 新上路的车辆添加到 runningCars集合中

    def preprocess(self):
        waittingRunCars = []
        waittingRunCarsDequeDict = {}
        for key, value in self.carsPlanTimeDict.items():
            tmpCars = {}
            verticalCar = []
            transverseCar = []
            for v in value:
                sourceCross = self.crossOrdDict[self.carsOrdDict[v].src]
                targetCross = self.crossOrdDict[self.carsOrdDict[v].dst]
                distance = abs(targetCross.coordinate[0]-sourceCross.coordinate[0]) + \
                           abs(targetCross.coordinate[1]-sourceCross.coordinate[1])
                tmpCars[v] = distance

                if (abs(targetCross.coordinate[0]-sourceCross.coordinate[0])>= \
                    abs(targetCross.coordinate[1]-sourceCross.coordinate[1])):
                    transverseCar.append(v)
                else:
                    verticalCar.append(v)
            waittingRunCars.extend(transverseCar)
            waittingRunCars.extend(verticalCar)
                # tmpCars[v] = self.carsOrdDict[v].speed
            # tmpCars = sorted(tmpCars.items(),key=lambda item:item[1],reverse=True)
            # waittingRunCars.extend([x[0] for x in tmpCars])

        return  waittingRunCars

    def manage_all(self):
        carNum = len(self.carsOrdDict)
        carCnt = 0

        planTimes = list(self.carsPlanTimeDict.keys())
        # 把汽车按照planTime排好序
        planTimes.sort()
        planTimeOrdIndex = {}
        cnt = 0
        for k in planTimes:
            cnt += len(self.carsPlanTimeDict[k])
            planTimeOrdIndex[k] = cnt   # waittingRunCars中以当前planTime结尾的index
        waittingRunCars = self.preprocess()
        # waittingRunCars = list(itertools.chain(*map(lambda k: self.carsPlanTimeDict[k], planTimes)))
        assert carNum == len(waittingRunCars)
        flag = 0
        while True:
            if carCnt == carNum and len(self.runningCars) == 0:
                print('done')
                break
            if carCnt == carNum:    # 已经没有新出发车辆
                newRunCars = []
                if flag == 0:
                    for road in self.roadsOrdDict.values():
                        road.gain_weight()
            else:

                newRunCarNum = 1270 - len(self.runningCars)     #
                newRunCars = waittingRunCars[carCnt: carCnt+newRunCarNum]
                if len(newRunCars) > 0:
                    lastCarID = newRunCars[-1]
                    lastCar = self.carsOrdDict[lastCarID]
                    if lastCar.planTime > self.timeSlice:   # 最后一辆车实际出发时间早于计划出发时间
                        lastCarIndex = planTimeOrdIndex[self.timeSlice] # 取到planTime是当前timeSlice的所有车辆
                        newRunCars = waittingRunCars[carCnt: lastCarIndex]

            carCnt += len(newRunCars)
            print(self.timeSlice, carCnt, len(self.runningCars), len(newRunCars))
            self.manage_once(newRunCars)
            self.timeSlice += 1

    def check_crosses(self):
        self.forwardCrowdedRoad = set()
        self.backwardCrowdedRoad = set()
        for crossID, cross in self.crossOrdDict.items():
            for roadID in cross.comeRoadIDs:
                if self.roadsOrdDict[roadID].src == crossID:
                    roadCarNum = self.roadsOrdDict[roadID].forwardCarNum
                    roadCarIDs = self.roadsOrdDict[roadID].forwardcarIDs
                    if self.roadsOrdDict[roadID].carCapcity < roadCarNum:
                        self.forwardCrowdedRoad.add(roadID)
                else:
                    roadCarNum = self.roadsOrdDict[roadID].backwardCarNum
                    roadCarIDs = self.roadsOrdDict[roadID].backwardcarIDs
                    if self.roadsOrdDict[roadID].carCapcity < roadCarNum:
                        self.backwardCrowdedRoad.add(roadID)

                # if self.roadsOrdDict[roadID].carCapcity+1 < roadCarNum:
                #     print("Blocking"," carCapcity:",self.roadsOrdDict[roadID].carCapcity)
                #     print("crossID:",crossID,"roadID:",roadID,"CarNum:",roadCarNum,":",roadCarIDs)


    def get_road_adjacency(self):
        vertexes = sorted(list(self.vertexes))
        egdes = self.edges
        mapRow = math.ceil(math.sqrt(len(vertexes)))*2+1
        mapCol = math.ceil(math.sqrt(len(vertexes)))*2+1
        gridMap = {i:[-1 for j in range(mapCol)] for i in range(mapRow)}
        coordX = list(filter(lambda x: x % 2 != 0,list(range(mapCol-1))))
        coordY = list(filter(lambda x: x % 2 != 0, list(range(mapRow-1))))

        for i in coordX:
            for j in coordY:
                curVertex = vertexes[(i-1)//2*(mapRow -1)//2+(j-1)//2]
                gridMap[i][j] = curVertex
                # 上
                try:
                    upVertex = vertexes[(i-3)//2*(mapRow -1)//2+(j-1)//2]
                except:
                    gridMap[i-1][j] = -1
                else:
                    for edge in self.adjacents[curVertex]:
                        if edge.src ==  curVertex and edge.dst == upVertex or edge.src == upVertex and edge.dst == curVertex:
                            gridMap[i-1][j] = edge.roadID
                # 下
                try:
                    downVertex = vertexes[(i+1)//2*(mapRow -1)//2+(j-1)//2]
                except:
                    gridMap[i+1][j] = -1
                else:
                    for edge in self.adjacents[curVertex]:
                        if edge.src ==  curVertex and edge.dst == downVertex or edge.src == downVertex and edge.dst == curVertex:
                            gridMap[i+1][j] = edge.roadID
                # 左
                try:
                    leftVertex = vertexes[(i-1)//2*(mapCol -1)//2+(j-3)//2]
                except:
                    gridMap[i][j-1] = -1
                else:
                    for edge in self.adjacents[curVertex]:
                        if edge.src ==  curVertex and edge.dst == leftVertex or edge.src == leftVertex and edge.dst == curVertex:
                            gridMap[i][j-1] = edge.roadID
                # 右
                try:
                    rightVertex = vertexes[(i-1)//2*(mapCol -1)//2+(j+1)//2]
                except:
                    gridMap[i][j+1] = -1
                else:
                    for edge in self.adjacents[curVertex]:
                        if edge.src ==  curVertex and edge.dst == rightVertex or edge.src == rightVertex and edge.dst == curVertex:
                            gridMap[i][j+1] = edge.roadID
        for i in range(len(gridMap)):
            for j in range(len(gridMap[0])):
                if gridMap[i][j] >= 5000: # 如果为道路
                    road = self.roadsOrdDict[gridMap[i][j]]
                    if gridMap[i+1][j] == -1 and gridMap[i-1][j] == -1: # 左右方向道路
                        if road.isDuplex:
                            if self.roadsOrdDict[gridMap[i][j]].dst == gridMap[i][j+1]:
                                road.forwardNextRoadState = [gridMap[i][j+2],gridMap[i-1][j+1],gridMap[i+1][j+1]]
                                if self.roadsOrdDict[gridMap[i][j]].src == gridMap[i][j-1]:
                                    road.backwardNextRoadState = [gridMap[i][j-2],gridMap[i-1][j-1],gridMap[i+1][j-1]]
                            elif self.roadsOrdDict[gridMap[i][j]].dst == gridMap[i][j-1]:
                                road.forwardNextRoadState = [gridMap[i][j-2],gridMap[i-1][j-1],gridMap[i+1][j-1]]
                                if self.roadsOrdDict[gridMap[i][j]].src == gridMap[i][j+1]:
                                    road.backwardNextRoadState = [gridMap[i][j+2],gridMap[i-1][j+1],gridMap[i+1][j+1]]
                            else:
                                continue
                        else:
                            if self.roadsOrdDict[gridMap[i][j]].dst == gridMap[i][j+1]:
                                road.forwardNextRoadState = [gridMap[i][j+2],gridMap[i-1][j+1],gridMap[i+1][j+1]]
                            elif self.roadsOrdDict[gridMap[i][j]].dst == gridMap[i][j-1]:
                                road.forwardNextRoadState = [gridMap[i][j-2],gridMap[i-1][j-1],gridMap[i+1][j-1]]
                            else:
                                continue

                    if gridMap[i][j-1] == -1 and gridMap[i][j+1] == -1:# 上下方向道路
                        if road.isDuplex:
                            if self.roadsOrdDict[gridMap[i][j]].dst == gridMap[i+1][j]:
                                road.forwardNextRoadState = [gridMap[i+2][j],gridMap[i+1][j+1],gridMap[i+1][j-1]]
                                if self.roadsOrdDict[gridMap[i][j]].src == gridMap[i-1][j]:
                                    road.backwardNextRoadState = [gridMap[i-2][j],gridMap[i-1][j-1],gridMap[i-1][j+1]]
                            elif self.roadsOrdDict[gridMap[i][j]].dst == gridMap[i-1][j]:
                                road.forwardNextRoadState = [gridMap[i-2][j],gridMap[i-1][j-1],gridMap[i-1][j+1]]
                                if self.roadsOrdDict[gridMap[i][j]].src == gridMap[i+1][j]:
                                    road.backwardNextRoadState = [gridMap[i+2][j],gridMap[i+1][j+1],gridMap[i+1][j-1]]
                            else:
                                continue
                        else:
                            if self.roadsOrdDict[gridMap[i][j]].dst == gridMap[i+1][j]:
                                road.forwardNextRoadState = [gridMap[i+2][j],gridMap[i+1][j+1],gridMap[i+1][j-1]]
                            elif self.roadsOrdDict[gridMap[i][j]].dst == gridMap[i][j-1]:
                                road.forwardNextRoadState = [gridMap[i-2][j],gridMap[i-1][j-1],gridMap[i-1][j+1]]
                            else:
                                continue
        return gridMap
