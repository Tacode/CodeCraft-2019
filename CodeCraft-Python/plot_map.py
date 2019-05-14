import os
import json
from graphviz import Digraph, Graph

def main(road_path):
    g = Graph('G')
    f = open(road_path, 'r')
    f.readline()
    for line in f.readlines():
        line = line.replace('(', '[')
        line = line.replace(')', ']')
        item = json.loads(line)
        g.edge(str(item[4]), str(item[5]))
        # if item[-1]:
        #     g.edge(str(item[5]), str(item[4]))
    f.close()
    g.view()

if __name__ == '__main__':
    road_path = '../../train_map/train1/road.txt'
    main(road_path)
