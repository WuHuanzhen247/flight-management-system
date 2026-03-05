#include <stdio.h>
#include <string.h>

#include "flight_system.h"

User *fs_find_user_by_name(const char *username)
{
    struct list_head *pos = NULL;
    list_for_each(pos, &g_user_head)
    {
        User *u = list_entry(pos, User, node);
        if (strcmp(u->username, username) == 0)
        {
            return u;
        }
    }
    return NULL;
}

void fs_register_user(void)
{
    char username[MAX_STR];
    char password[MAX_STR];
    char group_name[MAX_STR];

    fs_read_line("注册用户名: ", username, MAX_STR);
    if (fs_find_user_by_name(username))
    {
        printf("用户名已存在。\n");
        return;
    }

    fs_read_line("注册密码: ", password, MAX_STR);
    fs_read_line("用户类型(normal/student/soldier/elder): ", group_name, MAX_STR);
    if (group_name[0] == '\0')
    {
        fs_copy_text(group_name, MAX_STR, "normal");
    }

    fs_add_user(username, password, ROLE_USER, group_name, "");
    printf("注册成功。\n");
}

User *fs_login(Role role)
{
    char username[MAX_STR];
    char password[MAX_STR];

    fs_read_line("用户名: ", username, MAX_STR);
    fs_read_line("密码: ", password, MAX_STR);

    User *u = fs_find_user_by_name(username);
    if (!u || u->role != role || strcmp(u->password, password) != 0)
    {
        printf("登录失败。\n");
        return NULL;
    }

    printf("登录成功，欢迎 %s。\n", u->username);
    return u;
}

void fs_user_menu(User *u)
{
    while (1)
    {
        int c = fs_read_int("\n[用户菜单] 1.航班查询 2.模糊查询 3.航班预订 4.我的预订 5.取消预订 0.退出登录\n选择: ");
        if (c == 1)
        {
            fs_search_flight_exact_menu();
        }
        else if (c == 2)
        {
            fs_search_flight_fuzzy_menu();
        }
        else if (c == 3)
        {
            fs_book_flight(u);
        }
        else if (c == 4)
        {
            fs_show_user_bookings(u);
        }
        else if (c == 5)
        {
            fs_cancel_booking(u);
        }
        else if (c == 0)
        {
            break;
        }
        else
        {
            printf("无效选择。\n");
        }
    }
}
