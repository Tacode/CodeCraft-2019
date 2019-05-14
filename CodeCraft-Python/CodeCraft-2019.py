import logging
import sys
from solution import Solver
from read_inputs import read_car, read_cross, read_road
import time
logging.basicConfig(level=logging.DEBUG,
                    filename='../logs/CodeCraft-2019.log',
                    format='[%(asctime)s] %(levelname)s [%(funcName)s: %(filename)s, %(lineno)d] %(message)s',
                    datefmt='%Y-%m-%d %H:%M:%S',
                    filemode='a')

def main():
    start = time.time()
    if len(sys.argv) != 5:
        logging.info('please input args: car_path, road_path, cross_path, answerPath')
        exit(1)

    car_path = sys.argv[1]
    road_path = sys.argv[2]
    cross_path = sys.argv[3]
    answer_path = sys.argv[4]

    logging.info("car_path is %s" % (car_path))
    logging.info("road_path is %s" % (road_path))
    logging.info("cross_path is %s" % (cross_path))
    logging.info("answer_path is %s" % (answer_path))

    carsOrdDict = read_car(car_path)
    roadsOrdDict = read_road(road_path)
    crossOrdDict = read_cross(cross_path, roadsOrdDict)

    solver = Solver(carsOrdDict, roadsOrdDict, crossOrdDict)
    solver.manage_all()


    # write answer
    f = open(answer_path, 'w')
    for carID, car in carsOrdDict.items():
        line = '(' + str(carID) + ', ' + str(car.startTime)  + ', ' + ', '.join(map(str, car.histroyRoadsID)) + ')\n'
        f.write(line)
    f.close()
    print("Cost Time:{:.2f}s".format(time.time() - start))
# to read input file
# process
# to write output file


if __name__ == "__main__":
    main()
