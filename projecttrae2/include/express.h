#ifndef EXPRESS_H
#define EXPRESS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// 快递状态枚举
typedef enum {
    PENDING,    // 待取件
    PICKED,     // 已取件
    EXPIRED     // 已过期
} ExpressStatus;

// 快递信息结构体
typedef struct Express {
    char id[20];           // 快递单号
    char sender[50];       // 寄件人
    char receiver[50];     // 收件人
    char phone[12];        // 收件人电话
    float weight;          // 重量
    float cost;            // 费用
    time_t create_time;    // 创建时间
    ExpressStatus status;  // 快递状态
    int delivery_to_door;  // 是否需要到楼服务
    struct Express* next;  // 下一个节点
} Express;

// 生成快递单号
void generate_express_id(char* buffer);

// 创建快递节点
Express* create_express_node();

// 插入快递
void insert_express(Express** head);

// 删除快递
int delete_express(Express** head);

// 打印快递列表
void print_express_list(Express* head);

// 计算费用
float calculate_cost(float weight);

// 检查库存警报
void check_inventory_alerts(Express* head);

#endif // EXPRESS_H