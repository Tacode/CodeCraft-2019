import os
import time
from read_inputs import read_car, read_answer, read_road, read_cross
from container import Car, Road, Cross

class Judge(object):
    def __init__(self, carsDict, roadsDict, crossesDict ):
        self.crossesDict = crossesDict
        self.roadsDict = roadsDict
        self.carsDict = carsDict
        self.garage = {}
        self.transport = {}
        for dstCross in crossesDict.keys():
            self.transport[dstCross] = {}
        self.timeSlice = float('inf')
        for carID, car in carsDict.items():
            if car.startTime < self.timeSlice:
                self.timeSlice = car.startTime
        self.totalTimePieces = 0
        self.priorTotalTime = 0
        self.priorLastTime = 0
        self.garageCnt = 0
        self.runningCnt = 0
        self.finishCnt = 0
        self.preFinishCnt = 0
        # self.outFile = open('carOnRoad.txt', 'w')

    def init(self):
        # 根据每辆车的始发地将他们放入到神奇车库中
        
        self.garage[0] = {}  # 非优先车辆
        self.garage[1] = {}  # 优先车辆

        for x in self.crossesDict.keys(): # 以cross存放
            self.garage[0][x] = []
            self.garage[1][x] = []
        for x in self.carsDict.keys():
            if self.carsDict[x].priority:#判断优先不优先
                self.garage[1][self.carsDict[x].src].append(x)
            else:
                self.garage[0][self.carsDict[x].src].append(x)
        for x in self.garage[0].keys():  # 将x表示的cross中的非优先车辆按照出发时间，id排序
            tmpList = []
            startTimeList = list(set([self.carsDict[carID].startTime for carID in self.garage[0][x]]))
            startTimeList.sort()
            for startTime in startTimeList:
                tmp = [carID for carID in self.garage[0][x] if self.carsDict[carID].startTime == startTime]
                tmp.sort()  # 按照car id大小排序
                tmpList += tmp
            self.garage[0][x] = tmpList
            
        for x in self.garage[1].keys():  # 将cross中的优先车辆按照出发时间，id排序
            tmpList = []
            startTimeList = list(set([self.carsDict[carID].startTime for carID in self.garage[1][x]]))
            startTimeList.sort()
            for startTime in startTimeList:
                tmp = [carID for carID in self.garage[1][x] if self.carsDict[carID].startTime == startTime]
                tmp.sort()  # 按照car id大小排序
                tmpList += tmp
            self.garage[1][x] = tmpList

        # 路况信息。以cross为调度关键，所有朝着cross[i]行进的马路，维护n个队列，每个队列代表一个车道
        for x in self.roadsDict.keys():
            start = self.roadsDict[x].src
            end = self.roadsDict[x].dst
            numChannels = self.roadsDict[x].numChannel
            isDuplex = self.roadsDict[x].isDuplex

            self.transport[end][start] = [[] for i in range(numChannels)]
            self.transport[end][start].append(x)

            if isDuplex:
                self.transport[start][end] = [[] for i in range(numChannels)]
                self.transport[start][end].append(x)

    def carsInsideRoad(self):
        crossIdSorted = list(self.transport.keys())
        crossIdSorted.sort()
        for curCross in crossIdSorted:  # 对每个路口都要处理
            for preCross in self.transport[curCross].keys():  # 每个路口的四条马路
                curRoad = self.transport[curCross][preCross][-1]  # 当前的马路号
                for i in range(len(self.transport[curCross][preCross]) - 1):  # 当前马路的每个车道的号码
                    for j in range(len(self.transport[curCross][preCross][i])):  # 当前车道上的每一辆车

                        curCarID = self.transport[curCross][preCross][i][j]
                        curCar = self.carsDict[curCarID]
                        v1 = min(self.roadsDict[curRoad].speed, curCar.speed)
                        s1 = curCar.lenToDstCross  # 这辆车距离路口的距离
                        if j != 0:
                            preCar = self.carsDict[self.transport[curCross][preCross][i][j - 1]]

                        # 算一下当前这个车的S1，并判断这个车有没有可能会不会过路口，会过路口的话，就先不处理，标志为1
                        # 不会过路口话就看这个车能不能往前开，可能被标志为2，也可能被标志为0两种情况
                        # self.transport[curCross][preCross][channel][i-1]表示当前车的前一辆车，但是如果这辆车是本车道第一辆车的话，这个变量是不能成立的
                        if s1 < v1:  # 这辆车可能会过路口（也包括到达终点的情况）
                            if j == 0 or preCar.status != 2:
                                curCar.status = 1
                            else:
                                curCar.lenToDstCross = preCar.lenToDstCross + 1
                                curCar.status = 2
                        else:  # 这辆车绝对不会过路口
                            # 这辆车不会被妨碍，直接开到它该去的地方
                            if j == 0 or (curCar.lenToDstCross - preCar.lenToDstCross - 1) >= v1:
                                curCar.lenToDstCross = s1 - v1
                                curCar.status = 2
                            else:  # 它的前面有车妨碍它去到该去的地方
                                preCar = self.carsDict[self.transport[curCross][preCross][i][j - 1]]  # 前面一辆车的车号
                                if preCar.status == 2:  # 这辆车前面堵着它的那辆车（前车）已经处理过了，开到前车的屁股后面去
                                    curCar.lenToDstCross = preCar.lenToDstCross + 1
                                    curCar.status = 2
                                else:  # 堵着它的前车还没有被处理过，那么要等到堵着它的前车处理了，这辆车才能得到处理
                                    curCar.status = 0

    def driveCarInitList(self,isPriority):
        crossIdSorted = list(self.transport.keys())
        crossIdSorted.sort()
        for curCross in crossIdSorted:  # 每个路口可以发车的车辆都发车
            removeList = []
            carList = []
            for carID in self.garage[1][curCross]:
                if self.carsDict[carID].startTime <= self.timeSlice:
                    carList.append(carID)
                else:
                    break
            for carID in carList:
                if self.runToRoad(carID, curCross, None, False):
                    removeList.append(carID)
                    # self.outFile.write(str(carID) + ' ' + str(self.carsDict[carID].startTime) + ' ' + str(self.carsDict[carID].preset) + '\n')
            for carID in removeList:
                self.garage[1][curCross].pop(self.garage[1][curCross].index(carID))
            # 非优先车辆上路
            if not isPriority:
                removeList = []
                carList = []
                for carID in self.garage[0][curCross]:
                    if self.carsDict[carID].startTime <= self.timeSlice:
                        carList.append(carID)
                    else:
                        break
                # carList.sort()
                for carID in carList:
                    if self.runToRoad(carID, curCross, None, False):
                        removeList.append(carID)
                        # self.outFile.write(str(carID) + ' ' + str(self.carsDict[carID].startTime) + ' ' + str(self.carsDict[carID].preset) + '\n')
                for carID in removeList:
                    self.garage[0][curCross].pop(self.garage[0][curCross].index(carID))

    def runToRoad(self,carID, curCross, expectCross=None, isOneDirection=False):
        nextRoad = self.carsDict[carID].histroyRoadsID[0]
        nextCross = ({self.roadsDict[nextRoad].src, self.roadsDict[nextRoad].dst} - {curCross})
        assert len(nextCross)==1, print('next road wrong')
        nextCross = nextCross.pop()
        if isOneDirection:
            # if curNextCross != next_cross:
            if self.transport[expectCross][curCross][-1] != nextRoad:
                return False

        v = min(self.roadsDict[nextRoad].speed, self.carsDict[carID].speed)
        for i in range(self.roadsDict[nextRoad].numChannel):
            cur_channel_L = self.transport[nextCross][curCross][i][:]  # cur_channel_L表示对应马路上某一条车道上的全部车辆列表
            # 这条车道是空的，或者最后一辆车不会阻碍waiting_car，那么直接把waiting_car开到它该去的地方
            if not cur_channel_L or (self.carsDict[cur_channel_L[-1]].lenToDstCross < (self.roadsDict[nextRoad].length - v)):
                self.transport[nextCross][curCross][i].append(carID)
                new_s = self.roadsDict[nextRoad].length - v
                self.updateCarInfo(curCross, nextCross, new_s, carID)
                return True
            else:  # 车道上有车阻挡（包括满或不满，最后一辆为end或waitting状态）
                if self.carsDict[cur_channel_L[-1]].status == 2:
                    if self.carsDict[cur_channel_L[-1]].lenToDstCross < self.roadsDict[nextRoad].length - 1:
                        self.transport[nextCross][curCross][i].append(carID)
                        new_s = self.carsDict[cur_channel_L[-1]].lenToDstCross + 1
                        self.updateCarInfo(curCross, nextCross, new_s, carID)
                        return True
                    else:
                        if i == self.roadsDict[nextRoad].numChannel - 1:  # 直到最后一条车道都满了,waiting_car以为道路堵塞而不能发车
                            return False
                else:
                    return False

    def updateCarInfo(self,curCross, nextCross, s, carID):
        if self.carsDict[carID].curRoadID != self.transport[nextCross][curCross][-1]:
            self.carsDict[carID].routeIndex += 1
        self.carsDict[carID].dstCross = nextCross
        self.carsDict[carID].curRoadID = self.transport[nextCross][curCross][-1]
        self.carsDict[carID].lenToDstCross = s
        self.carsDict[carID].status = 2

    def carsNotFinishedState(self):
        waittingCars = []
        for curCross in self.transport.keys():  # 对每个路口都要处理
            for preCross in self.transport[curCross].keys():  # 每个路口的四条马路
                for i in range(len(self.transport[curCross][preCross][:-1])):  # 当前马路的每个车道的号码
                    for j in range(len(self.transport[curCross][preCross][i])):  # 当前车道上的每一辆车
                        carID = self.transport[curCross][preCross][i][j]
                        if self.carsDict[carID].status != 2:
                            waittingCars.append(carID)
        return waittingCars

    # 处理每条马路上可能会过路口的车，也就是状态为1的车辆
    def carsAcrossRoad(self):  # 处理每条马路上可能会过路口的车，也就是状态为1的车辆
        crossIdSorted = list(self.transport.keys())
        crossIdSorted.sort()
        for curCross in crossIdSorted:  # 每个路口
            item = []
            for x in self.transport[curCross].keys():
                item.append([x, self.transport[curCross][x][-1]])
            item = sorted(item, key=lambda x: x[1]) # 保存先前路口, 当前道路

            for preCross, curRoad in item:  # 当前路口的每一条马路
                flag = True  # flag代表当前这条马路上要出路口的车还有没有调度的必要
                while flag:
                    # 当前这个马路上有很多车道和很多的车，我们要找到过马路并且优先级最高的车，也就是 priorCar的位置
                    channelId, position = self.highestPriorCar(curCross, preCross)
                    if channelId == -1 and position == -1:  # 直接跳过这条马路
                        flag = False
                        break
                    assert position == 0  # position不为0表示道路最前方
        
                    pirorCar = self.transport[curCross][preCross][channelId][position]
                    direction, nextCross, nextRoad = self.turningOfCar(pirorCar, curCross, curRoad)
                    if self.isConflict(direction, self.carsDict[pirorCar].priority, curCross, curRoad):  # 这辆车会因为其他马路上的车要过马路而被堵塞
                        flag = False
                        break
                    if self.isMoveToEndState(pirorCar, curCross, preCross, channelId, nextRoad):
                        self.driveSingleCross(preCross, curCross)  # 单道路单方向优先车辆上路
                    else:
                        flag = False
                        break

    # 当前马路上要过路口并且优先级最高的车
    def highestPriorCar(self,curCross, preCross):
        forefrontCars = {} #存放等待出路口车辆以及该车位于第几个车道
        carsList = [] #存放等待出路口的车辆列表
        targetCar = -1
        channelId = -1
        position = -1  # 用这个变量来记录过马路并且优先级最高的车的位置
        for i in range(len(self.transport[curCross][preCross]) - 1):  # 每条车道
            if self.transport[curCross][preCross][i]:
                carID = self.transport[curCross][preCross][i][0]
                if self.carsDict[carID].status is 1:
                    forefrontCars[carID] = i  # 每条车道上最前方等待出路口的的车辆
                    carsList.append(carID)
        # sorted(carsList)
        isPrior = [self.carsDict[carID].priority for carID in carsList]
        minLocation = float('inf')
        if carsList:
            if sum(isPrior) ==1:  # 只有一个优先车辆，必定为优先级最高的车辆
                targetCar = carsList[isPrior.index(1)]
            elif sum(isPrior) > 1:  # 有多辆优先车辆
                positionList = [self.carsDict[carID].lenToDstCross for carID in carsList]
                for i in range(len(carsList)):
                    if (isPrior[i] == 1) and (positionList[i] < minLocation):  # 位置最靠前，车道号最小的优先车辆为优先级最高的车辆
                        targetCar = carsList[i]
                        minLocation = positionList[i]
            else:
                positionList = [self.carsDict[carID].lenToDstCross for carID in carsList]
                for i in range(len(carsList)):
                    if positionList[i] is min(positionList):
                        targetCar = carsList[i]
                        break
        if targetCar is not -1:
            channelId, position = forefrontCars[targetCar], 0
        return channelId, position

    def driveSingleCross(self, preCross, curCross):
        removeList = []
        carList = []
        for carID in self.garage[1][preCross]:  # 优先车辆
            if self.carsDict[carID].startTime <= self.timeSlice:
                carList.append(carID)
            else:
                break

        for carID in carList:
            if self.runToRoad(carID, preCross, curCross, True):
                removeList.append(carID)
                # self.outFile.write(str(carID) + ' ' + str(self.carsDict[carID].startTime) + ' ' + str(self.carsDict[carID].preset) + '\n')
        for carID in removeList:
            self.garage[1][preCross].pop(self.garage[1][preCross].index(carID))


    def getTurnInfo(self, idx, curCross, curRoad):
        # idx表示从当前道路开始，按照顺时针方向第idx个道路的顺位，取值为{1,2,3}
        roadIndex = (self.crossesDict[curCross].roadIDs.index(curRoad) + idx) % 4
        info = []
        curRoad = self.crossesDict[curCross].roadIDs[roadIndex]
        if curRoad != -1:
            preCross = -1
            for x in self.transport[curCross].keys():  # 判断第idx个道路朝向cur_cross的道路是否存在
                if self.transport[curCross][x][-1] == curRoad:
                    preCross = x
                    break
            if preCross != -1:
                channelId, position = self.highestPriorCar(curCross, preCross)
                if channelId == -1 and position == -1: return info
                carID = self.transport[curCross][preCross][channelId][position]
                direction, nextCross, nextRoad = self.turningOfCar(carID, curCross, curRoad)
                info += [carID, direction, self.carsDict[carID].priority, idx] #[车辆ID，转向，是否优先级车，idx]
        return info

    def isConflict(self, direction, isPrior, curCross, curRoad):
        info = {}
        for idx in [1,2,3]:
            tmp = self.getTurnInfo(idx, curCross, curRoad)
            if tmp:
                info[tmp[0]] = tmp[1:]

        for carID in info:
            if isPrior:  # 当前车辆是优先车辆
                if direction == 0:  # 直行
                    return False
                elif direction == -1:  # 左转
                    if info[carID][-1] == 3 and info[carID][0] == 0 and info[carID][1]:  # 顺时针第3辆车直行且优先
                        return True
                else:  # 右转
                    if info[carID][-1] == 1 and info[carID][0] == 0 and info[carID][1]:  # 顺时针第1辆车直行且优先
                        return True
                    if info[carID][-1] == 2 and info[carID][0] == -1 and info[carID][1]:  # 顺时针第2辆车左转且优先
                        return True
            else:  # 当前车辆非优先
                if direction == 0:  # 当前车辆直行
                    if info[carID][-1] == 1 and info[carID][0] == -1 and info[carID][1]:  # 顺时针第1辆车左转且优先
                        return True
                    if info[carID][-1] == 3 and info[carID][0] == 1 and info[carID][1]:  # 顺时针第3辆车右转且优先
                        return True
                elif direction == -1:  # 当前车辆左转
                    if info[carID][-1] == 3 and info[carID][0] == 0:  # 顺时针第3辆车直行
                        return True
                    if info[carID][-1] == 2 and info[carID][0] == 1 and info[carID][1]:  # 顺时针第2辆车右转且优先
                        return True
                else:  # 当前车辆右转
                    if info[carID][-1] == 1 and info[carID][0] == 0:  # 被顺时针第1个道路的直行车辆阻碍
                        return True
                    if info[carID][-1] == 2 and info[carID][0] == -1:  # 被顺时针第2个道路的左转车辆阻碍
                        return True
        return False

    # 判断每辆车的转向
    def turningOfCar(self, curCar, curCross, curRoad):
        if self.carsDict[curCar].dst == curCross:  # cur_car当前朝向的路口就是它的终点
            return 0, -1, -1

        roadIndex = self.carsDict[curCar].routeIndex
        nextRoad = self.carsDict[curCar].histroyRoadsID[roadIndex]
        nextCross = ({self.roadsDict[nextRoad].src, self.roadsDict[nextRoad].dst} - {curCross})
        assert len(nextCross) == 1, print('next road wrong',nextCross, curCar, curRoad,curCross)
        nextCross = nextCross.pop()

        try:
            index1 = self.crossesDict[curCross].roadIDs.index(curRoad)
            index2 = self.crossesDict[curCross].roadIDs.index(nextRoad)
        except:
            print(curCar, curRoad, curCross, nextRoad, nextCross)

        if (index1 - index2) % 2 == 0:  # 直行
            return 0, nextCross, nextRoad
        elif (index1 - index2 == -1) or (index1 == 3 and index2 == 0):  # 左转
            return -1, nextCross, nextRoad
        else:  # 右转
            return 1, nextCross, nextRoad

    # 当我们把一辆车过了马路后，与它同车道的后续车辆状态和位置可能都要跟着改变
    def updateFollowingCars(self, curCross, preCross, channelId):
        curRoad = self.transport[curCross][preCross][-1]
        for i in range(len(self.transport[curCross][preCross][channelId])):
            curCar = self.transport[curCross][preCross][channelId][i]
            v = min(self.roadsDict[curRoad].speed, self.carsDict[curCar].speed)

            if self.carsDict[curCar].status == 2:  # 已经处理过了（不出路口）、
                continue
            elif self.carsDict[curCar].status == 1:  # 等待处理（有可能出路口）
                if i == 0:  # 没有前车,等待过马路
                    break
                assert self.carsDict[self.transport[curCross][preCross][channelId][i - 1]].status == 2
                self.carsDict[curCar].lenToDstCross = self.carsDict[self.transport[curCross][preCross][channelId][i - 1]].lenToDstCross + 1
                self.carsDict[curCar].status = 2
            else:  # 不出路口，还没有被处理
                if i == 0 or self.carsDict[curCar].lenToDstCross - self.carsDict[self.transport[curCross][preCross][channelId][i - 1]].lenToDstCross > v:  # 不会被挡
                    self.carsDict[curCar].lenToDstCross -= v
                    self.carsDict[curCar].status = 2
                else:  # 前面会被挡
                    x = self.transport[curCross][preCross][channelId][i - 1]  # 挡住它的车肯定是终止状态
                    assert self.carsDict[x ].status == 2
                    self.carsDict[curCar].lenToDstCross =  self.carsDict[x].lenToDstCross + 1
                    self.carsDict[curCar].status = 2


    def isMoveToEndState(self, carID, curCross, preCross, channelId, nextRoad):
        if self.carsDict[carID].dst == curCross:  # 这辆车到达了终点
            self.finishCnt += 1
            self.transport[curCross][preCross][channelId].pop(0)
            self.updateFollowingCars(curCross, preCross, channelId)
            self.totalTimePieces += (self.timeSlice - self.carsDict[carID].planTime)
            if self.carsDict[carID].priority:
                self.priorTotalTime += (self.timeSlice - self.carsDict[carID].planTime)
                self.priorLastTime = self.timeSlice
            return True

        v2 = min(self.roadsDict[nextRoad].speed, self.carsDict[carID].speed)
        s2 = v2 - self.carsDict[carID].lenToDstCross
        if s2 <= 0:  # 下条道路的行驶距离为0
            self.carsDict[carID].lenToDstCross= 0
            self.carsDict[carID].status = 2
            self.updateFollowingCars(curCross, preCross, channelId)
            return True

        next_cross = ({self.roadsDict[nextRoad].src, self.roadsDict[nextRoad].dst} - {curCross})
        assert len(next_cross) == 1, print('next road wrong')
        next_cross = next_cross.pop()
        # nextRoad = transport[next_cross][curCross][-1]
        v = min(self.roadsDict[nextRoad].speed, self.carsDict[carID].speed)

        next_road_length = self.roadsDict[nextRoad].length
        for i in range(self.roadsDict[nextRoad].numChannel):  # 看下一条马路的每个车道能不能把pirorCar塞得下
            # 这条车道是空的，或者说这条车道的最后一辆车不会阻碍pirorCar，那么这辆车直接开过去它该去的地方，并且置为终结状态
            if not self.transport[next_cross][curCross][i] or self.carsDict[self.transport[next_cross][curCross][i][-1]].lenToDstCross < next_road_length - s2:
                new_s = next_road_length - s2
                self.updateCarInfo(curCross, next_cross, new_s, carID)
                self.transport[curCross][preCross][channelId].pop(0)
                self.transport[next_cross][curCross][i].append(carID)
                self.updateFollowingCars(curCross, preCross, channelId)
                return True
            else:  # 这条车道有车，且阻碍了当前车辆
                if self.carsDict[self.transport[next_cross][curCross][i][-1]].status == 2:  # 挡住pirorCar的车已经是终结状态了
                    if self.carsDict[self.transport[next_cross][curCross][i][-1]].lenToDstCross< next_road_length - 1:  # 这条车道还塞得下pirorCar
                        new_s = self.carsDict[self.transport[next_cross][curCross][i][-1]].lenToDstCross + 1
                        self.updateCarInfo(curCross, next_cross, new_s, carID)
                        self.transport[curCross][preCross][channelId].pop(0)
                        self.transport[next_cross][curCross][i].append(carID)
                        self.updateFollowingCars(curCross, preCross, channelId)
                        return True
                    else:
                        if i == self.roadsDict[nextRoad].numChannel - 1:  # 直到最后一个车道都塞不下pirorCar，那就把这辆车开到原来那条马路的最前面
                            self.carsDict[carID].lenToDstCross = 0
                            self.carsDict[carID].status = 2
                            self.updateFollowingCars(curCross, preCross, channelId)
                            return True
                else:  # 挡住它的车不是终结状态，还未处理，那么这辆车也不能进行任何处理，停在原地，并且整条马路都不会再进行任何处理
                    # flag = False
                    return False


    def isFinished(self):
        self.garageCnt = 0
        self.runningCnt = 0
        for x in self.garage[0].keys():
            self.garageCnt += len(self.garage[0][x])
        for x in self.garage[1].keys():
            self.garageCnt += len(self.garage[1][x])

        for cur_cross in self.transport.keys():
            for pre_cross in self.transport[cur_cross].keys():
                for channel in self.transport[cur_cross][pre_cross][:-1]:
                    self.runningCnt += len(channel)
        if self.garageCnt + self.runningCnt == 0:
            return True
        else:
            return False

    def judge(self):
        self.timeSlice -= 1
        while True:
            self.timeSlice += 1
            self.preFinishCnt = self.finishCnt
            
            # 新的一个时间片调度开始了，所有的车辆状态置为-1
            for x in self.carsDict.keys():
                self.carsDict[x].status = -1

            self.carsInsideRoad()  # 车辆标定与调度
            self.driveCarInitList(True)  # 优先车辆上路
            while True:
                waittingCars = self.carsNotFinishedState()
                self.carsAcrossRoad()
                newWaittingCars = self.carsNotFinishedState()
                if len(newWaittingCars) == 0:
                    break
                elif len(newWaittingCars) == len(waittingCars):
                    # for carID in waittingCars:
                    #     print('car:{}  next_cross: {}, curr_road:{}, s1: {}, status:{}'.
                    #         format(carID,self.carsDict[carID].dstCross,self.carsDict[carID].curRoadID, self.carsDict[carID].lenToDstCross
                    #                 ,self.carsDict[carID].status))
                    raise ValueError('dead lock! at time: {}'.format(self.timeSlice))
            # 第T个时间片，看看每个路口有没有车能够上路(出发时间大于T的车辆一定不能上路，小于等于T的车辆有可能上路)
            #carsInGarage(transport, garage, dist, Map, carSchedule, T, cars, roads, carRoute)
            
            self.driveCarInitList(False)  # 未上路车辆上路
            if self.isFinished():
                print('{0}  running: {1},  garage: {2}, finished: {3} +{4}'.format(self.timeSlice, self.runningCnt, self.garageCnt, self.finishCnt, self.finishCnt-self.preFinishCnt))
                break
            else:
                print('{0}  running: {1},  garage: {2}, finished: {3} +{4}'.format(self.timeSlice, self.runningCnt, self.garageCnt, self.finishCnt, self.finishCnt-self.preFinishCnt))
        
        self.printResult()

    def printResult(self):
        priorFirstTime = float('inf')
        for carID,car in self.carsDict.items():
            if car.priority and car.planTime < priorFirstTime:
                priorFirstTime = car.planTime

        print('ScheduleFinished')
        priT = self.priorLastTime - priorFirstTime
        print('piror: schedule time: {}, all schedule time: {}'.format(priT, self.priorTotalTime))
        print('total: schedule time: {}, all schedule time: {}'.format(self.timeSlice, self.totalTimePieces))

        # 计算a
        priNum = 0
        speed = []
        priSpeed = []
        startTime = []
        priStartTime = []
        startLocation = []
        endLocation = []
        priStartLocation = []
        priEndLocation = []
        for carID, car in self.carsDict.items():
            if car.priority:  # 是否优先
                priNum += 1
                priSpeed.append(car.speed)
                priStartLocation.append(car.src)
                priStartTime.append(car.planTime)
                priEndLocation.append(car.dst)
            speed.append(car.speed)
            startLocation.append(car.src)
            startTime.append(car.planTime)
            endLocation.append(car.dst)

        # 获取5位精准小数
        def get_prec_float(x):
            x *= 1000000
            x = int(x)  # 截取掉从第7位往后
            if x % 10 == 5: # 是5的话手动加1  round(2.122235, 5) = 2.12223， round(2.122236, 5) = 2.12224
                x += 1
            x /= 1000000
            return round(x, 5)

        carRatio = get_prec_float(len(self.carsDict) / priNum)
        speedRatio= get_prec_float(max(speed) / min(speed)) / get_prec_float(max(priSpeed) / min(priSpeed))
        speedRatio = get_prec_float(speedRatio)
        startTimeRatio = get_prec_float(max(startTime) / min(startTime)) / get_prec_float(max(priStartTime) / min(priStartTime))
        startTimeRatio = get_prec_float(startTimeRatio)
        startLocRatio = len(set(startLocation)) / len(set(priStartLocation))
        startLocRatio = get_prec_float(startLocRatio)
        endLocRatio = len(set(endLocation)) / len(set(priEndLocation))
        endLocRatio = get_prec_float(endLocRatio)

        a = 0.05 * carRatio + (speedRatio + startTimeRatio + startLocRatio + endLocRatio) * 0.2375
        # a = round(a, 5)
        # a = get_prec_float(a)
        b = 0.8 * carRatio + (speedRatio + startTimeRatio + startLocRatio + endLocRatio) * 0.05
        # b = get_prec_float(b)
        # b = round(b, 5)
        print('a is {}'.format(a))
        print('weighted: schedule time: {}'.format(round(self.timeSlice + a*priT)))
        print('b is {}'.format(b))
        print('weighted: all schedule time: {}'.format(round(self.totalTimePieces + b * self.priorTotalTime)))
