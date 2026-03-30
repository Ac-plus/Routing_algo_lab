# TJU计算机网络 路由算法实验

实验文档位于`report.md`。

实现了 **Distance Vector Routing Algorithm（距离向量路由算法，DV）** 的分布式仿真。系统模拟一个包含 **4 个路由节点（0–3）** 的网络，每个节点运行 DV 算法并通过消息交换更新自己的路由表。

网络仿真框架负责：

* 模拟节点之间的消息传递
* 维护事件队列
* 调用各节点的 DV 更新函数

DV 算法逻辑由 `node0.c`、`node1.c`、`node2.c`、`node3.c` 实现。


# 文件结构

```
.
├── prog3.c      # 网络仿真
├── node0.c      # node0 的 DV 算法实现
├── node1.c      # node1 的 DV 算法实现
├── node2.c      # node2 的 DV 算法实现
└── node3.c      # node3 的 DV 算法实现
```

# 结构说明

## 仿真器（prog3.c）

`prog3.c` 是网络仿真主体。其主要功能包括：

* 初始化网络环境
* 调用各节点初始化函数
* 模拟网络事件
* 调用节点更新函数

```
main()
 ├── init()
 │     ├── rtinit0()
 │     ├── rtinit1()
 │     ├── rtinit2()
 │     └── rtinit3()
 │
 └── 事件循环
       ├── 接收数据包
       ├── 调用 rtupdateX()
       └── 更新 DV 表
```

仿真采用 **事件驱动模型**：

* `FROM_LAYER2` ：节点收到路由更新
* `LINK_CHANGE` ：链路代价变化

## 节点实现

每个节点文件（如 `node0.c`）实现 DV 算法。主要函数：

### 1 初始化函数

```
rtinitX()
```

功能：

* 初始化距离向量表
* 设置邻居节点
* 发送初始 DV

例如 node0 的初始链路代价：

```
connectcosts0 = {0,1,3,7}
```

表示：

```
0 → 1 cost = 1
0 → 2 cost = 3
0 → 3 cost = 7
```

这些值会作为节点初始的距离向量。

### 2 更新函数

```
rtupdateX(struct rtpkt *pkt)
```

当收到邻居发送的 DV 时：

1. 更新距离表
2. 根据 Bellman-Ford 公式计算新的最短路径

公式：

$$
D_x(y) = \min \{ c(x,v) + D_v(y) \}
$$

如果 DV 发生变化：

* 向邻居发送新的 DV


### 3 发送函数

```
sendPkt2AdjX()
```

功能：

* 向所有邻居节点发送当前 DV

发送的数据结构：

```
struct rtpkt {
    int sourceid
    int destid
    int mincost[4]
}
```

### 4 调试函数

```
printdtX()
```

用于打印当前节点的 DV 表。例如输出格式：

```
via
D0 | 0 1 2 3
--------------
...
```

# 网络拓扑

链路代价：

| Link | Cost |
| ---- | ---- |
| 0-1  | 1    |
| 0-2  | 3    |
| 0-3  | 7    |
| 1-2  | 1    |
| 2-3  | 2    |


#  编译方法

使用 `gcc` 编译：

```bash
gcc prog3.c node0.c node1.c node2.c node3.c -o dv
```

# 运行方法

运行方法：

```bash
./dv
```

启动后会要求输入：

```
Enter TRACE:
```

TRACE 用于控制输出信息：

| TRACE | 含义    |
| ----- | ----- |
| 0     | 几乎无输出 |
| 1     | 基本输出  |
| 2     | 更详细   |
| 3     | 非常详细  |

一般建议：

```
Enter TRACE: 1
```


# 可选功能

可以开启 **链路变化模拟**。

在 `prog3.c` 中：

```c
#define LINKCHANGES 1
```

开启后：

* 在 t=10000 时修改链路 cost
* 在 t=20000 恢复
