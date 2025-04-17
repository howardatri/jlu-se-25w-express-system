#include "../include/express.h"
#include "../include/utils.h"
#include "../include/user.h"
#include "../include/user_level.h"
#include "../include/shelf.h"
#include "../include/finance.h"
#include "../include/payment.h"
#include "../include/storage.h"
#include <time.h>
#include <stdlib.h>

extern User* current_user;

// 函数原型声明
Shelf* allocate_shelf_for_express(Express* express);

void generate_express_id(char* buffer) {
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    int year = t->tm_year + 1900;
    int month = t->tm_mon + 1;
    int day = t->tm_mday;
    int seq = 1; // 默认从1开始
    
    // 从文件中读取最后一个单号
    FILE* fp = safe_fopen(EXPRESS_DATA_FILE, "r", 3);
    if (fp != NULL) {
        char line[512];
        char last_id[20] = "";
        
        // 读取文件中的所有行，找到最后一个单号
        while (fgets(line, sizeof(line), fp)) {
            sscanf(line, "%19s", last_id);
        }
        
        // 如果找到了最后一个单号，解析它
        if (strlen(last_id) > 0) {
            int last_year, last_month, last_day, last_seq;
            if (sscanf(last_id, "EX%04d%02d%02d%04d", &last_year, &last_month, &last_day, &last_seq) == 4) {
                // 如果是同一天，则序号加1，否则重新从1开始
                if (last_year == year && last_month == month && last_day == day) {
                    seq = last_seq + 1;
                    if (seq > 9999) seq = 1; // 超过9999则重新从1开始
                }
            }
        }
        
        fclose(fp);
    }
    
    // 生成新的单号
    sprintf(buffer, "EX%04d%02d%02d%04d", year, month, day, seq);
}

// 生成取件码
void generate_pickup_code(char* buffer) {
    // 生成6位随机数字作为取件码
    sprintf(buffer, "%06d", rand() % 1000000);
}

// 获取快递大小名称
const char* get_express_size_name(ExpressSize size) {
    switch (size) {
        case SIZE_SMALL: return "小型";
        case SIZE_MEDIUM: return "中型";
        case SIZE_LARGE: return "大型";
        default: return "未知";
    }
}

// 获取快递时效名称
const char* get_express_speed_name(ExpressSpeed speed) {
    switch (speed) {
        case NORMAL_SPEED: return "普通";
        case FAST_SPEED: return "快速";
        case EXPRESS_SPEED: return "特快";
        default: return "未知";
    }
}

// 获取快递状态名称
const char* get_express_status_name(ExpressStatus status) {
    switch (status) {
        case WAITING_SHIP: return "待发件";
        case PENDING: return "待取件";
        case PICKED: return "已取件";
        case SHIPPED: return "已发件";
        case EXPIRED: return "已过期";
        default: return "未知";
    }
}

// 发送取件码短信
void send_pickup_code(const char* phone, const char* pickup_code, const char* express_id, const char* shelf_id) {
    char msg[200];
    snprintf(msg, sizeof(msg),
             "您有一个快递已到达驿站，请及时取件。\n"
             "快递单号：%s\n"
             "取件码：%s\n"
             "存放货架：%s\n"
             "请在7天内凭取件码到驿站取件，过期将被退回。",
             express_id, pickup_code, shelf_id);
    send_sms_simulate(phone, msg);
}

// 验证取件码
int validate_pickup_code(Express* head, const char* express_id, const char* input_code) {
    Express* current = head;
    
    while (current != NULL) {
        if (strcmp(current->id, express_id) == 0) {
            if (strcmp(current->pickup_code, input_code) == 0) {
                return 1; // 取件码正确
            } else {
                return 0; // 取件码错误
            }
        }
        current = current->next;
    }
    
    return 0; // 未找到快递
}

Express* create_express_node() {
    Express* node = (Express*)malloc(sizeof(Express));
    if (node == NULL) {
        printf("内存分配失败！\n");
        return NULL;
    }
    
    // 生成快递单号
    generate_express_id(node->id);
    
    // 生成取件码
    generate_pickup_code(node->pickup_code);
    
    // 使用当前用户信息作为寄件人信息
    if (current_user != NULL && current_user->role == CUSTOMER) {
        strncpy(node->sender, current_user->username, sizeof(node->sender) - 1);
        node->sender[sizeof(node->sender) - 1] = '\0';
        
        strncpy(node->sender_phone, current_user->phone, sizeof(node->sender_phone) - 1);
        node->sender_phone[sizeof(node->sender_phone) - 1] = '\0';
    } else {
        clear_input_buffer();
        printf("请输入寄件人姓名: ");
        fgets(node->sender, sizeof(node->sender), stdin);
        node->sender[strcspn(node->sender, "\n")] = 0;
        
        do {
            clear_input_buffer();
            printf("请输入寄件人手机号: ");
            fgets(node->sender_phone, sizeof(node->sender_phone), stdin);
            node->sender_phone[strcspn(node->sender_phone, "\n")] = 0;
            if (!validate_phone(node->sender_phone)) {
                printf("手机号格式不正确，请输入11位数字且以1开头的手机号！\n");
            }
        } while (!validate_phone(node->sender_phone));
    }
    
    // 收件人姓名验证
    clear_input_buffer();
    do {
        printf("请输入收件人姓名: ");
        fgets(node->receiver, sizeof(node->receiver), stdin);
        node->receiver[strcspn(node->receiver, "\n")] = 0;
        
        if (strlen(node->receiver) == 0) {
            printf("收件人姓名不能为空，请重新输入！\n");
            continue;
        }
        
        // 添加收件人姓名确认环节
        printf("您输入的收件人姓名是: %s\n", node->receiver);
        printf("确认使用该收件人姓名? (1:确认 0:重新输入): ");
        int confirm = get_valid_choice(0, 1);
        if (confirm) {
            break;
        }
    } while (1);
    
    do {
        clear_input_buffer();
        printf("请输入收件人手机号: ");
        fgets(node->phone, sizeof(node->phone), stdin);
        node->phone[strcspn(node->phone, "\n")] = 0;
        
        if (!validate_phone(node->phone)) {
            printf("手机号格式不正确，请输入11位数字且以1开头的手机号！\n");
            continue;
        }
        
        // 添加收件人手机号确认环节
        printf("您输入的收件人手机号是: %s\n", node->phone);
        printf("确认使用该手机号? (1:确认 0:重新输入): ");
        clear_input_buffer();
        int confirm = get_valid_choice(0, 1);
        if (confirm) {
            break;
        }
    } while (1);
    
    // 收件地址验证
    clear_input_buffer();
    do {
        printf("请输入收件地址: ");
        fgets(node->fake_address, sizeof(node->fake_address), stdin);
        node->fake_address[strcspn(node->fake_address, "\n")] = '\0';
        
        if (strlen(node->fake_address) == 0) {
            printf("收件地址不能为空，请重新输入！\n");
            continue;
        }
        if (strlen(node->fake_address) >= sizeof(node->fake_address) - 1) {
            printf("收件地址过长，请控制在%d个字符以内！\n", (int)sizeof(node->fake_address) - 2);
            node->fake_address[0] = '\0'; // 清空输入，强制重新输入
            continue;
        }
        
        // 添加收件地址确认环节
        printf("您输入的收件地址是: %s\n", node->fake_address);
        printf("确认使用该地址? (1:确认 0:重新输入): ");
        int confirm = get_valid_choice(0, 1);
        if (confirm) {
            break;
        }
    } while (1);
    
    do {
        node->weight = get_positive_float("请输入快递重量(kg): ");
        
        // 添加快递重量确认环节
        printf("您输入的快递重量是: %.2fkg\n", node->weight);
        printf("确认使用该重量? (1:确认 0:重新输入): ");
        int confirm = get_valid_choice(0, 1);
        if (confirm) {
            break;
        }
    } while (1);
    
    // 根据重量给出大小建议
    ExpressSize suggested_size;
    if (node->weight <= 5.0) {
        suggested_size = SIZE_SMALL;
        printf("根据重量 %.2fkg，建议选择小型快递\n", node->weight);
    } else if (node->weight <= 20.0) {
        suggested_size = SIZE_MEDIUM;
        printf("根据重量 %.2fkg，建议选择中型快递\n", node->weight);
    } else {
        suggested_size = SIZE_LARGE;
        printf("根据重量 %.2fkg，建议选择大型快递\n", node->weight);
    }
    
    int size_choice;
    do {
        printf("请选择快递大小:\n");
        printf("1. 小型 (适合5kg以下)\n");
        printf("2. 中型 (适合5-20kg)\n");
        printf("3. 大型 (适合20kg以上)\n");
        size_choice = get_valid_choice(1, 3);
        
        // 添加快递大小确认环节
        printf("您选择的快递大小是: %s\n", 
               size_choice == 1 ? "小型" : 
               size_choice == 2 ? "中型" : "大型");
        printf("确认使用该快递大小? (1:确认 0:重新选择): ");
        int confirm = get_valid_choice(0, 1);
        if (confirm) {
            break;
        }
    } while (1);
    
    // 检查选择的大小是否与重量匹配
    if ((size_choice == 1 && node->weight > 5.0) ||
        (size_choice == 2 && node->weight > 20.0)) {
        printf("警告：您选择的快递大小可能不适合当前重量！\n");
        printf("是否确认选择？(1:是 0:否): ");
        int confirm = get_valid_choice(0, 1);
        if (!confirm) {
            // 使用建议的大小
            switch (suggested_size) {
                case SIZE_SMALL: size_choice = 1; break;
                case SIZE_MEDIUM: size_choice = 2; break;
                case SIZE_LARGE: size_choice = 3; break;
            }
            printf("已自动调整为建议的快递大小\n");
        }
    }
    
    switch (size_choice) {
        case 1: node->size = SIZE_SMALL; break;
        case 2: node->size = SIZE_MEDIUM; break;
        case 3: node->size = SIZE_LARGE; break;
        default: node->size = SIZE_SMALL;
    }
    
    // 贵重物品选择验证
    clear_input_buffer();
    do {
        printf("是否为贵重物品？(1:是 0:否): ");
        char input[10];
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = 0;
        
        if (strcmp(input, "0") == 0) {
            node->is_valuable = 0;
            printf("您选择了: 非贵重物品\n");
            printf("确认该选择? (1:确认 0:重新选择): ");
            int confirm = get_valid_choice(0, 1);
            if (confirm) break;
        } else if (strcmp(input, "1") == 0) {
            node->is_valuable = 1;
            printf("您选择了: 贵重物品\n");
            printf("确认该选择? (1:确认 0:重新选择): ");
            int confirm = get_valid_choice(0, 1);
            if (confirm) break;
        } else {
            printf("输入无效，请输入0或1！\n");
        }
    } while (1);
    
    int speed_choice;
    do {
        printf("请选择快递时效:\n");
        printf("1. 普通\n");
        printf("2. 快速\n");
        printf("3. 特快\n");
        speed_choice = get_valid_choice(1, 3);
        
        // 添加快递时效确认环节
        printf("您选择的快递时效是: %s\n", 
               speed_choice == 1 ? "普通" : 
               speed_choice == 2 ? "快速" : "特快");
        printf("确认使用该快递时效? (1:确认 0:重新选择): ");
        int confirm = get_valid_choice(0, 1);
        if (confirm) {
            break;
        }
    } while (1);
    switch (speed_choice) {
        case 1: node->speed = NORMAL_SPEED; break;
        case 2: node->speed = FAST_SPEED; break;
        case 3: node->speed = EXPRESS_SPEED; break;
        default: node->speed = NORMAL_SPEED;
    }
    
    do {
        node->distance = get_positive_float("请输入运送距离(公里): ");
        
        // 添加运送距离确认环节
        printf("您输入的运送距离是: %.2f公里\n", node->distance);
        printf("确认使用该距离? (1:确认 0:重新输入): ");
        int confirm = get_valid_choice(0, 1);
        if (confirm) {
            break;
        }
    } while (1);
    
    // 到楼服务选择验证
    clear_input_buffer();
    do {
        printf("是否需要到楼服务？(1:是 0:否): ");
        char input[10];
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = 0;
        
        if (strcmp(input, "0") == 0) {
            node->delivery_to_door = 0;
            printf("您选择了: 不需要到楼服务\n");
            printf("确认该选择? (1:确认 0:重新选择): ");
            int confirm = get_valid_choice(0, 1);
            if (confirm) break;
        } else if (strcmp(input, "1") == 0) {
            node->delivery_to_door = 1;
            printf("您选择了: 需要到楼服务\n");
            printf("确认该选择? (1:确认 0:重新选择): ");
            int confirm = get_valid_choice(0, 1);
            if (confirm) break;
        } else {
            printf("输入无效，请输入0或1！\n");
        }
    } while (1);
    
    // 计算会员折扣
    float discount = 1.0f;
    if (current_user != NULL && current_user->role == CUSTOMER) {
        discount = get_member_level_discount(current_user->level);
    }
    
    // 计算费用
    node->cost = calculate_cost(node->weight, node->size, node->is_valuable, node->speed, node->distance, discount, node->delivery_to_door);
    
    node->create_time = time(NULL);
    node->status = WAITING_SHIP; // 设置为待发件状态
    node->shelf_id[0] = '\0'; // 初始化为空字符串
    node->next = NULL;
    
    // 显示快递信息总览并确认
    printf("\n=== 快递信息总览 ===\n");
    printf("快递单号: %s\n", node->id);
    printf("寄件人: %s (%s)\n", node->sender, node->sender_phone);
    printf("收件人: %s (%s)\n", node->receiver, node->phone);
    printf("收件地址: %s\n", node->fake_address);
    printf("快递重量: %.2fkg\n", node->weight);
    printf("快递大小: %s\n", get_express_size_name(node->size));
    printf("快递时效: %s\n", get_express_speed_name(node->speed));
    printf("运送距离: %.2f公里\n", node->distance);
    printf("贵重物品: %s\n", node->is_valuable ? "是" : "否");
    printf("到楼服务: %s\n", node->delivery_to_door ? "是" : "否");
    printf("费用: %.2f元\n", node->cost);
    
    printf("\n确认以上快递信息? (1:确认 0:取消): ");
    int final_confirm = get_valid_choice(0, 1);
    if (!final_confirm) {
        free(node);
        printf("已取消寄件操作。\n");
        return NULL;
    }
    
    // 在寄件阶段不分配货架，等待管理员发件时再分配
    printf("快递已创建，等待管理员发件后将分配到合适的货架\n");
    
    // 更新用户累计消费金额并检查是否升级
    if (current_user != NULL && current_user->role == CUSTOMER) {
        current_user->total_cost += node->cost;
        
        // 检查是否达到升级条件
        MemberLevel new_level = check_and_update_member_level(current_user->total_cost, current_user->level);
        
        if (new_level > current_user->level) {
            current_user->level = new_level;
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "恭喜您已升级为%s！\n"
                     "当前累计消费：%.2f元\n"
                     "可享受%.1f折优惠",
                     get_member_level_name(new_level),
                     current_user->total_cost,
                     get_member_level_discount(new_level) * 10);
            send_sms_simulate(current_user->phone, msg);
            printf("\n恭喜！您已升级为%s，可享受%.1f折优惠！\n", 
                   get_member_level_name(new_level),
                   get_member_level_discount(new_level) * 10);
        }
        
        // 保存用户数据
        extern void save_users(const char* filename, User* head);
        extern User* load_users(const char* filename);
        
        User* user_list = load_users("data/users.txt");
        User* current = user_list;
        
        // 更新用户列表中的当前用户信息
        while (current != NULL) {
            if (strcmp(current->username, current_user->username) == 0) {
                current->total_cost = current_user->total_cost;
                current->level = current_user->level;
                break;
            }
            current = current->next;
        }
        
        // 保存更新后的用户列表
        save_users("data/users.txt", user_list);
    }
    
    return node;
}

void insert_express(Express** head) {
    Express* new_node = create_express_node();
    if (new_node == NULL || new_node == NULL) return; // 如果用户取消了寄件或内存分配失败
    
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
    printf("快递单号: %s\n", new_node->id);
    printf("取件码: %s\n", new_node->pickup_code);
    printf("费用: %.2f元\n", new_node->cost);
    
    // 处理支付
    printf("\n请选择支付方式:\n");
    printf("1. 现金\n");
    printf("2. 微信\n");
    printf("3. 支付宝\n");
    printf("4. 银行卡\n");
    printf("5. 其他\n");
    
    int payment_choice = get_valid_choice(1, 5);
    PaymentMethod method;
    
    switch (payment_choice) {
        case 1: method = PAYMENT_CASH; break;
        case 2: method = PAYMENT_WECHAT; break;
        case 3: method = PAYMENT_ALIPAY; break;
        case 4: method = PAYMENT_CARD; break;
        case 5: method = PAYMENT_OTHER; break;
        default: method = PAYMENT_CASH;
    }
    
    // 创建支付记录
    Payment* payment = create_payment(new_node->id, method, new_node->cost);
    if (payment == NULL) {
        printf("创建支付记录失败！\n");
        return;
    }
    
    // 加载交易记录
    Transaction* transaction_list = load_transactions(TRANSACTION_DATA_FILE);
    
    // 处理支付
    PaymentStatus status = process_payment_transaction(payment, &transaction_list);
    
    if (status == PAYMENT_SUCCESS) {
        printf("支付成功！\n");
        
        // 创建按日期命名的交易记录文件
        time_t now = time(NULL);
        struct tm* t = localtime(&now);
        char date_filename[100];
        sprintf(date_filename, "data/transactions_%04d%02d%02d.txt", 
                t->tm_year+1900, t->tm_mon+1, t->tm_mday);
        
        // 保存交易记录到日期文件
        FILE* fp = safe_fopen(date_filename, "a", 3);
        if (fp != NULL) {
            char time_str[20];
            strftime(time_str, sizeof(time_str), "%H:%M:%S", t);
            
            fprintf(fp, "%s %s %s %.2f %d %s\n", 
                    new_node->id, time_str, 
                    get_payment_method_name(method), 
                    new_node->cost, TRANSACTION_INCOME, 
                    new_node->sender);
            fclose(fp);
        }
        
        // 保存交易记录到主文件
        save_transactions(TRANSACTION_DATA_FILE, transaction_list);
        
        // 加载支付记录
        Payment* payment_list = load_payments(PAYMENT_DATA_FILE);
        
        // 添加新的支付记录
        if (payment_list == NULL) {
            payment_list = payment;
        } else {
            Payment* current = payment_list;
            while (current->next != NULL) {
                current = current->next;
            }
            current->next = payment;
        }
        
        // 保存支付记录
        save_payments(PAYMENT_DATA_FILE, payment_list);
        
        // 保存快递数据
        save_express(EXPRESS_DATA_FILE, *head);
    } else {
        printf("支付失败，请稍后重试！\n");
        free(payment);
    }
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
    printf("%-15s %-10s %-10s %-12s %-8s %-8s %-8s %-8s %-20s %s\n",
           "快递单号", "寄件人", "收件人", "收件人电话",
           "重量(kg)", "大小", "时效", "费用(元)", "创建时间", "状态");
    printf("----------------------------------------"
           "----------------------------------------\n");
    
    Express* current = head;
    int found = 0;
    
    while (current != NULL) {
        if (current->status != SHIPPED) {
            found = 1;
            char time_str[20];
            strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S",
                    localtime(&current->create_time));
            
            const char* status_str = get_express_status_name(current->status);
            const char* size_str = get_express_size_name(current->size);
            const char* speed_str = get_express_speed_name(current->speed);
            
            printf("%-15s %-10s %-10s %-12s %-8.2f %-8s %-8s %-8.2f %-20s %s\n",
                   current->id, current->sender, current->receiver,
                   current->phone, current->weight, size_str, speed_str,
                   current->cost, time_str, status_str);
        }
        current = current->next;
    }
    
    if (!found) {
        printf("\n当前没有未发件的快递！\n");
    }
}

float calculate_cost(float weight, ExpressSize size, int is_valuable, ExpressSpeed speed, float distance, float discount, int express_delivery_to_door) {
    float base_cost = 10.0f;  // 基础费用
    float size_factor = 1.0f; // 大小系数
    float speed_factor = 1.0f; // 时效系数
    float valuable_factor = 1.0f; // 贵重物品系数
    float distance_factor = 0.5f; // 距离系数（每公里增加费用）
    float delivery_fee = 0.0f; // 到楼服务费
    
    // 验证距离在合理范围内
    if (distance > 5000) {
        printf("警告：运送距离超过5000公里，已自动调整为5000公里\n");
        distance = 5000;
    }
    
    // 根据大小调整系数
    switch (size) {
        case SIZE_SMALL: size_factor = 1.0f; break;
        case SIZE_MEDIUM: size_factor = 1.5f; break;
        case SIZE_LARGE: size_factor = 2.0f; break;
    }
    
    // 根据时效调整系数
    switch (speed) {
        case NORMAL_SPEED: speed_factor = 1.0f; break;
        case FAST_SPEED: speed_factor = 1.5f; break;
        case EXPRESS_SPEED: speed_factor = 2.0f; break;
    }
    
    // 贵重物品额外收费
    if (is_valuable) {
        valuable_factor = 1.5f;
    }
    
    // 计算基础运费
    float weight_cost = weight <= 1.0f ? base_cost : base_cost + (weight - 1.0f) * 5.0f;
    float total_cost = weight_cost * size_factor * speed_factor * valuable_factor + distance * distance_factor;
    
    // 添加到楼服务费（如果需要）
    if (express_delivery_to_door) {
        delivery_fee = 5.0f;
        total_cost += delivery_fee;
    }
    
    // 检查并应用可用的优惠活动
    Promotion* promotions = load_promotions("data/promotions.txt");
    if (promotions != NULL) {
        // 查找适用的优惠活动
        Promotion* available_promotions = find_available_promotions(promotions, total_cost);
        if (available_promotions != NULL) {
            // 应用优惠活动
            total_cost = apply_promotion(available_promotions, total_cost);
            // 不调用未定义的free_promotion_list函数
            // 手动释放可用活动列表
            Promotion* current = available_promotions;
            Promotion* next;
            while (current != NULL) {
                next = current->next;
                free(current);
                current = next;
            }
        }
        // 手动释放加载的活动列表
        Promotion* current = promotions;
        Promotion* next;
        while (current != NULL) {
            next = current->next;
            free(current);
            current = next;
        }
    }
    
    // 应用会员折扣
    total_cost *= discount;
    
    return total_cost;
}

void check_inventory_alerts(Express* head) {
    time_t current_time = time(NULL);
    const int alert_threshold = 7 * 24 * 60 * 60;  // 7天
    int pending_count = 0;
    int expired_count = 0;
    int waiting_ship_count = 0;
    int total_inventory = 0;
    
    Express* current = head;
    while (current != NULL) {
        if (current->status == PENDING) {
            pending_count++;
            total_inventory++;
            double diff_time = difftime(current_time, current->create_time);
            
            if (diff_time > alert_threshold) {
                expired_count++;
                current->status = EXPIRED;
            }
        } else if (current->status == WAITING_SHIP) {
            waiting_ship_count++;
            total_inventory++;
        }
        current = current->next;
    }
    
    printf("\n库存状态:\n");
    printf("待取件快递数量: %d\n", pending_count);
    printf("待发件快递数量: %d\n", waiting_ship_count);
    printf("总库存数量: %d\n", total_inventory);
    
    // 显示预警信息
    if (expired_count > 0) {
        printf("警告：有 %d 个快递已超过7天未取！\n", expired_count);
    }
    
    // 添加总库存数量预警
    const int inventory_threshold = 90; // 设置总库存预警阈值为90
    if (total_inventory > inventory_threshold) {
        printf("警告：总库存数量已超过%d，请及时处理！\n", inventory_threshold);
    } else {
        printf("库存状态正常\n");
    }
}

// 管理员发件处理
void admin_ship_express(Express** head) {
    if (*head == NULL) {
        printf("快递列表为空！\n");
        return;
    }
    
    // 打印待发件的快递列表
    printf("\n待发件快递列表:\n");
    printf("%-15s %-10s %-10s %-12s %-8s %-8s %-20s\n",
           "快递单号", "寄件人", "收件人", "收件人电话",
           "重量(kg)", "费用(元)", "创建时间");
    printf("----------------------------------------"
           "----------------------------------------\n");
    
    Express* current = *head;
    int found = 0;
    int waiting_count = 0;
    
    while (current != NULL) {
        if (current->status == WAITING_SHIP) {
            char time_str[20];
            strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S",
                    localtime(&current->create_time));
            
            printf("%-15s %-10s %-10s %-12s %-8.2f %-8.2f %-20s\n",
                   current->id, current->sender, current->receiver,
                   current->phone, current->weight, current->cost,
                   time_str);
            found = 1;
            waiting_count++;
        }
        current = current->next;
    }
    
    if (!found) {
        printf("没有待发件的快递！\n");
        return;
    }
    
    printf("\n共有 %d 个待发件快递\n", waiting_count);
    printf("请选择操作:\n");
    printf("1. 发出指定单号的快递\n");
    printf("2. 一键发出所有待发件快递\n");
    printf("3. 返回上级菜单\n");
    
    int choice = get_valid_choice(1, 3);
    
    switch (choice) {
        case 1: {
            char express_id[20];
            printf("\n请输入要发件的快递单号: ");
            scanf("%19s", express_id);
            while (getchar() != '\n');
            
            current = *head;
            while (current != NULL) {
                if (strcmp(current->id, express_id) == 0) {
                    if (current->status == WAITING_SHIP) {
                        // 在发件时分配货架
                        Shelf* allocated_shelf = allocate_shelf_for_express(current);
                        if (allocated_shelf != NULL) {
                            printf("快递已分配到货架: %s，当前货架存放数量：%d/%d\n", 
                                   allocated_shelf->id, allocated_shelf->current_count, allocated_shelf->capacity);
                        } else {
                            printf("警告：无法分配货架！\n");
                        }
                        
                        current->status = PENDING;
                        printf("\n快递 %s 已成功发件！\n", express_id);
                        
                        // 发送短信通知
                        char msg[200];
                        snprintf(msg, sizeof(msg),
                                 "您的快递（单号：%s）已发出。\n"
                                 "收件人：%s\n"
                                 "收件人电话：%s\n"
                                 "重量：%.2fkg\n"
                                 "费用：%.2f元",
                                 current->id, current->receiver,
                                 current->phone, current->weight, current->cost);
                        send_sms_simulate(current->sender_phone, msg);
                        
                        // 发送取件码短信
                        send_pickup_code(current->phone, current->pickup_code, current->id, current->shelf_id);
                        
                        // 保存快递数据
                        save_express(EXPRESS_DATA_FILE, *head);
                    } else {
                        printf("该快递状态不是待发件！\n");
                    }
                    return;
                }
                current = current->next;
            }
            printf("未找到该快递单号！\n");
            break;
        }
        case 2: {
            int shipped_count = 0;
            current = *head;
            while (current != NULL) {
                if (current->status == WAITING_SHIP) {
                    // 在发件时分配货架
                    Shelf* allocated_shelf = allocate_shelf_for_express(current);
                    if (allocated_shelf != NULL) {
                        printf("快递 %s 已分配到货架: %s，当前货架存放数量：%d/%d\n", 
                               current->id, allocated_shelf->id, allocated_shelf->current_count, allocated_shelf->capacity);
                    } else {
                        printf("警告：快递 %s 无法分配货架！\n", current->id);
                    }
                    
                    current->status = PENDING;
                    shipped_count++;
                    
                    // 发送短信通知
                    char msg[200];
                    snprintf(msg, sizeof(msg),
                             "您的快递（单号：%s）已发出。\n"
                             "收件人：%s\n"
                             "收件人电话：%s\n"
                             "重量：%.2fkg\n"
                             "费用：%.2f元",
                             current->id, current->receiver,
                             current->phone, current->weight, current->cost);
                    send_sms_simulate(current->sender_phone, msg);
                    
                    // 发送取件码短信
                    send_pickup_code(current->phone, current->pickup_code, current->id, current->shelf_id);
                }
                current = current->next;
            }
            
            if (shipped_count > 0) {
                printf("\n已成功发出 %d 个快递！\n", shipped_count);
                // 保存快递数据
                save_express(EXPRESS_DATA_FILE, *head);
            }
            break;
        }
        case 3:
            return;
    }
}

// 自动分配货架
Shelf* allocate_shelf_for_express(Express* express) {
    // 每次操作前重新加载最新的货架数据
    Shelf* shelf_list = load_shelves("data/shelves.txt");
    
    // 根据快递大小选择对应的货架大小
    ShelfSize shelf_size;
    switch (express->size) {
        case SIZE_SMALL: shelf_size = SHELF_SMALL; break;
        case SIZE_MEDIUM: shelf_size = SHELF_MEDIUM; break;
        case SIZE_LARGE: shelf_size = SHELF_LARGE; break;
        default: shelf_size = SHELF_SMALL;
    }
    
    // 查找可用货架
    Shelf* available_shelf = find_available_shelf(shelf_list, shelf_size);
    
    // 如果没有找到可用货架，创建新货架
    if (available_shelf == NULL) {
        // 检查是否有任何货架
        if (shelf_list == NULL) {
            // 初始化三种大小的货架各一个
            printf("\n系统初始化货架...\n");
            Shelf* small_shelf = create_shelf(SHELF_SMALL, 20);
            
            // 保存第一个货架并更新货架列表
            if (small_shelf != NULL) {
                add_shelf(&shelf_list, small_shelf);
                printf("已创建小型货架: %s (容量: 20)\n", small_shelf->id);
                // 保存货架数据以更新文件
                save_shelves("data/shelves.txt", shelf_list);
            }
            
            Shelf* medium_shelf = create_shelf(SHELF_MEDIUM, 50);
            if (medium_shelf != NULL) {
                add_shelf(&shelf_list, medium_shelf);
                printf("已创建中型货架: %s (容量: 50)\n", medium_shelf->id);
                // 保存货架数据以更新文件
                save_shelves("data/shelves.txt", shelf_list);
            }
            
            Shelf* large_shelf = create_shelf(SHELF_LARGE, 100);
            if (large_shelf != NULL) {
                add_shelf(&shelf_list, large_shelf);
                printf("已创建大型货架: %s (容量: 100)\n", large_shelf->id);
                // 保存货架数据以更新文件
                save_shelves("data/shelves.txt", shelf_list);
            }
            
            // 重新查找可用货架
            available_shelf = find_available_shelf(shelf_list, shelf_size);
        } else {
            // 创建对应大小的新货架
            int capacity;
            switch (shelf_size) {
                case SHELF_SMALL: capacity = 20; break;
                case SHELF_MEDIUM: capacity = 50; break;
                case SHELF_LARGE: capacity = 100; break;
                default: capacity = 20;
            }
            
            printf("\n创建新的%s货架...\n", get_shelf_size_name(shelf_size));
            Shelf* new_shelf = create_shelf(shelf_size, capacity);
            if (new_shelf != NULL) {
                add_shelf(&shelf_list, new_shelf);
                printf("已创建%s货架: %s (容量: %d)\n", 
                       get_shelf_size_name(shelf_size), new_shelf->id, capacity);
                // 保存货架数据以更新文件
                save_shelves("data/shelves.txt", shelf_list);
                // 重新加载货架数据以确保获取最新状态
                shelf_list = load_shelves("data/shelves.txt");
                // 重新查找可用货架
                available_shelf = find_available_shelf(shelf_list, shelf_size);
            }
        }
    }
    
    // 分配货架
    if (available_shelf != NULL) {
        // 更新快递的货架ID
        strncpy(express->shelf_id, available_shelf->id, sizeof(express->shelf_id) - 1);
        express->shelf_id[sizeof(express->shelf_id) - 1] = '\0';
        
        // 更新货架的当前存放数量
        available_shelf->current_count++;
        
        // 如果货架已满，更新状态
        if (available_shelf->current_count >= available_shelf->capacity) {
            available_shelf->status = SHELF_FULL;
        }
        
        printf("快递已自动分配到货架: %s\n", available_shelf->id);
        printf("快递已成功分配到货架，当前货架存放数量：%d/%d\n", 
               available_shelf->current_count, available_shelf->capacity);
        
        // 保存货架数据 - 确保更新后的数据被正确写入文件
        // 先遍历货架列表，找到对应的货架并更新
        Shelf* current = shelf_list;
        while (current != NULL) {
            if (strcmp(current->id, available_shelf->id) == 0) {
                current->current_count = available_shelf->current_count;
                current->status = available_shelf->status;
                break;
            }
            current = current->next;
        }
        
        // 保存更新后的货架数据
        save_shelves("data/shelves.txt", shelf_list);
        
        // 返回分配的货架指针
        return available_shelf;
    } else {
        printf("警告：无法分配货架！\n");
        return NULL;
    }
}