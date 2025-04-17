#ifndef FINANCE_H
#define FINANCE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// 交易类型枚举
typedef enum {
    TRANSACTION_INCOME,    // 收入
    TRANSACTION_EXPENSE,   // 支出
    TRANSACTION_REFUND     // 退款
} TransactionType;

// 支付方式枚举
typedef enum {
    PAYMENT_CASH,          // 现金
    PAYMENT_WECHAT,        // 微信
    PAYMENT_ALIPAY,        // 支付宝
    PAYMENT_CARD,          // 银行卡
    PAYMENT_OTHER          // 其他
} PaymentMethod;

// 交易记录结构体
typedef struct Transaction {
    char id[20];                // 交易ID
    char express_id[20];        // 关联的快递单号（如果有）
    TransactionType type;       // 交易类型
    PaymentMethod payment;      // 支付方式
    float amount;               // 金额
    time_t time;                // 交易时间
    char description[100];      // 交易描述
    struct Transaction* next;   // 下一个节点
} Transaction;

// 日报表结构体
typedef struct DailyReport {
    char date[11];              // 日期 (YYYY-MM-DD)
    float total_income;         // 总收入
    float total_expense;        // 总支出
    float total_refund;         // 总退款
    int express_count;          // 快递数量
    struct DailyReport* next;   // 下一个节点
} DailyReport;

// 月报表结构体
typedef struct MonthlyReport {
    char month[8];              // 月份 (YYYY-MM)
    float total_income;         // 总收入
    float total_expense;        // 总支出
    float total_refund;         // 总退款
    int express_count;          // 快递数量
    struct MonthlyReport* next; // 下一个节点
} MonthlyReport;

// 生成交易ID
void generate_transaction_id(char* buffer);

// 创建交易记录
Transaction* create_transaction(const char* express_id, TransactionType type, 
                               PaymentMethod payment, float amount, 
                               const char* description);

// 添加交易记录
void add_transaction(Transaction** head, Transaction* new_transaction);

// 从文件加载交易记录
Transaction* load_transactions(const char* filename);

// 保存交易记录到文件
void save_transactions(const char* filename, Transaction* head);

// 生成日报表
DailyReport* generate_daily_report(Transaction* transactions, const char* date);

// 生成月报表
MonthlyReport* generate_monthly_report(Transaction* transactions, const char* month);

// 打印日报表
void print_daily_report(DailyReport* report);

// 打印月报表
void print_monthly_report(MonthlyReport* report);

// 保存日报表到文件
void save_daily_report(const char* filename, DailyReport* report);

// 保存月报表到文件
void save_monthly_report(const char* filename, MonthlyReport* report);

// 模拟支付处理
int process_payment(PaymentMethod method, float amount, const char* description);

// 财务报表菜单
void finance_report_menu(Transaction* transactions);

#endif // FINANCE_H