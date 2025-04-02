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
            fprintf(fp, "admin admin 14743483358 1\n");
            fclose(fp);
            printf("已创建新的用户数据文件并添加默认管理员账户：%s\n", filename);
            
            // 创建默认管理员节点
            User* admin = (User*)malloc(sizeof(User));
            if (admin != NULL) {
                strcpy(admin->username, "admin");
                strcpy(admin->password, "admin");
                strcpy(admin->phone, "14743483358");
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
        
        if (sscanf(line, "%49s %49s %11s %d",
                   node->username, node->password,
                   node->phone, (int*)&node->role) != 4) {
            free(node);
            continue;
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
            strcpy(admin->phone, "14743483358");
            admin->role = ADMIN;
            admin->next = NULL;
            
            // 保存到文件
            fp = safe_fopen(filename, "w", 3);
            if (fp != NULL) {
                fprintf(fp, "admin admin 14743483358 1\n");
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
        if (fprintf(fp, "%s %s %s %d\n",
                current->username, current->password,
                current->phone, current->role) < 0) {
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
    
    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        Express* node = (Express*)malloc(sizeof(Express));
        if (node == NULL) {
            printf("内存分配失败！\n");
            fclose(fp);
            return head;
        }
        
        if (sscanf(line, "%19s %49s %49s %11s %f %f %ld %d",
                   node->id, node->sender, node->receiver,
                   node->phone, &node->weight, &node->cost,
                   &node->create_time, (int*)&node->status) != 8) {
            free(node);
            continue;
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
        if (fprintf(fp, "%s %s %s %s %.2f %.2f %ld %d\n",
                current->id, current->sender, current->receiver,
                current->phone, current->weight, current->cost,
                current->create_time, current->status) < 0) {
            printf("写入快递数据时发生错误！\n");
            break;
        }
        current = current->next;
    }
    
    fclose(fp);
    printf("快递数据已保存到文件：%s\n", filename);
}