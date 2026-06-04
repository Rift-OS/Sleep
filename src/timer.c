#include "timer.h"

// 1ミリ秒あたりにCPUのTSCが進むカウント数（このファイル内のみで保持）
static uint64_t tsc_freq_per_ms = 0;

/**
 * CPUのタイムスタンプカウンタ（TSC）の現在値を取得
 */
static inline uint64_t rdtsc(void) {
    uint32_t low, high;
    __asm__ volatile("rdtsc" : "=a"(low), "=d"(high));
    return ((uint64_t)high << 32) | low;
}

/**
 * スリープタイマーの初期化（1ミリ秒あたりのTSC周波数を計測）
 */
void init_sleep_timer(void) {
    // ACPI PM タイマーの周波数は 3,579,545 Hz
    // 1ミリ秒待つためのPMタイマーのカウント数は 約3580 カウント
    const uint32_t pm_clicks_per_ms = 3580;
    
    // 開始時のPMタイマー値とTSC値を取得
    uint32_t start_pm = __builtin_inl(ACPI_PM_TIMER_PORT) & 0xFFFFFF;
    uint64_t start_tsc = rdtsc();
    
    // PMタイマーが3580カウント進むまで待機
    while (1) {
        uint32_t current_pm = __builtin_inl(ACPI_PM_TIMER_PORT) & 0xFFFFFF;
        
        // 24ビットタイマーのオーバーフロー（桁あふれ）を考慮した差分計算
        uint32_t diff = (current_pm >= start_pm) ? 
                        (current_pm - start_pm) : 
                        (0x1000000 + current_pm - start_pm);
        
        if (diff >= pm_clicks_per_ms) {
            break;
        }
    }
    
    uint64_t end_tsc = rdtsc();
    
    // 1ミリ秒あたりのTSCカウント数を確定
    tsc_freq_per_ms = end_tsc - start_tsc;
}

/**
 * 指定したミリ秒数だけスリープする
 */
void msleep(uint32_t ms) {
    // 初期化されていない場合は安全のため即復帰
    if (tsc_freq_per_ms == 0) {
        return;
    }

    uint64_t start_tsc = rdtsc();
    uint64_t total_ticks = (uint64_t)ms * tsc_freq_per_ms;

    // 指定カウントが経過するまでCPUを監視（pauseで消費電力を抑える）
    while ((rdtsc() - start_tsc) < total_ticks) {
        __asm__ volatile("pause"); 
    }
}
#include "timer.h"

// 1ミリ秒あたりにCPUのTSCが進むカウント数（このファイル内のみで保持）
static uint64_t tsc_freq_per_ms = 0;

/**
 * CPUのタイムスタンプカウンタ（TSC）の現在値を取得
 */
static inline uint64_t rdtsc(void) {
    uint32_t low, high;
    __asm__ volatile("rdtsc" : "=a"(low), "=d"(high));
    return ((uint64_t)high << 32) | low;
}

/**
 * スリープタイマーの初期化（1ミリ秒あたりのTSC周波数を計測）
 */
void init_sleep_timer(void) {
    // ACPI PM タイマーの周波数は 3,579,545 Hz
    // 1ミリ秒待つためのPMタイマーのカウント数は 約3580 カウント
    const uint32_t pm_clicks_per_ms = 3580;
    
    // 開始時のPMタイマー値とTSC値を取得
    uint32_t start_pm = __builtin_inl(ACPI_PM_TIMER_PORT) & 0xFFFFFF;
    uint64_t start_tsc = rdtsc();
    
    // PMタイマーが3580カウント進むまで待機
    while (1) {
        uint32_t current_pm = __builtin_inl(ACPI_PM_TIMER_PORT) & 0xFFFFFF;
        
        // 24ビットタイマーのオーバーフロー（桁あふれ）を考慮した差分計算
        uint32_t diff = (current_pm >= start_pm) ? 
                        (current_pm - start_pm) : 
                        (0x1000000 + current_pm - start_pm);
        
        if (diff >= pm_clicks_per_ms) {
            break;
        }
    }
    
    uint64_t end_tsc = rdtsc();
    
    // 1ミリ秒あたりのTSCカウント数を確定
    tsc_freq_per_ms = end_tsc - start_tsc;
}

/**
 * 指定したミリ秒数だけスリープする
 */
void msleep(uint32_t ms) {
    // 初期化されていない場合は安全のため即復帰
    if (tsc_freq_per_ms == 0) {
        return;
    }

    uint64_t start_tsc = rdtsc();
    uint64_t total_ticks = (uint64_t)ms * tsc_freq_per_ms;

    // 指定カウントが経過するまでCPUを監視（pauseで消費電力を抑える）
    while ((rdtsc() - start_tsc) < total_ticks) {
        __asm__ volatile("pause"); 
    }
}
