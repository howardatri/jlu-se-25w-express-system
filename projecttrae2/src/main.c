#include "../include/utils.h"
#include "../include/express.h"
#include "../include/user.h"
#include "../include/storage.h"

User* current_user = NULL;
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define USERS_FILE "data/users.txt"
#define EXPRESS_FILE "data/express.txt"

// 显示管理员菜单
void show_admin_menu() {
    printf("\n=== 快递驿站管理系统（管理员）===\n");
    printf("1. 添加快递\n");
    printf("2. 删除快递\n");
    printf("3. 查看所有快递\n");
    printf("4. 查看库存状态\n");
    printf("5. 退出登录\n");
    printf("6. 退出系统\n");
}

// 显示用户菜单
void show_user_menu() {
    printf("\n=== 快递驿站管理系统（用户）===\n");
    printf("1. 寄件\n");
    printf("2. 查询快递\n");
    printf("3. 取件\n");
    printf("4. 退出登录\n");
    printf("5. 退出系统\n");
}

// 显示取件子菜单
void show_pickup_menu() {
    printf("\n=== 取件方式 ===\n");
    printf("1. 自取快递\n");
    printf("2. 代取快递\n");
    printf("3. 返回上级菜单\n");
}

// 处理取件操作
void pick_express(Express** express_list, const char* phone) {
    int choice;
    do {
        clear_screen();
        show_pickup_menu();
        choice = get_valid_choice(1, 3);
        
        switch (choice) {
            case 1:
                user_pick_express(express_list, phone);
                save_express(EXPRESS_FILE, *express_list);
                break;
            case 2: {
                char pickup_phone[12];
                do {
                    printf("请输入代取人手机号: ");
                    scanf("%11s", pickup_phone);
                    while (getchar() != '\n');
                } while (!validate_phone(pickup_phone));
                user_pick_express(express_list, pickup_phone);
                save_express(EXPRESS_FILE, *express_list);
                break;
            }
            case 3:
                return;
        }
        pause_program();
    } while (1);
}

// 管理员操作处理
void handle_admin_operations(Express** express_list) {
    int choice;
    do {
        clear_screen();
        show_admin_menu();
        choice = get_valid_choice(1, 6);
        
        switch (choice) {
            case 1:
                insert_express(express_list);
                save_express(EXPRESS_FILE, *express_list);
                break;
            case 2:
                if (delete_express(express_list)) {
                    save_express(EXPRESS_FILE, *express_list);
                }
                break;
            case 3:
                print_express_list(*express_list);
                break;
            case 4:
                check_inventory_alerts(*express_list);
                break;
            case 5:
                printf("\n已退出登录！\n");
                return;
            case 6:
                printf("\n感谢使用，再见！\n");
                exit(0);
        }
        pause_program();
    } while (1);
}

// 用户操作处理
void handle_user_operations(Express** express_list, const char* phone) {
    int choice;
    do {
        clear_screen();
        show_user_menu();
        choice = get_valid_choice(1, 5);
        
        switch (choice) {
            case 1:
                insert_express(express_list);
                save_express(EXPRESS_FILE, *express_list);
                break;
            case 2:
                user_query_express(*express_list, phone);  // 直接传入当前用户的手机号
                break;
            case 3:
                pick_express(express_list, phone);  // 使用封装的取件函数
                break;
            case 4:
                printf("\n已退出登录！\n");
                return;
            case 5:
                printf("\n感谢使用，再见！\n");
                exit(0);
        }
        pause_program();
    } while (1);
}

int main() {
    
    // 初始化异常处理
    init_exception_handlers();
    
    // 加载用户和快递数据
    User* user_list = load_users(USERS_FILE);
    Express* express_list = load_express(EXPRESS_FILE);
    
    int choice;
    do {
        clear_screen();
        printf("=== 快递驿站管理系统 ===\n");
        printf("1. 管理员登录\n");
        printf("2. 用户登录\n");
        printf("3. 用户注册\n");
        printf("4. 退出系统\n");
        
        choice = get_valid_choice(1, 4);
        
        switch (choice) {
            case 1: {
                User* current_user = user_login(user_list);
                if (current_user != NULL) {
                    if (current_user->role == ADMIN) {
                        handle_admin_operations(&express_list);
                    } else {
                        printf("\n权限不足，请使用管理员账号登录！\n");
                    }
                }
                break;
            }
            case 2: {
                User* current_user = user_login(user_list);
                if (current_user != NULL) {
                    if (current_user->role == CUSTOMER) {
                        handle_user_operations(&express_list, current_user->phone);
                    } else {
                        printf("\n请使用普通用户账号登录！\n");
                    }
                }
                break;
            }
            case 3:
                user_register(&user_list);
                save_users(USERS_FILE, user_list);
                break;
            case 4:
                printf("\n感谢使用，再见！\n");
                // 保存数据并清理内存
                save_users(USERS_FILE, user_list);
                save_express(EXPRESS_FILE, express_list);
                // 释放链表内存
                while (user_list != NULL) {
                    User* temp = user_list;
                    user_list = user_list->next;
                    free(temp);
                }
                while (express_list != NULL) {
                    Express* temp = express_list;
                    express_list = express_list->next;
                    free(temp);
                }
                return 0;
        }
        pause_program();
    } while (1);
    
    return 0;
}