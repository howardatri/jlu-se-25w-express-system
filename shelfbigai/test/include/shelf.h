#ifndef SHELF_H
#define SHELF_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// 货架大小枚举
typedef enum {
    SHELF_SMALL,    // 小型货架
    SHELF_MEDIUM,   // 中型货架
    SHELF_LARGE     // 大型货架
} ShelfSize;

// 货架状态枚举
typedef enum {
    SHELF_NORMAL,   // 正常
    SHELF_FULL,     // 已满
    SHELF_BROKEN,   // 损坏
    SHELF_MAINTENANCE // 维护中
} ShelfStatus;

// 货架结构体
typedef struct Shelf {
    char id[20];           // 货架ID
    ShelfSize size;        // 货架大小
    int capacity;          // 容量
    int current_count;     // 当前存放数量
    ShelfStatus status;    // 货架状态
    time_t last_maintenance; // 最后维护时间
    struct Shelf* next;    // 下一个节点
} Shelf;

// 生成货架ID
void generate_shelf_id(char* buffer, ShelfSize size);

// 创建货架
Shelf* create_shelf(ShelfSize size, int capacity);

// 添加货架
void add_shelf(Shelf** head, Shelf* new_shelf);

// 删除货架
int delete_shelf(Shelf** head, const char* shelf_id);

// 更新货架信息
int update_shelf(Shelf* head, const char* shelf_id, ShelfStatus status);

// 查询货架
Shelf* find_shelf_by_id(Shelf* head, const char* shelf_id);

// 查询特定大小的可用货架
Shelf* find_available_shelf(Shelf* head, ShelfSize size);

// 打印货架列表
void print_shelf_list(Shelf* head);

// 从文件加载货架数据
Shelf* load_shelves(const char* filename);

// 保存货架数据到文件
void save_shelves(const char* filename, Shelf* head);

// 检查货架容量警告
void check_shelf_capacity_alerts(Shelf* head);

// 维护货架
void maintain_shelf(Shelf* head, const char* shelf_id);

// 获取货架大小名称
const char* get_shelf_size_name(ShelfSize size);

// 获取货架状态名称
const char* get_shelf_status_name(ShelfStatus status);

// 货架管理菜单
void shelf_management_menu(Shelf** shelf_head);

#endif // SHELF_H