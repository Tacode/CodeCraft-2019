import math
from collections import defaultdict, OrderedDict, deque
from utils import DefaultOrderedDict
'''
汽车状态status: 0: 在车库中还未行驶； 1:到达行驶时间等待上路； 2：等待行驶，在出路口或者前方车辆在等待行驶；
    3： 终止状态，行驶了一个时间单位， 4：结束状态，已经到达目的地。
汽车在车道的位置为 从进入开始算1，依次往后加1，
    如道路长度为10，从左往右行驶  ->  1 2 3 ... 8 9 10
    同一条车道从右往左行驶 则为   <-  10 9 8 ... 3 2 1
'''
class Car():
    def __init__(self, id, src, dst, speed, planTime):
        self.id = id
        self.src = src
        self.dst = dst
        self.speed = speed   # 最大车速
        self.planTime = planTime

        self.status = 0
        self.startTime = 0
        self.direction = None   # 车辆行驶方向， 0，1，2， 分别是直行，左转，右转。
        self.location = None    # 车辆当前所在位置，道路，车道号，哪一个位置，
        self.road = None    # 车辆正在或即将上的路
        self.curCross = self.src
        self.nextCross = 0  # 即将到达的下一个路口
        self.runSpeed = 0   # 车辆行驶速度
        self.availableSpeed = 0 # 车辆在道路上可行驶的最大速度
        self.preRoadID = None
        self.runningRoadID = None
        self.nextRoadID = None
        self.isForward = None   # 在当前道路的行驶方向
        self.histroyRoadsID = []   # 历史路径

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
        if isDuplex:
            self.numBackwardCar = 0
            # self.channelsBackward = DefaultOrderedDict(lambda: deque())
            self.backwardCarTime = {}
        self.numTotalCar = 0
        self.reverseForbidden = False  # 用于寻路时双向路判断禁止调头
        # 路的基础花费是耗时，每条路的耗时不同，大概在[1.25, 5]之间，除以max归一化[0.25, 1]
        self.baseWeight = self.length / (self.speed)
        self.maxCarNum = int(self.length * self.numChannel * 0.6)   # 该道路可承载的最大车辆数

        self.carCapcity = self.length * self.numChannel
        self.forwardNextRoadState = [-1,-1,-1] # 下一个路口的道路id [直，左，右]
        self.backwardNextRoadState = [-1,-1,-1]
        self.forwardcarIDs = []
        self.backwardcarIDs = []
        self.forwardCarNum = 0
        self.backwardCarNum = 0
        self.isForbidden = False


    def update_slice(self, waittingRouterCars):
        del_keys = []
        self.forwardcarIDs.clear()

        # self.forwardCarNum = len(self.forwardCarTime)
        for k in self.forwardCarTime.keys():
            self.forwardCarTime[k] -= 1
            if self.forwardCarTime[k] < 0: # 已经驶出该路 从该路中删除
                del_keys.append(k)
            elif self.forwardCarTime[k] < 1:  # 即将驶出该路 加入寻路集合
                waittingRouterCars.add(k)
        for k in  del_keys:
            del self.forwardCarTime[k]


        for k in self.forwardCarTime.keys():
            self.forwardcarIDs.append(k)
        self.forwardCarNum = len(self.forwardcarIDs)
        if not self.isDuplex:
            return

        del_keys = []
        self.backwardcarIDs.clear()

        for k in self.backwardCarTime.keys():
            self.backwardCarTime[k] -= 1
            if self.backwardCarTime[k] < 0:
                del_keys.append(k)
            elif self.backwardCarTime[k] < 1:  #即将驶出该路
                waittingRouterCars.add(k)
        for k in  del_keys:
            del self.backwardCarTime[k]

        for k in self.backwardCarTime.keys():
            self.backwardcarIDs.append(k)
        self.backwardCarNum = len(self.backwardcarIDs)

    def normalize(self):
        if self.baseWeight <= 0.2:
            self.baseWeight = 1
        elif self.baseWeight <= 0.4:
            self.baseWeight = 2
        elif self.baseWeight <= 0.6:
            self.baseWeight = 4
        else:
            self.baseWeight = 5
    def gain_weight(self):
        self.baseWeight *= 2

    @property
    def forward_weight(self):
        if self.reverseForbidden:   #禁止再选这条路，即禁止调头
            return float('inf')

        carNum = len(self.forwardCarTime)
        carDensity = carNum/(self.numChannel * self.length) # carDensity 是车辆密度应 < 0.6
        if self.isForbidden:
            return float('inf')
        else:
            if carNum > self.maxCarNum:   # 道路车辆数达上限， 权重给极大，不再选取该路
                return 1000 + carDensity
        # carDensity 在[0, 0.6]之间，在权重中的系数beta并不完全一样，应该是阶梯状上升的
        # carDensity越大，路越拥挤，提高权重尽量不选这辆车
            beta = 1
            if carDensity <= 0.2:
                beta = 1
            elif carDensity <= 0.3:
                beta = 2
            elif carDensity <= 0.4:
                beta = 3
            elif carDensity <= 0.5:
                beta = 4
            else:
                beta = 5
            # carWeight  = carDensity * beta
            carWeight = beta
            return self.baseWeight + carWeight     #权重包括道路固有的权重，以及路上车辆所占有的权重

    @property
    def backward_weight(self):
        if not self.isDuplex:
            return None
        if self.reverseForbidden:
            return float('inf')
        carNum = len(self.backwardCarTime)
        carDensity = carNum/(self.numChannel * self.length)
        if self.isForbidden:
            return float('inf')
        else:
            if carNum > self.maxCarNum:
                return 1000 + carDensity

            beta = 1
            if carDensity <= 0.2:
                beta = 1
            elif carDensity <= 0.3:
                beta = 2
            elif carDensity <= 0.4:
                beta = 3
            elif carDensity <= 0.5:
                beta = 4
            else:   # 0.5 < carDensity <= 0.6
                beta = 5
            # carWeight  = carDensity * beta
            carWeight = beta
            return self.baseWeight + carWeight

class Cross():
    def __init__(self, id, *roadIDs):
        self.id = id
        roadIDs = list(filter(lambda x: x!= -1, roadIDs))   # 去掉无用的roadID 并排序
        roadIDs.sort()
        self.coordinate = (math.ceil(id/8), (id-1)%8 + 1) # cross 所在的位置坐标 用于计算A* 预估函数。其中8可修改
        self.roadIDs = roadIDs
        self.reachCrossIDs = [] # 从该路口出发到达的路口ID
        self.reachRoadIDs = []  # 从该路口可到达的路ID
        self.comeRoadIDs = []   # 可到达该路口的路径ID

class Location():
    def __init__(self, roadID, channel, position, isForward):
        self.roadID = roadID
        self.channel = channel
        self.position = position
        self.isForward = isForward

class Edge():
    def __init__(self, roadID, src, dst, weight, isForward):
        self.roadID = roadID
        self.src = src
        self.dst = dst
        self.weight = weight
        self.isForward = isForward

class MinPQ(object):
    """
    最小优先队列，存储顶点到起始点的最小距离
    """
    def __init__(self):
        self.queue = [(0, 0)]
        # print("create a min Priority Queue to record the distance")

    def is_empty(self):
        return len(self.queue) == 1

    def size(self):
        return len(self.queue) - 1

    def min(self):
        return self.queue[1]

    def insert(self, vertex, new_value):
        self.queue.append((vertex, new_value))
        self.swim(self.size())

    def del_min(self):
        self.queue[1], self.queue[-1] = self.queue[-1], self.queue[1]
        temp = self.queue.pop(-1)
        self.sink(1)
        return temp

    def swim(self, index):
        while index > 1 and self.queue[index // 2][1] > self.queue[index][1]:
            self.queue[index //
                       2], self.queue[index] = self.queue[index], self.queue[
                index // 2]
            index = index // 2

    def sink(self, index):
        while 2 * index <= self.size():
            next_level = 2 * index
            if next_level < self.size() and self.queue[next_level][
                1] > self.queue[next_level + 1][1]:
                next_level += 1
            if self.queue[index][1] <= self.queue[next_level][1]:
                return
            self.queue[index], self.queue[next_level] = self.queue[
                                                            next_level], self.queue[index]
            index = next_level

    def contains(self, vertex):
        for index in range(1, len(self.queue)):
            if self.queue[index][0] == vertex:
                return True
        return False

    def change_dist(self, vertex, new_dist):
        for index in range(1, len(self.queue)):
            if self.queue[index][0] == vertex:
                self.queue[index] = (vertex, new_dist)

class Graph():
    def __init__(self):
        self.vertexes = set()
        self.edges = set()
        self.adjacents = defaultdict(lambda: set())

    def add_edge(self, road):
        self.vertexes.add(road.src)
        self.vertexes.add(road.dst)
        self.adjacents[road.src].add(road)
        if road.isDuplex:
            self.adjacents[road.dst].add(road)
        self.edges.add(road)

    def get_adjacents(self, vertex):
        return self.adjacents[vertex]
