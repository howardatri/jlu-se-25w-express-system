#ifndef PROMOTION_H
#define PROMOTION_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// 优惠活动类型枚举
typedef enum {
    PROMOTION_DISCOUNT,     // 折扣优惠
    PROMOTION_CASH_BACK,    // 满减优惠
    PROMOTION_FREE_SHIPPING, // 免运费
    PROMOTION_GIFT          // 赠品
} PromotionType;

// 优惠活动状态枚举
typedef enum {
    PROMOTION_ACTIVE,    // 活动中
    PROMOTION_INACTIVE,  // 未开始
    PROMOTION_EXPIRED    // 已结束
} PromotionStatus;

// 优惠活动结构体
typedef struct Promotion {
    char id[20];                // 活动ID
    char name[50];              // 活动名称
    char description[200];      // 活动描述
    PromotionType type;         // 活动类型
    float value;                // 优惠值（折扣率或满减金额）
    float threshold;            // 门槛值（满多少可用）
    time_t start_time;          // 开始时间
    time_t end_time;            // 结束时间
    PromotionStatus status;     // 活动状态
    struct Promotion* next;     // 下一个节点
} Promotion;

// 生成活动ID
void generate_promotion_id(char* buffer);

// 创建优惠活动
Promotion* create_promotion(const char* name, const char* description, 
                           PromotionType type, float value, float threshold,
                           time_t start_time, time_t end_time);

// 添加优惠活动
void add_promotion(Promotion** head, Promotion* new_promotion);

// 删除优惠活动
int delete_promotion(Promotion** head, const char* promotion_id);

// 更新优惠活动状态
void update_promotion_status(Promotion* head);

// 查询优惠活动
Promotion* find_promotion_by_id(Promotion* head, const char* promotion_id);

// 查询当前可用的优惠活动
Promotion* find_available_promotions(Promotion* head, float order_amount);

// 应用优惠活动计算最终价格
float apply_promotion(Promotion* promotion, float original_price);

// 打印优惠活动列表
void print_promotion_list(Promotion* head);

// 从文件加载优惠活动数据
Promotion* load_promotions(const char* filename);

// 保存优惠活动数据到文件
void save_promotions(const char* filename, Promotion* head);

// 优惠活动管理菜单
void promotion_management_menu(Promotion** promotion_head);

// 获取活动类型名称
const char* get_promotion_type_name(PromotionType type);

// 获取活动状态名称
const char* get_promotion_status_name(PromotionStatus status);

#endif // PROMOTION_H