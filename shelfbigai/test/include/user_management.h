#ifndef USER_MANAGEMENT_H
#define USER_MANAGEMENT_H

#include "user.h"

// 显示用户管理菜单
void show_user_management_menu();

// 打印用户列表
void print_user_list(User* head);

// 添加新用户
void add_new_user(User** head);

// 修改用户信息
void modify_user_info(User** head);

// 删除用户
void delete_user(User** head);

// 用户管理主菜单
void user_management_menu(User** head);

#endif // USER_MANAGEMENT_H