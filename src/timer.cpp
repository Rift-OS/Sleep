#include "timer.hpp"

namespace Hardware {

// 静的メンバ変数の実体定義
volatile uint64_t Timer::tsc_freq_per_ms = 0;

void Timer::init() {
    // ACPI PM タイマーの周波数は 3,579,545 Hz
    // 1ミリ秒待つためのPMタイマーのカウント数は 約3580 カウント
    constexpr uint32_t pm_clicks_per_ms = 3580;

    // 開始時のPMタイマー値とTSC値を取得
    uint32_t start_pm = __builtin_inl(ACPI_PM_TIMER_PORT) & 0xFFFFFF;
    uint64_t start_tsc = rdtsc();

    // PMタイマーが3580カウント進むまで待機
    while (true) {
        uint32_t current_pm = __builtin_inl(ACPI_PM_TIMER_PORT) & 0xFFFFFF;
        
        // 24ビットタイマーのオーバーフロー（桁あふれ）を考慮した差分計算
        uint32_t diff = (current_pm >= start_pm) 
                        ? (current_pm - start_pm) 
                        : ((0x1000000U + current_pm) - start_pm);
        
        if (diff >= pm_clicks_per_ms) {
            break;
        }
        __asm__ volatile("pause" ::: "memory");
    }

    uint64_t end_tsc = rdtsc();

    // 1ミリ秒あたりのTSCカウント数を確定
    tsc_freq_per_ms = end_tsc - start_tsc;
}

void Timer::msleep(uint32_t ms) {
    // 初期化されていない場合は安全のため即復帰
    if (tsc_freq_per_ms == 0) {
        return;
    }

    uint64_t start_tsc = rdtsc();
    uint64_t total_ticks = static_cast<uint64_t>(ms) * tsc_freq_per_ms;

    // 指定カウントが経過するまでCPUを監視（pauseで消費電力を抑える）
    while ((rdtsc() - start_tsc) < total_ticks) {
        __asm__ volatile("pause" ::: "memory");
    }
}

} // namespace Hardware
