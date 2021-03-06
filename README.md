队伍名：咸鱼打挺(西电)

队员： 支涛， 唐裕亮， 武林璐

一句话，比赛很开心，参赛体验很棒，有漂亮的小雨姐带队，玩和比赛都很开心~ ps:西北交际花军军认识了几个“小三”

本咸鱼队初赛复赛西北赛区第二，决赛手残抽中B组第五(让我哭一会，嘤嘤嘤)，我们自己跑了决赛后面的地图，十六进八调度时间2067,大概1/2概率进八强，让我再哭一会。

哈哈，这是西北赛区回来聚会的照片，那位拿了最优代码奖的咖啡还没请我们xxoo呢。

![](https://github.com/Tacode/CodeCraft-2019/raw/master/img/image2.jpeg)



此代码为决赛时使用的代码还有初赛Python代码，决赛包括了车辆调度C++代码以及车牌识别Python代码。车辆调度融合了判题器，并采取了解决死锁的相应方法；车牌识别代码线上识别精度能够达到98.3%, 代码很烂,膜各位大佬~

## 算法



#### 1.总体思路

**Dijkstra最短路径算法+动态寻路**

Dijkstra是求单点到n点的最短路径方法，我们要求的是单点到单点的最短路径，所以搜寻到目标点就可以停止，使用最小堆优化，时间复杂度可由O(n^2)变为O(nlog(n))。

**建模**

关键在于对路网建模，我们的路权函数是该车通过该路的最短时间加上该路的拥挤程度，拥挤程度是该路上的车辆密度(车辆数/路的面积), 即越拥挤越不建议选这条路，可以尽量避免死锁，即，weight = road.length/min(car.speed, road.speed)+carNum/(road.length * road.channel)，这里我们加了一个约束，每条道路上的车辆密度不会超过0.6, 超过0.6,就给该路设置一个非常大的权重避开此路。

我们这个模型是没有使用判题器的，即没有实现复杂的交通规则，全部**简化处理**，认为一辆车在一条路上都会以最大速度行驶 v = min(car.speed, road.speed),还有过路口也是很简单的在当前路以v1花费0.3s行驶完剩下的距离s1，下一条路行驶的距离s2就是以v2行驶0.7s，就这么简单粗暴。

相当于我们是不知道一辆车在当前时刻t在路上的哪条车道的哪一个位置的(实现判题器才可以知道)。我们是用简化的模型预估当前时刻每条路上有几辆车，但是，但是，它的调度时间居然灰常灰常准! haha,简直不可思议，awesome。很诡异的是后面我们实现判题器后，可以获得详细准确的路况信息(当前时刻，每条路上几辆车在哪个车道哪个位置)，再使用动态寻路反而效果大打折扣。

复赛和决赛才实现了判题器，先用python写的，然后改成C艹，特别感谢粤港澳赛区的皮卡丘队何诚慷大佬开源的可视化判题器，初赛用了他们的判题器:

[皮卡丘开源判题器](https://github.com/AkatsukiCC/huawei2019-with-visualization)
[咖啡最美代码奖](https://github.com/XavierCai1996/Huawei2019CodeCraft)
[冠军咕咕咕](https://github.com/kongroo/Huawei-CodeCraft-2019)

膜拜各位大佬，决赛很开心认识各位大佬，哈哈见到了田师傅。

**寻路**

寻路是采用动态寻路，每到一个路口都会根据**当前路况**重新寻路，即在每个路口寻的到目的地的路只使用一步，后续都会重新寻路，动态寻路有个问题是可能会调头，给当前路设置一个标志位即可避免调头。动态寻路会产生环，静态寻路不会产生环。

#### 2.发车策略

对于发车，我们并没有搞什么骚操作，设立了一个发车的队列长度$QueueLength$,同时定义了一个地图的总面积量，就是所有路的面积，双向路算两条：
$$Area = \sum_i^n RoadLength \times nChannel$$
因此，队列长度就定义为：
$$QueueLength = Area \times \xi$$
其中的$\xi$，代表地图上最多可以同时跑的车辆密度(车辆数/面积)，是一个超参数，取多大需要根据经验估计。在初赛和复赛时，能够拿到地图，因此对于这个$\xi$我们能够取一个较优的值，但对于决赛，后面不给出地图，因此只能我们靠经验给出了，由于我们后面采取了相应的应对死锁的手段，还是能够保证得到一个有效answer。

在复赛的时候，我们根据最后一辆优先车辆上路，最后一批预置车辆上路划分了三个不同阶段的$\xi$,并根据路况信息的反馈上下浮动队列长度，这是借鉴了TCP协议拥塞控制的思想，小有提升。决赛我们舍弃了这种复杂的思想，直接针对预置车辆设立了最后一辆不可调度的预置车辆前后两个阶段，每个阶段的$\xi$固定，不在上下浮动，初始设立两个阶段的$\xi$为`0.14`,`0.17`。

#### 3.车辆调度

车辆调度我们采用模拟车辆运行的方式，之前写的一个采用判题器运行的方式并没有这种方式效果好。寻路算法采用了`Dijkstra`算法，根据优先级，先调度优先级高的，然后调度普通车辆。所谓的模拟车辆运行，我们采用了一个粗略的车辆通过一段路的运行时间$forwardTime$:
$$forwardTime = \frac{RoadLength}{min(carSpeed, roadSpeed)}$$
每一个时间片减1,当该值小于0时，该车路口等待寻路，所以我们的模型是动态寻路过程，每辆车到每个路口都会根据路口信息寻路，对于有向图的权重我们设立了一个基础权重和车辆密度权重，具体的权重利用在`search_path`函数中。同时我们考虑到了脱尾问题(最后没有车辆能够加入到道路了)，在脱尾阶段，我们重新更换了权重，使寻路时更加偏重道路最短的路径，而不是车辆密度小的路径。
另外，对于自由调度的预置车辆选择问题，我们选取了最后出发的5%的预置车辆重新选路，5%的预置车辆更换出发时间，这个策略起到了很好的提升作用。

#### 4.解决死锁

应对死锁，我们将自己写的C++判题器程序融合进去，跑完我们的调度器之后，然后就进入判题程序，如果检测到死锁，那么更改两个阶段的$\xi$，重新调度。

#### 5.车牌识别 

车牌识别魔改了ResNet34网络，在第四个Block之后增加9个分支，对应的车牌中的9个字符，每个分支都是由两层全连接构成。期间，本想利用数据合成程序来生成车牌数据扩充数据集，通过训练发现效果不佳，可能合成的数据分布和官方的不太一致。因此，并没有做过多的数据增强的操作。最终得到了98.3%左右的识别精度。。

#### 6.文件构成
`CodeCraft-2019`:决赛时C++代码

`CodeCraft-Python`:初赛时Python代码

`Judge-CPP`:独立出来的C++版判题器

`Judge-python`：独立出来的Python版判题器

`Plate-Recognition`:车牌识别Python代码

`build.sh`: CodeCraft-2019代码编译脚本

`run1.sh`: CodeCraft-2019代码运行地图一脚本

`run2.sh`: CodeCraft-2019代码运行地图二脚本

