import math
from collections import defaultdict, OrderedDict, deque

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


