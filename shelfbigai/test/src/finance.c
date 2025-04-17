#include "../include/finance.h"
#include "../include/utils.h"
#include "../include/storage.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

// 生成交易ID
void generate_transaction_id(char* buffer) {
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    int year = t->tm_year + 1900;
    int month = t->tm_mon + 1;
    int day = t->tm_mday;
    int seq = 1; // 默认从1开始
    
    // 从文件中读取最后一个交易单号
    FILE* fp = safe_fopen(TRANSACTION_DATA_FILE, "r", 3);
    if (fp != NULL) {
        char line[512];
        char last_id[20] = "";
        
        // 读取文件中的所有行，找到最后一个交易单号
        while (fgets(line, sizeof(line), fp)) {
            sscanf(line, "%19s", last_id);
        }
        
        // 如果找到了最后一个交易单号，解析它
        if (strlen(last_id) > 0) {
            int last_year, last_month, last_day, last_seq;
            if (sscanf(last_id, "TR%04d%02d%02d%04d", &last_year, &last_month, &last_day, &last_seq) == 4) {
                // 如果是同一天，则序号加1，否则重新从1开始
                if (last_year == year && last_month == month && last_day == day) {
                    seq = last_seq + 1;
                    if (seq > 9999) seq = 1; // 超过9999则重新从1开始
                }
            }
        }
        
        fclose(fp);
    }
    
    // 生成新的交易单号
    sprintf(buffer, "TR%04d%02d%02d%04d", year, month, day, seq);
}

// 创建交易记录
Transaction* create_transaction(const char* express_id, TransactionType type, 
                               PaymentMethod payment, float amount, 
                               const char* description) {
    Transaction* transaction = (Transaction*)malloc(sizeof(Transaction));
    if (transaction == NULL) {
        printf("内存分配失败！\n");
        return NULL;
    }
    
    // 生成交易ID
    generate_transaction_id(transaction->id);
    
    // 设置关联的快递单号（如果有）
    if (express_id != NULL) {
        strncpy(transaction->express_id, express_id, sizeof(transaction->express_id) - 1);
        transaction->express_id[sizeof(transaction->express_id) - 1] = '\0';
    } else {
        transaction->express_id[0] = '\0';
    }
    
    // 设置交易类型、支付方式和金额
    transaction->type = type;
    transaction->payment = payment;
    transaction->amount = amount;
    transaction->time = time(NULL);
    
    // 设置交易描述
    if (description != NULL) {
        strncpy(transaction->description, description, sizeof(transaction->description) - 1);
        transaction->description[sizeof(transaction->description) - 1] = '\0';
    } else {
        transaction->description[0] = '\0';
    }
    
    transaction->next = NULL;
    return transaction;
}

// 添加交易记录
void add_transaction(Transaction** head, Transaction* new_transaction) {
    if (new_transaction == NULL) return;
    
    if (*head == NULL) {
        *head = new_transaction;
    } else {
        Transaction* current = *head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_transaction;
    }
    
    printf("\n交易记录已添加成功！\n");
    printf("交易ID: %s\n", new_transaction->id);
}

// 从文件加载交易记录
Transaction* load_transactions(const char* filename) {
    FILE* fp = safe_fopen(filename, "r", 3);
    if (fp == NULL) {
        // 创建data目录
        system("mkdir data 2>nul");
        // 创建空文件
        fp = safe_fopen(filename, "w", 3);
        if (fp != NULL) {
            fclose(fp);
            printf("已创建新的交易记录数据文件：%s\n", filename);
        }
        return NULL;
    }
    
    Transaction* head = NULL;
    Transaction* current = NULL;
    
    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        Transaction* node = (Transaction*)malloc(sizeof(Transaction));
        if (node == NULL) {
            printf("内存分配失败！\n");
            fclose(fp);
            return head;
        }
        
        int type, payment;
        if (sscanf(line, "%19s %19s %d %d %f %ld %99[^\n]",
                   node->id, node->express_id, &type, &payment,
                   &node->amount, &node->time, node->description) != 7) {
            // 尝试读取日期文件格式的交易记录
            char time_str[20];
            if (sscanf(line, "%19s %19s %19s %f %d %99[^\n]",
                      node->id, time_str, node->express_id, 
                      &node->amount, &type, node->description) == 6) {
                // 设置默认支付方式为现金
                payment = PAYMENT_CASH;
                // 设置时间为当前时间
                node->time = time(NULL);
            } else {
                free(node);
                continue;
            }
        }
        
        node->type = (TransactionType)type;
        node->payment = (PaymentMethod)payment;
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

// 保存交易记录到文件
void save_transactions(const char* filename, Transaction* head) {
    // 使用"w"模式打开文件，这样每次都会重写整个文件，避免重复记录
    FILE* fp = safe_fopen(filename, "w", 3);
    if (fp == NULL) {
        printf("无法打开文件 %s 进行写入！\n", filename);
        return;
    }
    
    Transaction* current = head;
    while (current != NULL) {
        if (fprintf(fp, "%s %s %d %d %.2f %ld %s\n",
                current->id, current->express_id, current->type, current->payment,
                current->amount, current->time, current->description) < 0) {
            printf("写入交易记录数据时发生错误！\n");
            break;
        }
        current = current->next;
    }
    
    fclose(fp);
    printf("交易记录数据已保存到文件：%s\n", filename);
}

// 生成日报表
DailyReport* generate_daily_report(Transaction* transactions, const char* date) {
    if (transactions == NULL) {
        printf("交易记录为空！\n");
        return NULL;
    }
    
    DailyReport* report = (DailyReport*)malloc(sizeof(DailyReport));
    if (report == NULL) {
        printf("内存分配失败！\n");
        return NULL;
    }
    
    // 初始化报表数据
    strncpy(report->date, date, sizeof(report->date) - 1);
    report->date[sizeof(report->date) - 1] = '\0';
    report->total_income = 0.0f;
    report->total_expense = 0.0f;
    report->total_refund = 0.0f;
    report->express_count = 0;
    report->next = NULL;
    
    // 解析日期字符串为时间结构
    struct tm report_date = {0};
    sscanf(date, "%d-%d-%d", &report_date.tm_year, &report_date.tm_mon, &report_date.tm_mday);
    report_date.tm_year -= 1900;  // 年份需要减去1900
    report_date.tm_mon -= 1;      // 月份从0开始
    
    // 计算报表日期的起始和结束时间戳
    time_t start_time = mktime(&report_date);
    report_date.tm_hour = 23;
    report_date.tm_min = 59;
    report_date.tm_sec = 59;
    time_t end_time = mktime(&report_date);
    
    // 统计交易数据
    Transaction* current = transactions;
    while (current != NULL) {
        // 检查交易是否在指定日期范围内
        if (current->time >= start_time && current->time <= end_time) {
            // 根据交易类型累加金额
            switch (current->type) {
                case TRANSACTION_INCOME:
                    report->total_income += current->amount;
                    if (strlen(current->express_id) > 0) {
                        report->express_count++;
                    }
                    break;
                case TRANSACTION_EXPENSE:
                    report->total_expense += current->amount;
                    break;
                case TRANSACTION_REFUND:
                    report->total_refund += current->amount;
                    break;
            }
        }
        current = current->next;
    }
    
    return report;
}

// 生成月报表
MonthlyReport* generate_monthly_report(Transaction* transactions, const char* month) {
    if (transactions == NULL) {
        printf("交易记录为空！\n");
        return NULL;
    }
    
    MonthlyReport* report = (MonthlyReport*)malloc(sizeof(MonthlyReport));
    if (report == NULL) {
        printf("内存分配失败！\n");
        return NULL;
    }
    
    // 初始化报表数据
    strncpy(report->month, month, sizeof(report->month) - 1);
    report->month[sizeof(report->month) - 1] = '\0';
    report->total_income = 0.0f;
    report->total_expense = 0.0f;
    report->total_refund = 0.0f;
    report->express_count = 0;
    report->next = NULL;
    
    // 解析月份字符串为时间结构
    struct tm report_month = {0};
    sscanf(month, "%d-%d", &report_month.tm_year, &report_month.tm_mon);
    report_month.tm_year -= 1900;  // 年份需要减去1900
    report_month.tm_mon -= 1;      // 月份从0开始
    report_month.tm_mday = 1;       // 月初
    
    // 计算报表月份的起始时间戳
    time_t start_time = mktime(&report_month);
    
    // 计算下个月的起始时间戳作为结束时间
    report_month.tm_mon += 1;
    if (report_month.tm_mon > 11) {
        report_month.tm_mon = 0;
        report_month.tm_year += 1;
    }
    time_t end_time = mktime(&report_month) - 1; // 减1秒得到当月最后一秒
    
    // 统计交易数据
    Transaction* current = transactions;
    while (current != NULL) {
        // 检查交易是否在指定月份范围内
        if (current->time >= start_time && current->time <= end_time) {
            // 根据交易类型累加金额
            switch (current->type) {
                case TRANSACTION_INCOME:
                    report->total_income += current->amount;
                    if (strlen(current->express_id) > 0) {
                        report->express_count++;
                    }
                    break;
                case TRANSACTION_EXPENSE:
                    report->total_expense += current->amount;
                    break;
                case TRANSACTION_REFUND:
                    report->total_refund += current->amount;
                    break;
            }
        }
        current = current->next;
    }
    
    return report;
}

// 打印日报表
void print_daily_report(DailyReport* report) {
    if (report == NULL) {
        printf("报表为空！\n");
        return;
    }
    
    printf("\n===== 日报表 (%s) =====\n", report->date);
    printf("总收入: %.2f元\n", report->total_income);
    printf("总支出: %.2f元\n", report->total_expense);
    printf("总退款: %.2f元\n", report->total_refund);
    printf("净收入: %.2f元\n", report->total_income - report->total_expense - report->total_refund);
    printf("快递数量: %d个\n", report->express_count);
    printf("===========================\n");
}

// 打印月报表
void print_monthly_report(MonthlyReport* report) {
    if (report == NULL) {
        printf("报表为空！\n");
        return;
    }
    
    printf("\n===== 月报表 (%s) =====\n", report->month);
    printf("总收入: %.2f元\n", report->total_income);
    printf("总支出: %.2f元\n", report->total_expense);
    printf("总退款: %.2f元\n", report->total_refund);
    printf("净收入: %.2f元\n", report->total_income - report->total_expense - report->total_refund);
    printf("快递数量: %d个\n", report->express_count);
    printf("===========================\n");
}

// 保存日报表到文件
void save_daily_report(const char* filename, DailyReport* report) {
    if (report == NULL) {
        printf("报表为空，无法保存！\n");
        return;
    }
    
    FILE* fp = safe_fopen(filename, "w", 3);
    if (fp == NULL) {
        printf("无法打开文件 %s 进行写入！\n", filename);
        return;
    }
    
    fprintf(fp, "===== 日报表 (%s) =====\n", report->date);
    fprintf(fp, "总收入: %.2f元\n", report->total_income);
    fprintf(fp, "总支出: %.2f元\n", report->total_expense);
    fprintf(fp, "总退款: %.2f元\n", report->total_refund);
    fprintf(fp, "净收入: %.2f元\n", report->total_income - report->total_expense - report->total_refund);
    fprintf(fp, "快递数量: %d个\n", report->express_count);
    fprintf(fp, "===========================\n");
    
    fclose(fp);
    printf("日报表已保存到文件：%s\n", filename);
}

// 保存月报表到文件
void save_monthly_report(const char* filename, MonthlyReport* report) {
    if (report == NULL) {
        printf("报表为空，无法保存！\n");
        return;
    }
    
    FILE* fp = safe_fopen(filename, "w", 3);
    if (fp == NULL) {
        printf("无法打开文件 %s 进行写入！\n", filename);
        return;
    }
    
    fprintf(fp, "===== 月报表 (%s) =====\n", report->month);
    fprintf(fp, "总收入: %.2f元\n", report->total_income);
    fprintf(fp, "总支出: %.2f元\n", report->total_expense);
    fprintf(fp, "总退款: %.2f元\n", report->total_refund);
    fprintf(fp, "净收入: %.2f元\n", report->total_income - report->total_expense - report->total_refund);
    fprintf(fp, "快递数量: %d个\n", report->express_count);
    fprintf(fp, "===========================\n");
    
    fclose(fp);
    printf("月报表已保存到文件：%s\n", filename);
}

// 模拟支付处理
int process_payment(PaymentMethod method, float amount, const char* description) {
    printf("\n正在处理支付...");
    
    // 模拟支付延迟
    for (int i = 0; i < 3; i++) {
        printf(".");
        fflush(stdout);
        
#ifdef _WIN32
        Sleep(500);
#else
        usleep(500000);
#endif
    }
    
    // 获取支付方式名称
    const char* method_name;
    switch (method) {
        case PAYMENT_CASH: method_name = "现金"; break;
        case PAYMENT_WECHAT: method_name = "微信"; break;
        case PAYMENT_ALIPAY: method_name = "支付宝"; break;
        case PAYMENT_CARD: method_name = "银行卡"; break;
        case PAYMENT_OTHER: method_name = "其他"; break;
        default: method_name = "未知"; break;
    }
    
    // 模拟随机成功或失败（90%成功率）
    int success = (rand() % 10) < 9;
    
    if (success) {
        printf("\n支付成功！\n");
        printf("支付方式: %s\n", method_name);
        printf("支付金额: %.2f元\n", amount);
        printf("交易描述: %s\n", description);
    } else {
        printf("\n支付失败！请重试。\n");
    }
    
    return success;
}

// 财务报表菜单
void finance_report_menu(Transaction* transactions) {
    int choice;
    char date_str[11];
    char month_str[8];
    char year_str[5];
    char filename[100];
    char transactions_filename[100];
    
    do {
        printf("\n===== 财务报表系统 =====\n");
        printf("1. 生成日报表\n");
        printf("2. 生成月报表\n");
        printf("3. 生成年报表\n");
        printf("4. 查看交易记录\n");
        printf("5. 返回上级菜单\n");
        
        choice = get_valid_choice(1, 5);
        
        switch (choice) {
            case 1: {
                // 获取日期输入
                time_t now = time(NULL);
                struct tm* t = localtime(&now);
                sprintf(date_str, "%04d-%02d-%02d", t->tm_year+1900, t->tm_mon+1, t->tm_mday);
                
                printf("请输入日期 (YYYY-MM-DD) [%s]: ", date_str);
                char input[20];
                fgets(input, sizeof(input), stdin);
                input[strcspn(input, "\n")] = '\0';
                
                if (strlen(input) > 0) {
                    strncpy(date_str, input, sizeof(date_str) - 1);
                    date_str[sizeof(date_str) - 1] = '\0';
                }
                
                // 从主交易文件加载所有交易记录
                Transaction* all_transactions = load_transactions(TRANSACTION_DATA_FILE);
                
                // 生成日报表
                DailyReport* report = generate_daily_report(all_transactions, date_str);
                if (report != NULL) {
                    print_daily_report(report);
                    
                    // 生成日期对应的交易记录文件名
                    sprintf(transactions_filename, "data/transactions_%s.txt", date_str);
                    sprintf(filename, "data/daily_report_%s.txt", date_str);
                    
                    // 询问是否保存报表
                    printf("是否保存报表到文件？(1:是 0:否): ");
                    int save_choice;
                    scanf("%d", &save_choice);
                    while (getchar() != '\n');
                    
                    if (save_choice) {
                        // 保存日报表
                        save_daily_report(filename, report);
                        printf("日报表已保存到文件：%s\n", filename);
                        
                        // 将符合日期的交易记录保存到单独的文件
                        FILE* fp = safe_fopen(transactions_filename, "w", 3);
                        if (fp != NULL) {
                            // 解析日期字符串为时间结构
                            struct tm report_date = {0};
                            sscanf(date_str, "%d-%d-%d", &report_date.tm_year, &report_date.tm_mon, &report_date.tm_mday);
                            report_date.tm_year -= 1900;  // 年份需要减去1900
                            report_date.tm_mon -= 1;      // 月份从0开始
                            
                            // 计算报表日期的起始和结束时间戳
                            time_t start_time = mktime(&report_date);
                            report_date.tm_hour = 23;
                            report_date.tm_min = 59;
                            report_date.tm_sec = 59;
                            time_t end_time = mktime(&report_date);
                            
                            // 保存符合日期的交易记录
                            Transaction* current = all_transactions;
                            while (current != NULL) {
                                if (current->time >= start_time && current->time <= end_time) {
                                    fprintf(fp, "%s %s %d %d %.2f %ld %s\n",
                                            current->id, current->express_id, current->type, current->payment,
                                            current->amount, current->time, current->description);
                                }
                                current = current->next;
                            }
                            fclose(fp);
                            printf("该日期的交易记录已保存到文件：%s\n", transactions_filename);
                        }
                    }
                    
                    free(report);
                }
                
                // 释放内存
                while (all_transactions != NULL) {
                    Transaction* temp = all_transactions;
                    all_transactions = all_transactions->next;
                    free(temp);
                }
                break;
            }
            case 2: {
                // 获取月份输入
                time_t now = time(NULL);
                struct tm* t = localtime(&now);
                sprintf(month_str, "%04d-%02d", t->tm_year+1900, t->tm_mon+1);
                
                printf("请输入月份 (YYYY-MM) [%s]: ", month_str);
                char input[20];
                fgets(input, sizeof(input), stdin);
                input[strcspn(input, "\n")] = '\0';
                
                if (strlen(input) > 0) {
                    strncpy(month_str, input, sizeof(month_str) - 1);
                    month_str[sizeof(month_str) - 1] = '\0';
                }
                
                // 从主交易文件加载所有交易记录
                Transaction* all_transactions = load_transactions(TRANSACTION_DATA_FILE);
                
                // 生成月报表
                MonthlyReport* report = generate_monthly_report(all_transactions, month_str);
                if (report != NULL) {
                    print_monthly_report(report);
                    
                    // 生成月份对应的交易记录文件名
                    sprintf(transactions_filename, "data/transactions_%s.txt", month_str);
                    sprintf(filename, "data/monthly_report_%s.txt", month_str);
                    
                    // 询问是否保存报表
                    printf("是否保存报表到文件？(1:是 0:否): ");
                    int save_choice;
                    scanf("%d", &save_choice);
                    while (getchar() != '\n');
                    
                    if (save_choice) {
                        // 保存月报表
                        save_monthly_report(filename, report);
                        printf("月报表已保存到文件：%s\n", filename);
                        
                        // 将符合月份的交易记录保存到单独的文件
                        FILE* fp = safe_fopen(transactions_filename, "w", 3);
                        if (fp != NULL) {
                            // 解析月份字符串为时间结构
                            struct tm report_month = {0};
                            sscanf(month_str, "%d-%d", &report_month.tm_year, &report_month.tm_mon);
                            report_month.tm_year -= 1900;  // 年份需要减去1900
                            report_month.tm_mon -= 1;      // 月份从0开始
                            report_month.tm_mday = 1;       // 月初
                            
                            // 计算报表月份的起始时间戳
                            time_t start_time = mktime(&report_month);
                            
                            // 计算下个月的起始时间戳作为结束时间
                            report_month.tm_mon += 1;
                            if (report_month.tm_mon > 11) {
                                report_month.tm_mon = 0;
                                report_month.tm_year += 1;
                            }
                            time_t end_time = mktime(&report_month) - 1; // 减1秒得到当月最后一秒
                            
                            // 保存符合月份的交易记录
                            Transaction* current = all_transactions;
                            while (current != NULL) {
                                if (current->time >= start_time && current->time <= end_time) {
                                    fprintf(fp, "%s %s %d %d %.2f %ld %s\n",
                                            current->id, current->express_id, current->type, current->payment,
                                            current->amount, current->time, current->description);
                                }
                                current = current->next;
                            }
                            fclose(fp);
                            printf("该月份的交易记录已保存到文件：%s\n", transactions_filename);
                        }
                    }
                    
                    free(report);
                }
                
                // 释放内存
                while (all_transactions != NULL) {
                    Transaction* temp = all_transactions;
                    all_transactions = all_transactions->next;
                    free(temp);
                }
                break;
            }
            case 3: {
                // 获取年份输入
                time_t now = time(NULL);
                struct tm* t = localtime(&now);
                sprintf(year_str, "%04d", t->tm_year+1900);
                
                printf("请输入年份 (YYYY) [%s]: ", year_str);
                char input[20];
                fgets(input, sizeof(input), stdin);
                input[strcspn(input, "\n")] = '\0';
                
                if (strlen(input) > 0) {
                    strncpy(year_str, input, sizeof(year_str) - 1);
                    year_str[sizeof(year_str) - 1] = '\0';
                }
                
                // 从主交易文件加载所有交易记录
                Transaction* all_transactions = load_transactions(TRANSACTION_DATA_FILE);
                
                // 生成年报表（使用月报表结构，但按年统计）
                MonthlyReport* report = (MonthlyReport*)malloc(sizeof(MonthlyReport));
                if (report == NULL) {
                    printf("内存分配失败！\n");
                    break;
                }
                
                // 初始化报表数据
                strncpy(report->month, year_str, sizeof(report->month) - 1);
                report->month[sizeof(report->month) - 1] = '\0';
                report->total_income = 0.0f;
                report->total_expense = 0.0f;
                report->total_refund = 0.0f;
                report->express_count = 0;
                report->next = NULL;
                
                // 解析年份字符串为时间结构
                struct tm report_year = {0};
                sscanf(year_str, "%d", &report_year.tm_year);
                report_year.tm_year -= 1900;  // 年份需要减去1900
                report_year.tm_mon = 0;       // 1月
                report_year.tm_mday = 1;       // 1日
                
                // 计算报表年份的起始时间戳
                time_t start_time = mktime(&report_year);
                
                // 计算下一年的起始时间戳作为结束时间
                report_year.tm_year += 1;
                time_t end_time = mktime(&report_year) - 1; // 减1秒得到当年最后一秒
                
                // 统计交易数据
                Transaction* current = all_transactions;
                while (current != NULL) {
                    // 检查交易是否在指定年份范围内
                    if (current->time >= start_time && current->time <= end_time) {
                        // 根据交易类型累加金额
                        switch (current->type) {
                            case TRANSACTION_INCOME:
                                report->total_income += current->amount;
                                if (strlen(current->express_id) > 0) {
                                    report->express_count++;
                                }
                                break;
                            case TRANSACTION_EXPENSE:
                                report->total_expense += current->amount;
                                break;
                            case TRANSACTION_REFUND:
                                report->total_refund += current->amount;
                                break;
                        }
                    }
                    current = current->next;
                }
                
                // 打印年报表
                printf("\n===== 年报表 (%s) =====\n", year_str);
                printf("总收入: %.2f元\n", report->total_income);
                printf("总支出: %.2f元\n", report->total_expense);
                printf("总退款: %.2f元\n", report->total_refund);
                printf("净收入: %.2f元\n", report->total_income - report->total_expense - report->total_refund);
                printf("快递数量: %d个\n", report->express_count);
                printf("===========================\n");
                
                // 生成年份对应的交易记录文件名和报表文件名
                sprintf(transactions_filename, "data/transactions_%s.txt", year_str);
                sprintf(filename, "data/yearly_report_%s.txt", year_str);
                
                // 询问是否保存报表
                printf("是否保存报表到文件？(1:是 0:否): ");
                int save_choice;
                scanf("%d", &save_choice);
                while (getchar() != '\n');
                
                if (save_choice) {
                    // 保存年报表
                    FILE* fp = safe_fopen(filename, "w", 3);
                    if (fp != NULL) {
                        fprintf(fp, "===== 年报表 (%s) =====\n", year_str);
                        fprintf(fp, "总收入: %.2f元\n", report->total_income);
                        fprintf(fp, "总支出: %.2f元\n", report->total_expense);
                        fprintf(fp, "总退款: %.2f元\n", report->total_refund);
                        fprintf(fp, "净收入: %.2f元\n", report->total_income - report->total_expense - report->total_refund);
                        fprintf(fp, "快递数量: %d个\n", report->express_count);
                        fprintf(fp, "===========================\n");
                        fclose(fp);
                        printf("年报表已保存到文件：%s\n", filename);
                    }
                    
                    // 将符合年份的交易记录保存到单独的文件
                    fp = safe_fopen(transactions_filename, "w", 3);
                    if (fp != NULL) {
                        // 保存符合年份的交易记录
                        current = all_transactions;
                        while (current != NULL) {
                            if (current->time >= start_time && current->time <= end_time) {
                                fprintf(fp, "%s %s %d %d %.2f %ld %s\n",
                                        current->id, current->express_id, current->type, current->payment,
                                        current->amount, current->time, current->description);
                            }
                            current = current->next;
                        }
                        fclose(fp);
                        printf("该年份的交易记录已保存到文件：%s\n", transactions_filename);
                    }
                }
                
                free(report);
                
                // 释放内存
                while (all_transactions != NULL) {
                    Transaction* temp = all_transactions;
                    all_transactions = all_transactions->next;
                    free(temp);
                }
                break;
            }
            case 4: {
                // 从主交易文件加载所有交易记录
                Transaction* all_transactions = load_transactions(TRANSACTION_DATA_FILE);
                
                printf("\n===== 交易记录列表 =====\n");
                printf("交易ID        快递单号        类型    支付方式  金额      时间                描述\n");
                printf("---------------------------------------------------------------------------------\n");
                
                Transaction* current = all_transactions;
                int count = 0;
                
                while (current != NULL) {
                    char time_str[20];
                    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&current->time));
                    
                    const char* type_str;
                    switch (current->type) {
                        case TRANSACTION_INCOME: type_str = "收入"; break;
                        case TRANSACTION_EXPENSE: type_str = "支出"; break;
                        case TRANSACTION_REFUND: type_str = "退款"; break;
                        default: type_str = "未知"; break;
                    }
                    
                    const char* payment_str;
                    switch (current->payment) {
                        case PAYMENT_CASH: payment_str = "现金"; break;
                        case PAYMENT_WECHAT: payment_str = "微信"; break;
                        case PAYMENT_ALIPAY: payment_str = "支付宝"; break;
                        case PAYMENT_CARD: payment_str = "银行卡"; break;
                        case PAYMENT_OTHER: payment_str = "其他"; break;
                        default: payment_str = "未知"; break;
                    }
                    
                    printf("%-13s %-15s %-7s %-9s %-9.2f %-19s %s\n",
                           current->id,
                           current->express_id[0] ? current->express_id : "无",
                           type_str,
                           payment_str,
                           current->amount,
                           time_str,
                           current->description);
                    
                    current = current->next;
                    count++;
                }
                
                if (count == 0) {
                    printf("暂无交易记录！\n");
                } else {
                    printf("---------------------------------------------------------------------------------\n");
                    printf("共 %d 条交易记录\n", count);
                }
                
                // 释放内存
                while (all_transactions != NULL) {
                    Transaction* temp = all_transactions;
                    all_transactions = all_transactions->next;
                    free(temp);
                }
                break;
            }
            case 5:
                return;
        }
        
        pause_program();
    } while (1);
}