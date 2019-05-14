import os
from judge import Judge
from read_inputs import read_car, read_cross, read_road, read_answer
import time

def main():
    trainIndex = 2
    rootPath = '../../exam_map/train' + str(trainIndex)
    carPath = os.path.join(rootPath, 'car.txt')
    roadPath = os.path.join(rootPath, 'road.txt')
    crossPath = os.path.join(rootPath, 'cross.txt')
    answerPath = os.path.join(rootPath, 'answer.txt')
    presetAnswerPath = os.path.join(rootPath, 'presetAnswer.txt')
    start = time.time()
    
    carsDict = read_car(carPath)
    roadsDict = read_road(roadPath)
    crossesDict = read_cross(crossPath, roadsDict)
    read_answer(carsDict, answerPath, presetAnswerPath)
    judge = Judge(carsDict, roadsDict, crossesDict)
    judge.init()
    judge.judge()
        
    print("total run time: ", time.time() - start)
    print("map {} done!".format(trainIndex))

if __name__ == '__main__':
    main()
