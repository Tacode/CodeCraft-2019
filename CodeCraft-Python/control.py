from container import Car, Road, Cross


car = Car()
road = Road()
cross = Cross()

class Control():
    def __init__(self, cars, roads, crosses):
        self.cars = cars
        self.roads = roads
        self.crosses = crosses
        self.timeSlice = 1

    def manage_road(self):  # 只调度路上的车辆数
        pass


