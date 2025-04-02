#ifndef STORAGE_H
#define STORAGE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "user.h"
#include "express.h"

// 数据文件路径
#define USER_DATA_FILE "data/users.txt"
#define EXPRESS_DATA_FILE "data/express.txt"
#define SMS_LOG_FILE "data/sms.log"

// 从文件加载用户数据
User* load_users(const char* filename);

// 保存用户数据到文件
void save_users(const char* filename, User* head);

// 从文件加载快递数据
Express* load_express(const char* filename);

// 保存快递数据到文件
void save_express(const char* filename, Express* head);

#endif // STORAGE_H