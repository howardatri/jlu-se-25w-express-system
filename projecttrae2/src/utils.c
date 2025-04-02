#include "../include/utils.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

void clear_screen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void pause_program() {
    printf("\n按回车键继续...");
    while (getchar() != '\n');
    getchar();
}

int validate_phone(const char* phone) {
    if (strlen(phone) != 11) return 0;
    if (phone[0] != '1') return 0;
    
    for (int i = 0; i < 11; i++) {
        if (phone[i] < '0' || phone[i] > '9') return 0;
    }
    return 1;
}

float get_positive_float(const char* prompt) {
    float value;
    char input[100];
    
    do {
        printf("%s", prompt);
        fgets(input, sizeof(input), stdin);
        
        if (sscanf(input, "%f", &value) != 1 || value <= 0) {
            printf("请输入一个正数！\n");
            continue;
        }
        break;
    } while (1);
    
    return value;
}

int get_valid_choice(int min, int max) {
    int choice;
    char input[100];
    
    do {
        printf("请输入选项 (%d-%d): ", min, max);
        fgets(input, sizeof(input), stdin);
        
        if (sscanf(input, "%d", &choice) != 1 || choice < min || choice > max) {
            printf("无效的选项，请重新输入！\n");
            continue;
        }
        break;
    } while (1);
    
    return choice;
}

FILE* safe_fopen(const char* filename, const char* mode, int retries) {
    FILE* fp = NULL;
    int attempt = 0;
    
    while (attempt < retries) {
        fp = fopen(filename, mode);
        if (fp != NULL) break;
        
        printf("打开文件 %s 失败，重试中... (%d/%d)\n", 
               filename, attempt + 1, retries);
        
#ifdef _WIN32
        Sleep(1000);
#else
        sleep(1);
#endif
        attempt++;
    }
    
    return fp;
}

void signal_handler(int signum) {
    printf("\n捕获到信号 %d，程序即将退出...\n", signum);
    exit(signum);
}

void init_exception_handlers() {
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
#ifdef _WIN32
    signal(SIGBREAK, signal_handler);
#else
    signal(SIGQUIT, signal_handler);
#endif
}