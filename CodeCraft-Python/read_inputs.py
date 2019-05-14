import json
from collections import OrderedDict, defaultdict
from container import Car, Road, Cross, Location

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
        for roadID in cross.roadIDs:    # 把从每个路口出发可以直接到达的下一个路口ID, 路ID记录下来
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

