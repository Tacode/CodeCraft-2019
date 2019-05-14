import json
from collections import OrderedDict, defaultdict
from container import Car, Road, Cross

def read_car(car_path):
    f = open(car_path, 'r')
    f.readline()
    carsOrdDict = OrderedDict()
    for line in f.readlines():
        line = line.replace('(', '[')
        line = line.replace(')', ']')
        item = json.loads(line)
        car = Car(*item)
        carsOrdDict[item[0]] = car
    f.close()
    return carsOrdDict

def read_road(road_path):
    f = open(road_path, 'r')
    f.readline()
    roadsOrdDict = OrderedDict()
    for line in f.readlines():
        line = line.replace('(', '[')
        line = line.replace(')', ']')
        item = json.loads(line)
        road = Road(*item)
        roadsOrdDict[item[0]] = road
    f.close()
    return roadsOrdDict

def read_cross(cross_path, roadsOrdDict):
    f = open(cross_path, 'r')
    f.readline()
    crossesOrdDict = OrderedDict()
    for line in f.readlines():
        line = line.replace('(', '[')
        line = line.replace(')', ']')
        item = json.loads(line)
        cross = Cross(*item)
        for roadID in cross.avaliableRoadIDs:    # 把从每个路口出发可以直接到达的下一个路口ID, 路ID记录下来
            road = roadsOrdDict[roadID]
            if road.isDuplex:
                cross.reachRoadIDs.append(roadID)
                cross.comeRoadIDs.append(roadID)
                if road.src == cross.id:
                    cross.reachCrossIDs.append(road.dst)
                else:
                    cross.reachCrossIDs.append(road.src)
            elif road.src == cross.id:
                cross.reachRoadIDs.append(roadID)
                cross.reachCrossIDs.append(road.dst)
            else:
                cross.comeRoadIDs.append(roadID)
        crossesOrdDict[item[0]] = cross
    f.close()
    return crossesOrdDict


def read_answer(carsDict:dict, answerPath:str, presetAnswerPath:str):
    f = open(presetAnswerPath)
    line = f.readline()
    line = f.readline()
    carNum = 0
    while line:
        line = line.strip()
        line = line[1:len(line) - 1].replace(' ', '')
        item = line.split(',')
        carsDict[int(item[0])].startTime = int(item[1])
        roadRoute = []
        for i in range(2, len(item)):
            roadRoute.append(int(item[i]))
        carsDict[int(item[0])].histroyRoadsID = roadRoute
        line = f.readline()
        carNum += 1
    f.close()

    f = open(answerPath)
    line = f.readline()
    while line:
        line = line.strip()
        line = line[1:len(line) - 1].replace(' ', '')
        item = line.split(',')
        carsDict[int(item[0])].startTime = int(item[1])
        roadRoute = []
        for i in range(2, len(item)):
            roadRoute.append(int(item[i]))
        carsDict[int(item[0])].histroyRoadsID = roadRoute
        line = f.readline()
        carNum += 1
        assert carsDict[int(item[0])].startTime >= carsDict[int(item[0])].planTime
    f.close()

    

