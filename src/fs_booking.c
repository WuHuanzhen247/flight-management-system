#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "flight_system.h"

int fs_booked_count(const Flight *f)
{
    int count = 0;
    struct list_head *pos = NULL;
    list_for_each(pos, (struct list_head *)&f->booking_head)
    {
        count++;
    }
    return count;
}

Booking *fs_find_booking_by_user(Flight *f, const char *username)
{
    struct list_head *pos = NULL;
    list_for_each(pos, &f->booking_head)
    {
        Booking *b = list_entry(pos, Booking, node);
        if (strcmp(b->username, username) == 0)
        {
            return b;
        }
    }
    return NULL;
}

void fs_show_user_bookings(User *u)
{
    struct list_head *fp = NULL;
    int found = 0;

    list_for_each(fp, &g_flight_head)
    {
        Flight *f = list_entry(fp, Flight, node);
        Booking *b = fs_find_booking_by_user(f, u->username);
        if (b)
        {
            printf("航班:%s 航司:%s 舱位:%d 实付:%.2f\n", f->flight_no, f->airline, b->cabin, b->paid_amount);
            found = 1;
        }
    }

    if (!found)
    {
        printf("暂无预订记录。\n");
    }
}

static double fs_base_price_by_cabin(const Flight *f, int cabin)
{
    if (cabin == CABIN_BUSINESS)
        return f->price_business;
    if (cabin == CABIN_FIRST)
        return f->price_first;
    return f->price_economy;
}

void fs_book_flight(User *u)
{
    char flight_no[MAX_STR];
    char airline[MAX_STR];

    fs_read_line("输入要预订的航班号: ", flight_no, MAX_STR);
    fs_read_line("输入航司(联合航班请指定航司): ", airline, MAX_STR);

    Flight *f = fs_find_flight_exact(flight_no, airline[0] ? airline : NULL);
    if (!f)
    {
        printf("未找到对应航班。\n");
        return;
    }

    if (f->available_seats <= 0)
    {
        printf("预订失败: 该航班已满座。\n");
        return;
    }

    if (fs_find_booking_by_user(f, u->username))
    {
        printf("你已预订该航班。\n");
        return;
    }

    int cabin = fs_read_int("舱位: 1.经济舱 2.商务舱 3.头等舱\n选择: ");
    if (cabin < CABIN_ECONOMY || cabin > CABIN_FIRST)
    {
        printf("舱位选择无效。\n");
        return;
    }

    double base = fs_base_price_by_cabin(f, cabin);
    double discount = fs_group_discount(u->user_group);
    double fare = base * discount;
    double total = fare + FUEL_FEE + AIRPORT_FEE;

    Booking *b = (Booking *)malloc(sizeof(Booking));
    if (!b)
    {
        printf("内存不足，预订失败。\n");
        return;
    }

    memset(b, 0, sizeof(Booking));
    fs_copy_text(b->username, MAX_STR, u->username);
    b->cabin = cabin;
    b->paid_amount = total;

    INIT_LIST_HEAD(&b->node);
    list_add_tail(&b->node, &f->booking_head);
    f->available_seats--;

    printf("预订成功。票面价:%.2f 折扣后:%.2f 燃油费:%.2f 机建费:%.2f 实付:%.2f\n",
           base, fare, FUEL_FEE, AIRPORT_FEE, total);
}

void fs_cancel_booking(User *u)
{
    char flight_no[MAX_STR];
    char airline[MAX_STR];

    fs_show_user_bookings(u);
    fs_read_line("输入要取消的航班号: ", flight_no, MAX_STR);
    fs_read_line("输入航司: ", airline, MAX_STR);

    Flight *f = fs_find_flight_exact(flight_no, airline[0] ? airline : NULL);
    if (!f)
    {
        printf("未找到该航班。\n");
        return;
    }

    Booking *b = fs_find_booking_by_user(f, u->username);
    if (!b)
    {
        printf("你没有该航班预订记录。\n");
        return;
    }

    list_del(&b->node);
    f->available_seats++;

    {
        double refundable_part = b->paid_amount - FUEL_FEE - AIRPORT_FEE;
        double refund;
        if (refundable_part < 0)
        {
            refundable_part = 0;
        }
        refund = refundable_part * (1.0 - REFUND_FEE_RATE);
        printf("退票成功。退还金额:%.2f (燃油/机建费不退，手续费%.0f%%)\n", refund, REFUND_FEE_RATE * 100.0);
    }

    free(b);
}

void fs_free_booking_list(Flight *f)
{
    struct list_head *pos = NULL;
    struct list_head *n = NULL;

    list_for_each_safe(pos, n, &f->booking_head)
    {
        Booking *b = list_entry(pos, Booking, node);
        list_del(&b->node);
        free(b);
    }
}

void fs_drop_latest_booking(Flight *f)
{
    if (list_empty(&f->booking_head))
    {
        return;
    }

    struct list_head *tail = f->booking_head.prev;
    Booking *b = list_entry(tail, Booking, node);
    printf("因缩座移除最新购票用户: %s\n", b->username);
    list_del(&b->node);
    free(b);
}

int fs_remove_user_bookings(const char *username)
{
    struct list_head *fp = NULL;
    int removed = 0;

    list_for_each(fp, &g_flight_head)
    {
        Flight *f = list_entry(fp, Flight, node);
        struct list_head *bp = NULL;
        struct list_head *bn = NULL;

        list_for_each_safe(bp, bn, &f->booking_head)
        {
            Booking *b = list_entry(bp, Booking, node);
            if (strcmp(b->username, username) != 0)
            {
                continue;
            }

            list_del(&b->node);
            free(b);
            if (f->available_seats < f->total_seats)
            {
                f->available_seats++;
            }
            removed++;
        }
    }

    return removed;
}
