#include "../include/user.h"
#include "../include/utils.h"
#include <time.h>

User* user_login(User* head) {
    char username[50];
    char password[50];
    
    printf("请输入用户名: ");
    scanf("%49s", username);
    while (getchar() != '\n');
    
    printf("请输入密码: ");
    scanf("%49s", password);
    while (getchar() != '\n');
    
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

void user_register(User** head) {
    User* new_user = (User*)malloc(sizeof(User));
    if (new_user == NULL) {
        printf("内存分配失败！\n");
        return;
    }
    
    printf("请输入用户名: ");
    scanf("%49s", new_user->username);
    while (getchar() != '\n');
    
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
    
    printf("请输入密码: ");
    scanf("%49s", new_user->password);
    while (getchar() != '\n');
    
    do {
        printf("请输入手机号: ");
        scanf("%11s", new_user->phone);
        while (getchar() != '\n');
    } while (!validate_phone(new_user->phone));
    
    // 设置为普通用户
    new_user->role = CUSTOMER;
    new_user->level = NORMAL;  // 初始化为普通会员
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
        if (strcmp(current->phone, phone) == 0) {
            found = 1;
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
    int picked_count = 0;
    
    while (current != NULL) {
        if (strcmp(current->phone, phone) == 0 && 
            current->status == PENDING) {
            current->status = PICKED;
            picked_count++;
            
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
        }
        current = current->next;
    }
    
    if (picked_count > 0) {
        printf("\n成功取件 %d 个快递！\n", picked_count);
    } else {
        printf("\n没有待取件的快递！\n");
    }
}

void send_sms_simulate(const char* phone, const char* msg) {
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