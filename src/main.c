#include <stdio.h>
#include <locale.h>

#include "flight_system.h"

static void fs_init_console_encoding(void)
{
    setlocale(LC_ALL, "");
}

int main(void)
{
    fs_init_console_encoding();
    fs_bootstrap();

    printf("==== 航班管理系统(结构体 + 内核链表 + 模块化) ====\n");

    while (1)
    {
        int c = fs_read_int("\n1.用户注册 2.用户登录 3.管理员登录 4.显示全部航班 0.退出系统\n选择: ");
        if (c == 1)
        {
            fs_register_user();
        }
        else if (c == 2)
        {
            User *u = fs_login(ROLE_USER);
            if (u)
            {
                fs_user_menu(u);
            }
        }
        else if (c == 3)
        {
            User *a = fs_login(ROLE_ADMIN);
            if (a)
            {
                fs_admin_menu(a);
            }
        }
        else if (c == 4)
        {
            fs_show_all_flights_for(NULL);
        }
        else if (c == 0)
        {
            printf("系统退出。\n");
            break;
        }
        else
        {
            printf("无效选择。\n");
        }
    }

    fs_cleanup();
    return 0;
}
