import os
from solution import Solver
from read_inputs import read_car, read_cross, read_road
import time
def main():
    start = time.time()
    train_map1_path = ['train_map/train1/car.txt',
                        'train_map/train1/road.txt',
                        'train_map/train1/cross.txt' ]

    train_map2_path = ['train_map/train2/car.txt',
                        'train_map/train2/road.txt',
                        'train_map/train2/cross.txt' ]

    use_map1 = False
    train_map_path = train_map1_path if use_map1 else train_map2_path

    car_path = train_map_path[0]
    road_path = train_map_path[1]
    cross_path = train_map_path[2]


    answer_path = os.path.join(os.path.dirname(train_map_path[0]), 'answer.txt')

    carsOrdDict = read_car(car_path)
    roadsOrdDict = read_road(road_path)
    crossOrdDict = read_cross(cross_path, roadsOrdDict)

    solver = Solver(carsOrdDict, roadsOrdDict, crossOrdDict)
    solver.manage_all()

    # write answer
    loopCarCnt = 0
    f = open(answer_path, 'w')
    for carID, car in carsOrdDict.items():
        if len(set(car.histroyRoadsID)) != len(car.histroyRoadsID):
            loopCarCnt += 1
        line = '(' + str(carID) + ', ' + str(car.startTime)  + ', ' + ', '.join(map(str, car.histroyRoadsID)) + ')\n'
        f.write(line)
    f.close()
    print('loopCarCnt', loopCarCnt)
    if use_map1:
        print('  map 1 done')
    else:
        print('  map 2 done')
    print("Cost Time:{:.2f}s".format(time.time()-start))
if __name__ == '__main__':
    main()
