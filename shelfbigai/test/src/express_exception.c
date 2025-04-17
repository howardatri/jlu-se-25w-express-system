#include "../include/express_exception.h"
#include "../include/utils.h"
#include "../include/storage.h"
#define EXCEPTION_DATA_FILE "data/exceptions.txt"

// 生成异常记录ID
void generate_exception_id(char* buffer) {
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    static int seq = 0;
    sprintf(buffer, "EX%04d%02d%02d%04d", t->tm_year+1900, t->tm_mon+1, t->tm_mday, ++seq%10000);
}

// 创建异常记录
ExpressException* create_exception_record(const char* express_id, ExpressExceptionType type, const char* description) {
    ExpressException* record = (ExpressException*)malloc(sizeof(ExpressException));
    if (record == NULL) {
        printf("内存分配失败！\n");
        return NULL;
    }
    
    // 生成异常记录ID
    generate_exception_id(record->id);
    
    // 设置关联的快递单号
    strncpy(record->express_id, express_id, sizeof(record->express_id) - 1);
    record->express_id[sizeof(record->express_id) - 1] = '\0';
    
    // 设置异常类型和状态
    record->type = type;
    record->status = EX_STATUS_PENDING; // 初始状态为待处理
    
    // 设置异常描述
    strncpy(record->description, description, sizeof(record->description) - 1);
    record->description[sizeof(record->description) - 1] = '\0';
    
    // 清空解决方案
    record->solution[0] = '\0';
    
    // 设置时间
    record->create_time = time(NULL);
    record->update_time = record->create_time;
    
    // 设置下一个节点为NULL
    record->next = NULL;
    
    return record;
}

// 添加异常记录
void add_exception_record(ExpressException** head, ExpressException* new_record) {
    if (new_record == NULL) return;
    
    if (*head == NULL) {
        *head = new_record;
    } else {
        ExpressException* current = *head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_record;
    }
    
    printf("\n异常记录已添加成功！\n");
    printf("异常记录ID: %s\n", new_record->id);
}

// 更新异常记录状态
void update_exception_status(ExpressException* head, const char* exception_id, ExpressExceptionStatus new_status, const char* solution) {
    ExpressException* current = head;
    
    while (current != NULL) {
        if (strcmp(current->id, exception_id) == 0) {
            current->status = new_status;
            current->update_time = time(NULL);
            
            if (solution != NULL && strlen(solution) > 0) {
                strncpy(current->solution, solution, sizeof(current->solution) - 1);
                current->solution[sizeof(current->solution) - 1] = '\0';
            }
            
            printf("\n异常记录状态已更新！\n");
            return;
        }
        current = current->next;
    }
    
    printf("\n未找到异常记录ID: %s\n", exception_id);
}

// 查询快递异常记录
ExpressException* find_exception_by_express_id(ExpressException* head, const char* express_id) {
    ExpressException* current = head;
    ExpressException* result = NULL;
    int found = 0;
    
    while (current != NULL) {
        if (strcmp(current->express_id, express_id) == 0) {
            if (!found) {
                printf("\n查询结果:\n");
                printf("%-15s %-15s %-10s %-12s %-20s %-20s\n",
                       "异常记录ID", "快递单号", "异常类型", "处理状态",
                       "创建时间", "更新时间");
                printf("----------------------------------------"
                       "----------------------------------------\n");
                found = 1;
            }
            
            char create_time_str[20];
            char update_time_str[20];
            strftime(create_time_str, sizeof(create_time_str), "%Y-%m-%d %H:%M:%S",
                    localtime(&current->create_time));
            strftime(update_time_str, sizeof(update_time_str), "%Y-%m-%d %H:%M:%S",
                    localtime(&current->update_time));
            
            const char* type_str = get_exception_type_name(current->type);
            const char* status_str = get_exception_status_name(current->status);
            
            printf("%-15s %-15s %-10s %-12s %-20s %-20s\n",
                   current->id, current->express_id, type_str, status_str,
                   create_time_str, update_time_str);
            
            if (result == NULL) {
                result = current;
            }
        }
        current = current->next;
    }
    
    if (!found) {
        printf("\n未找到快递单号 %s 的异常记录！\n", express_id);
    } else {
        printf("\n详细信息:\n");
        printf("异常描述: %s\n", result->description);
        if (strlen(result->solution) > 0) {
            printf("解决方案: %s\n", result->solution);
        }
    }
    
    return result;
}

// 打印异常记录列表
void print_exception_list(ExpressException* head) {
    if (head == NULL) {
        printf("异常记录列表为空！\n");
        return;
    }
    
    printf("\n异常记录列表:\n");
    printf("%-15s %-15s %-10s %-12s %-20s %-20s\n",
           "异常记录ID", "快递单号", "异常类型", "处理状态",
           "创建时间", "更新时间");
    printf("----------------------------------------"
           "----------------------------------------\n");
    
    ExpressException* current = head;
    while (current != NULL) {
        char create_time_str[20];
        char update_time_str[20];
        strftime(create_time_str, sizeof(create_time_str), "%Y-%m-%d %H:%M:%S",
                localtime(&current->create_time));
        strftime(update_time_str, sizeof(update_time_str), "%Y-%m-%d %H:%M:%S",
                localtime(&current->update_time));
        
        const char* type_str = get_exception_type_name(current->type);
        const char* status_str = get_exception_status_name(current->status);
        
        printf("%-15s %-15s %-10s %-12s %-20s %-20s\n",
               current->id, current->express_id, type_str, status_str,
               create_time_str, update_time_str);
        
        current = current->next;
    }
}

// 从文件加载异常记录
ExpressException* load_exceptions(const char* filename) {
    FILE* fp = safe_fopen(filename, "r", 3);
    if (fp == NULL) {
        // 创建data目录
        system("mkdir data 2>nul");
        // 创建空文件
        fp = safe_fopen(filename, "w", 3);
        if (fp != NULL) {
            fclose(fp);
            printf("已创建新的异常记录数据文件：%s\n", filename);
        }
        return NULL;
    }
    
    ExpressException* head = NULL;
    ExpressException* current = NULL;
    
    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        ExpressException* node = (ExpressException*)malloc(sizeof(ExpressException));
        if (node == NULL) {
            printf("内存分配失败！\n");
            fclose(fp);
            return head;
        }
        
        int type, status;
        if (sscanf(line, "%19s %19s %d %d %199[^|]|%199[^|]|%ld %ld",
                   node->id, node->express_id, &type, &status,
                   node->description, node->solution,
                   &node->create_time, &node->update_time) != 8) {
            free(node);
            continue;
        }
        
        node->type = (ExpressExceptionType)type;
        node->status = (ExpressExceptionStatus)status;
        node->next = NULL;
        
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

// 保存异常记录到文件
void save_exceptions(const char* filename, ExpressException* head) {
    FILE* fp = safe_fopen(filename, "w", 3);
    if (fp == NULL) {
        printf("无法打开文件 %s 进行写入！\n", filename);
        return;
    }
    
    ExpressException* current = head;
    while (current != NULL) {
        if (fprintf(fp, "%s %s %d %d %s|%s|%ld %ld\n",
                current->id, current->express_id, current->type, current->status,
                current->description, current->solution,
                current->create_time, current->update_time) < 0) {
            printf("写入异常记录数据时发生错误！\n");
            break;
        }
        current = current->next;
    }
    
    fclose(fp);
    printf("异常记录数据已保存到文件：%s\n", filename);
}

// 处理快递异常
void handle_express_exception(Express** express_head, ExpressException** exception_head) {
    if (*express_head == NULL) {
        printf("快递列表为空！\n");
        return;
    }
    
    // 打印快递列表
    print_express_list(*express_head);
    
    // 选择要处理的快递
    char express_id[20];
    printf("\n请输入要处理异常的快递单号: ");
    scanf("%19s", express_id);
    while (getchar() != '\n');
    
    // 查找快递
    Express* current = *express_head;
    int found = 0;
    
    while (current != NULL) {
        if (strcmp(current->id, express_id) == 0) {
            found = 1;
            break;
        }
        current = current->next;
    }
    
    if (!found) {
        printf("未找到该快递单号！\n");
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
    }
}

// 检查快递异常状态
void check_exception_status(ExpressException* head) {
    if (head == NULL) {
        printf("异常记录列表为空！\n");
        return;
    }
    
    int pending_count = 0;
    int handling_count = 0;
    int resolved_count = 0;
    int closed_count = 0;
    
    ExpressException* current = head;
    while (current != NULL) {
        switch (current->status) {
            case EX_STATUS_PENDING: pending_count++; break;
            case EX_STATUS_HANDLING: handling_count++; break;
            case EX_STATUS_RESOLVED: resolved_count++; break;
            case EX_STATUS_CLOSED: closed_count++; break;
        }
        current = current->next;
    }
    
    printf("\n异常状态统计:\n");
    printf("待处理: %d\n", pending_count);
    printf("处理中: %d\n", handling_count);
    printf("已解决: %d\n", resolved_count);
    printf("已关闭: %d\n", closed_count);
    printf("总计: %d\n", pending_count + handling_count + resolved_count + closed_count);
}

// 获取异常类型名称
const char* get_exception_type_name(ExpressExceptionType type) {
    switch (type) {
        case EX_DAMAGED: return "快递损坏";
        case EX_LOST: return "快递丢失";
        case EX_WRONG_DELIVERY: return "误领";
        case EX_FAKE_PICKUP: return "冒领";
        case EX_REJECTED: return "拒收";
        case EX_OTHER: return "其他";
        default: return "未知";
    }
}

// 获取异常状态名称
const char* get_exception_status_name(ExpressExceptionStatus status) {
    switch (status) {
        case EX_STATUS_PENDING: return "待处理";
        case EX_STATUS_HANDLING: return "处理中";
        case EX_STATUS_RESOLVED: return "已解决";
        case EX_STATUS_CLOSED: return "已关闭";
        default: return "未知";
    }
}

// 按类型查询异常记录
void find_exceptions_by_type(ExpressException* head, ExpressExceptionType type) {
    if (head == NULL) {
        printf("异常记录列表为空！\n");
        return;
    }
    
    int found = 0;
    ExpressException* current = head;
    
    printf("\n查询结果 - 异常类型: %s\n", get_exception_type_name(type));
    printf("%-15s %-15s %-10s %-12s %-20s %-20s\n",
           "异常记录ID", "快递单号", "异常类型", "处理状态",
           "创建时间", "更新时间");
    printf("----------------------------------------"
           "----------------------------------------\n");
    
    while (current != NULL) {
        if (current->type == type) {
            found = 1;
            
            char create_time_str[20];
            char update_time_str[20];
            strftime(create_time_str, sizeof(create_time_str), "%Y-%m-%d %H:%M:%S",
                    localtime(&current->create_time));
            strftime(update_time_str, sizeof(update_time_str), "%Y-%m-%d %H:%M:%S",
                    localtime(&current->update_time));
            
            const char* type_str = get_exception_type_name(current->type);
            const char* status_str = get_exception_status_name(current->status);
            
            printf("%-15s %-15s %-10s %-12s %-20s %-20s\n",
                   current->id, current->express_id, type_str, status_str,
                   create_time_str, update_time_str);
        }
        current = current->next;
    }
    
    if (!found) {
        printf("未找到类型为 %s 的异常记录！\n", get_exception_type_name(type));
    }
}

// 按状态查询异常记录
void find_exceptions_by_status(ExpressException* head, ExpressExceptionStatus status) {
    if (head == NULL) {
        printf("异常记录列表为空！\n");
        return;
    }
    
    int found = 0;
    ExpressException* current = head;
    
    printf("\n查询结果 - 处理状态: %s\n", get_exception_status_name(status));
    printf("%-15s %-15s %-10s %-12s %-20s %-20s\n",
           "异常记录ID", "快递单号", "异常类型", "处理状态",
           "创建时间", "更新时间");
    printf("----------------------------------------"
           "----------------------------------------\n");
    
    while (current != NULL) {
        if (current->status == status) {
            found = 1;
            
            char create_time_str[20];
            char update_time_str[20];
            strftime(create_time_str, sizeof(create_time_str), "%Y-%m-%d %H:%M:%S",
                    localtime(&current->create_time));
            strftime(update_time_str, sizeof(update_time_str), "%Y-%m-%d %H:%M:%S",
                    localtime(&current->update_time));
            
            const char* type_str = get_exception_type_name(current->type);
            const char* status_str = get_exception_status_name(current->status);
            
            printf("%-15s %-15s %-10s %-12s %-20s %-20s\n",
                   current->id, current->express_id, type_str, status_str,
                   create_time_str, update_time_str);
        }
        current = current->next;
    }
    
    if (!found) {
        printf("未找到状态为 %s 的异常记录！\n", get_exception_status_name(status));
    }
}

// 批量更新异常状态
int batch_update_exception_status(ExpressException* head, ExpressExceptionStatus old_status, ExpressExceptionStatus new_status) {
    if (head == NULL) {
        printf("异常记录列表为空！\n");
        return 0;
    }
    
    int count = 0;
    ExpressException* current = head;
    
    while (current != NULL) {
        if (current->status == old_status) {
            current->status = new_status;
            current->update_time = time(NULL);
            count++;
        }
        current = current->next;
    }
    
    if (count > 0) {
        printf("已成功将 %d 条异常记录从 %s 状态更新为 %s 状态！\n", 
               count, get_exception_status_name(old_status), get_exception_status_name(new_status));
    } else {
        printf("未找到状态为 %s 的异常记录！\n", get_exception_status_name(old_status));
    }
    
    return count;
}

// 删除异常记录
int delete_exception_record(ExpressException** head, const char* exception_id) {
    if (*head == NULL) {
        printf("异常记录列表为空！\n");
        return 0;
    }
    
    ExpressException* current = *head;
    ExpressException* prev = NULL;
    
    while (current != NULL && strcmp(current->id, exception_id) != 0) {
        prev = current;
        current = current->next;
    }
    
    if (current == NULL) {
        printf("未找到异常记录ID: %s\n", exception_id);
        return 0;
    }
    
    if (prev == NULL) {
        *head = current->next;
    } else {
        prev->next = current->next;
    }
    
    free(current);
    printf("异常记录 %s 已成功删除！\n", exception_id);
    return 1;
}

// 异常处理主菜单
void exception_management_menu(Express** express_head, ExpressException** exception_head) {
    int choice;
    char express_id[20];
    char exception_id[20];
    char solution[200];
    int type_choice, status_choice;
    int old_status_choice, new_status_choice;
    
    do {
        printf("\n===== 异常处理系统 =====\n");
        printf("1. 添加异常记录\n");
        printf("2. 查看所有异常记录\n");
        printf("3. 按快递单号查询异常记录\n");
        printf("4. 按异常类型查询记录\n");
        printf("5. 按处理状态查询记录\n");
        printf("6. 更新异常记录状态\n");
        printf("7. 批量更新异常状态\n");
        printf("8. 删除异常记录\n");
        printf("9. 查看异常状态统计\n");
        printf("0. 返回上级菜单\n");
        
        choice = get_valid_choice(0, 9);
        
        switch (choice) {
            case 1:
                handle_express_exception(express_head, exception_head);
                save_exceptions(EXCEPTION_DATA_FILE, *exception_head);
                pause_program();
                break;
                
            case 2:
                print_exception_list(*exception_head);
                pause_program();
                break;
                
            case 3:
                printf("请输入要查询的快递单号: ");
                scanf("%19s", express_id);
                while (getchar() != '\n');
                find_exception_by_express_id(*exception_head, express_id);
                pause_program();
                break;
                
            case 4:
                printf("\n请选择要查询的异常类型:\n");
                printf("1. 快递损坏\n");
                printf("2. 快递丢失\n");
                printf("3. 误领\n");
                printf("4. 冒领\n");
                printf("5. 拒收\n");
                printf("6. 其他\n");
                
                type_choice = get_valid_choice(1, 6);
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
                
                find_exceptions_by_type(*exception_head, type);
                pause_program();
                break;
                
            case 5:
                printf("\n请选择要查询的处理状态:\n");
                printf("1. 待处理\n");
                printf("2. 处理中\n");
                printf("3. 已解决\n");
                printf("4. 已关闭\n");
                
                status_choice = get_valid_choice(1, 4);
                ExpressExceptionStatus status;
                
                switch (status_choice) {
                    case 1: status = EX_STATUS_PENDING; break;
                    case 2: status = EX_STATUS_HANDLING; break;
                    case 3: status = EX_STATUS_RESOLVED; break;
                    case 4: status = EX_STATUS_CLOSED; break;
                    default: status = EX_STATUS_PENDING; break;
                }
                
                find_exceptions_by_status(*exception_head, status);
                pause_program();
                break;
                
            case 6:
                printf("请输入要更新的异常记录ID: ");
                scanf("%19s", exception_id);
                while (getchar() != '\n');
                
                printf("\n请选择新的处理状态:\n");
                printf("1. 待处理\n");
                printf("2. 处理中\n");
                printf("3. 已解决\n");
                printf("4. 已关闭\n");
                
                status_choice = get_valid_choice(1, 4);
                ExpressExceptionStatus new_status;
                
                switch (status_choice) {
                    case 1: new_status = EX_STATUS_PENDING; break;
                    case 2: new_status = EX_STATUS_HANDLING; break;
                    case 3: new_status = EX_STATUS_RESOLVED; break;
                    case 4: new_status = EX_STATUS_CLOSED; break;
                    default: new_status = EX_STATUS_PENDING; break;
                }
                
                printf("请输入解决方案(如果有): ");
                fgets(solution, sizeof(solution), stdin);
                solution[strcspn(solution, "\n")] = '\0'; // 移除换行符
                
                update_exception_status(*exception_head, exception_id, new_status, solution);
                save_exceptions(EXCEPTION_DATA_FILE, *exception_head);
                pause_program();
                break;
                
            case 7:
                printf("\n请选择要批量更新的原始状态:\n");
                printf("1. 待处理\n");
                printf("2. 处理中\n");
                printf("3. 已解决\n");
                printf("4. 已关闭\n");
                
                old_status_choice = get_valid_choice(1, 4);
                ExpressExceptionStatus old_status;
                
                switch (old_status_choice) {
                    case 1: old_status = EX_STATUS_PENDING; break;
                    case 2: old_status = EX_STATUS_HANDLING; break;
                    case 3: old_status = EX_STATUS_RESOLVED; break;
                    case 4: old_status = EX_STATUS_CLOSED; break;
                    default: old_status = EX_STATUS_PENDING; break;
                }
                
                printf("\n请选择要更新为的新状态:\n");
                printf("1. 待处理\n");
                printf("2. 处理中\n");
                printf("3. 已解决\n");
                printf("4. 已关闭\n");
                
                new_status_choice = get_valid_choice(1, 4);
                ExpressExceptionStatus batch_new_status;
                
                switch (new_status_choice) {
                    case 1: batch_new_status = EX_STATUS_PENDING; break;
                    case 2: batch_new_status = EX_STATUS_HANDLING; break;
                    case 3: batch_new_status = EX_STATUS_RESOLVED; break;
                    case 4: batch_new_status = EX_STATUS_CLOSED; break;
                    default: batch_new_status = EX_STATUS_PENDING; break;
                }
                
                if (batch_update_exception_status(*exception_head, old_status, batch_new_status) > 0) {
                    save_exceptions(EXCEPTION_DATA_FILE, *exception_head);
                }
                pause_program();
                break;
                
            case 8:
                printf("请输入要删除的异常记录ID: ");
                scanf("%19s", exception_id);
                while (getchar() != '\n');
                
                if (delete_exception_record(exception_head, exception_id)) {
                    save_exceptions(EXCEPTION_DATA_FILE, *exception_head);
                }
                pause_program();
                break;
                
            case 9:
                check_exception_status(*exception_head);
                pause_program();
                break;
                
            case 0:
                printf("返回上级菜单...\n");
                break;
                
            default:
                printf("无效的选择，请重新输入！\n");
                break;
        }
    } while (choice != 0);
}