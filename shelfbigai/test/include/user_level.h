#ifndef USER_LEVEL_H
#define USER_LEVEL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 会员等级枚举（五级）
typedef enum {
    LEVEL_NORMAL,     // 普通会员
    LEVEL_ONE,        // 一级会员
    LEVEL_TWO,        // 二级会员
    LEVEL_THREE,      // 三级会员
    LEVEL_FOUR        // 四级会员
} MemberLevel;

// 会员等级升级阈值（消费金额）
#define LEVEL_ONE_THRESHOLD 500.0f     // 普通会员升级到一级会员的阈值
#define LEVEL_TWO_THRESHOLD 1000.0f    // 一级会员升级到二级会员的阈值
#define LEVEL_THREE_THRESHOLD 2000.0f  // 二级会员升级到三级会员的阈值
#define LEVEL_FOUR_THRESHOLD 5000.0f   // 三级会员升级到四级会员的阈值

// 会员等级折扣
#define LEVEL_NORMAL_DISCOUNT 0.98f    // 普通会员折扣
#define LEVEL_ONE_DISCOUNT 0.95f       // 一级会员折扣
#define LEVEL_TWO_DISCOUNT 0.92f       // 二级会员折扣
#define LEVEL_THREE_DISCOUNT 0.88f     // 三级会员折扣
#define LEVEL_FOUR_DISCOUNT 0.85f      // 四级会员折扣

// 获取会员等级名称
const char* get_member_level_name(MemberLevel level);

// 获取会员等级折扣
float get_member_level_discount(MemberLevel level);

// 检查并更新会员等级
MemberLevel check_and_update_member_level(float total_cost, MemberLevel current_level);

// 获取下一级会员所需消费金额
float get_next_level_threshold(MemberLevel current_level);

#endif // USER_LEVEL_H