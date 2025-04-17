#include "../include/payment.h"
#include "../include/utils.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#define Sleep(x) usleep((x)*1000)
#endif

// 生成支付ID
void generate_payment_id(char* buffer) {
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    int year = t->tm_year + 1900;
    int month = t->tm_mon + 1;
    int day = t->tm_mday;
    int seq = 1; // 默认从1开始
    
    // 从文件中读取最后一个支付单号
    FILE* fp = safe_fopen(PAYMENT_DATA_FILE, "r", 3);
    if (fp != NULL) {
        char line[512];
        char last_id[20] = "";
        
        // 读取文件中的所有行，找到最后一个支付单号
        while (fgets(line, sizeof(line), fp)) {
            sscanf(line, "%19s", last_id);
        }
        
        // 如果找到了最后一个支付单号，解析它
        if (strlen(last_id) > 0) {
            int last_year, last_month, last_day, last_seq;
            if (sscanf(last_id, "PM%04d%02d%02d%04d", &last_year, &last_month, &last_day, &last_seq) == 4) {
                // 如果是同一天，则序号加1，否则重新从1开始
                if (last_year == year && last_month == month && last_day == day) {
                    seq = last_seq + 1;
                    if (seq > 9999) seq = 1; // 超过9999则重新从1开始
                }
            }
        }
        
        fclose(fp);
    }
    
    // 生成新的支付单号
    sprintf(buffer, "PM%04d%02d%02d%04d", year, month, day, seq);
}

// 创建支付记录
Payment* create_payment(const char* express_id, PaymentMethod method, float amount) {
    Payment* payment = (Payment*)malloc(sizeof(Payment));
    if (payment == NULL) {
        printf("内存分配失败！\n");
        return NULL;
    }
    
    // 生成支付ID
    generate_payment_id(payment->id);
    
    // 设置关联的快递单号
    if (express_id != NULL) {
        strncpy(payment->express_id, express_id, sizeof(payment->express_id) - 1);
        payment->express_id[sizeof(payment->express_id) - 1] = '\0';
    } else {
        payment->express_id[0] = '\0';
    }
    
    // 设置支付方式和金额
    payment->method = method;
    payment->amount = amount;
    payment->time = time(NULL);
    payment->status = PAYMENT_PENDING;
    payment->transaction_id[0] = '\0';
    payment->next = NULL;
    
    return payment;
}

// 处理支付
PaymentStatus process_payment_transaction(Payment* payment, Transaction** transaction_list) {
    if (payment == NULL) {
        return PAYMENT_FAILED;
    }
    
    // 获取原始费用（假设原始费用为支付金额的1.2倍，作为示例）
    float original_cost = payment->amount * 1.2;
    
    // 模拟支付处理
    printf("\n正在处理支付...\n");
    printf("支付方式: %s\n", get_payment_method_name(payment->method));
    printf("原始费用: %.2f元\n", original_cost);
    printf("优惠后金额: %.2f元\n", payment->amount);
    printf("节省金额: %.2f元\n", original_cost - payment->amount);
    
    // 创建交易记录
    char description[100];
    snprintf(description, sizeof(description), "快递费用支付 (单号: %s)", payment->express_id);
    
    Transaction* transaction = create_transaction(
        payment->express_id,
        TRANSACTION_INCOME,
        payment->method,
        payment->amount,
        description
    );
    
    if (transaction == NULL) {
        payment->status = PAYMENT_FAILED;
        return PAYMENT_FAILED;
    }
    
    // 模拟支付处理延迟
    printf("正在处理支付，请稍候...\n");
    Sleep(3000); // 延迟3秒模拟支付处理
    
    // 添加交易记录
    add_transaction(transaction_list, transaction);
    
    // 更新支付状态和关联的交易ID
    payment->status = PAYMENT_SUCCESS;
    strncpy(payment->transaction_id, transaction->id, sizeof(payment->transaction_id) - 1);
    payment->transaction_id[sizeof(payment->transaction_id) - 1] = '\0';
    
    printf("支付处理成功！\n");
    printf("交易ID: %s\n", transaction->id);
    
    return PAYMENT_SUCCESS;
}

// 获取支付状态名称
const char* get_payment_status_name(PaymentStatus status) {
    switch (status) {
        case PAYMENT_SUCCESS: return "支付成功";
        case PAYMENT_FAILED: return "支付失败";
        case PAYMENT_PENDING: return "处理中";
        default: return "未知状态";
    }
}

// 获取支付方式名称
const char* get_payment_method_name(PaymentMethod method) {
    switch (method) {
        case PAYMENT_CASH: return "现金";
        case PAYMENT_WECHAT: return "微信";
        case PAYMENT_ALIPAY: return "支付宝";
        case PAYMENT_CARD: return "银行卡";
        case PAYMENT_OTHER: return "其他";
        default: return "未知方式";
    }
}

// 从文件加载支付记录
Payment* load_payments(const char* filename) {
    FILE* fp = safe_fopen(filename, "r", 3);
    if (fp == NULL) {
        // 创建data目录
        system("mkdir data 2>nul");
        // 创建空文件
        fp = safe_fopen(filename, "w", 3);
        if (fp != NULL) {
            fclose(fp);
            printf("已创建新的支付记录数据文件：%s\n", filename);
        }
        return NULL;
    }
    
    Payment* head = NULL;
    Payment* current = NULL;
    
    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        Payment* node = (Payment*)malloc(sizeof(Payment));
        if (node == NULL) {
            printf("内存分配失败！\n");
            fclose(fp);
            return head;
        }
        
        int method, status;
        if (sscanf(line, "%19s %19s %d %f %ld %d %19s",
                   node->id, node->express_id, &method, &node->amount,
                   &node->time, &status, node->transaction_id) != 7) {
            free(node);
            continue;
        }
        
        node->method = (PaymentMethod)method;
        node->status = (PaymentStatus)status;
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

// 保存支付记录到文件
void save_payments(const char* filename, Payment* head) {
    FILE* fp = safe_fopen(filename, "w", 3);
    if (fp == NULL) {
        printf("无法打开文件 %s 进行写入！\n", filename);
        return;
    }
    
    Payment* current = head;
    while (current != NULL) {
        if (fprintf(fp, "%s %s %d %.2f %ld %d %s\n",
                current->id, current->express_id, current->method, current->amount,
                current->time, current->status, current->transaction_id) < 0) {
            printf("写入支付数据时发生错误！\n");
            break;
        }
        current = current->next;
    }
    
    fclose(fp);
    printf("支付数据已保存到文件：%s\n", filename);
}