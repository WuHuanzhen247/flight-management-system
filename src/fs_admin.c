#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "flight_system.h"

static void fs_admin_user_manage_add(void)
{
    char name[MAX_STR];
    char pwd[MAX_STR];
    char group_name[MAX_STR];
    char airline[MAX_STR];
    int role;

    fs_read_line("用户名: ", name, MAX_STR);
    if (fs_find_user_by_name(name))
    {
        printf("用户名已存在。\n");
        return;
    }

    fs_read_line("密码: ", pwd, MAX_STR);
    role = fs_read_int("角色: 0用户 1管理员\n选择: ");
    fs_read_line("用户组(normal/student/soldier/elder): ", group_name, MAX_STR);
    if (!group_name[0])
    {
        fs_copy_text(group_name, MAX_STR, "normal");
    }

    if (role == ROLE_ADMIN)
    {
        fs_read_line("管理员所属航司(或ALL): ", airline, MAX_STR);
        if (!airline[0])
        {
            printf("管理员必须指定航司。\n");
            return;
        }
        fs_add_user(name, pwd, ROLE_ADMIN, group_name, airline);
    }
    else
    {
        fs_add_user(name, pwd, ROLE_USER, group_name, "");
    }

    printf("添加用户成功。\n");
}

static void fs_admin_user_manage_delete(void)
{
    char name[MAX_STR];
    struct list_head *pos = NULL;
    struct list_head *n = NULL;

    fs_read_line("要删除的用户名: ", name, MAX_STR);

    list_for_each_safe(pos, n, &g_user_head)
    {
        User *u = list_entry(pos, User, node);
        if (strcmp(u->username, name) == 0)
        {
            int removed_bookings = fs_remove_user_bookings(u->username);
            list_del(&u->node);
            free(u);
            printf("删除成功，同时清理预订记录 %d 条。\n", removed_bookings);
            return;
        }
    }

    printf("未找到该用户。\n");
}

static void fs_admin_user_manage_update(void)
{
    char name[MAX_STR];
    char buf[MAX_STR];

    fs_read_line("要更新的用户名: ", name, MAX_STR);
    User *u = fs_find_user_by_name(name);
    if (!u)
    {
        printf("未找到该用户。\n");
        return;
    }

    fs_read_line("新密码(回车不改): ", buf, MAX_STR);
    if (buf[0])
        fs_copy_text(u->password, MAX_STR, buf);

    fs_read_line("新用户组(回车不改): ", buf, MAX_STR);
    if (buf[0])
        fs_copy_text(u->user_group, MAX_STR, buf);

    if (u->role == ROLE_ADMIN)
    {
        fs_read_line("新航司(回车不改): ", buf, MAX_STR);
        if (buf[0])
            fs_copy_text(u->airline, MAX_STR, buf);
    }

    printf("更新成功。\n");
}

static void fs_admin_user_manage_list(void)
{
    struct list_head *pos = NULL;
    printf("用户列表:\n");

    list_for_each(pos, &g_user_head)
    {
        User *u = list_entry(pos, User, node);
        printf("用户名:%s 角色:%s 用户组:%s", u->username,
               u->role == ROLE_ADMIN ? "管理员" : "用户", u->user_group);
        if (u->role == ROLE_ADMIN)
        {
            printf(" 航司:%s", u->airline);
        }
        printf("\n");
    }
}

void fs_admin_user_manage_menu(void)
{
    while (1)
    {
        int c = fs_read_int("\n[用户管理] 1.添加 2.删除 3.更新 4.查看 0.返回\n选择: ");
        if (c == 1)
        {
            fs_admin_user_manage_add();
        }
        else if (c == 2)
        {
            fs_admin_user_manage_delete();
        }
        else if (c == 3)
        {
            fs_admin_user_manage_update();
        }
        else if (c == 4)
        {
            fs_admin_user_manage_list();
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

void fs_admin_menu(User *admin)
{
    while (1)
    {
        int c = fs_read_int("\n[管理员菜单] 1.航班录入 2.航班删除 3.航班更新 4.航班显示 5.用户管理 0.退出登录\n选择: ");
        if (c == 1)
        {
            fs_admin_add_flight(admin);
        }
        else if (c == 2)
        {
            fs_admin_delete_flight(admin);
        }
        else if (c == 3)
        {
            fs_admin_update_flight(admin);
        }
        else if (c == 4)
        {
            fs_show_all_flights_for(admin);
        }
        else if (c == 5)
        {
            fs_admin_user_manage_menu();
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
