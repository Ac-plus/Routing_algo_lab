# 路由协议算法的设计与实现


> - 天津大学计算机网络——路由算法实验
> - 分数：100

# 一、报告摘要

本实验的主要目标为实现模拟距离矢量路由协议（Distance Vector，以下简称DV路由协议）的算法。通过使用C语言编写相关程序，通过网络仿真测试，能够实现路由数据收发、最小距离求解等功能。

在协议设计及实现方面，通过阅读实验指导书，结合已经给出的代码框架，对路由节点初始化（rtinit）、路由节点更新（rtupdate）和路由传输的最短距离求解等进行了编程设计。该项目完成后，能够模拟DV路由协议的功能，实现网络仿真下的数据传输。

# 二、协议设计

## **2.1** DV路由协议原理

### **2.1.1** **距离矢量表**

距离矢量表（以下均简称DV表）是由路由器维护的表，记录了从当前节点到达网络中其他路由节点的开销。本实验中，DV表的列是源节点经过（via）的节点，行是到达（dest）的节点。例如，路由节点 $D_0$ 的DV表中的第2行第1列即为D0经过1号路由节点D1到达2号路由节点D2的总距离。

### **2.1.2** **协议原理**

本实验主要需要实现该协议的核心部分，即路由的最短路径求解的Bellman-Ford算法。

该算法的抽象模型是用于计算单源有向图的最短路径的，而在路由协议中也会用它来求解各个路由节点的最小可达路径代价问题。设 $D_x(y)$ 表示节点`x`到节点`y`的最短路径长， $c(x, v)$ 表示节点`x`到其相邻节点`v`的路径长，则对于每一个节点`y`，都有

$$
D_x(y) = \min_v\{c(x, v) + D_v(y)\}
$$

循环多次使用上式，得到一个最终的$D_x(y)$，这就是`x`到`y`的最短路径长。

对于每个节点，当其自身发现到其它邻居节点的路径成本（cost）发生变化的时候，就会计算更新路由表，并且把自己的路由表传送给所有邻节点。除此之外，它也会接收相邻节点发来的路由表更新信息并计算更新自己的路由表，并且将更新之后的表发回去。

## **2.2** DV路由协议

实验中所模拟的网络仿真环境包括虚拟网络层（如layer2）和4个路由器节点，是一个已规划好的网络结构，如图2-1所示。

<img width="274" height="217" alt="image" src="https://github.com/user-attachments/assets/e31e86f2-a7d8-4efa-b260-65c5856ec8aa" />

图2-1 网络结构示意图

每个路由节点维护一张路由DV表。当节点用路由算法计算出最短路径后，即可实现节点间数据的传输，如图2-2所示。

<img width="505" height="389" alt="image" src="https://github.com/user-attachments/assets/721ed7bd-30e1-4b8a-8fe3-0102fea9523a" />

图2-2 路由协议执行示意图

路由协议的基本规则如下：

（1）相邻路由节点之间会进行通信和数据交互，从而更新节点的DV表；

（2）每隔一段时间，框架代码所运行的网络仿真环境会驱动路由节点汇总一段时间内接收到的来自邻居节点发来的DV数据；

（3）各个路由节点据此运用2.1.2中介绍的算法寻找抵达其它路由节点的最短路径；

（4）节点的路由DV表更新后，会再将其广播给相邻节点。

以上四步一直循环进行下去，直至传输事件（event）全部结束。

 

# **2.3** **数据结构设计**

### **2.3.1** DV表

实验代码中，每个路由节点的程序代码（`node.c`）都定义了DV表的数据结构，其元素由该节点经过某相邻节点到达其它节点的最短路径长度。初始定义其为一个 `int [4][4]` 型的数组结构。

图2-3所示的是D3表在某步输出的结果。分析可知，由于节点和自己的距离不必讨论，故行列均不包含本身；此外Node3只和Node1和Node2相邻，所以列只显示了与Node3相邻的节点，而行则显示了除Node3外的所有节点。

<img width="287" height="197" alt="image" src="https://github.com/user-attachments/assets/64aae1ef-bd04-492e-882c-d480a8896259" />

图2-3 DV表数据结构示例

### **2.3.2** 路由数据包`rtpkt`

rtpkt由三个部分组成，分别为src_id、dst_id和mincost，如图2-4所示。

 

| src_id | dst_id | *mincost |
| ------ | ------ | -------- |
|        |        |          |

图2-4 rtpkt数据结构

其中，src_id指示该数据包的来源，即发送了该数据包的节点号；dst_id指示该数据包的去向，即该数据包将被发往的节点号；mincost[i]为该数据包到路由节点Node i的最短路径开销。

### 2.3.3 事件列表`event`

event是在main等函数中使用的数据结构，已经被封装好，如图2-5所示。事件列表用于串联起每个赋给各个路由节点的事件，引发节点作出反应，也参与最后trace的输出。

 

| evtime | evtype | eventity | rtpktptr | *prev | *next |
| ------ | ------ | -------- | -------- | ----- | ----- |
|        |        |          |          |       |       |

图2-5 event list数据结构

evetime、evtype是事件的发生时刻和事件类型，eventity是和事件发生位置有关的int变量，rtpktptr指针指向与该event的关联数据包。后面两个prev和next是递归定义的event类型指针，分别指向之前一个和之后一个事件，由此也将事件列表构造成了一个双向链表结构。

 

# 三、协议实现

## **3.1** **初始化函数**

这部分需要实现的是rtinit0(), rtinit1(), rtinit2(), rtinit3()四个函数，分别对四个路由节点做初始化操作。对于每个节点，rtinit主要功能是先将DV表中的相关值置为无穷大（定义无穷大为999），然后给DV表赋值。之后将这些DV数据打包，传送到layer2层，以发送给所有相邻节点。

以Node0为例，rtinit0()的伪代码如下：
```
Function rtinit0: BEGIN
	打印初始化信息
	将D0表的所有元素置为INF
	for 所有节点i do
		if 节点Node0可达i then
			计算Node0到所有相邻节点的距离，填入数组min0[i]
		endif
		不可达的min[i]置INF
	endfor
	对于所有节点i，将min0[i]赋给对应D0[i][0]
	for Node 0的所有相邻节点i do
		initPkt0. src_id=0
		initPkt0. dest_id=Node[i]
		调用createrpkt()打包DV数据，形成initPkt0
		tolayer2(initPkt0)
		打印发送的数据包信息
	endfor
END
```

## **3.2** **更新函数**

这部分需要实现的是rtupdate0(), rtupdate1(), rtupdate2(), rtupdate3()四个函数，分别对四个路由节点作更新操作。首先需要获取数据包的src_id、dst_id和mincost等信息，然后检验发送方和接收方的节点序号是否正确无误（对于Node i接收的包，0<src_id<4且dst_id==i即为正确）。之后，利用收到的packet数据重新计算DV表，如果有更新则将其打印出来，并发给相邻节点。

以Node0为例，rtupdate0()的伪代码如下。其中d[0,i]在其它节点Node x的更新函数中为d[x, i]。
```
Function rtupdate0: BEGIN
	取出rcvdpkt的src_id, dst_id和mincost[]
	打印更新信息
	if 数据包的src_id, dst_id错误 then
		打印错误信息，return
	endif
	for 所有四个节点i do
		d[0, i]=D0[src_id][0]+rcvdpkt.mincost[i]
		if d[0, i]超出INF then
			d[0, i]=INF
		endif
		if d[0, i]发生变化 then
			将新值赋给D0表
			if d[0, i]比min0[i]还要小 then
				将min0[i]更新为d[0, i]
				更新标识位置1
			endif
			else if D0表对应的值与min0相等 then
				找到cost最小的相邻节点，以此赋值
				更新标志位置1
			endif
		endif
	endfor
	if 更新标识位为1 then
		打印更新信息
		printdt0(&D0)
		for Node 0的所有相邻节点i do
			outPkt. src_id=0，outPkt. dest_id=Node[i]
			调用createrpkt()打包DV数据，形成outPkt
			tolayer2(outPkt)
			打印发送的数据包信息
		endfor
	endif
	else if 更新标识位为0 then
		打印未更新信息
		printdt0(&D0)
	endif
END

```

 

# 四、实验结果及分析

## **4.1** **运行准备**

本实验的环境准备、输入参数等信息如下表1所示。其中mmm为prog3.c的jimsrand()函数中的最大整型数，INF为每个node.c定义的“无穷大”数值。

表1 实验准备



| 环境准备     | **实验环境** | **操作系统** | **编程语言** |
| ------------ | ------------ | ------------ | ------------ |
|              | Dev-C++ 5.11 | Windows 10   | C            |
| 参数配置     | **trace**    | **mmm**      | **INF**      |
|             | 2            | 32767        | 999          |

按上述参数配置好环境和基础代码参数后，编译Dev项目并运行，即可得到输出结果。

## **4.2** **运行结果**

运行代码后得到结果截图。以下分Node0~Node3四个部分展示本实验执行的初始化（rtinit）、更新（rtupdate）等输出结果。

```
Enter TRACE:2

[Node0] initializing...
[Node0] transports initial DV {0,1,3,7} to Node1
[Node0] transports initial DV {0,1,3,7} to Node2
[Node0] transports initial DV {0,1,3,7} to Node3
[Node0] initialized.

[Node1] initializing...
[Node1] transports initial DV {1,0,1,999} to Node0
[Node1] transports initial DV {1,0,1,999} to Node2
[Node1] initialized

[Node2] initializing
[Node2] transports initial DV {3,1,0,2} to Node0
[Node2] transports initial DV {3,1,0,2} to Node1
[Node2] transports initial DV {3,1,0,2} to Node3
[Node2] initialized

[Node3] initializing
[Node3] transports initial DV {7,999,2,0} to Node0
[Node3] transports initial DV {7,999,2,0} to Node2
[Node3] initialized

MAIN: rcv event, t=0.094, at 1 src: 0, dest: 1, contents:   0   1   3   7
[Node1] is updating...
[Node1] DV table updated:
             via
   D1 |    0     2
  ----|-----------
     0|    1   999
dest 2|    4   999
     3|    8   999
[Node1] transports updated DV {1, 0, 1, 8} to neighbor Node0
[Node1] transports updated DV {1, 0, 1, 8} to neighbor Node2
[Node1] updated

MAIN: rcv event, t=0.427, at 1 src: 2, dest: 1, contents:   3   1   0   2
[Node1] is updating...
[Node1] DV table updated:
             via
   D1 |    0     2
  ----|-----------
     0|    1     4
dest 2|    4     1
     3|    8     3
[Node1] transports updated DV {1, 0, 1, 3} to neighbor Node0
[Node1] transports updated DV {1, 0, 1, 3} to neighbor Node2
[Node1] updated

MAIN: rcv event, t=0.998, at 0 src: 1, dest: 0, contents:   1   0   1 999
[Node0] is updating...
[Node0] DV table updated:
                via
   D0 |    1     2    3
  ----|-----------------
     1|    1   999   999
dest 2|    2   999   999
     3|  999   999   999
[Node0] transports updated DV {0, 1, 2, 7} to neighbor Node1
[Node0] transports updated DV {0, 1, 2, 7} to neighbor Node2
[Node0] transports updated DV {0, 1, 2, 7} to neighbor Node3
[Node0] updated

MAIN: rcv event, t=1.244, at 3 src: 0, dest: 3, contents:   0   1   3   7
[Node3] is updating...
[Node3] DV table updated:


             via
   D3 |    0     2
  ----|-----------
     0|    7   999
dest 1|    8   999
     2|   10   999
[Node3] transports updated DV {7, 8, 2, 0} to neighbor Node0
[Node3] transports updated DV {7, 8, 2, 0} to neighbor Node2
[Node3] updated

MAIN: rcv event, t=1.514, at 2 src: 0, dest: 2, contents:   0   1   3   7
[Node2] is updating...

[Node2] DV table updated:

                via
   D2 |    0     1    3
  ----|-----------------
     0|    3   999   999
dest 1|    4   999   999
     3|   10   999   999
[Node2]  transports updated DV {3, 1, 0, 2} to neighbor Node0
[Node2]  transports updated DV {3, 1, 0, 2} to neighbor Node1
[Node2]  transports updated DV {3, 1, 0, 2} to neighbor Node3
[Node2] updated

MAIN: rcv event, t=1.685, at 0 src: 2, dest: 0, contents:   3   1   0   2
[Node0] is updating...
[Node0] DV table updated:
                via
   D0 |    1     2    3
  ----|-----------------
     1|    1     4   999
dest 2|    2     3   999
     3|  999     5   999
[Node0] transports updated DV {0, 1, 2, 5} to neighbor Node1
[Node0] transports updated DV {0, 1, 2, 5} to neighbor Node2
[Node0] transports updated DV {0, 1, 2, 5} to neighbor Node3
[Node0] updated

MAIN: rcv event, t=2.171, at 3 src: 2, dest: 3, contents:   3   1   0   2
[Node3] is updating...
[Node3] DV table updated:


             via
   D3 |    0     2
  ----|-----------
     0|    7     5
dest 1|    8     3
     2|   10     2
[Node3] transports updated DV {5, 3, 2, 0} to neighbor Node0
[Node3] transports updated DV {5, 3, 2, 0} to neighbor Node2
[Node3] updated

MAIN: rcv event, t=2.399, at 0 src: 3, dest: 0, contents:   7 999   2   0
[Node0] is updating...
[Node0] not updated
                via
   D0 |    1     2    3
  ----|-----------------
     1|    1     4   999
dest 2|    2     3     9
     3|  999     5     7
[Node0] updated

MAIN: rcv event, t=2.489, at 0 src: 1, dest: 0, contents:   1   0   1   8
[Node0] is updating...
[Node0] not updated
                via
   D0 |    1     2    3
  ----|-----------------
     1|    1     4   999
dest 2|    2     3     9
     3|    9     5     7
[Node0] updated

MAIN: rcv event, t=2.667, at 2 src: 1, dest: 2, contents:   1   0   1 999
[Node2] is updating...

[Node2] DV table updated:

                via
   D2 |    0     1    3
  ----|-----------------
     0|    3     2   999
dest 1|    4     1   999
     3|   10   999   999
[Node2]  transports updated DV {2, 1, 0, 2} to neighbor Node0
[Node2]  transports updated DV {2, 1, 0, 2} to neighbor Node1
[Node2]  transports updated DV {2, 1, 0, 2} to neighbor Node3
[Node2] updated

MAIN: rcv event, t=2.823, at 1 src: 0, dest: 1, contents:   0   1   2   7
[Node1] is updating...
[Node1] not updated
             via
   D1 |    0     2
  ----|-----------
     0|    1     4
dest 2|    3     1
     3|    8     3
[Node1] updated

MAIN: rcv event, t=3.242, at 1 src: 2, dest: 1, contents:   3   1   0   2
[Node1] is updating...
[Node1] not updated
             via
   D1 |    0     2
  ----|-----------
     0|    1     4
dest 2|    3     1
     3|    8     3
[Node1] updated

MAIN: rcv event, t=3.361, at 0 src: 1, dest: 0, contents:   1   0   1   3
[Node0] is updating...
[Node0] DV table updated:
                via
   D0 |    1     2    3
  ----|-----------------
     1|    1     4   999
dest 2|    2     3     9
     3|    4     5     7
[Node0] transports updated DV {0, 1, 2, 4} to neighbor Node1
[Node0] transports updated DV {0, 1, 2, 4} to neighbor Node2
[Node0] transports updated DV {0, 1, 2, 4} to neighbor Node3
[Node0] updated

MAIN: rcv event, t=3.780, at 3 src: 0, dest: 3, contents:   0   1   2   7
[Node3] is updating...
[Node3] not updated
             via
   D3 |    0     2
  ----|-----------
     0|    7     5
dest 1|    8     3
     2|    9     2
[Node3] updated

MAIN: rcv event, t=3.798, at 2 src: 3, dest: 2, contents:   7 999   2   0
[Node2] is updating...

[Node2] DV table updated:

                via
   D2 |    0     1    3
  ----|-----------------
     0|    3     2     9
dest 1|    4     1   999
     3|   10   999     2
[Node2]  transports updated DV {2, 1, 0, 2} to neighbor Node0
[Node2]  transports updated DV {2, 1, 0, 2} to neighbor Node1
[Node2]  transports updated DV {2, 1, 0, 2} to neighbor Node3
[Node2] updated

MAIN: rcv event, t=3.915, at 0 src: 3, dest: 0, contents:   7   8   2   0
[Node0] is updating...
[Node0] not updated
                via
   D0 |    1     2    3
  ----|-----------------
     1|    1     4    15
dest 2|    2     3     9
     3|    4     5     7
[Node0] updated

MAIN: rcv event, t=4.774, at 2 src: 1, dest: 2, contents:   1   0   1   8
[Node2] is updating...

[Node2] not updated
                via
   D2 |    0     1    3
  ----|-----------------
     0|    3     2     9
dest 1|    4     1   999
     3|   10     9     2
[Node2] updated

MAIN: rcv event, t=4.967, at 1 src: 0, dest: 1, contents:   0   1   2   5
[Node1] is updating...
[Node1] not updated
             via
   D1 |    0     2
  ----|-----------
     0|    1     4
dest 2|    3     1
     3|    6     3
[Node1] updated

MAIN: rcv event, t=5.190, at 0 src: 2, dest: 0, contents:   3   1   0   2
[Node0] is updating...
[Node0] not updated
                via
   D0 |    1     2    3
  ----|-----------------
     1|    1     4    15
dest 2|    2     3     9
     3|    4     5     7
[Node0] updated

MAIN: rcv event, t=5.464, at 3 src: 2, dest: 3, contents:   3   1   0   2
[Node3] is updating...
[Node3] not updated
             via
   D3 |    0     2
  ----|-----------
     0|    7     5
dest 1|    8     3
     2|    9     2
[Node3] updated

MAIN: rcv event, t=5.606, at 1 src: 2, dest: 1, contents:   2   1   0   2
[Node1] is updating...
[Node1] not updated
             via
   D1 |    0     2
  ----|-----------
     0|    1     3
dest 2|    3     1
     3|    6     3
[Node1] updated

MAIN: rcv event, t=5.742, at 0 src: 3, dest: 0, contents:   5   3   2   0
[Node0] is updating...
[Node0] not updated
                via
   D0 |    1     2    3
  ----|-----------------
     1|    1     4    10
dest 2|    2     3     9
     3|    4     5     7
[Node0] updated

MAIN: rcv event, t=5.760, at 2 src: 1, dest: 2, contents:   1   0   1   3
[Node2] is updating...

[Node2] not updated
                via
   D2 |    0     1    3
  ----|-----------------
     0|    3     2     9
dest 1|    4     1   999
     3|   10     4     2
[Node2] updated

MAIN: rcv event, t=6.259, at 3 src: 0, dest: 3, contents:   0   1   2   5
[Node3] is updating...
[Node3] not updated
             via
   D3 |    0     2
  ----|-----------
     0|    7     5
dest 1|    8     3
     2|    9     2
[Node3] updated

MAIN: rcv event, t=6.578, at 1 src: 0, dest: 1, contents:   0   1   2   4
[Node1] is updating...
[Node1] not updated
             via
   D1 |    0     2
  ----|-----------
     0|    1     3
dest 2|    3     1
     3|    5     3
[Node1] updated

MAIN: rcv event, t=6.771, at 2 src: 0, dest: 2, contents:   0   1   2   7
[Node2] is updating...

[Node2] not updated
                via
   D2 |    0     1    3
  ----|-----------------
     0|    3     2     9
dest 1|    4     1   999
     3|   10     4     2
[Node2] updated

MAIN: rcv event, t=6.947, at 3 src: 2, dest: 3, contents:   2   1   0   2
[Node3] is updating...
[Node3] DV table updated:


             via
   D3 |    0     2
  ----|-----------
     0|    7     4
dest 1|    8     3
     2|    9     2
[Node3] transports updated DV {4, 3, 2, 0} to neighbor Node0
[Node3] transports updated DV {4, 3, 2, 0} to neighbor Node2
[Node3] updated

MAIN: rcv event, t=7.304, at 0 src: 2, dest: 0, contents:   2   1   0   2
[Node0] is updating...
[Node0] not updated
                via
   D0 |    1     2    3
  ----|-----------------
     1|    1     4    10
dest 2|    2     3     9
     3|    4     5     7
[Node0] updated

MAIN: rcv event, t=7.317, at 0 src: 2, dest: 0, contents:   2   1   0   2
[Node0] is updating...
[Node0] not updated
                via
   D0 |    1     2    3
  ----|-----------------
     1|    1     4    10
dest 2|    2     3     9
     3|    4     5     7
[Node0] updated

MAIN: rcv event, t=7.613, at 3 src: 0, dest: 3, contents:   0   1   2   4
[Node3] is updating...
[Node3] not updated
             via
   D3 |    0     2
  ----|-----------
     0|    7     4
dest 1|    8     3
     2|    9     2
[Node3] updated

MAIN: rcv event, t=7.650, at 2 src: 3, dest: 2, contents:   7   8   2   0
[Node2] is updating...

[Node2] not updated
                via
   D2 |    0     1    3
  ----|-----------------
     0|    3     2     9
dest 1|    4     1    10
     3|   10     4     2
[Node2] updated

MAIN: rcv event, t=7.708, at 1 src: 2, dest: 1, contents:   2   1   0   2
[Node1] is updating...
[Node1] not updated
             via
   D1 |    0     2
  ----|-----------
     0|    1     3
dest 2|    3     1
     3|    5     3
[Node1] updated

MAIN: rcv event, t=8.713, at 3 src: 2, dest: 3, contents:   2   1   0   2
[Node3] is updating...
[Node3] not updated
             via
   D3 |    0     2
  ----|-----------
     0|    7     4
dest 1|    8     3
     2|    9     2
[Node3] updated

MAIN: rcv event, t=8.958, at 2 src: 0, dest: 2, contents:   0   1   2   5
[Node2] is updating...

[Node2] not updated
                via
   D2 |    0     1    3
  ----|-----------------
     0|    3     2     9
dest 1|    4     1    10
     3|    8     4     2
[Node2] updated

MAIN: rcv event, t=9.169, at 0 src: 3, dest: 0, contents:   4   3   2   0
[Node0] is updating...
[Node0] not updated
                via
   D0 |    1     2    3
  ----|-----------------
     1|    1     4    10
dest 2|    2     3     9
     3|    4     5     7
[Node0] updated

MAIN: rcv event, t=9.809, at 2 src: 3, dest: 2, contents:   5   3   2   0
[Node2] is updating...

[Node2] not updated
                via
   D2 |    0     1    3
  ----|-----------------
     0|    3     2     7
dest 1|    4     1     5
     3|    8     4     2
[Node2] updated

MAIN: rcv event, t=10.823, at 2 src: 0, dest: 2, contents:   0   1   2   4
[Node2] is updating...

[Node2] not updated
                via
   D2 |    0     1    3
  ----|-----------------
     0|    3     2     7
dest 1|    4     1     5
     3|    7     4     2
[Node2] updated

MAIN: rcv event, t=11.561, at 2 src: 3, dest: 2, contents:   4   3   2   0
[Node2] is updating...

[Node2] not updated
                via
   D2 |    0     1    3
  ----|-----------------
     0|    3     2     6
dest 1|    4     1     5
     3|    7     4     2
[Node2] updated

MAIN: rcv event, t=10000.000, at -1MAIN: rcv event, t=20000.000, at 12517712
Simulator terminated at t=20000.000000, no packets in medium
```

### **4.2.1 Node0**

Node0在数据传输过程中的DV表更新、数据包收发等事件。(a)中，程序先将邻接点距离初始值赋给D0，然后在需要更新时，利用rcvdpkt的mincost表，根据Bellman-Ford算法更新D0表，每步由蓝色圈出。最后黄色圈出的部分即为D0表的**最终更新结果**。


### **4.2.2 Node1**

Node1在数据传输过程中的DV表更新、数据包收发等事件。(a)中，程序先将邻接点距离初始值赋给D1，然后在需要更新时，利用rcvdpkt的mincost表，根据Bellman-Ford算法更新D1表，每步由蓝色圈出。最后黄色圈出的部分即为D1表的**最终更新结果**。

### **4.2.3 Node2**

Node2在数据传输过程中的DV表更新、数据包收发等事件。(a)中，程序先将邻接点距离初始值赋给D2，然后在需要更新时，利用rcvdpkt的mincost表，根据Bellman-Ford算法更新D2表，每步由蓝色圈出。最后黄色圈出的部分即为D2表的**最终更新结果**。

 

### **4.2.4 Node3**

Node3在数据传输过程中的DV表更新、数据包收发等事件。(a)中，程序先将邻接点距离初始值赋给D3，然后在需要更新时，利用rcvdpkt的mincost表，根据Bellman-Ford算法更新D3表，每步由蓝色圈出。最后黄色圈出的部分即为D3表的**最终更新结果**。


# 五、实验遇到的问题和解决方法

## **5.1** **遇到的问题**

本实验中遇到的问题如下：

（1）编译后输入`trace=2`运行，发现无法得到输出，显示随机值不匹配；

（2）一开始`D3`表输出一直都是`INF`，无法输出正确结果；

（3）DV表输出的结果有时无法检查出来。

 

## **5.2** **解决方法**

针对上述问题，通过询问同学、上网查询资料等方法，解决方法分别如下：

（1）修改了`jimsrand()`函数里的`mmm`值，调整为`RAND_MAX`，再次编译即可通过；

（2）修改了`node3.c`中关于`dst`赋值的语句，问题即得到解决；

（3）在代码调试的时候，先让`printdt()`函数输出完整的DV表，这样方便挑出错误之处，最后再将DV表的输出改回实验所要求的形式。

 
