#include "../include/shelf.h"
#include "../include/utils.h"
#include "../include/storage.h"

// 生成货架ID
void generate_shelf_id(char* buffer, ShelfSize size) {
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    int year = t->tm_year + 1900;
    int month = t->tm_mon + 1;
    int day = t->tm_mday;
    int seq = 1; // 默认从1开始
    char size_code = 'S'; // 默认为小型货架
    
    // 根据货架大小设置标识符
    switch (size) {
        case SHELF_SMALL: size_code = 'S'; break;
        case SHELF_MEDIUM: size_code = 'M'; break;
        case SHELF_LARGE: size_code = 'L'; break;
        default: size_code = 'S';
    }
    
    // 从文件中读取同类型的最后一个货架ID
    FILE* fp = safe_fopen(SHELF_DATA_FILE, "r", 3);
    if (fp != NULL) {
        char line[512];
        char last_id[20] = "";
        int last_year = 0, last_month = 0, last_day = 0, last_seq = 0;
        char last_size_code = '\0';
        
        // 读取文件中的所有行，找到同类型的最后一个货架ID
        while (fgets(line, sizeof(line), fp)) {
            char current_id[20];
            int current_year, current_month, current_day, current_seq;
            char current_size_code;
            
            if (sscanf(line, "%19s", current_id) == 1 && 
                sscanf(current_id, "SH%c%04d%02d%02d%04d", &current_size_code, &current_year, &current_month, &current_day, &current_seq) == 5) {
                
                // 只关注与当前货架同类型的ID
                if (current_size_code == size_code) {
                    // 更新最后一个同类型货架的信息
                    strcpy(last_id, current_id);
                    last_size_code = current_size_code;
                    last_year = current_year;
                    last_month = current_month;
                    last_day = current_day;
                    last_seq = current_seq;
                }
            }
        }
        
        // 如果找到了同类型的货架ID，检查是否是同一天的
        if (strlen(last_id) > 0) {
            // 如果是同一天同一类型，则序号加1，否则重新从1开始
            if (last_year == year && last_month == month && last_day == day) {
                seq = last_seq + 1;
                if (seq > 9999) seq = 1; // 超过9999则重新从1开始
            }
        }
        
        fclose(fp);
    }
    
    // 生成新的货架ID，包含货架类型标识符
    sprintf(buffer, "SH%c%04d%02d%02d%04d", size_code, year, month, day, seq);
}

// 创建货架
Shelf* create_shelf(ShelfSize size, int capacity) {
    Shelf* shelf = (Shelf*)malloc(sizeof(Shelf));
    if (shelf == NULL) {
        printf("内存分配失败！\n");
        return NULL;
    }
    
    // 生成货架ID，传入货架大小
    generate_shelf_id(shelf->id, size);
    
    // 设置货架属性
    shelf->size = size;
    shelf->capacity = capacity;
    shelf->current_count = 0;
    shelf->status = SHELF_NORMAL;
    shelf->last_maintenance = time(NULL);
    shelf->next = NULL;
    
    return shelf;
}

// 添加货架
// 添加货架
void add_shelf(Shelf** head, Shelf* new_shelf) {
    if (new_shelf == NULL) return;
    
    // 每次添加前重新加载最新的货架数据
    Shelf* updated_shelves = load_shelves("data/shelves.txt");
    if (updated_shelves != NULL) {
        // 使用更新后的货架数据
        *head = updated_shelves;
    }
    
    if (*head == NULL) {
        *head = new_shelf;
    } else {
        Shelf* current = *head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_shelf;
    }
    
    // 保存更新后的货架数据
    save_shelves("data/shelves.txt", *head);
    
    printf("\n货架已添加成功！\n");
    printf("货架ID: %s\n", new_shelf->id);
}

// 删除货架
int delete_shelf(Shelf** head, const char* shelf_id) {
    // 每次删除前重新加载最新的货架数据
    Shelf* updated_shelves = load_shelves("data/shelves.txt");
    if (updated_shelves != NULL) {
        // 使用更新后的货架数据
        *head = updated_shelves;
    }
    
    if (*head == NULL) {
        printf("货架列表为空！\n");
        return 0;
    }
    
    Shelf* current = *head;
    Shelf* prev = NULL;
    
    while (current != NULL && strcmp(current->id, shelf_id) != 0) {
        prev = current;
        current = current->next;
    }
    
    if (current == NULL) {
        printf("未找到该货架ID！\n");
        return 0;
    }
    
    // 检查货架是否为空
    if (current->current_count > 0) {
        printf("货架上还有快递，无法删除！\n");
        return 0;
    }
    
    if (prev == NULL) {
        *head = current->next;
    } else {
        prev->next = current->next;
    }
    
    free(current);
    
    // 保存更新后的货架数据
    save_shelves("data/shelves.txt", *head);
    
    printf("货架已删除成功！\n");
    return 1;
}

// 更新货架信息
int update_shelf(Shelf* head, const char* shelf_id, ShelfStatus status) {
    // 每次更新前重新加载最新的货架数据
    Shelf* updated_shelves = load_shelves("data/shelves.txt");
    if (updated_shelves != NULL) {
        // 使用更新后的货架数据
        head = updated_shelves;
    }
    
    Shelf* current = head;
    
    while (current != NULL && strcmp(current->id, shelf_id) != 0) {
        current = current->next;
    }
    
    if (current == NULL) {
        printf("未找到该货架ID！\n");
        return 0;
    }
    
    current->status = status;
    
    // 如果状态是维护中，更新最后维护时间
    if (status == SHELF_MAINTENANCE) {
        current->last_maintenance = time(NULL);
    }
    
    // 保存更新后的货架数据
    save_shelves("data/shelves.txt", head);
    
    printf("货架状态已更新！\n");
    return 1;
}

// 查询货架
Shelf* find_shelf_by_id(Shelf* head, const char* shelf_id) {
    // 每次查询前重新加载最新的货架数据
    Shelf* updated_shelves = load_shelves("data/shelves.txt");
    if (updated_shelves != NULL) {
        // 使用更新后的货架数据
        head = updated_shelves;
    }
    
    Shelf* current = head;
    
    while (current != NULL) {
        if (strcmp(current->id, shelf_id) == 0) {
            // 返回找到的货架，但确保它是从最新加载的数据中找到的
            return current;
        }
        current = current->next;
    }
    
    return NULL;
}

// 查询特定大小的可用货架
Shelf* find_available_shelf(Shelf* head, ShelfSize size) {
    // 每次查询前重新加载最新的货架数据
    Shelf* updated_shelves = load_shelves("data/shelves.txt");
    if (updated_shelves != NULL) {
        // 使用更新后的货架数据
        head = updated_shelves;
    }
    
    Shelf* current = head;
    
    // 添加调试信息
    printf("查找%s货架...", get_shelf_size_name(size));
    int found_count = 0;
    
    while (current != NULL) {
        // 检查货架大小和状态
        if (current->size == size) {
            found_count++;
            // 检查货架状态和容量
            if (current->status == SHELF_NORMAL && 
                current->current_count < current->capacity) {
                printf("找到可用货架: %s (当前存放: %d/%d)\n", 
                       current->id, current->current_count, current->capacity);
                return current;
            }
        }
        current = current->next;
    }
    
    if (found_count == 0) {
        printf("未找到%s货架\n", get_shelf_size_name(size));
    } else {
        printf("找到%d个%s货架，但都不可用\n", found_count, get_shelf_size_name(size));
    }
    
    return NULL;
}

// 打印货架列表
void print_shelf_list(Shelf* head) {
    // 每次打印前重新加载最新的货架数据
    Shelf* updated_shelves = load_shelves("data/shelves.txt");
    if (updated_shelves != NULL) {
        // 使用更新后的货架数据
        head = updated_shelves;
    }
    
    if (head == NULL) {
        printf("货架列表为空！\n");
        return;
    }
    
    printf("\n货架列表:\n");
    printf("%-15s %-10s %-10s %-15s %-10s %-20s\n",
           "货架ID", "大小", "状态", "容量/当前数量", "使用率", "最后维护时间");
    printf("----------------------------------------"
           "----------------------------------------\n");
    
    Shelf* current = head;
    while (current != NULL) {
        char maintenance_time[20];
        strftime(maintenance_time, sizeof(maintenance_time), "%Y-%m-%d %H:%M:%S",
                localtime(&current->last_maintenance));
        
        const char* size_str = get_shelf_size_name(current->size);
        const char* status_str = get_shelf_status_name(current->status);
        
        float usage_rate = (current->capacity > 0) ? 
                          (float)current->current_count / current->capacity * 100 : 0;
        
        printf("%-15s %-10s %-10s %-3d/%-11d %-9.1f%% %-20s\n",
               current->id, size_str, status_str,
               current->current_count, current->capacity, usage_rate,
               maintenance_time);
        
        current = current->next;
    }
}

// 从文件加载货架数据
Shelf* load_shelves(const char* filename) {
    FILE* fp = safe_fopen(filename, "r", 3);
    if (fp == NULL) {
        // 创建data目录
        system("mkdir data 2>nul");
        // 创建空文件
        fp = safe_fopen(filename, "w", 3);
        if (fp != NULL) {
            fclose(fp);
            printf("已创建新的货架数据文件：%s\n", filename);
        }
        return NULL;
    }
    
    Shelf* head = NULL;
    Shelf* current = NULL;
    
    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        Shelf* node = (Shelf*)malloc(sizeof(Shelf));
        if (node == NULL) {
            printf("内存分配失败！\n");
            fclose(fp);
            return head;
        }
        
        int size, status;
        if (sscanf(line, "%19s %d %d %d %d %ld",
                   node->id, &size, &node->capacity, &node->current_count,
                   &status, &node->last_maintenance) != 6) {
            free(node);
            continue;
        }
        
        node->size = (ShelfSize)size;
        node->status = (ShelfStatus)status;
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

// 保存货架数据到文件
void save_shelves(const char* filename, Shelf* head) {
    FILE* fp = safe_fopen(filename, "w", 3);
    if (fp == NULL) {
        printf("无法打开文件 %s 进行写入！\n", filename);
        return;
    }
    
    Shelf* current = head;
    while (current != NULL) {
        if (fprintf(fp, "%s %d %d %d %d %ld\n",
                current->id, current->size, current->capacity,
                current->current_count, current->status,
                current->last_maintenance) < 0) {
            printf("写入货架数据时发生错误！\n");
            break;
        }
        current = current->next;
    }
    
    fclose(fp);
    printf("货架数据已保存到文件：%s\n", filename);
}

// 检查货架容量警告
void check_shelf_capacity_alerts(Shelf* head) {
    // 每次检查前重新加载最新的货架数据
    Shelf* updated_shelves = load_shelves("data/shelves.txt");
    if (updated_shelves != NULL) {
        // 使用更新后的货架数据
        head = updated_shelves;
    }
    
    if (head == NULL) {
        printf("货架列表为空！\n");
        return;
    }
    
    int alert_count = 0;
    printf("\n货架容量警告:\n");
    
    Shelf* current = head;
    while (current != NULL) {
        // 如果货架使用率超过90%，发出警告
        if (current->status == SHELF_NORMAL && 
            current->current_count >= current->capacity * 0.9) {
            
            const char* size_str = get_shelf_size_name(current->size);
            float usage_rate = (float)current->current_count / current->capacity * 100;
            
            printf("警告：货架 %s (%s) 即将满载！使用率: %.1f%%\n",
                   current->id, size_str, usage_rate);
            
            alert_count++;
            
            // 如果已满，更新状态
            if (current->current_count >= current->capacity) {
                current->status = SHELF_FULL;
                printf("货架 %s 已更新为满载状态！\n", current->id);
            }
        }
        current = current->next;
    }
    
    if (alert_count == 0) {
        printf("所有货架容量正常。\n");
    }
    
    // 保存更新后的货架数据
    save_shelves("data/shelves.txt", head);
}

// 维护货架
void maintain_shelf(Shelf* head, const char* shelf_id) {
    // 每次维护前重新加载最新的货架数据
    Shelf* updated_shelves = load_shelves("data/shelves.txt");
    if (updated_shelves != NULL) {
        // 使用更新后的货架数据
        head = updated_shelves;
    }
    
    Shelf* shelf = find_shelf_by_id(head, shelf_id);
    
    if (shelf == NULL) {
        printf("未找到该货架ID！\n");
        return;
    }
    
    // 检查货架是否为空
    if (shelf->current_count > 0) {
        printf("货架上还有快递，无法进行维护！\n");
        return;
    }
    
    // 更新状态和维护时间
    shelf->status = SHELF_MAINTENANCE;
    shelf->last_maintenance = time(NULL);
    
    // 保存更新后的货架数据
    save_shelves("data/shelves.txt", head);
    
    printf("货架 %s 已进入维护状态。\n", shelf_id);
}

// 获取货架大小名称
const char* get_shelf_size_name(ShelfSize size) {
    switch (size) {
        case SHELF_SMALL: return "小型";
        case SHELF_MEDIUM: return "中型";
        case SHELF_LARGE: return "大型";
        default: return "未知";
    }
}

// 获取货架状态名称
const char* get_shelf_status_name(ShelfStatus status) {
    switch (status) {
        case SHELF_NORMAL: return "正常";
        case SHELF_FULL: return "已满";
        case SHELF_BROKEN: return "损坏";
        case SHELF_MAINTENANCE: return "维护中";
        default: return "未知";
    }
}

// 货架管理菜单
void shelf_management_menu(Shelf** shelf_head) {
    int choice;
    
    do {
        printf("\n===== 货架管理系统 =====\n");
        printf("1. 添加货架\n");
        printf("2. 删除货架\n");
        printf("3. 更新货架状态\n");
        printf("4. 查询货架\n");
        printf("5. 查看所有货架\n");
        printf("6. 检查货架容量警告\n");
        printf("7. 返回上级菜单\n");
        
        choice = get_valid_choice(1, 7);
        
        switch (choice) {
            case 1: {
                // 添加货架
                printf("\n请选择货架大小:\n");
                printf("1. 小型货架\n");
                printf("2. 中型货架\n");
                printf("3. 大型货架\n");
                
                int size_choice = get_valid_choice(1, 3);
                ShelfSize size;
                int capacity;
                
                switch (size_choice) {
                    case 1:
                        size = SHELF_SMALL;
                        capacity = 20; // 小型货架默认容量
                        break;
                    case 2:
                        size = SHELF_MEDIUM;
                        capacity = 50; // 中型货架默认容量
                        break;
                    case 3:
                        size = SHELF_LARGE;
                        capacity = 100; // 大型货架默认容量
                        break;
                    default:
                        size = SHELF_SMALL;
                        capacity = 20;
                }
                
                printf("请输入货架容量 [%d]: ", capacity);
                char input[20];
                fgets(input, sizeof(input), stdin);
                input[strcspn(input, "\n")] = '\0';
                
                if (strlen(input) > 0) {
                    int new_capacity = atoi(input);
                    if (new_capacity > 0) {
                        capacity = new_capacity;
                    }
                }
                
                Shelf* new_shelf = create_shelf(size, capacity);
                if (new_shelf != NULL) {
                    add_shelf(shelf_head, new_shelf);
                    // 移除重复的保存操作，因为add_shelf函数已经包含了重新加载最新数据的逻辑
                    // save_shelves("data/shelves.txt", *shelf_head);
                }
                break;
            }
            case 2: {
                // 删除货架
                if (*shelf_head == NULL) {
                    printf("货架列表为空！\n");
                    break;
                }
                
                //todo printlist
                
                char shelf_id[20];
                printf("\n请输入要删除的货架ID: ");
                scanf("%19s", shelf_id);
                while (getchar() != '\n');
                
                delete_shelf(shelf_head, shelf_id);
                break;
            }
            case 3: {
                // 更新货架状态
                if (*shelf_head == NULL) {
                    printf("货架列表为空！\n");
                    break;
                }
                
                print_shelf_list(*shelf_head);
                
                char shelf_id[20];
                printf("\n请输入要更新的货架ID: ");
                scanf("%19s", shelf_id);
                while (getchar() != '\n');
                
                printf("\n请选择新的货架状态:\n");
                printf("1. 正常\n");
                printf("2. 已满\n");
                printf("3. 损坏\n");
                printf("4. 维护中\n");
                
                int status_choice = get_valid_choice(1, 4);
                ShelfStatus status;
                
                switch (status_choice) {
                    case 1: status = SHELF_NORMAL; break;
                    case 2: status = SHELF_FULL; break;
                    case 3: status = SHELF_BROKEN; break;
                    case 4: status = SHELF_MAINTENANCE; break;
                    default: status = SHELF_NORMAL;
                }
                
                update_shelf(*shelf_head, shelf_id, status);
                break;
            }
            case 4: {
                // 查询货架
                if (*shelf_head == NULL) {
                    printf("货架列表为空！\n");
                    break;
                }
                
                char shelf_id[20];
                printf("请输入要查询的货架ID: ");
                scanf("%19s", shelf_id);
                while (getchar() != '\n');
                
                Shelf* shelf = find_shelf_by_id(*shelf_head, shelf_id);
                if (shelf != NULL) {
                    printf("\n货架信息:\n");
                    printf("%-15s %-10s %-10s %-15s %-10s %-20s\n",
                           "货架ID", "大小", "状态", "容量/当前数量", "使用率", "最后维护时间");
                    printf("----------------------------------------"
                           "----------------------------------------\n");
                    
                    char maintenance_time[20];
                    strftime(maintenance_time, sizeof(maintenance_time), "%Y-%m-%d %H:%M:%S",
                            localtime(&shelf->last_maintenance));
                    
                    const char* size_str = get_shelf_size_name(shelf->size);
                    const char* status_str = get_shelf_status_name(shelf->status);
                    
                    float usage_rate = (shelf->capacity > 0) ? 
                                      (float)shelf->current_count / shelf->capacity * 100 : 0;
                    
                    printf("%-15s %-10s %-10s %-3d/%-11d %-9.1f%% %-20s\n",
                           shelf->id, size_str, status_str,
                           shelf->current_count, shelf->capacity, usage_rate,
                           maintenance_time);
                } else {
                    printf("未找到该货架ID！\n");
                }
                break;
            }
            case 5:
                // 查看所有货架
                print_shelf_list(*shelf_head);
                break;
            case 6:
                // 检查货架容量警告
                check_shelf_capacity_alerts(*shelf_head);
                break;
            case 7:
                return;
        }
        
        pause_program();
    } while (1);
}