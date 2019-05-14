
class Edge(object):
    def __init__(self, source, destine, weight, roadId):
        self.source = source
        self.destine = destine
        self.weight  = weight
        self.roadId = roadId

class Graph(object):
    def __init__(self):
        self.vertices = set([])
        self.edges = set([])
        self.adjacents = {}

    def add_edge(self, edge):
        self.vertices.add(edge.source)
        self.vertices.add(edge.destine)
        if edge.source not in self.adjacents.keys():
            self.adjacents[edge.source] = set([])
        self.adjacents[edge.source].add(edge)
        self.edges.add(edge)
        # print("add edge from {} to {}, weight {}".format(edge.source, edge.destine, edge.weight))
    def remove_all_edge(self):
        self.vertices = set([])
        self.edges = set([])
        self.adjacents = {}

    def get_adjacents(self, vertex):
        # print("get the adjacent vertices of vertex {}".format(vertex))
        if vertex not in self.adjacents.keys():
            return set([])
        return self.adjacents[vertex]

    def vertex_number(self):
        return len(self.vertices)

    def edge_number(self):
        return len(self.edges)

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
        """
        弹出队列中距离最小的那个顶点
        """
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

class ShortestPath(object):
    """
    Dijsktra算法找到一个顶点到所有顶点的最短路径
    """
    def __init__(self, graph, start_point):
        """
        Args:
            graph:建立的地图
            start_point：起始点
        """
        self.dist_to = {}
        self.edge_to = {}
        for vertex in graph.vertices:
            self.dist_to[vertex] = float("inf")
        self.dist_to[start_point] = start_point
        self.start_point = start_point
        self.dist_queue = MinPQ()
        self.dist_queue.insert(start_point, self.dist_to[start_point])
        # print("insert the start point into the priority queue and initialize the distance")
        while not self.dist_queue.is_empty():
            vertex, _ = self.dist_queue.del_min()
            # print("grow the mini-distance tree by poping vertex {} from the queue".format(vertex))
            for edge in graph.get_adjacents(vertex):
                self.relax(edge)

    def relax(self, edge):
        # print("relax edge from {} to {}".format(edge.source, edge.destine))
        source = edge.source
        destine = edge.destine
        if self.dist_to[destine] > self.dist_to[source] + edge.weight:
            self.dist_to[destine] = self.dist_to[source] + edge.weight
            self.edge_to[destine] = edge
            if self.dist_queue.contains(destine):
                self.dist_queue.change_dist(destine, self.dist_to[destine])
            else:
                self.dist_queue.insert(destine, self.dist_to[destine])

    def dist_to(self, vertex):
        """
        返回起始点到vertex的最短距离
        """
        return self.dist_to[vertex]

    def path_to(self, vertex):
        """
        返回起始点到vertex的最短路径
        """
        lPath = [vertex]
        path = []
        temp_vertex = vertex
        while temp_vertex != self.start_point:
            try:
                temp_road = self.edge_to[temp_vertex].roadId
                temp_vertex = self.edge_to[temp_vertex].source
            except:
                lPath = []
                break
            else:
                lPath.append(temp_vertex)
                path.append(temp_road)
        lPath.reverse()
        path.reverse()
        return lPath,path

def get_edges(roads):
    edges = []
    for id,road in roads.items():
        edges.append([road.src, road.dst, road.length/road.speed,id])
        if road.isDuplex:
            edges.append([road.dst, road.src, road.length/road.speed,id])
    return edges

def get_nodes(crosses):
    nodes = []
    for id,cross in crosses.items():
        nodes.append(id)
    return nodes



#-------------------Test-------------------
def main(list_Grph,from_point,to_point):
    G = Graph()
    #Create an  graph
    for item in list_Grph:
        G.add_edge(Edge(item[0], item[1], item[2]))
          # you will need to the next line of code if the graph is a disorderly
          #test.add_edge(Edge(item[1], item[0],item[2]))
    print(G.edge_number())
    path = ShortestPath(G, from_point)
    distPath = path.path_to(to_point);
    print(distPath)

if __name__ == "__main__":
    edge_1=[1,2,1]
    edge_2=[2,5,1]
    edge_3=[2,4,1]
    edge_4=[2,3,2]
    edge_5=[3,4,1]
    edge_6=[3,6,2]
    edge_7=[4,6,1]
    edge_8=[6,4,1]
    list_edge =[edge_1,edge_2,edge_3,edge_4,edge_5,edge_6,edge_7,edge_8]
    main(list_edge,5,6)
