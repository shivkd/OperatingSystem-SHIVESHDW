#ifndef INCLUDE_USERMODE_H
#define INCLUDE_USERMODE_H
void enter_user_mode(void (*entry)(void)) __attribute__((noreturn));
void user_entry(void) __attribute__((noreturn));
#endif
