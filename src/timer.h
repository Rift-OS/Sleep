#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

// ACPI PMタイマーのI/Oポート番号 (環境やFADTの値に合わせて変更)
#define ACPI_PM_TIMER_PORT 0x0608 

// 関数のプロトタイプ宣言
void init_sleep_timer(void);
void msleep(uint32_t ms);

#endif
