#include "../include/utils.h"
#include "../include/express.h"
#include "../include/user.h"
#include "../include/storage.h"
#include "../include/shelf.h"
#include "../include/express_exception.h"
#include "../include/finance.h"
#include "../include/promotion.h"
#include "../include/user_level.h"
#include "../include/payment.h"
#include "../include/user_management.h"


User* current_user = NULL;
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 数据文件路径已在storage.h中定义
void clearScreen() {
    #ifdef _WIN32
    system("cls");
    #else
    system("clear");
    #endif
}

// 光标定位函数（使用ANSI转义序列）
void setCursorPos(int x, int y) {
    printf("\033[%d;%dH", y, x);
}

// 绘制小鸟函数
void drawBird(int x, int y, int direction) {
    setCursorPos(x, y);
    if (direction == 1) { // 向右飞
        printf("       _ |\\   ");
        setCursorPos(x, y+1);
        printf("       \\|||");
        setCursorPos(x, y+2);
        printf("     //========\\\\ ");
        setCursorPos(x, y+3);
        printf("    //    、  ，\\\\ ");
        setCursorPos(x, y+4);
        printf(" \\ ||    ■    ■  || \n");
        setCursorPos(x, y+5);
        printf("——■||        /\\  ||  ");
        setCursorPos(x, y+6);
        printf(" / ||        --  ||  ");
        setCursorPos(x, y+7);
        printf("    \\        \\/  // ");
        setCursorPos(x, y+8);
        printf("     \\__________//");
    } else { // 向左飞
        printf(" _//  ");
        setCursorPos(x, y+1);
        printf("<') ");
        setCursorPos(x, y+2);
        printf("(_==");
    }
}

// 绘制蛋函数
void drawEgg(int x, int y) {
    setCursorPos(x, y);
    printf("  _  ");
    setCursorPos(x, y+1);
    printf(" / \\ ");
    setCursorPos(x, y+2);
    printf(" \\_/ ");
}

// 绘制包裹函数
void drawPackage(int x, int y) {
    setCursorPos(x, y);
    printf(" ____________ ");
    setCursorPos(x, y+1);
    printf("|  菜鸟驿站   |");
    setCursorPos(x, y+2);
    printf("|____________|");
    setCursorPos(x, y+3);
    printf("  \\|/ ");
}

// 愤怒的小鸟动画
void angryBirdsAnimation() {
    // 第一阶段：小鸟从左上角飞向中间
    for (int i = 1; i <= 20; i++) {
        clearScreen();
        drawBird(i, i, 1); // 向右飞
        fflush(stdout);
        Sleep(50);
    }
    
    // 第二阶段：小鸟下蛋
    for (int i = 0; i < 3; i++) {
        clearScreen();
        drawBird(20, 20, 1);
        if (i == 1) drawEgg(22, 21);
        fflush(stdout);
        Sleep(200);
    }
    
    // 第三阶段：小鸟向右上角飞走
    for (int i = 1; i <= 20; i++) {
        clearScreen();
        drawBird(20+i, 20-i, 1);
        drawEgg(22, 21);
        fflush(stdout);
        Sleep(50);
    }
    
    // 第四阶段：蛋坠落
    for (int i = 1; i <= 10; i++) {
        clearScreen();
        drawEgg(22, 21+i);
        fflush(stdout);
        Sleep(100);
    }
    
    // 第五阶段：蛋变成包裹
    for (int i = 0; i < 3; i++) {
        clearScreen();
        if (i % 2 == 0) {
            drawEgg(22, 31);
        } else {
            drawPackage(20, 30);
        }
        fflush(stdout);
        Sleep(300);
    }
    
    // 最终显示包裹
    clearScreen();
    drawPackage(20, 30);
    fflush(stdout);
    Sleep(1000);
}

void user_report_exception(Express** express_head, ExpressException** exception_head, const char* user_phone);

// 显示管理员菜单
void show_admin_menu() {
    printf("\n=== 快递驿站管理系统（管理员）===\n");
    printf("1. 添加快递\n");
    printf("2. 删除快递\n");
    printf("3. 查看所有快递\n");
    printf("4. 查看库存状态\n");
    printf("5. 货架管理\n");
    printf("6. 异常处理\n");
    printf("7. 财务报表\n");
    printf("8. 优惠活动管理\n");
    printf("9. 发件处理\n");
    printf("10. 用户管理\n");
    printf("11. 退出登录\n");
    printf("12. 退出系统\n");
}

// 显示用户菜单
void show_user_menu() {
    printf("\n=== 快递驿站管理系统（用户）===\n");
    printf("1. 寄件\n");
    printf("2. 查询快递\n");
    printf("3. 取件\n");
    printf("4. 查看会员等级\n");
    printf("5. 查看优惠活动\n");
    printf("6. 异常反馈\n");
    printf("7. 退出登录\n");
    printf("8. 退出系统\n");
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
                save_express(EXPRESS_DATA_FILE, *express_list);
                break;
            case 2: {
                char pickup_phone[12];
                char pickup_name[50];
                
                // 输入代取人姓名
                printf("请输入代取人姓名: ");
                fgets(pickup_name, sizeof(pickup_name), stdin);
                pickup_name[strcspn(pickup_name, "\n")] = 0;
                
                // 检查姓名是否为空
                if (strlen(pickup_name) == 0) {
                    printf("代取人姓名不能为空！\n");
                    pause_program();
                    continue;
                }
                
                // 输入代取人手机号
                do {
                    printf("请输入代取人手机号: ");
                    fgets(pickup_phone, sizeof(pickup_phone), stdin);
                    pickup_phone[strcspn(pickup_phone, "\n")] = 0;
                    if (!validate_phone(pickup_phone)) {
                        printf("手机号格式不正确，请输入11位数字且以1开头的手机号！\n");
                    }
                } while (!validate_phone(pickup_phone));
                
                // 显示代取信息确认
                printf("\n=== 代取信息确认 ===\n");
                printf("代取人姓名: %s\n", pickup_name);
                printf("代取人手机号: %s\n", pickup_phone);
                printf("确认代取信息？(1:是 0:否): ");
                int confirm;
                scanf("%d", &confirm);
                while (getchar() != '\n');
                
                if (!confirm) {
                    printf("已取消代取操作。\n");
                    pause_program();
                    continue;
                }
                
                // 调用代取函数
                user_pick_express(express_list, pickup_phone);
                save_express(EXPRESS_DATA_FILE, *express_list);
                break;
            }
            case 3:
                return;
        }
        pause_program();
    } while (1);
}

// 管理员操作处理
void handle_admin_operations(Express** express_list, Shelf** shelf_list, ExpressException** exception_list, Transaction** transaction_list, Promotion** promotion_list, User** user_list) {
    int choice;
    do {
        clear_screen();
        show_admin_menu();
        choice = get_valid_choice(1, 12);
        
        switch (choice) {
            case 1:
                insert_express(express_list);
                save_express(EXPRESS_DATA_FILE, *express_list);
                break;
            case 2:
                if (delete_express(express_list)) {
                    save_express(EXPRESS_DATA_FILE, *express_list);
                }
                break;
            case 3:
                print_express_list(*express_list);
                break;
            case 4:
                check_inventory_alerts(*express_list);
                check_shelf_capacity_alerts(*shelf_list);
                break;
            case 5:
                shelf_management_menu(shelf_list);
                save_shelves(SHELF_DATA_FILE, *shelf_list);
                break;
            case 6:
                exception_management_menu(express_list, exception_list);
                save_exceptions(EXCEPTION_DATA_FILE, *exception_list);
                save_express(EXPRESS_DATA_FILE, *express_list);
                break;
            case 7:
                finance_report_menu(*transaction_list);
                break;
            case 8:
                promotion_management_menu(promotion_list);
                save_promotions(PROMOTION_DATA_FILE, *promotion_list);
                break;
            case 9:
                admin_ship_express(express_list);
                save_express(EXPRESS_DATA_FILE, *express_list);
                break;
            case 10:
                user_management_menu(user_list);
                save_users(USER_DATA_FILE, *user_list);
                break;
            case 11:
                printf("\n已退出登录！\n");
                return;
            case 12:
                printf("\n感谢使用，再见！\n");
                exit(0);
        }
        pause_program();
    } while (1);
}

// 显示用户会员信息
void show_user_level_info(User* user) {
    printf("\n=== 会员信息 ===\n");
    printf("用户名: %s\n", user->username);
    printf("会员等级: %s\n", get_member_level_name(user->level));
    printf("累计消费: %.2f元\n", user->total_cost);
    printf("当前折扣: %.1f折\n", get_member_level_discount(user->level) * 10);
    
    float next_threshold = get_next_level_threshold(user->level);
    if (next_threshold > 0) {
        printf("距离下一级还需消费: %.2f元\n", next_threshold - user->total_cost);
    } else {
        printf("已达到最高会员等级\n");
    }
}

// 用户操作处理
void handle_user_operations(Express** express_list, User* current_user, Promotion** promotion_list, ExpressException** exception_list) {
    int choice;
    do {
        clear_screen();
        show_user_menu();
        choice = get_valid_choice(1, 8);
        
        switch (choice) {
            case 1:
                insert_express(express_list);
                save_express(EXPRESS_DATA_FILE, *express_list);
                break;
            case 2:
                user_query_express(*express_list, current_user->phone);
                break;
            case 3:
                pick_express(express_list, current_user->phone);
                save_express(EXPRESS_DATA_FILE, *express_list);
                break;
            case 4:
                show_user_level_info(current_user);
                break;
            case 5:
                // 更新优惠活动状态
                update_promotion_status(*promotion_list);
                print_promotion_list(*promotion_list);
                break;
            case 6:
                user_report_exception(express_list, exception_list, current_user->phone);
                break;
            case 7:
                printf("\n已退出登录！\n");
                return;
            case 8:
                printf("\n感谢使用，再见！\n");
                exit(0);
        }
        pause_program();
    } while (1);
}

// 用户异常反馈功能
void user_report_exception(Express** express_head, ExpressException** exception_head, const char* user_phone) {
    if (*express_head == NULL) {
        printf("快递列表为空！\n");
        return;
    }
    
    // 查找与用户相关的快递（寄件人或收件人是本账户的）
    Express* current = *express_head;
    int found = 0;
    
    printf("\n=== 与您相关的快递列表 ===\n");
    printf("%-15s %-10s %-10s %-8s %-8s %-20s %s\n",
           "快递单号", "寄件人", "收件人", "重量(kg)",
           "费用(元)", "创建时间", "状态");
    printf("----------------------------------------"
           "----------------------------------------\n");
    
    while (current != NULL) {
        // 检查是否是用户相关的快递（寄件人或收件人是本账户的）
        if (strcmp(current->sender_phone, user_phone) == 0 || 
            strcmp(current->phone, user_phone) == 0) {
            found = 1;
            char time_str[20];
            strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S",
                    localtime(&current->create_time));
            
            const char* status_str = get_express_status_name(current->status);
            
            printf("%-15s %-10s %-10s %-8.2f %-8.2f %-20s %s\n",
                   current->id, current->sender, current->receiver,
                   current->weight, current->cost, time_str, status_str);
        }
        current = current->next;
    }
    
    if (!found) {
        printf("未找到与您相关的快递信息！\n");
        return;
    }
    
    // 选择要处理的快递
    char express_id[20];
    printf("\n请输入要反馈异常的快递单号: ");
    scanf("%19s", express_id);
    while (getchar() != '\n');
    
    // 查找快递并验证是否与用户相关
    current = *express_head;
    found = 0;
    
    while (current != NULL) {
        if (strcmp(current->id, express_id) == 0 && 
            (strcmp(current->sender_phone, user_phone) == 0 || 
             strcmp(current->phone, user_phone) == 0)) {
            found = 1;
            break;
        }
        current = current->next;
    }
    
    if (!found) {
        printf("未找到该快递单号或该快递与您无关！\n");
        return;
    }
    
    // 选择异常类型
    printf("\n请选择异常类型:\n");
    printf("1. 快递损坏\n");
    printf("2. 快递丢失\n");
    printf("3. 误领\n");
    printf("4. 冒领\n");
    printf("5. 拒收\n");
    printf("6. 其他\n");
    
    int type_choice = get_valid_choice(1, 6);
    ExpressExceptionType type;
    
    switch (type_choice) {
        case 1: type = EX_DAMAGED; break;
        case 2: type = EX_LOST; break;
        case 3: type = EX_WRONG_DELIVERY; break;
        case 4: type = EX_FAKE_PICKUP; break;
        case 5: type = EX_REJECTED; break;
        case 6: type = EX_OTHER; break;
        default: type = EX_OTHER; break;
    }
    
    // 输入异常描述
    char description[200];
    do {
        printf("请输入异常描述: ");
        fgets(description, sizeof(description), stdin);
        description[strcspn(description, "\n")] = '\0'; // 移除换行符
        
        if (strlen(description) == 0) {
            printf("异常描述不能为空，请重新输入！\n");
        }
    } while (strlen(description) == 0);
    
    // 创建异常记录
    ExpressException* new_record = create_exception_record(express_id, type, description);
    if (new_record != NULL) {
        add_exception_record(exception_head, new_record);
        save_exceptions(EXCEPTION_DATA_FILE, *exception_head);
    }
}

int main() {
    angryBirdsAnimation();
    // 初始化随机数种子
    srand((unsigned int)time(NULL));
    
    // 初始化异常处理
    init_exception_handlers();
    
    // 加载所有数据
    User* user_list = load_users(USER_DATA_FILE);
    Express* express_list = load_express(EXPRESS_DATA_FILE);
    Shelf* shelf_list = load_shelves(SHELF_DATA_FILE);
    ExpressException* exception_list = load_exceptions(EXCEPTION_DATA_FILE);
    Transaction* transaction_list = load_transactions(TRANSACTION_DATA_FILE);
    Promotion* promotion_list = load_promotions(PROMOTION_DATA_FILE);
    Payment* payment_list = load_payments(PAYMENT_DATA_FILE);
    
    // 检查并初始化货架
    if (shelf_list == NULL) {
        printf("\n系统初始化货架...\n");
        Shelf* small_shelf = create_shelf(SHELF_SMALL, 20);
        Shelf* medium_shelf = create_shelf(SHELF_MEDIUM, 50);
        Shelf* large_shelf = create_shelf(SHELF_LARGE, 100);
        
        if (small_shelf != NULL) {
            add_shelf(&shelf_list, small_shelf);
            printf("已创建小型货架: %s (容量: 20)\n", small_shelf->id);
        }
        if (medium_shelf != NULL) {
            add_shelf(&shelf_list, medium_shelf);
            printf("已创建中型货架: %s (容量: 50)\n", medium_shelf->id);
        }
        if (large_shelf != NULL) {
            add_shelf(&shelf_list, large_shelf);
            printf("已创建大型货架: %s (容量: 100)\n", large_shelf->id);
        }
        
        // 保存初始化的货架数据
        save_shelves(SHELF_DATA_FILE, shelf_list);
    }
    
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
                        handle_admin_operations(&express_list, &shelf_list, &exception_list, &transaction_list, &promotion_list, &user_list);
                    } else {
                        printf("\n权限不足，请使用管理员账号登录！\n");
                    }
                }
                break;
            }
            case 2: {
                current_user = user_login(user_list);
                if (current_user != NULL) {
                    if (current_user->role == CUSTOMER) {
                        handle_user_operations(&express_list, current_user, &promotion_list, &exception_list);
                    } else {
                        printf("\n请使用普通用户账号登录！\n");
                    }
                }
                break;
            }
            case 3:
                user_register(&user_list);
                save_users(USER_DATA_FILE, user_list);
                break;
            case 4:
                printf("\n感谢使用，再见！\n");
                // 保存所有数据
                save_users(USER_DATA_FILE, user_list);
                save_express(EXPRESS_DATA_FILE, express_list);
                save_shelves(SHELF_DATA_FILE, shelf_list);
                save_exceptions(EXCEPTION_DATA_FILE, exception_list);
                save_transactions(TRANSACTION_DATA_FILE, transaction_list);
                save_promotions(PROMOTION_DATA_FILE, promotion_list);
                save_payments(PAYMENT_DATA_FILE, payment_list);
                
                // 释放所有链表内存
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
                while (shelf_list != NULL) {
                    Shelf* temp = shelf_list;
                    shelf_list = shelf_list->next;
                    free(temp);
                }
                while (exception_list != NULL) {
                    ExpressException* temp = exception_list;
                    exception_list = exception_list->next;
                    free(temp);
                }
                while (transaction_list != NULL) {
                    Transaction* temp = transaction_list;
                    transaction_list = transaction_list->next;
                    free(temp);
                }
                while (promotion_list != NULL) {
                    Promotion* temp = promotion_list;
                    promotion_list = promotion_list->next;
                    free(temp);
                }
                while (payment_list != NULL) {
                    Payment* temp = payment_list;
                    payment_list = payment_list->next;
                    free(temp);
                }
                return 0;
        }
        pause_program();
    } while (1);
    
    return 0;
}