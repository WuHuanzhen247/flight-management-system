#ifndef FLIGHT_SYSTEM_H
#define FLIGHT_SYSTEM_H

#include "kernel_list.h"

#define MAX_STR 64
#define FUEL_FEE 50.0
#define AIRPORT_FEE 60.0
#define REFUND_FEE_RATE 0.10

typedef enum
{
    ROLE_USER = 0,
    ROLE_ADMIN = 1
} Role;

typedef enum
{
    CABIN_ECONOMY = 1,
    CABIN_BUSINESS = 2,
    CABIN_FIRST = 3
} CabinType;

typedef struct User
{
    char username[MAX_STR];
    char password[MAX_STR];
    Role role;
    char user_group[MAX_STR];
    char airline[MAX_STR];
    struct list_head node;
} User;

typedef struct Booking
{
    char username[MAX_STR];
    int cabin;
    double paid_amount;
    struct list_head node;
} Booking;

typedef struct Flight
{
    char flight_no[MAX_STR];
    char from[MAX_STR];
    char to[MAX_STR];
    char depart[MAX_STR];
    char arrive[MAX_STR];
    char airline[MAX_STR];
    int total_seats;
    int available_seats;
    double price_economy;
    double price_business;
    double price_first;
    struct list_head booking_head;
    struct list_head node;
} Flight;

extern struct list_head g_user_head;
extern struct list_head g_flight_head;

int fs_printf(const char *fmt, ...);
#define printf fs_printf

void fs_copy_text(char *dst, int dst_size, const char *src);
void fs_trim_newline(char *s);
void fs_read_line(const char *prompt, char *buf, int len);
int fs_read_int(const char *prompt);
double fs_read_double(const char *prompt);
int fs_parse_hhmm(const char *time_str);
double fs_group_discount(const char *group_name);

void fs_bootstrap(void);
void fs_cleanup(void);
void fs_add_user(const char *name, const char *pwd, Role role, const char *group_name, const char *airline);
void fs_add_flight(const char *flight_no,
                   const char *from,
                   const char *to,
                   const char *depart,
                   const char *arrive,
                   const char *airline,
                   int seats,
                   double pe,
                   double pb,
                   double pf);

User *fs_find_user_by_name(const char *username);
User *fs_login(Role role);
void fs_register_user(void);
void fs_user_menu(User *u);

Flight *fs_find_flight_exact(const char *flight_no, const char *airline_or_null);
void fs_print_flight(const Flight *f);
void fs_show_all_flights_for(const User *viewer_or_null);
void fs_search_flight_exact_menu(void);
void fs_search_flight_fuzzy_menu(void);
int fs_admin_can_manage_flight(const User *admin, const Flight *f);
void fs_admin_add_flight(User *admin);
void fs_admin_delete_flight(User *admin);
void fs_admin_update_flight(User *admin);

int fs_booked_count(const Flight *f);
Booking *fs_find_booking_by_user(Flight *f, const char *username);
void fs_show_user_bookings(User *u);
void fs_book_flight(User *u);
void fs_cancel_booking(User *u);
void fs_free_booking_list(Flight *f);
void fs_drop_latest_booking(Flight *f);
int fs_remove_user_bookings(const char *username);

void fs_admin_user_manage_menu(void);
void fs_admin_menu(User *admin);

#endif
