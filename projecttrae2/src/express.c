#include "../include/express.h"
#include "../include/utils.h"
#include "../include/user.h"
#include <time.h>

extern User* current_user;

void generate_express_id(char* buffer) {
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    static int seq = 0;
    sprintf(buffer, "%04d%02d%02d%04d", t->tm_year+1900, t->tm_mon+1, t->tm_mday, ++seq%10000);
}

Express* create_express_node() {
    Express* node = (Express*)malloc(sizeof(Express));
    if (node == NULL) {
        printf("内存分配失败！\n");
        return NULL;
    }
    
    generate_express_id(node->id);
    
    printf("请输入寄件人姓名: ");
    scanf("%49s", node->sender);
    while (getchar() != '\n');
    
    printf("请输入收件人姓名: ");
    scanf("%49s", node->receiver);
    while (getchar() != '\n');
    
    do {
        printf("请输入收件人手机号: ");
        scanf("%11s", node->phone);
        while (getchar() != '\n');
    } while (!validate_phone(node->phone));
    
    node->weight = get_positive_float("请输入快递重量(kg): ");
    
    printf("是否需要到楼服务？(1:是 0:否): ");
    scanf("%d", &node->delivery_to_door);
    while (getchar() != '\n');
    
    node->cost = calculate_cost(node->weight);
    node->create_time = time(NULL);
    node->status = PENDING;
    node->next = NULL;
    
    // 更新用户累计消费金额并检查是否升级
    if (current_user != NULL && current_user->role == CUSTOMER) {
        current_user->total_cost += node->cost;
        
        // 检查是否达到升级条件
        if (current_user->level == NORMAL && current_user->total_cost >= 1000.0f) {
            current_user->level = GOLD;
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "恭喜您已升级为金牌会员！\n"
                     "当前累计消费：%.2f元\n"
                     "可享受9折优惠",
                     current_user->total_cost);
            send_sms_simulate(current_user->phone, msg);
            printf("\n恭喜！您已升级为金牌会员，可享受9折优惠！\n");
        }
    }
    
    return node;
}

void insert_express(Express** head) {
    Express* new_node = create_express_node();
    if (new_node == NULL) return;
    
    if (*head == NULL) {
        *head = new_node;
    } else {
        Express* current = *head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_node;
    }
    
    printf("\n快递信息已添加成功！\n");
    printf("费用: %.2f元\n", new_node->cost);
}

int delete_express(Express** head) {
    if (*head == NULL) {
        printf("快递列表为空！\n");
        return 0;
    }
    
    char id[20];
    printf("请输入要删除的快递单号: ");
    scanf("%19s", id);
    while (getchar() != '\n');
    
    Express* current = *head;
    Express* prev = NULL;
    
    while (current != NULL && strcmp(current->id, id) != 0) {
        prev = current;
        current = current->next;
    }
    
    if (current == NULL) {
        printf("未找到该快递单号！\n");
        return 0;
    }
    
    if (prev == NULL) {
        *head = current->next;
    } else {
        prev->next = current->next;
    }
    
    free(current);
    printf("快递信息已删除成功！\n");
    return 1;
}

void print_express_list(Express* head) {
    if (head == NULL) {
        printf("快递列表为空！\n");
        return;
    }
    
    printf("\n快递列表:\n");
    printf("%-15s %-10s %-10s %-12s %-8s %-8s %-20s %s\n",
           "快递单号", "寄件人", "收件人", "收件人电话",
           "重量(kg)", "费用(元)", "创建时间", "状态");
    printf("----------------------------------------"
           "----------------------------------------\n");
    
    Express* current = head;
    while (current != NULL) {
        char time_str[20];
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S",
                localtime(&current->create_time));
        
        const char* status_str;
        switch (current->status) {
            case PENDING: status_str = "待取件"; break;
            case PICKED: status_str = "已取件"; break;
            case EXPIRED: status_str = "已过期"; break;
            default: status_str = "未知"; break;
        }
        
        printf("%-15s %-10s %-10s %-12s %-8.2f %-8.2f %-20s %s\n",
               current->id, current->sender, current->receiver,
               current->phone, current->weight, current->cost,
               time_str, status_str);
        
        current = current->next;
    }
}

float calculate_cost(float weight) {
    float base_cost = 10.0f;  // 基础费用
    float unit_cost = 5.0f;   // 每公斤增加费用
    float delivery_fee = 0.0f; // 到楼服务费
    
    // 计算基础运费
    float total_cost = weight <= 1.0f ? base_cost : base_cost + (weight - 1.0f) * unit_cost;
    
    // 添加到楼服务费（如果需要）
    if (current_user != NULL && current_user->delivery_to_door) {
        delivery_fee = (current_user->level == GOLD) ? 3.0f : 5.0f;
        total_cost += delivery_fee;
    }
    
    // 应用会员折扣
    if (current_user != NULL && current_user->level == GOLD) {
        total_cost *= 0.9f;  // 金牌会员9折
    } else {
        total_cost *= 0.95f; // 普通会员9.5折
    }
    
    // 应用满减优惠（满100减20）
    if (total_cost >= 100.0f) {
        total_cost -= 20.0f;
    }
    
    return total_cost;
}

void check_inventory_alerts(Express* head) {
    time_t current_time = time(NULL);
    const int alert_threshold = 7 * 24 * 60 * 60;  // 7天
    int pending_count = 0;
    int expired_count = 0;
    
    Express* current = head;
    while (current != NULL) {
        if (current->status == PENDING) {
            pending_count++;
            double diff_time = difftime(current_time, current->create_time);
            
            if (diff_time > alert_threshold) {
                expired_count++;
                current->status = EXPIRED;
            }
        }
        current = current->next;
    }
    
    printf("\n库存状态:\n");
    printf("待取件快递数量: %d\n", pending_count);
    if (expired_count > 0) {
        printf("警告：有 %d 个快递已超过7天未取！\n", expired_count);
    }
}