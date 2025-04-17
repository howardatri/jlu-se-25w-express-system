#ifndef PAYMENT_H
#define PAYMENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "finance.h"

// 支付数据文件路径
#define PAYMENT_DATA_FILE "data/payments.txt"

// 支付状态枚举
typedef enum {
    PAYMENT_SUCCESS,    // 支付成功
    PAYMENT_FAILED,     // 支付失败
    PAYMENT_PENDING     // 支付处理中
} PaymentStatus;

// 支付结构体
typedef struct Payment {
    char id[20];                // 支付ID
    char express_id[20];        // 关联的快递单号
    PaymentMethod method;       // 支付方式
    float amount;               // 金额
    time_t time;                // 支付时间
    PaymentStatus status;       // 支付状态
    char transaction_id[20];    // 关联的交易记录ID
    struct Payment* next;       // 下一个节点
} Payment;

// 生成支付ID
void generate_payment_id(char* buffer);

// 创建支付记录
Payment* create_payment(const char* express_id, PaymentMethod method, float amount);

// 处理支付
PaymentStatus process_payment_transaction(Payment* payment, Transaction** transaction_list);

// 获取支付状态名称
const char* get_payment_status_name(PaymentStatus status);

// 获取支付方式名称
const char* get_payment_method_name(PaymentMethod method);

// 从文件加载支付记录
Payment* load_payments(const char* filename);

// 保存支付记录到文件
void save_payments(const char* filename, Payment* head);

#endif // PAYMENT_H