#ifndef USER_H
#define USER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "express.h"
#include "user_level.h"

// 用户角色枚举
typedef enum {
    ADMIN,      // 管理员
    CUSTOMER    // 普通用户
} UserRole;

// 会员等级在user_level.h中定义

// 用户信息结构体
typedef struct User {
    char username[50];     // 用户名
    char password[50];     // 密码
    char phone[12];        // 手机号
    UserRole role;         // 用户角色
    MemberLevel level;     // 会员等级
    float total_cost;      // 累计消费金额
    struct User* next;     // 下一个节点
} User;

// 用户登录
User* user_login(User* head);

// 检查密码强度
int check_password_strength(const char* password);

// 显示密码强度建议
void show_password_strength_advice(const char* password);

// 用户注册
void user_register(User** head);

// 用户查询快递
void user_query_express(Express* head, const char* phone);

// 用户取件
void user_pick_express(Express** head, const char* phone);

// 模拟发送短信
void send_sms_simulate(const char* phone, const char* msg);

#endif // USER_H