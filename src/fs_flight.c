#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "flight_system.h"

static int fs_is_super_admin(const User *u)
{
    return (u->role == ROLE_ADMIN && strcmp(u->airline, "ALL") == 0);
}

Flight *fs_find_flight_exact(const char *flight_no, const char *airline_or_null)
{
    struct list_head *pos = NULL;
    list_for_each(pos, &g_flight_head)
    {
        Flight *f = list_entry(pos, Flight, node);
        if (strcmp(f->flight_no, flight_no) == 0)
        {
            if (airline_or_null == NULL || airline_or_null[0] == '\0' || strcmp(f->airline, airline_or_null) == 0)
            {
                return f;
            }
        }
    }
    return NULL;
}

void fs_print_flight(const Flight *f)
{
    printf("航班号:%s 航司:%s %s->%s 出发:%s 到达:%s 座位:%d/%d 票价[经/商/头]=%.2f/%.2f/%.2f\n",
           f->flight_no, f->airline, f->from, f->to, f->depart, f->arrive,
           f->available_seats, f->total_seats,
           f->price_economy, f->price_business, f->price_first);
}

int fs_admin_can_manage_flight(const User *admin, const Flight *f)
{
    if (fs_is_super_admin(admin))
    {
        return 1;
    }
    return strcmp(admin->airline, f->airline) == 0;
}

void fs_show_all_flights_for(const User *viewer_or_null)
{
    struct list_head *pos = NULL;
    int found = 0;

    list_for_each(pos, &g_flight_head)
    {
        Flight *f = list_entry(pos, Flight, node);
        if (viewer_or_null && viewer_or_null->role == ROLE_ADMIN && !fs_admin_can_manage_flight(viewer_or_null, f))
        {
            continue;
        }
        fs_print_flight(f);
        found = 1;
    }

    if (!found)
    {
        printf("没有符合条件的航班。\n");
    }
}

void fs_search_flight_exact_menu(void)
{
    int cond = fs_read_int("查询方式: 1.航班号 2.出发地+目的地 3.出发时间\n选择: ");
    struct list_head *pos = NULL;
    int found = 0;

    if (cond == 1)
    {
        char flight_no[MAX_STR];
        fs_read_line("输入航班号: ", flight_no, MAX_STR);
        list_for_each(pos, &g_flight_head)
        {
            Flight *f = list_entry(pos, Flight, node);
            if (strcmp(f->flight_no, flight_no) == 0)
            {
                fs_print_flight(f);
                found = 1;
            }
        }
    }
    else if (cond == 2)
    {
        char from[MAX_STR];
        char to[MAX_STR];
        fs_read_line("输入出发地: ", from, MAX_STR);
        fs_read_line("输入目的地: ", to, MAX_STR);
        list_for_each(pos, &g_flight_head)
        {
            Flight *f = list_entry(pos, Flight, node);
            if (strcmp(f->from, from) == 0 && strcmp(f->to, to) == 0)
            {
                fs_print_flight(f);
                found = 1;
            }
        }
    }
    else if (cond == 3)
    {
        char depart[MAX_STR];
        fs_read_line("输入出发时间(HH:MM): ", depart, MAX_STR);
        list_for_each(pos, &g_flight_head)
        {
            Flight *f = list_entry(pos, Flight, node);
            if (strcmp(f->depart, depart) == 0)
            {
                fs_print_flight(f);
                found = 1;
            }
        }
    }
    else
    {
        printf("无效选择。\n");
        return;
    }

    if (!found)
    {
        printf("未找到匹配航班。\n");
    }
}

void fs_search_flight_fuzzy_menu(void)
{
    int cond = fs_read_int("模糊查询: 1.价格段 2.起飞时间段\n选择: ");
    struct list_head *pos = NULL;
    int found = 0;

    if (cond == 1)
    {
        double p = fs_read_double("输入目标价格: ");
        double low = p - 200.0;
        double high = p + 200.0;
        printf("价格段: %.2f ~ %.2f\n", low, high);

        list_for_each(pos, &g_flight_head)
        {
            Flight *f = list_entry(pos, Flight, node);
            if (f->price_economy >= low && f->price_economy <= high)
            {
                fs_print_flight(f);
                found = 1;
            }
        }
    }
    else if (cond == 2)
    {
        char t[MAX_STR];
        int center;
        int low;
        int high;
        fs_read_line("输入起飞时间(HH:MM): ", t, MAX_STR);
        center = fs_parse_hhmm(t);
        if (center < 0)
        {
            printf("时间格式错误。\n");
            return;
        }

        low = center - 30;
        high = center + 30;

        list_for_each(pos, &g_flight_head)
        {
            Flight *f = list_entry(pos, Flight, node);
            int ft = fs_parse_hhmm(f->depart);
            if (ft >= 0 && ft >= low && ft <= high)
            {
                fs_print_flight(f);
                found = 1;
            }
        }
    }
    else
    {
        printf("无效选择。\n");
        return;
    }

    if (!found)
    {
        printf("未找到匹配航班。\n");
    }
}

void fs_admin_add_flight(User *admin)
{
    char flight_no[MAX_STR];
    char from[MAX_STR];
    char to[MAX_STR];
    char depart[MAX_STR];
    char arrive[MAX_STR];
    char airline[MAX_STR];
    int seats;
    double pe;
    double pb;
    double pf;

    fs_read_line("航班号: ", flight_no, MAX_STR);
    fs_read_line("出发地: ", from, MAX_STR);
    fs_read_line("目的地: ", to, MAX_STR);
    fs_read_line("出发时间(HH:MM): ", depart, MAX_STR);
    fs_read_line("到达时间(HH:MM): ", arrive, MAX_STR);
    seats = fs_read_int("座位数: ");
    pe = fs_read_double("经济舱价格: ");
    pb = fs_read_double("商务舱价格: ");
    pf = fs_read_double("头等舱价格: ");

    if (seats <= 0)
    {
        printf("座位数必须大于0。\n");
        return;
    }

    if (fs_is_super_admin(admin))
    {
        fs_read_line("航司: ", airline, MAX_STR);
        if (airline[0] == '\0')
        {
            printf("航司不能为空。\n");
            return;
        }
        fs_add_flight(flight_no, from, to, depart, arrive, airline, seats, pe, pb, pf);
    }
    else
    {
        fs_add_flight(flight_no, from, to, depart, arrive, admin->airline, seats, pe, pb, pf);
    }

    printf("录入成功。\n");
}

void fs_admin_delete_flight(User *admin)
{
    char flight_no[MAX_STR];
    char airline[MAX_STR];
    struct list_head *pos = NULL;
    struct list_head *n = NULL;

    fs_read_line("输入要删除的航班号: ", flight_no, MAX_STR);
    fs_read_line("输入航司(超管可选): ", airline, MAX_STR);

    list_for_each_safe(pos, n, &g_flight_head)
    {
        Flight *f = list_entry(pos, Flight, node);
        if (strcmp(f->flight_no, flight_no) != 0)
            continue;
        if (airline[0] != '\0' && strcmp(f->airline, airline) != 0)
            continue;
        if (!fs_admin_can_manage_flight(admin, f))
            continue;

        fs_free_booking_list(f);
        list_del(&f->node);
        free(f);
        printf("删除成功。\n");
        return;
    }

    printf("未找到可删除航班。\n");
}

void fs_admin_update_flight(User *admin)
{
    char flight_no[MAX_STR];
    char airline[MAX_STR];
    char buf[MAX_STR];

    fs_read_line("输入要更新的航班号: ", flight_no, MAX_STR);
    fs_read_line("输入航司: ", airline, MAX_STR);

    Flight *f = fs_find_flight_exact(flight_no, airline[0] ? airline : NULL);
    if (!f || !fs_admin_can_manage_flight(admin, f))
    {
        printf("未找到可更新航班。\n");
        return;
    }

    fs_read_line("新出发地(回车不改): ", buf, MAX_STR);
    if (buf[0])
        fs_copy_text(f->from, MAX_STR, buf);

    fs_read_line("新航班号(回车不改): ", buf, MAX_STR);
    if (buf[0])
        fs_copy_text(f->flight_no, MAX_STR, buf);

    fs_read_line("新目的地(回车不改): ", buf, MAX_STR);
    if (buf[0])
        fs_copy_text(f->to, MAX_STR, buf);

    fs_read_line("新出发时间(回车不改): ", buf, MAX_STR);
    if (buf[0])
        fs_copy_text(f->depart, MAX_STR, buf);

    fs_read_line("新到达时间(回车不改): ", buf, MAX_STR);
    if (buf[0])
        fs_copy_text(f->arrive, MAX_STR, buf);

    fs_read_line("新总座位(回车不改): ", buf, MAX_STR);
    if (buf[0])
    {
        int new_total = atoi(buf);
        if (new_total > 0)
        {
            if (new_total < fs_booked_count(f))
            {
                while (fs_booked_count(f) > new_total)
                {
                    fs_drop_latest_booking(f);
                }
            }
            f->total_seats = new_total;
            f->available_seats = new_total - fs_booked_count(f);
        }
    }

    fs_read_line("新经济舱价(回车不改): ", buf, MAX_STR);
    if (buf[0])
        f->price_economy = atof(buf);

    fs_read_line("新商务舱价(回车不改): ", buf, MAX_STR);
    if (buf[0])
        f->price_business = atof(buf);

    fs_read_line("新头等舱价(回车不改): ", buf, MAX_STR);
    if (buf[0])
        f->price_first = atof(buf);

    printf("更新完成。\n");
}
