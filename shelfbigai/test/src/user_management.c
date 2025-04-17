#include "../include/user_management.h"
#include "../include/user.h"
#include "../include/storage.h"
#include "../include/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 显示用户管理菜单
void show_user_management_menu() {
    printf("\n=== 用户管理系统 ===\n");
    printf("1. 查看用户列表\n");
    printf("2. 添加新用户\n");
    printf("3. 修改用户信息\n");
    printf("4. 删除用户\n");
    printf("5. 返回上级菜单\n");
}

// 打印用户列表
void print_user_list(User* head) {
    printf("\n=== 用户列表 ===\n");
    printf("%-20s %-15s %-15s %-10s\n", "用户名", "手机号", "会员等级", "角色");
    printf("----------------------------------------------------\n");
    
    User* current = head;
    while (current != NULL) {
        const char* role = (current->role == ADMIN) ? "管理员" : "普通用户";
        const char* level = get_member_level_name(current->level);
        printf("%-20s %-15s %-15s %-10s\n",
               current->username,
               current->phone,
               level,
               role);
        current = current->next;
    }
}

// 添加新用户
void add_new_user(User** head) {
    user_register(head);
}

// 修改用户信息
void modify_user_info(User** head) {
    char username[50];
    printf("请输入要修改的用户名: ");
    fgets(username, sizeof(username), stdin);
    username[strcspn(username, "\n")] = 0;
    
    User* current = *head;
    while (current != NULL) {
        if (strcmp(current->username, username) == 0) {
            // 不允许修改管理员信息
            if (current->role == ADMIN) {
                printf("不能修改管理员信息！\n");
                return;
            }
            
            printf("\n=== 修改用户信息 ===\n");
            printf("1. 修改密码\n");
            printf("2. 修改手机号\n");
            printf("3. 修改会员等级\n");
            printf("4. 取消修改\n");
            
            int choice = get_valid_choice(1, 4);
            switch (choice) {
                case 1: {
                    char new_password[50];
                    printf("请输入新密码: ");
                    fgets(new_password, sizeof(new_password), stdin);
                    new_password[strcspn(new_password, "\n")] = 0;
                    
                    // 检查密码是否为空
                    if (strlen(new_password) == 0) {
                        printf("密码不能为空，请重新输入！\n");
                        return;
                    }
                    
                    // 检查密码强度
                    show_password_strength_advice(new_password);
                    int strength = check_password_strength(new_password);
                    if (strength <= 2) {
                        printf("密码强度较弱，是否继续？(1:是 0:否): ");
                        int continue_choice;
                        scanf("%d", &continue_choice);
                        clear_input_buffer();
                        if (!continue_choice) return;
                    }
                    strcpy(current->password, new_password);
                    printf("密码修改成功！\n");
                    break;
                }
                case 2: {
                    char new_phone[12];
                    do {
                        clear_input_buffer();
                        printf("请输入新手机号: ");
                        fgets(new_phone, sizeof(new_phone), stdin);
                        new_phone[strcspn(new_phone, "\n")] = 0;
                        if (!validate_phone(new_phone)) {
                            printf("手机号格式不正确，请输入11位数字且以1开头的手机号！\n");
                        }
                    } while (!validate_phone(new_phone));
                    strcpy(current->phone, new_phone);
                    printf("手机号修改成功！\n");
                    break;
                }
                case 3: {
                    printf("\n=== 会员等级 ===\n");
                    printf("1. 普通会员\n");
                    printf("2. 一级会员\n");
                    printf("3. 二级会员\n");
                    printf("4. 三级会员\n");
                    printf("5. 四级会员\n");
                    
                    int level_choice = get_valid_choice(1, 5);
                    current->level = (MemberLevel)(level_choice - 1);
                    printf("会员等级修改成功！\n");
                    
                    // 保存用户数据到文件
                    save_users(USER_DATA_FILE, *head);
                    break;
                }
                case 4:
                    return;
            }
            return;
        }
        current = current->next;
    }
    printf("未找到该用户！\n");
}

// 删除用户
void delete_user(User** head) {
    char username[50];
    printf("请输入要删除的用户名: ");
    fgets(username, sizeof(username), stdin);
    username[strcspn(username, "\n")] = 0;
    
    User* current = *head;
    User* prev = NULL;
    
    while (current != NULL) {
        if (strcmp(current->username, username) == 0) {
            // 不允许删除管理员账户
            if (current->role == ADMIN) {
                printf("不能删除管理员账户！\n");
                return;
            }
            
            // 确认删除
            printf("确定要删除用户 %s 吗？(1:是 0:否): ", username);
            int confirm;
            scanf("%d", &confirm);
            while (getchar() != '\n');
            
            if (!confirm) {
                printf("已取消删除操作。\n");
                return;
            }
            
            if (prev == NULL) {
                *head = current->next;
            } else {
                prev->next = current->next;
            }
            free(current);
            printf("用户删除成功！\n");
            
            // 保存用户数据到文件
            save_users(USER_DATA_FILE, *head);
            return;
        }
        prev = current;
        current = current->next;
    }
    printf("未找到该用户！\n");
}

// 用户管理主菜单
void user_management_menu(User** head) {
    int choice;
    do {
        clear_screen();
        show_user_management_menu();
        choice = get_valid_choice(1, 5);
        
        switch (choice) {
            case 1:
                print_user_list(*head);
                break;
            case 2:
                add_new_user(head);
                save_users(USER_DATA_FILE, *head);
                break;
            case 3:
                modify_user_info(head);
                break;
            case 4:
                delete_user(head);
                break;
            case 5:
                return;
        }
        pause_program();
    } while (1);
}