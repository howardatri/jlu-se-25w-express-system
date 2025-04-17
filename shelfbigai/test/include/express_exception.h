#ifndef EXPRESS_EXCEPTION_H
#define EXPRESS_EXCEPTION_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "express.h"

// 快递异常类型枚举
typedef enum {
    EX_NONE,            // 无异常
    EX_DAMAGED,         // 快递损坏
    EX_LOST,            // 快递丢失
    EX_WRONG_DELIVERY,  // 误领
    EX_FAKE_PICKUP,     // 冒领
    EX_REJECTED,        // 拒收
    EX_OTHER            // 其他异常
} ExpressExceptionType;

// 异常处理状态枚举
typedef enum {
    EX_STATUS_PENDING,  // 待处理
    EX_STATUS_HANDLING, // 处理中
    EX_STATUS_RESOLVED, // 已解决
    EX_STATUS_CLOSED    // 已关闭
} ExpressExceptionStatus;

// 快递异常记录结构体
typedef struct ExpressException {
    char id[20];                    // 异常记录ID
    char express_id[20];            // 关联的快递单号
    ExpressExceptionType type;      // 异常类型
    ExpressExceptionStatus status;  // 处理状态
    char description[200];          // 异常描述
    char solution[200];             // 解决方案
    time_t create_time;             // 创建时间
    time_t update_time;             // 更新时间
    struct ExpressException* next;  // 下一个节点
} ExpressException;

// 生成异常记录ID
void generate_exception_id(char* buffer);

// 创建异常记录
ExpressException* create_exception_record(const char* express_id, ExpressExceptionType type, const char* description);

// 添加异常记录
void add_exception_record(ExpressException** head, ExpressException* new_record);

// 更新异常记录状态
void update_exception_status(ExpressException* head, const char* exception_id, ExpressExceptionStatus new_status, const char* solution);

// 查询快递异常记录
ExpressException* find_exception_by_express_id(ExpressException* head, const char* express_id);

// 打印异常记录列表
void print_exception_list(ExpressException* head);

// 从文件加载异常记录
ExpressException* load_exceptions(const char* filename);

// 保存异常记录到文件
void save_exceptions(const char* filename, ExpressException* head);

// 处理快递异常
void handle_express_exception(Express** express_head, ExpressException** exception_head);

// 检查快递异常状态
void check_exception_status(ExpressException* head);

// 异常处理主菜单
void exception_management_menu(Express** express_head, ExpressException** exception_head);

// 按类型查询异常记录
void find_exceptions_by_type(ExpressException* head, ExpressExceptionType type);

// 按状态查询异常记录
void find_exceptions_by_status(ExpressException* head, ExpressExceptionStatus status);

// 批量更新异常状态
int batch_update_exception_status(ExpressException* head, ExpressExceptionStatus old_status, ExpressExceptionStatus new_status);

// 删除异常记录
int delete_exception_record(ExpressException** head, const char* exception_id);

// 获取异常类型名称
const char* get_exception_type_name(ExpressExceptionType type);

// 获取异常状态名称
const char* get_exception_status_name(ExpressExceptionStatus status);

#endif // EXPRESS_EXCEPTION_H