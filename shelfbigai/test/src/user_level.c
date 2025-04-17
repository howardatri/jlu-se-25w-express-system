#include "../include/user_level.h"

// 获取会员等级名称
const char* get_member_level_name(MemberLevel level) {
    switch (level) {
        case LEVEL_NORMAL: return "普通会员";
        case LEVEL_ONE: return "一级会员";
        case LEVEL_TWO: return "二级会员";
        case LEVEL_THREE: return "三级会员";
        case LEVEL_FOUR: return "四级会员";
        default: return "未知等级";
    }
}

// 获取会员等级折扣
float get_member_level_discount(MemberLevel level) {
    switch (level) {
        case LEVEL_NORMAL: return LEVEL_NORMAL_DISCOUNT;
        case LEVEL_ONE: return LEVEL_ONE_DISCOUNT;
        case LEVEL_TWO: return LEVEL_TWO_DISCOUNT;
        case LEVEL_THREE: return LEVEL_THREE_DISCOUNT;
        case LEVEL_FOUR: return LEVEL_FOUR_DISCOUNT;
        default: return 1.0f; // 无折扣
    }
}

// 检查并更新会员等级
MemberLevel check_and_update_member_level(float total_cost, MemberLevel current_level) {
    // 根据累计消费金额判断应该的等级
    MemberLevel new_level;
    
    if (total_cost >= LEVEL_FOUR_THRESHOLD) {
        new_level = LEVEL_FOUR;
    } else if (total_cost >= LEVEL_THREE_THRESHOLD) {
        new_level = LEVEL_THREE;
    } else if (total_cost >= LEVEL_TWO_THRESHOLD) {
        new_level = LEVEL_TWO;
    } else if (total_cost >= LEVEL_ONE_THRESHOLD) {
        new_level = LEVEL_ONE;
    } else {
        new_level = LEVEL_NORMAL;
    }
    
    // 只有等级提升时才返回新等级，否则保持原等级
    // 这样可以防止因为退款等原因导致的等级下降
    if (new_level > current_level) {
        return new_level;
    } else {
        return current_level;
    }
}

// 获取下一级会员所需消费金额
float get_next_level_threshold(MemberLevel current_level) {
    switch (current_level) {
        case LEVEL_NORMAL: return LEVEL_ONE_THRESHOLD;
        case LEVEL_ONE: return LEVEL_TWO_THRESHOLD;
        case LEVEL_TWO: return LEVEL_THREE_THRESHOLD;
        case LEVEL_THREE: return LEVEL_FOUR_THRESHOLD;
        case LEVEL_FOUR: return -1.0f; // 已经是最高等级
        default: return -1.0f;
    }
}