#ifndef EXPRESS_H
#define EXPRESS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// 快递状态枚举
typedef enum {
    WAITING_SHIP, // 待发件
    PENDING,     // 待取件
    PICKED,      // 已取件
    SHIPPED,     // 已发件
    EXPIRED      // 已过期
} ExpressStatus;

// 快递大小枚举
typedef enum {
    SIZE_SMALL,  // 小型快递
    SIZE_MEDIUM, // 中型快递
    SIZE_LARGE   // 大型快递
} ExpressSize;

// 快递时效枚举
typedef enum {
    NORMAL_SPEED,  // 普通
    FAST_SPEED,    // 快速
    EXPRESS_SPEED  // 特快
} ExpressSpeed;

// 快递信息结构体
typedef struct Express {
    char id[20];           // 快递单号
    char sender[50];       // 寄件人
    char sender_phone[12]; // 寄件人电话
    char receiver[50];     // 收件人
    char phone[12];        // 收件人电话
    char pickup_code[7];   // 取件码
    char fake_address[100]; // 收件地址
    float weight;          // 重量
    float cost;            // 费用
    ExpressSize size;      // 快递大小
    int is_valuable;       // 是否贵重物品
    ExpressSpeed speed;    // 快递时效
    float distance;        // 距离（公里）
    time_t create_time;    // 创建时间
    ExpressStatus status;  // 快递状态
    int delivery_to_door;  // 是否需要到楼服务
    char shelf_id[20];     // 存放的货架ID
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
float calculate_cost(float weight, ExpressSize size, int is_valuable, ExpressSpeed speed, float distance, float discount, int express_delivery_to_door);

// 检查库存警报
void check_inventory_alerts(Express* head);

// 获取快递大小名称
const char* get_express_size_name(ExpressSize size);

// 获取快递时效名称
const char* get_express_speed_name(ExpressSpeed speed);

// 获取快递状态名称
const char* get_express_status_name(ExpressStatus status);

// 生成取件码
void generate_pickup_code(char* buffer);

// 发送取件码短信
void send_pickup_code(const char* phone, const char* pickup_code, const char* express_id, const char* shelf_id);

// 验证取件码
int validate_pickup_code(Express* head, const char* express_id, const char* input_code);

// 管理员发件处理
void admin_ship_express(Express** head);

// 检查库存警报
void check_inventory_alerts(Express* head);

#endif // EXPRESS_H