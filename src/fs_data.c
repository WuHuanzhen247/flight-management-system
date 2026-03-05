#include <stdlib.h>
#include <string.h>

#include "flight_system.h"

struct list_head g_user_head;
struct list_head g_flight_head;

void fs_add_user(const char *name, const char *pwd, Role role, const char *group_name, const char *airline)
{
    User *u = (User *)malloc(sizeof(User));
    if (!u)
    {
        return;
    }

    memset(u, 0, sizeof(User));
    u->role = role;
    fs_copy_text(u->username, MAX_STR, name);
    fs_copy_text(u->password, MAX_STR, pwd);
    fs_copy_text(u->user_group, MAX_STR, group_name ? group_name : "normal");
    fs_copy_text(u->airline, MAX_STR, airline ? airline : "");

    INIT_LIST_HEAD(&u->node);
    list_add_tail(&u->node, &g_user_head);
}

void fs_add_flight(const char *flight_no,
                   const char *from,
                   const char *to,
                   const char *depart,
                   const char *arrive,
                   const char *airline,
                   int seats,
                   double pe,
                   double pb,
                   double pf)
{
    Flight *f = (Flight *)malloc(sizeof(Flight));
    if (!f)
    {
        return;
    }

    memset(f, 0, sizeof(Flight));
    fs_copy_text(f->flight_no, MAX_STR, flight_no);
    fs_copy_text(f->from, MAX_STR, from);
    fs_copy_text(f->to, MAX_STR, to);
    fs_copy_text(f->depart, MAX_STR, depart);
    fs_copy_text(f->arrive, MAX_STR, arrive);
    fs_copy_text(f->airline, MAX_STR, airline);

    f->total_seats = seats;
    f->available_seats = seats;
    f->price_economy = pe;
    f->price_business = pb;
    f->price_first = pf;

    INIT_LIST_HEAD(&f->booking_head);
    INIT_LIST_HEAD(&f->node);
    list_add_tail(&f->node, &g_flight_head);
}

void fs_bootstrap(void)
{
    INIT_LIST_HEAD(&g_user_head);
    INIT_LIST_HEAD(&g_flight_head);

    fs_add_user("admin_all", "123456", ROLE_ADMIN, "normal", "ALL");
    fs_add_user("admin_cn", "123456", ROLE_ADMIN, "normal", "ChinaAir");
    fs_add_user("alice", "123456", ROLE_USER, "student", "");

    fs_add_flight("CA1001", "广州", "北京", "09:00", "12:00", "ChinaAir", 20, 800, 1600, 2600);
    fs_add_flight("MU2002", "上海", "深圳", "14:30", "17:10", "EasternAir", 18, 680, 1300, 2100);
    fs_add_flight("CA1001", "广州", "北京", "09:00", "12:00", "PartnerAir", 10, 760, 1500, 2450);
}

void fs_cleanup(void)
{
    struct list_head *pos = NULL;
    struct list_head *n = NULL;

    list_for_each_safe(pos, n, &g_flight_head)
    {
        Flight *f = list_entry(pos, Flight, node);
        fs_free_booking_list(f);
        list_del(&f->node);
        free(f);
    }

    list_for_each_safe(pos, n, &g_user_head)
    {
        User *u = list_entry(pos, User, node);
        list_del(&u->node);
        free(u);
    }
}
