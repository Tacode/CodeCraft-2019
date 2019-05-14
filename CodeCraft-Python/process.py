from container import Car, Road, Cross, Location
from map import ShortestPath,Graph,Edge
import math
from collections import OrderedDict


class Solution():
    def __init__(self, roads, crosses):
        self.roads = roads
        self.crosses = crosses
        self.edges = []
        self.graph = Graph()
        self.__init_edges()

    def __init_edges(self):
        """
        初始化最开始的边，边权为车道长度
        """
        for id,road in self.roads.items():
            self.edges.append([road.src, road.dst, road.length, id])
            if road.isDuplex:
                self.edges.append([road.dst, road.src, road.length, id])
        for item in self.edges:
            self.graph.add_edge(Edge(*item))

    def update_edges(self,carSpeed):
        """
        不同车辆更新不同的边权
        """
        newEdges = self.edges.copy()
        for edge,newEdge in zip(self.edges,newEdges):
            newEdge[2] = edge[2]/min(carSpeed,self.roads[edge[3]].speed)
        self.graph.remove_all_edge()
        for item in newEdges:
            self.graph.add_edge(Edge(*item))

    def get_path(self, source, destine):
        """
        利用迪杰斯特拉算法得到车辆去往目的地的最短路径
        Return:
            crossPath: 用路口id表示的最短路径 list
            roadPath:  用道路id表示的最短路径 list
        """
        allPath = ShortestPath(self.graph, source)
        crossPath, roadPath = allPath.path_to(destine)

        return crossPath, roadPath

    def get_runtime(self,roadPath,carSpeed):
        """
        计算车辆运行时间
        Args:
            roadPath:车辆调度最短路径
            carSpeed:车辆运行速度
        """
        if len(roadPath) == 1:
            return math.ceil(self.roads[roadPath[0]].length/min(self.roads[roadPath[0]].speed,carSpeed))
        s1, s2 = self.roads[roadPath[0]].length, self.roads[roadPath[1]].length
        runTime = 0
        for i in range(len(roadPath)):
            runSpeed = min(self.roads[roadPath[i]].speed,carSpeed)
            if (s1 % runSpeed == 0):
                t = s1 / runSpeed
                s1 = 0
            else:
                t = math.floor(s1 / runSpeed)
                s1 = s1 - t * runSpeed
                if s1 >= self.roads[roadPath[1]].speed:
                    t = t+1
                    s1 = 0
                else:
                    t = t+1
                    s2 = s2-(min(self.roads[roadPath[i+1]].speed,carSpeed)-s1)
            runTime += t
            s1 = s2
            if i+2 > len(roadPath)-1:
                runTime += math.ceil(s1/min(self.roads[roadPath[i+1]].speed,carSpeed))
                break
            s2 = self.roads[roadPath[i+2]].length
        return int(runTime)

    def write_file(self,result:list, answerPath:str):
        f = open(answerPath,'w')
        f.write("#(card,StartTime,RoadId...)\n")
        for r in result:
            string = str(tuple(r))
            f.write(string+'\n')
        f.close()

    def set_path(self, cars:dict):
        for id, car in cars.items():
            self.update_edges(car.speed)
            cross , path = self.get_path(car.src, car.dst)
            car.set_path(path)
            car.set_cross(cross)

    def one_process(self,cars:dict, sortedCars:dict) ->list:
        """
        将车辆一辆一辆处理
        """
        result = []
        startTime = 1
        for (id,planTime) in sortedCars:
            runTime = self.get_runtime(cars[id].get_path(),cars[id].speed)
            result.append([id,startTime] + cars[id].get_path())
            startTime += runTime
            if startTime <= planTime:
                startTime = planTime
        return result

    def batch_process(self, sortedCars:dict)-> list:
        result = []
        routeList = []
        for planTime, carBatch in sortedCars.items():
            print('预计出发时间: ',planTime)
            for i, car in enumerate(carBatch):
                print([self.get_runtime(car.get_path()[:i+1],car.speed) for i in range(1,len(car.get_path()))])
                print(car.get_path())
                break
            break
        return result

    def judge_abnormal(self, roads, routeList, currRoute):
        """
        判断车辆是否堵死
        Args:
            routeList: 已经遍历车辆的最短路径
            currRoute: 当前车辆的最短路径
        """
        if len(routeList) == 0:
            return False
        for route in routeList:
            isExistSameRoute = [a-b for a,b in zip(route,currRoute)]
            if 0 in isExistSameRoute:
                sameRouteIndex = [i for i, x in enumerate(isExistSameRoute) if x==0]
                for sameIndex in sameRouteIndex:
                    if roads[route[sameIndex-1]].dst == roads[currRoute[sameIndex-1]].dst:
                        return True
                return False
            else:
                return False

class CarProcess(object):
    def __init__(self,cars:dict):
        self.cars = cars

    def divid_batches(self) -> dict:
        """
        将车辆按计划出发时间划分
        Return:
            dict(planTime: car)
        """
        carsBatches = {}
        for _, car in self.cars.items():
            if car.planTime not in carsBatches.keys():
                carsBatches[car.planTime] = [car]
            else:
                carsBatches[car.planTime].append(car)
        orderCarsBatches = OrderedDict(sorted(carsBatches.items(),key=lambda item:item[0]))
        return orderCarsBatches

    def divid_one(self) -> dict:
        """
        将车辆一辆一辆排序出发
        Return:
            dict(carId:planTime)
        """
        orderCarsOne = {}
        for id, car in self.cars.items():
            orderCarsOne[id] = car.planTime
        orderCarsOne = sorted(orderCarsOne.items(),key=lambda item:item[1])
        return orderCarsOne
