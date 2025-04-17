#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

// 清屏函数
void clear_screen();

// 暂停程序执行
void pause_program();

// 验证手机号格式
int validate_phone(const char* phone);

// 获取正浮点数输入
float get_positive_float(const char* prompt);

// 获取有效的选择输入
int get_valid_choice(int min, int max);

// 安全的文件打开函数
FILE* safe_fopen(const char* filename, const char* mode, int retries);

// 初始化异常处理
void init_exception_handlers();

// 清空输入缓冲区
void clear_input_buffer();


#endif // UTILS_H