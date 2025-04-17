#include "../include/user.h"
#include "../include/utils.h"
#include "../include/shelf.h"
#include "../include/express.h"
#include "../include/storage.h"
#include <time.h>
#include <ctype.h>

User* user_login(User* head) {
    char username[50];
    char password[50];
    
    printf("请输入用户名: ");
    fgets(username, sizeof(username), stdin);
    username[strcspn(username, "\n")] = 0;
    
    printf("请输入密码: ");
    fgets(password, sizeof(password), stdin);
    password[strcspn(password, "\n")] = 0;
    
    User* current = head;
    while (current != NULL) {
        if (strcmp(current->username, username) == 0 &&
            strcmp(current->password, password) == 0) {
            printf("\n登录成功！\n");
            return current;
        }
        current = current->next;
    }
    
    printf("\n用户名或密码错误！\n");
    return NULL;
}

// 检查密码强度
int check_password_strength(const char* password) {
    int length = strlen(password);
    int has_upper = 0, has_lower = 0, has_digit = 0, has_special = 0;
    
    if (length < 6) {
        return 0; // 密码太短
    }
    
    for (int i = 0; i < length; i++) {
        if (isupper(password[i])) has_upper = 1;
        else if (islower(password[i])) has_lower = 1;
        else if (isdigit(password[i])) has_digit = 1;
        else has_special = 1;
    }
    
    int strength = has_upper + has_lower + has_digit + has_special;
    return strength;
}

// 显示密码强度建议
void show_password_strength_advice(const char* password) {
    int strength = check_password_strength(password);
    
    printf("\n密码强度: ");
    if (strength <= 1) {
        printf("非常弱\n");
        printf("建议: 密码应至少包含大写字母、小写字母、数字和特殊字符中的三种，且长度不少于6位。\n");
    } else if (strength == 2) {
        printf("弱\n");
        printf("建议: 密码应增加更多类型的字符，如大写字母、特殊字符等。\n");
    } else if (strength == 3) {
        printf("中等\n");
        printf("建议: 密码强度尚可，但可以增加其他类型的字符使其更强。\n");
    } else {
        printf("强\n");
        printf("建议: 密码强度良好。\n");
    }
}

void user_register(User** head) {
    User* new_user = (User*)malloc(sizeof(User));
    if (new_user == NULL) {
        printf("内存分配失败！\n");
        return;
    }
    
    // 用户名验证
    do {
        printf("请输入用户名: ");
        fgets(new_user->username, sizeof(new_user->username), stdin);
        new_user->username[strcspn(new_user->username, "\n")] = 0;
        
        if (strlen(new_user->username) < 3) {
            printf("用户名长度不能少于3个字符，请重新输入！\n");
            continue;
        } else if (strlen(new_user->username) > 20) {
            printf("用户名长度不能超过20个字符，请重新输入！\n");
            continue;
        }
        
        // 添加用户名确认环节
        printf("您输入的用户名是: %s\n", new_user->username);
        printf("确认使用该用户名? (1:确认 0:重新输入): ");
        int confirm = get_valid_choice(0, 1);
        if (confirm) {
            break;
        }
    } while (1);
    
    // 检查用户名是否已存在
    User* current = *head;
    while (current != NULL) {
        if (strcmp(current->username, new_user->username) == 0) {
            printf("该用户名已存在！\n");
            free(new_user);
            return;
        }
        current = current->next;
    }
    
    char password[50];
    char confirm_password[50];
    int password_ok = 0;
    
    while (!password_ok) {
        printf("请输入密码: ");
        fgets(password, sizeof(password), stdin);
        password[strcspn(password, "\n")] = 0;
        
        // 检查密码是否为空
        if (strlen(password) == 0) {
            printf("密码不能为空，请重新输入！\n");
            continue;
        }
        
        // 检查密码强度
        show_password_strength_advice(password);
        int strength = check_password_strength(password);
        
        if (strength <= 2) {
            printf("密码强度较弱，是否继续？(1:是 0:否): ");
            int continue_choice;
            scanf("%d", &continue_choice);
            clear_input_buffer(); // 使用新的清空缓冲区函数
            
            if (!continue_choice) {
                continue; // 重新输入密码
            }
        }
        
        // 确认密码
        printf("请再次输入密码确认: ");
        fgets(confirm_password, sizeof(confirm_password), stdin);
        confirm_password[strcspn(confirm_password, "\n")] = 0;
        
        if (strcmp(password, confirm_password) != 0) {
            printf("两次输入的密码不一致，请重新输入！\n");
            continue;
        }
        
        password_ok = 1;
    }
    
    // 保存密码
    strcpy(new_user->password, password);
    
    do {
        printf("请输入手机号: ");
        fgets(new_user->phone, sizeof(new_user->phone), stdin);
        new_user->phone[strcspn(new_user->phone, "\n")] = 0;
        
        if (!validate_phone(new_user->phone)) {
            printf("手机号格式不正确，请输入11位数字且以1开头的手机号！\n");
            clear_input_buffer(); // 使用新的清空缓冲区函数
            continue;
        }
        
        // 添加手机号确认环节
        printf("您输入的手机号是: %s\n", new_user->phone);
        printf("确认使用该手机号? (1:确认 0:重新输入): ");
        clear_input_buffer(); // 使用新的清空缓冲区函数
        int confirm = get_valid_choice(0, 1);
        if (confirm) {
            break;
        }
    } while (1);
    
    // 设置为普通用户
    new_user->role = CUSTOMER;
    new_user->level = LEVEL_NORMAL;  // 初始化为普通会员
    new_user->total_cost = 0.0f;  // 初始化累计消费金额
    
    new_user->next = *head;
    *head = new_user;
    
    printf("\n注册成功！\n");
}

void user_query_express(Express* head, const char* phone) {
    int found = 0;
    Express* current = head;
    
    printf("\n查询结果:\n");
    printf("%-15s %-10s %-10s %-8s %-8s %-20s %s\n",
           "快递单号", "寄件人", "收件人", "重量(kg)",
           "费用(元)", "创建时间", "状态");
    printf("----------------------------------------"
           "----------------------------------------\n");
    
    while (current != NULL) {
        if (strcmp(current->phone, phone) == 0 && current->status != SHIPPED) {
            found = 1;
            char time_str[20];
            strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S",
                    localtime(&current->create_time));
            
            const char* status_str;
            switch (current->status) {
                case PENDING: status_str = "待取件"; break;
                case PICKED: status_str = "已取件"; break;
                case SHIPPED: status_str = "已发件"; break;
                case EXPIRED: status_str = "已过期"; break;
                default: status_str = "未知"; break;
            }
            
            printf("%-15s %-10s %-10s %-8.2f %-8.2f %-20s %s\n",
                   current->id, current->sender, current->receiver,
                   current->weight, current->cost, time_str, status_str);
        }
        current = current->next;
    }
    
    if (!found) {
        printf("未找到相关快递信息！\n");
    }
}

void user_pick_express(Express** head, const char* phone) {
    Express* current = *head;
    int found = 0;
    int picked_count = 0;
    
    // 加载货架数据
    Shelf* shelf_list = load_shelves("data/shelves.txt");
    
    // 首先显示该手机号关联的待取件快递列表
    printf("\n=== 待取件快递列表 ===\n");
    printf("%-15s %-10s %-10s %-8s %-8s %-15s %-7s\n",
           "快递单号", "寄件人", "收件人", "重量(kg)",
           "费用(元)", "存放货架", "取件码");
    printf("----------------------------------------"
           "----------------------------------------\n");
    
    // 先遍历一次，显示所有待取件的快递
    current = *head;
    while (current != NULL) {
        if (strcmp(current->phone, phone) == 0 && 
            (current->status == PENDING || current->status == SHIPPED)) {
            found = 1;
            printf("%-15s %-10s %-10s %-8.2f %-8.2f %-15s %-7s\n",
                   current->id, current->sender, current->receiver,
                   current->weight, current->cost, current->shelf_id, current->pickup_code);
        }
        current = current->next;
    }
    
    if (!found) {
        printf("\n没有待取件的快递！\n");
        return;
    }
    
    // 选择要取的快递
    char express_id[20];
    printf("\n请输入要取件的快递单号: ");
    fgets(express_id, sizeof(express_id), stdin);
    express_id[strcspn(express_id, "\n")] = 0;
    
    // 查找对应的快递
    current = *head;
    while (current != NULL) {
        if (strcmp(current->id, express_id) == 0 && 
            strcmp(current->phone, phone) == 0 && 
            (current->status == PENDING || current->status == SHIPPED)) {
            
            // 验证取件码
            char input_code[7];
            printf("请输入取件码: ");
            fgets(input_code, sizeof(input_code), stdin);
            input_code[strcspn(input_code, "\n")] = 0;
            
            if (strcmp(current->pickup_code, input_code) != 0) {
                printf("\n取件码错误，取件失败！\n");
                return;
            }
            
            current->status = PICKED;
            picked_count++;
            
            // 更新货架信息 - 减少货架上的快递数量
            if (strlen(current->shelf_id) > 0) {
                // 直接在货架列表中查找对应的货架，而不是使用find_shelf_by_id函数
                // 这样可以确保我们修改的是货架列表中的节点，而不是一个临时节点
                Shelf* shelf = shelf_list;
                while (shelf != NULL) {
                    if (strcmp(shelf->id, current->shelf_id) == 0) {
                        if (shelf->current_count > 0) {
                            printf("\n更新货架[%s]数量: %d -> %d\n", 
                                   shelf->id, shelf->current_count, shelf->current_count - 1);
                            shelf->current_count--;
                            
                            // 如果货架之前是满的，现在有空位了，更新状态为正常
                            if (shelf->status == SHELF_FULL && shelf->current_count < shelf->capacity) {
                                shelf->status = SHELF_NORMAL;
                                printf("货架[%s]状态从'已满'更新为'正常'\n", shelf->id);
                            }
                        }
                        break;
                    }
                    shelf = shelf->next;
                }
                
                // 立即保存货架数据，确保数据更新到文件
                save_shelves("data/shelves.txt", shelf_list);
            }
            
            // 发送短信通知
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "您的快递（单号：%s）已被成功取件。\n"
                     "寄件人：%s\n"
                     "重量：%.2fkg\n"
                     "费用：%.2f元",
                     current->id, current->sender,
                     current->weight, current->cost);
            send_sms_simulate(phone, msg);
            
            printf("\n取件成功！\n");
            break;
        }
        current = current->next;
    }
    
    if (picked_count > 0) {
        // 快递数据已更新，保存快递数据
        // 注意：货架数据已在更新货架时保存，这里不需要再次保存
        save_express(EXPRESS_DATA_FILE, *head);
    } else {
        printf("\n未找到对应的快递或快递状态不正确！\n");
    }
}

void send_sms_simulate(const char* phone, const char* msg) {
    // 确保data目录存在
    system("mkdir data 2>nul");
    
    FILE* fp = safe_fopen("data/sms.log", "a", 3);
    if (fp == NULL) {
        printf("无法记录短信日志！\n");
        return;
    }
    
    time_t now = time(NULL);
    char time_str[20];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S",
            localtime(&now));
    
    fprintf(fp, "[%s] 发送短信到 %s:\n%s\n\n",
            time_str, phone, msg);
    fclose(fp);
    
    printf("\n短信已发送到 %s\n", phone);
}