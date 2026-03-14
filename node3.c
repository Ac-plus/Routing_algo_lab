/**
 * @file node3.c
 * @brief node3 DV算法的实现
 * @details 实现DV算法中node3部分, 包括初始化, 更新和解决cost改变
 * @author geniusLcy
 * @email im.lichenyi@gmail.com
 * @version 1.0.0
 * @date 2021-06-01
 */
#include <stdio.h>

#define INFINITY 999 /**< 不可达: cost为无穷, 设置为999 */
#define NODEVALUE 3 /**< 本结点编号, 用来识别初始的邻接结点以及其开销 */
#define PRINTVERSION 1 /**< 打印帮助函数的版本: 0为原始版本, 1为自定义版本 */
#define DEBUG /**<方便调试, 会打印辅助确认的输出语句 */

/**
 * @brief 传递消息结构体
 *
 * @details a rtpkt is the packet sent from one routing update process to
 * another via the call tolayer3()
 */
extern struct rtpkt {
    int sourceid;       /**< id of sending router sending this pkt */
    int destid;         /**< id of router to which pkt being sent
                         (must be an immediate neighbor) */
    int mincost[4];    /**< min cost to node 0 ... 3 */
};

void tolayer2(struct rtpkt packet);

extern int TRACE;
extern int YES;
extern int NO;


int adjNodes3[2] = {0, 2}; /**< node3邻接的结点列表 */
int adjNodesNum3 = 2; /**< node3邻接结点的数量 */
int connectcosts3[4] = {7, INFINITY, 2, 0}; /**< node3初始感知到它与其他结点的cost,
                                            结点序号用下标表示 */

/**
 * @brief node2的距离向量表
 *
 * @details
 * - node0传入的DV放在第0行;
 * - node1传入的DV放在第1行;
 * - node2传入的DV放在第2行;
 * - node3维护的DV放在第3行.
 */
struct distance_table {
    int costs[4][4];
} dt3;

/* students to write the following two routines, and maybe some others */

void printdt3(struct distance_table *dtptr);

void sendPkt2Adj3() {
    int i, j, currAdjNode;         /**< loop variable pre-declaration */

    /* 对于每一个邻接的结点, 先造包再通过layer2发送 */
    for (i = 0; i < adjNodesNum3; i++) {
        currAdjNode = adjNodes3[i];
        /* make packet */
        struct rtpkt sndpkt;
        sndpkt.sourceid = NODEVALUE;
        sndpkt.destid = currAdjNode;
        for (j = 0; j < 4; j++) {
            sndpkt.mincost[j] = dt3.costs[NODEVALUE][j];
        }
        /* send packet */
        tolayer2(sndpkt);
    }
}

/**
 * @brief 初始化node3
 *
 * @param 无参数
 * @return void
 * @retval 无返回值
 */
void rtinit3() {
    int i, j; /**< loop variable pre-declaration */

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            if (i == NODEVALUE) { /* 在node3中, 自己保存的DV在第3行 */
                /* for all destinations y in N:
                 *      Dx(y) = c(x,y)
                 *      if y is not a neighbor
                 *      then c(x, y) = INFINITY  */
                dt3.costs[i][j] = connectcosts3[j];
            } else { /* 别的结点发送的DV存放在1~3行 */
                /* for each neighbor w:
                 *      Dw (y) = ?
                 *      for all destinations y in N */
                dt3.costs[i][j] = INFINITY;
            }
        }
    }

    sendPkt2Adj3();
}

/**
 * @brief 当node2收到消息更新自己的DV表
 *
 * @param rcvdpkt 从别的node收到的消息
 * @return void
 * @retval 无返回值
 */
void rtupdate3(struct rtpkt *rcvdpkt) {
    int i, j; /**< loop variable pre-declaration */

    /* 先根据sourceid存到自己的DV表中 */
    int source = rcvdpkt->sourceid;
    for (i = 0; i < 4; i++) {
        dt3.costs[source][i] = rcvdpkt->mincost[i];
    }

    /* 更新之后的costs */
    int updatedcost[4];
    for (i = 0; i < 4; i++) {
        updatedcost[i] = dt3.costs[NODEVALUE][i];
    }

    /* 根据发来的DV更新自己的DV */
    int updateFlag = 0;
    for (i = 0; i < 4; i++) {
        int cdcostsum = dt3.costs[NODEVALUE][source] + dt3.costs[source][i];
        if (cdcostsum < updatedcost[i]) {
            updateFlag = 1;
            updatedcost[i] = cdcostsum;
        }
    }

    /* 如果DV发生了改变, 那么通知所有邻接的结点 */
    if (updateFlag) {
        /* 把更新的缓存值写入自己的DV表 */
        for (i = 0; i < 4; i++) {
            dt3.costs[NODEVALUE][i] = updatedcost[i];
        }
#ifdef DEBUG
        printf("Node3 DV changed, send new DV to adj nodes.\n");
#endif
        /* 通知所有邻接的结点 */
        sendPkt2Adj3();
    } else { /* 如果没有更新达到稳定了, 那么打印以下自己的DV表 */
#ifdef DEBUG
        printf("Node3 DV remain, wait and do nothing.\n");
#endif
    }

#ifdef DEBUG
    printdt3(&dt3);
#endif
}

/**
 * @brief 打印node3 DV表信息的辅助函数, 需要自己调用
 *
 * @param dtptr DV表的指针
 * @return void
 * @retval 无返回值
 */
void printdt3(struct distance_table *dtptr) {
#if PRINTVERSION /* 自定义的 printdt3 */
    /* 将会打印完整的DV表 */
    printf("                via          \n");
    printf("   D3 |    0     1    2    3 \n");
    printf("  ----|----------------------\n");
    printf("     0|  %3d   %3d   %3d   %3d\n", dtptr->costs[0][0],
           dtptr->costs[0][1], dtptr->costs[0][2], dtptr->costs[0][3]);
    printf("dest 1|  %3d   %3d   %3d   %3d\n", dtptr->costs[1][0],
           dtptr->costs[1][1], dtptr->costs[1][2], dtptr->costs[1][3]);
    printf("     2|  %3d   %3d   %3d   %3d\n", dtptr->costs[2][0],
           dtptr->costs[2][1], dtptr->costs[2][2], dtptr->costs[2][3]);
    printf("     3|  %3d   %3d   %3d   %3d\n", dtptr->costs[3][0],
           dtptr->costs[3][1], dtptr->costs[3][2], dtptr->costs[3][3]);
#else /* 原来代码中给出的 printdt3 */
    printf("             via     \n");
    printf("   D3 |    0     2 \n");
    printf("  ----|-----------\n");
    printf("     0|  %3d   %3d\n", dtptr->costs[0][0], dtptr->costs[0][2]);
    printf("dest 1|  %3d   %3d\n", dtptr->costs[1][0], dtptr->costs[1][2]);
    printf("     2|  %3d   %3d\n", dtptr->costs[2][0], dtptr->costs[2][2]);
#endif
}






