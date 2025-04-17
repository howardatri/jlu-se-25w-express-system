#include "../include/storage.h"
#include "../include/utils.h"

User* load_users(const char* filename) {
    FILE* fp = safe_fopen(filename, "r", 3);
    User* head = NULL;
    
    // 如果文件不存在或为空，创建默认管理员账户
    if (fp == NULL) {
        // 创建data目录
        system("mkdir data 2>nul");
        // 创建文件并写入默认管理员账户
        fp = safe_fopen(filename, "w", 3);
        if (fp != NULL) {
            fprintf(fp, "admin admin 14743483356 0\n");
            fclose(fp);
            printf("已创建新的用户数据文件并添加默认管理员账户：%s\n", filename);
            
            // 创建默认管理员节点
            User* admin = (User*)malloc(sizeof(User));
            if (admin != NULL) {
                strcpy(admin->username, "admin");
                strcpy(admin->password, "admin");
                strcpy(admin->phone, "14743483356");
                admin->role = ADMIN;
                admin->next = NULL;
                return admin;
            }
        }
        return NULL;
    }
    
    User* current = NULL;
    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        User* node = (User*)malloc(sizeof(User));
        if (node == NULL) {
            printf("内存分配失败！\n");
            fclose(fp);
            return head;
        }
        
        int level;
        if (sscanf(line, "%49s %49s %11s %d %d %f",
                   node->username, node->password,
                   node->phone, (int*)&node->role, &level,
                   &node->total_cost) != 6) {
            // 兼容旧格式
            if (sscanf(line, "%49s %49s %11s %d",
                       node->username, node->password,
                       node->phone, (int*)&node->role) != 4) {
                free(node);
                continue;
            }
            // 设置默认值
            node->level = LEVEL_NORMAL;
            node->total_cost = 0.0f;
        } else {
            node->level = (MemberLevel)level;
        }
        
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
    
    // 如果文件存在但为空，创建默认管理员账户
    if (head == NULL) {
        User* admin = (User*)malloc(sizeof(User));
        if (admin != NULL) {
            strcpy(admin->username, "admin");
            strcpy(admin->password, "admin");
            strcpy(admin->phone, "14743483356");
            admin->role = ADMIN;
            admin->next = NULL;
            
            // 保存到文件
            fp = safe_fopen(filename, "w", 3);
            if (fp != NULL) {
                fprintf(fp, "admin admin 14743483356 0\n");
                fclose(fp);
                printf("已添加默认管理员账户\n");
            }
            
            return admin;
        }
    }
    
    return head;
}

void save_users(const char* filename, User* head) {
    FILE* fp = safe_fopen(filename, "w", 3);
    if (fp == NULL) {
        printf("无法打开文件 %s 进行写入！\n", filename);
        return;
    }
    
    User* current = head;
    while (current != NULL) {
        if (fprintf(fp, "%s %s %s %d %d %.2f\n",
                current->username, current->password,
                current->phone, current->role, current->level,
                current->total_cost) < 0) {
            printf("写入用户数据时发生错误！\n");
            break;
        }
        current = current->next;
    }
    
    fclose(fp);
    printf("用户数据已保存到文件：%s\n", filename);
}

Express* load_express(const char* filename) {
    FILE* fp = safe_fopen(filename, "r", 3);
    if (fp == NULL) {
        // 创建data目录
        system("mkdir data 2>nul");
        // 创建空文件
        fp = safe_fopen(filename, "w", 3);
        if (fp != NULL) {
            fclose(fp);
            printf("已创建新的快递数据文件：%s\n", filename);
        }
        return NULL;
    }
    
    Express* head = NULL;
    Express* current = NULL;
    int line_number = 0;
    int successful_nodes = 0;
    
    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        line_number++;
        Express* node = (Express*)malloc(sizeof(Express));
        if (node == NULL) {
            printf("错误：在处理第%d行时内存分配失败！\n", line_number);
            fclose(fp);
            return head;
        }
        
        int size, is_valuable, speed, status, delivery_to_door;
        if (sscanf(line, "%19s %49s %11s %49s %11s %6s %99s %f %f %d %d %d %f %ld %d %d %19s",
                   node->id, node->sender, node->sender_phone, node->receiver,
                   node->phone, node->pickup_code, node->fake_address,
                   &node->weight, &node->cost, &size, &is_valuable,
                   &speed, &node->distance, &node->create_time, 
                   &status, &delivery_to_door, node->shelf_id) != 17) {
            printf("错误：第%d行数据格式不正确，已跳过该记录\n", line_number);
            free(node);
            continue;
        }
        
        // 验证数据有效性
        if (strlen(node->id) == 0 || strlen(node->sender) == 0 || 
            strlen(node->sender_phone) == 0 || strlen(node->receiver) == 0 || 
            strlen(node->phone) == 0 || strlen(node->pickup_code) == 0) {
            printf("错误：第%d行存在空字段，已跳过该记录\n", line_number);
            free(node);
            continue;
        }
        
        node->size = (ExpressSize)size;
        node->is_valuable = is_valuable;
        node->speed = (ExpressSpeed)speed;
        node->status = (ExpressStatus)status;
        node->delivery_to_door = delivery_to_door;
        node->next = NULL;
        
        if (head == NULL) {
            head = node;
            current = node;
        } else {
            current->next = node;
            current = node;
        }
        
        successful_nodes++;
    }
    
    fclose(fp);
    printf("快递数据加载完成：共处理%d行数据，成功加载%d个快递记录\n", 
           line_number, successful_nodes);
    return head;
}

void save_express(const char* filename, Express* head) {
    FILE* fp = safe_fopen(filename, "w", 3);
    if (fp == NULL) {
        printf("无法打开文件 %s 进行写入！\n", filename);
        return;
    }
    
    Express* current = head;
    while (current != NULL) {
        if (fprintf(fp, "%s %s %s %s %s %s %s %.2f %.2f %d %d %d %.2f %ld %d %d %s\n",
                current->id, current->sender, current->sender_phone, current->receiver,
                current->phone, current->pickup_code, current->fake_address,
                current->weight, current->cost, current->size, current->is_valuable,
                current->speed, current->distance, current->create_time, 
                current->status, current->delivery_to_door, current->shelf_id) < 0) {
            printf("写入快递数据时发生错误！\n");
            break;
        }
        current = current->next;
    }
    
    fclose(fp);
    printf("快递数据已保存到文件：%s\n", filename);
}

// 这些函数已在各自的模块文件中实现，这里不再重复定义
// Shelf* load_shelves(const char* filename);
// void save_shelves(const char* filename, Shelf* head);
// ExpressException* load_exceptions(const char* filename);
// void save_exceptions(const char* filename, ExpressException* head);
// Transaction* load_transactions(const char* filename);
// void save_transactions(const char* filename, Transaction* head);
// Promotion* load_promotions(const char* filename);
// void save_promotions(const char* filename, Promotion* head);