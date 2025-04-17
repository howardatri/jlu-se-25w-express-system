#include "../include/promotion.h"
#include "../include/utils.h"

// 生成活动ID
void generate_promotion_id(char* buffer) {
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    
    // 读取现有活动列表，找到最后一个活动ID
    FILE* file = fopen("data/promotions.txt", "r");
    char last_id[20] = "";
    char line[500];
    
    if (file != NULL) {
        while (fgets(line, sizeof(line), file) != NULL) {
            char id[20];
            if (sscanf(line, "%19s", id) == 1) {
                // 确保我们总是使用最后一个有效的ID
                strcpy(last_id, id);
            }
        }
        fclose(file);
    }
    
    // 解析最后一个活动ID的日期和序列号
    int seq = 1;
    if (strlen(last_id) > 0) {
        int year, month, day, last_seq;
        // 修改格式字符串，确保正确匹配4位数字的年份、2位数字的月份和日期，以及4位数字的序列号
        if (sscanf(last_id, "PM%04d%02d%02d%04d", &year, &month, &day, &last_seq) == 4) {
            // 检查是否是同一天
            if (year == t->tm_year + 1900 && month == t->tm_mon + 1 && day == t->tm_mday) {
                // 如果是同一天，序列号加1
                seq = last_seq + 1;
                if (seq >= 10000) seq = 1; // 如果超过9999，重置为1
            }
            // 如果不是同一天，seq保持为1，表示当天的第一个活动
        } else {
            printf("警告：无法解析上一个活动ID格式：%s\n", last_id);
        }
    }
    
    sprintf(buffer, "PM%04d%02d%02d%04d", t->tm_year+1900, t->tm_mon+1, t->tm_mday, seq);
    
    // 调试信息
    printf("生成新的活动ID: %s (基于上一个ID: %s)\n", buffer, last_id);

}

// 创建优惠活动
Promotion* create_promotion(const char* name, const char* description, 
                           PromotionType type, float value, float threshold,
                           time_t start_time, time_t end_time) {
    Promotion* promotion = (Promotion*)malloc(sizeof(Promotion));
    if (promotion == NULL) {
        printf("内存分配失败！\n");
        return NULL;
    }
    
    // 生成活动ID
    generate_promotion_id(promotion->id);
    
    // 设置活动名称
    strncpy(promotion->name, name, sizeof(promotion->name) - 1);
    promotion->name[sizeof(promotion->name) - 1] = '\0';
    
    // 设置活动描述
    strncpy(promotion->description, description, sizeof(promotion->description) - 1);
    promotion->description[sizeof(promotion->description) - 1] = '\0';
    
    // 设置活动类型和优惠值
    promotion->type = type;
    promotion->value = value;
    promotion->threshold = threshold;
    
    // 设置活动时间
    promotion->start_time = start_time;
    promotion->end_time = end_time;
    
    // 根据当前时间设置活动状态
    time_t now = time(NULL);
    if (now < start_time) {
        promotion->status = PROMOTION_INACTIVE;
    } else if (now > end_time) {
        promotion->status = PROMOTION_EXPIRED;
    } else {
        promotion->status = PROMOTION_ACTIVE;
    }
    
    promotion->next = NULL;
    return promotion;
}

// 添加优惠活动
void add_promotion(Promotion** head, Promotion* new_promotion) {
    if (new_promotion == NULL) return;
    
    if (*head == NULL) {
        *head = new_promotion;
    } else {
        Promotion* current = *head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_promotion;
    }
    
    printf("\n优惠活动已添加成功！\n");
    printf("活动ID: %s\n", new_promotion->id);
}

// 删除优惠活动
int delete_promotion(Promotion** head, const char* promotion_id) {
    if (*head == NULL) {
        printf("优惠活动列表为空！\n");
        return 0;
    }
    
    Promotion* current = *head;
    Promotion* prev = NULL;
    
    while (current != NULL && strcmp(current->id, promotion_id) != 0) {
        prev = current;
        current = current->next;
    }
    
    if (current == NULL) {
        printf("未找到该活动ID！\n");
        return 0;
    }
    
    if (prev == NULL) {
        *head = current->next;
    } else {
        prev->next = current->next;
    }
    
    free(current);
    printf("优惠活动已删除成功！\n");
    return 1;
}

// 更新优惠活动状态
void update_promotion_status(Promotion* head) {
    if (head == NULL) {
        return;
    }
    
    time_t now = time(NULL);
    Promotion* current = head;
    int updated = 0;
    
    while (current != NULL) {
        PromotionStatus old_status = current->status;
        
        if (now < current->start_time) {
            current->status = PROMOTION_INACTIVE;
        } else if (now > current->end_time) {
            current->status = PROMOTION_EXPIRED;
        } else {
            current->status = PROMOTION_ACTIVE;
        }
        
        if (old_status != current->status) {
            updated++;
        }
        
        current = current->next;
    }
    
    if (updated > 0) {
        printf("已更新 %d 个活动的状态。\n", updated);
    }
}

// 查询优惠活动
Promotion* find_promotion_by_id(Promotion* head, const char* promotion_id) {
    Promotion* current = head;
    
    while (current != NULL) {
        if (strcmp(current->id, promotion_id) == 0) {
            return current;
        }
        current = current->next;
    }
    
    return NULL;
}

// 查询当前可用的优惠活动
Promotion* find_available_promotions(Promotion* head, float order_amount) {
    if (head == NULL) {
        return NULL;
    }
    
    // 先更新所有活动状态
    update_promotion_status(head);
    
    Promotion* available_head = NULL;
    Promotion* available_tail = NULL;
    Promotion* current = head;
    
    while (current != NULL) {
        // 检查活动是否可用（状态为活动中且订单金额满足门槛）
        if (current->status == PROMOTION_ACTIVE && order_amount >= current->threshold) {
            // 创建一个新节点，复制当前活动信息
            Promotion* new_node = (Promotion*)malloc(sizeof(Promotion));
            if (new_node != NULL) {
                memcpy(new_node, current, sizeof(Promotion));
                new_node->next = NULL;
                
                // 添加到可用活动链表
                if (available_head == NULL) {
                    available_head = new_node;
                    available_tail = new_node;
                } else {
                    available_tail->next = new_node;
                    available_tail = new_node;
                }
            }
        }
        current = current->next;
    }
    
    return available_head;
}

// 应用优惠活动
float apply_promotion(Promotion* promotion, float original_price) {
    // 为了保持兼容性，将参数重命名
    Promotion* head = promotion;
    float original_cost = original_price;
    char applied_promotion_name[50] = "";
    if (head == NULL) return original_cost;
    
    // 更新活动状态
    update_promotion_status(head);
    
    float final_cost = original_cost;
    Promotion* best_promotion = NULL;
    float max_discount = 0.0f;
    
    // 查找最优惠的活动
    Promotion* current = head;
    while (current != NULL) {
        if (current->status == PROMOTION_ACTIVE && original_cost >= current->threshold) {
            float discount_amount = 0.0f;
            
            switch (current->type) {
                case PROMOTION_DISCOUNT:
                    // 折扣优惠，value是折扣率（如0.8表示8折）
                    discount_amount = original_cost * (1.0f - current->value);
                    break;
                case PROMOTION_CASH_BACK:
                    // 满减优惠，value是减免金额
                    discount_amount = current->value;
                    break;
                case PROMOTION_FREE_SHIPPING:
                    // 免运费，value是最大免除金额
                    discount_amount = (original_cost < current->value) ? original_cost : current->value;
                    break;
            }
            
            if (discount_amount > max_discount) {
                max_discount = discount_amount;
                best_promotion = current;
            }
        }
        current = current->next;
    }
    
    // 应用最优惠的活动
    if (best_promotion != NULL) {
        switch (best_promotion->type) {
            case PROMOTION_DISCOUNT:
                // 修正：value是折扣率（如0.8表示8折），直接乘以原价
                final_cost = original_cost * best_promotion->value;
                printf("已应用折扣优惠：%.1f折，优惠后价格：%.2f元\n", best_promotion->value * 10, final_cost);
                printf("活动名称：%s\n", best_promotion->name);
                break;
            case PROMOTION_CASH_BACK:
                final_cost = original_cost - best_promotion->value;
                printf("已应用满减优惠：减%.2f元，优惠后价格：%.2f元\n", best_promotion->value, final_cost);
                printf("活动名称：%s\n", best_promotion->name);
                break;
            case PROMOTION_FREE_SHIPPING:
                final_cost = original_cost - ((original_cost < best_promotion->value) ? original_cost : best_promotion->value);
                printf("已应用免运费优惠，优惠后价格：%.2f元\n", final_cost);
                printf("活动名称：%s\n", best_promotion->name);
                break;
        }
        
        // 记录应用的优惠活动名称
        if (applied_promotion_name != NULL) {
            strcpy(applied_promotion_name, best_promotion->name);
        }
    }
    
    return final_cost;
}

// 打印优惠活动列表
void print_promotion_list(Promotion* head) {
    if (head == NULL) {
        printf("优惠活动列表为空！\n");
        return;
    }
    
    // 先更新所有活动状态
    update_promotion_status(head);
    
    printf("\n优惠活动列表:\n");
    printf("%-15s %-20s %-10s %-10s %-15s %-20s %-20s\n",
           "活动ID", "活动名称", "活动类型", "活动状态", "优惠值/门槛",
           "开始时间", "结束时间");
    printf("----------------------------------------"
           "----------------------------------------\n");
    
    Promotion* current = head;
    while (current != NULL) {
        char start_time_str[20];
        char end_time_str[20];
        strftime(start_time_str, sizeof(start_time_str), "%Y-%m-%d %H:%M:%S",
                localtime(&current->start_time));
        strftime(end_time_str, sizeof(end_time_str), "%Y-%m-%d %H:%M:%S",
                localtime(&current->end_time));
        
        const char* type_str = get_promotion_type_name(current->type);
        const char* status_str = get_promotion_status_name(current->status);
        
        char value_str[30];
        if (current->type == PROMOTION_DISCOUNT) {
            sprintf(value_str, "%.1f折/满%.2f元", current->value * 10, current->threshold);
        } else {
            sprintf(value_str, "%.2f元/满%.2f元", current->value, current->threshold);
        }
        
        printf("%-15s %-20s %-10s %-10s %-15s %-20s %-20s\n",
               current->id, current->name, type_str, status_str,
               value_str, start_time_str, end_time_str);
        
        current = current->next;
    }
}

// 从文件加载优惠活动数据
Promotion* load_promotions(const char* filename) {
    FILE* fp = safe_fopen(filename, "r", 3);
    if (fp == NULL) {
        // 创建data目录
        system("mkdir data 2>nul");
        // 创建空文件
        fp = safe_fopen(filename, "w", 3);
        if (fp != NULL) {
            fclose(fp);
            printf("已创建新的优惠活动数据文件：%s\n", filename);
        }
        return NULL;
    }
    
    Promotion* head = NULL;
    Promotion* current = NULL;
    
    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        Promotion* node = (Promotion*)malloc(sizeof(Promotion));
        if (node == NULL) {
            printf("内存分配失败！\n");
            fclose(fp);
            return head;
        }
        
        int type;
        if (sscanf(line, "%19s %49[^|]|%199[^|]|%d %f %f %ld %ld",
                   node->id, node->name, node->description, &type, &node->value,
                   &node->threshold, &node->start_time, &node->end_time) != 8) {
            free(node);
            continue;
        }
        
        node->type = (PromotionType)type;
        node->next = NULL;
        
        // 根据当前时间计算活动状态
        time_t now = time(NULL);
        if (node->start_time <= 0 || node->end_time <= 0) {
            node->status = PROMOTION_INACTIVE;
        } else if (now < node->start_time) {
            node->status = PROMOTION_INACTIVE;
        } else if (now > node->end_time) {
            node->status = PROMOTION_EXPIRED;
        } else {
            node->status = PROMOTION_ACTIVE;
        }
        
        if (head == NULL) {
            head = node;
            current = node;
        } else {
            current->next = node;
            current = node;
        }
    }
    
    fclose(fp);
    return head;
}

// 保存优惠活动数据到文件
void save_promotions(const char* filename, Promotion* head) {
    FILE* fp = safe_fopen(filename, "w", 3);
    if (fp == NULL) {
        printf("无法打开文件 %s 进行写入！\n", filename);
        return;
    }
    
    // 在保存前更新所有活动的状态
    update_promotion_status(head);
    
    Promotion* current = head;
    while (current != NULL) {
        // 保存时间戳时不保存状态，而是在加载时根据时间重新计算
        if (fprintf(fp, "%s %s|%s|%d %f %f %ld %ld %d\n",
                current->id, current->name, current->description, current->type,
                current->value, current->threshold, current->start_time,
                current->end_time, current->status) < 0) {
            printf("写入优惠活动数据时发生错误！\n");
            break;
        }
        current = current->next;
    }
    
    fclose(fp);
    printf("优惠活动数据已保存到文件：%s\n", filename);
}

// 获取活动类型名称
const char* get_promotion_type_name(PromotionType type) {
    switch (type) {
        case PROMOTION_DISCOUNT: return "折扣";
        case PROMOTION_CASH_BACK: return "满减";
        case PROMOTION_FREE_SHIPPING: return "免运费";
        case PROMOTION_GIFT: return "赠品";
        default: return "未知";
    }
}

// 获取活动状态名称
const char* get_promotion_status_name(PromotionStatus status) {
    switch (status) {
        case PROMOTION_ACTIVE: return "活动中";
        case PROMOTION_INACTIVE: return "未开始";
        case PROMOTION_EXPIRED: return "已结束";
        default: return "未知";
    }
}

// 优惠活动管理菜单
void promotion_management_menu(Promotion** promotion_head) {
    int choice;
    
    do {
        printf("\n===== 优惠活动管理系统 =====\n");
        printf("1. 添加优惠活动\n");
        printf("2. 删除优惠活动\n");
        printf("3. 查询优惠活动\n");
        printf("4. 查看所有优惠活动\n");
        printf("5. 返回上级菜单\n");
        
        choice = get_valid_choice(1, 5);
        
        switch (choice) {
            case 1: {
                // 添加优惠活动
                char name[50];
                char description[200];
                PromotionType type;
                float value, threshold;
                time_t start_time, end_time;
                struct tm start_tm = {0}, end_tm = {0};
                char start_date[20], end_date[20];
                
                printf("请输入活动名称: ");
                fgets(name, sizeof(name), stdin);
                name[strcspn(name, "\n")] = '\0';
                
                printf("请输入活动描述: ");
                fgets(description, sizeof(description), stdin);
                description[strcspn(description, "\n")] = '\0';
                
                printf("\n请选择活动类型:\n");
                printf("1. 折扣优惠\n");
                printf("2. 满减优惠\n");
                printf("3. 免运费\n");
                
                int type_choice = get_valid_choice(1, 3);
                switch (type_choice) {
                    case 1: type = PROMOTION_DISCOUNT; break;
                    case 2: type = PROMOTION_CASH_BACK; break;
                    case 3: type = PROMOTION_FREE_SHIPPING; break;
                    default: type = PROMOTION_DISCOUNT;
                }
                
                if (type == PROMOTION_DISCOUNT) {
                    printf("请输入折扣率(0-1之间，如0.8表示8折): ");
                    scanf("%f", &value);
                    while (getchar() != '\n');
                    
                    if (value <= 0 || value >= 1) {
                        printf("折扣率必须在0-1之间！已设为默认值0.9\n");
                        value = 0.9f;
                    }
                } else {
                    printf("请输入优惠金额: ");
                    scanf("%f", &value);
                    while (getchar() != '\n');
                    
                    if (value <= 0) {
                        printf("优惠金额必须大于0！已设为默认值10\n");
                        value = 10.0f;
                    }
                }
                
                printf("请输入门槛金额(满多少可用): ");
                scanf("%f", &threshold);
                while (getchar() != '\n');
                
                if (threshold < 0) {
                    printf("门槛金额不能为负数！已设为默认值0\n");
                    threshold = 0.0f;
                }
                
                // 获取当前时间作为默认值
                time_t now = time(NULL);
                struct tm* t = localtime(&now);
                sprintf(start_date, "%04d-%02d-%02d", t->tm_year+1900, t->tm_mon+1, t->tm_mday);
                
                // 默认结束时间为三个月后
                t->tm_mon += 3;
                if (t->tm_mon > 11) {
                    t->tm_year += t->tm_mon / 12;
                    t->tm_mon = t->tm_mon % 12;
                }
                sprintf(end_date, "%04d-%02d-%02d", t->tm_year+1900, t->tm_mon+1, t->tm_mday);
                
                printf("请输入开始日期(YYYY-MM-DD) [%s]: ", start_date);
                char input[20];
                fgets(input, sizeof(input), stdin);
                input[strcspn(input, "\n")] = '\0';
                
                if (strlen(input) > 0) {
                    strncpy(start_date, input, sizeof(start_date) - 1);
                    start_date[sizeof(start_date) - 1] = '\0';
                }
                
                printf("请输入结束日期(YYYY-MM-DD) [%s]: ", end_date);
                fgets(input, sizeof(input), stdin);
                input[strcspn(input, "\n")] = '\0';
                
                if (strlen(input) > 0) {
                    strncpy(end_date, input, sizeof(end_date) - 1);
                    end_date[sizeof(end_date) - 1] = '\0';
                }
                
                // 验证并解析日期字符串
                int start_year, start_month, start_day;
                int end_year, end_month, end_day;
                int valid_start_date = 0;
                int valid_end_date = 0;
                
                // 验证开始日期格式
                if (sscanf(start_date, "%d-%d-%d", &start_year, &start_month, &start_day) == 3) {
                    if (start_year >= 1900 && start_month >= 1 && start_month <= 12 && start_day >= 1 && start_day <= 31) {
                        valid_start_date = 1;
                        memset(&start_tm, 0, sizeof(struct tm));
                        start_tm.tm_year = start_year - 1900;  // 年份需要减去1900
                        start_tm.tm_mon = start_month - 1;     // 月份从0开始
                        start_tm.tm_mday = start_day;
                        start_tm.tm_hour = 0;
                        start_tm.tm_min = 0;
                        start_tm.tm_sec = 0;
                        start_tm.tm_isdst = -1;  // 让系统自动判断是否为夏令时
                    }
                }
                
                if (!valid_start_date) {
                    printf("开始日期格式无效，已使用当前日期！\n");
                    time_t now = time(NULL);
                    start_tm = *localtime(&now);  // 使用localtime而不是直接赋值
                    start_tm.tm_hour = 0;
                    start_tm.tm_min = 0;
                    start_tm.tm_sec = 0;
                }
                
                // 验证结束日期格式
                if (sscanf(end_date, "%d-%d-%d", &end_year, &end_month, &end_day) == 3) {
                    if (end_year >= 1900 && end_month >= 1 && end_month <= 12 && end_day >= 1 && end_day <= 31) {
                        valid_end_date = 1;
                        memset(&end_tm, 0, sizeof(struct tm));
                        end_tm.tm_year = end_year - 1900;
                        end_tm.tm_mon = end_month - 1;
                        end_tm.tm_mday = end_day;
                        end_tm.tm_hour = 23;
                        end_tm.tm_min = 59;
                        end_tm.tm_sec = 59;
                        end_tm.tm_isdst = -1;  // 让系统自动判断是否为夏令时
                    }
                }
                
                if (!valid_end_date) {
                    printf("结束日期格式无效，已使用默认结束日期（一个月后）！\n");
                    time_t now = time(NULL);
                    end_tm = *localtime(&now);  // 使用localtime而不是直接赋值
                    end_tm.tm_mon += 1;
                    if (end_tm.tm_mon > 11) {
                        end_tm.tm_mon = 0;
                        end_tm.tm_year += 1;
                    }
                    end_tm.tm_hour = 23;
                    end_tm.tm_min = 59;
                    end_tm.tm_sec = 59;
                }
                
                start_time = mktime(&start_tm);
                end_time = mktime(&end_tm);
                
                // 确保结束时间晚于开始时间
                if (difftime(end_time, start_time) <= 0) {
                    printf("结束时间必须晚于开始时间，已自动调整为开始时间后一个月！\n");
                    end_tm = start_tm;
                    end_tm.tm_mon += 1;
                    if (end_tm.tm_mon > 11) {
                        end_tm.tm_mon = 0;
                        end_tm.tm_year += 1;
                    }
                    end_tm.tm_hour = 23;
                    end_tm.tm_min = 59;
                    end_tm.tm_sec = 59;
                    end_time = mktime(&end_tm);
                }
                
                // 创建优惠活动
                Promotion* new_promotion = create_promotion(name, description, type, value, threshold, start_time, end_time);
                if (new_promotion != NULL) {
                    add_promotion(promotion_head, new_promotion);
                }
                break;
            }
            case 2: {
                // 删除优惠活动
                if (*promotion_head == NULL) {
                    printf("优惠活动列表为空！\n");
                    break;
                }
                
                print_promotion_list(*promotion_head);
                
                char promotion_id[20];
                printf("\n请输入要删除的活动ID: ");
                scanf("%19s", promotion_id);
                while (getchar() != '\n');
                
                delete_promotion(promotion_head, promotion_id);
                break;
            }
            case 3: {
                // 查询优惠活动
                if (*promotion_head == NULL) {
                    printf("优惠活动列表为空！\n");
                    break;
                }
                
                char promotion_id[20];
                printf("请输入要查询的活动ID: ");
                scanf("%19s", promotion_id);
                while (getchar() != '\n');
                
                Promotion* promotion = find_promotion_by_id(*promotion_head, promotion_id);
                if (promotion != NULL) {
                    // 更新活动状态
                    update_promotion_status(promotion);
                    
                    printf("\n活动信息:\n");
                    printf("活动ID: %s\n", promotion->id);
                    printf("活动名称: %s\n", promotion->name);
                    printf("活动描述: %s\n", promotion->description);
                    printf("活动类型: %s\n", get_promotion_type_name(promotion->type));
                    printf("活动状态: %s\n", get_promotion_status_name(promotion->status));
                    
                    if (promotion->type == PROMOTION_DISCOUNT) {
                        printf("折扣率: %.1f折\n", promotion->value * 10);
                    } else {
                        printf("优惠金额: %.2f元\n", promotion->value);
                    }
                    
                    printf("门槛金额: %.2f元\n", promotion->threshold);
                    
                    char start_time_str[20];
                    char end_time_str[20];
                    strftime(start_time_str, sizeof(start_time_str), "%Y-%m-%d %H:%M:%S",
                            localtime(&promotion->start_time));
                    strftime(end_time_str, sizeof(end_time_str), "%Y-%m-%d %H:%M:%S",
                            localtime(&promotion->end_time));
                    
                    printf("开始时间: %s\n", start_time_str);
                    printf("结束时间: %s\n", end_time_str);
                } else {
                    printf("未找到该活动ID！\n");
                }
                break;
            }
            case 4:
                // 查看所有优惠活动
                print_promotion_list(*promotion_head);
                break;
            case 5:
                return;
        }
        
        pause_program();
    } while (1);
}