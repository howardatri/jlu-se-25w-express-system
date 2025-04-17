#ifndef STORAGE_H
#define STORAGE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "user.h"
#include "express.h"
#include "shelf.h"
#include "express_exception.h"
#include "finance.h"
#include "promotion.h"

// 数据文件路径
#define USER_DATA_FILE "data/users.txt"
#define EXPRESS_DATA_FILE "data/express.txt"
#define SMS_LOG_FILE "data/sms.log"
#define SHELF_DATA_FILE "data/shelves.txt"
#define EXCEPTION_DATA_FILE "data/exceptions.txt"
#define TRANSACTION_DATA_FILE "data/transactions.txt"
#define PROMOTION_DATA_FILE "data/promotions.txt"
#define PAYMENT_DATA_FILE "data/payments.txt"

// 从文件加载用户数据
User* load_users(const char* filename);

// 保存用户数据到文件
void save_users(const char* filename, User* head);

// 从文件加载快递数据
Express* load_express(const char* filename);

// 保存快递数据到文件
void save_express(const char* filename, Express* head);

// 从文件加载货架数据
Shelf* load_shelves(const char* filename);

// 保存货架数据到文件
void save_shelves(const char* filename, Shelf* head);

// 从文件加载异常记录数据
ExpressException* load_exceptions(const char* filename);

// 保存异常记录数据到文件
void save_exceptions(const char* filename, ExpressException* head);

// 从文件加载交易记录数据
Transaction* load_transactions(const char* filename);

// 保存交易记录数据到文件
void save_transactions(const char* filename, Transaction* head);

// 从文件加载优惠活动数据
Promotion* load_promotions(const char* filename);

// 保存优惠活动数据到文件
void save_promotions(const char* filename, Promotion* head);

#endif // STORAGE_H