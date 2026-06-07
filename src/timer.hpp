#ifndef TIMER_HPP
#define TIMER_HPP

#include <cstdint>

namespace Hardware {

class Timer {
public:
    // 削除されたコンストラクタ（静的クラスとしてのみ使用させ、インスタンス化を防ぐ）
    Timer() = delete;

    /**
     * @brief スリープタイマーの初期化（1ミリ秒あたりのTSC周波数を計測）
     */
    static void init();

    /**
     * @brief 指定したミリ秒数だけスリープする
     * @param ms スリープするミリ秒数
     */
    static void msleep(uint32_t ms);

private:
    // ACPI PMタイマーのI/Oポート番号 (環境やFADTの値に合わせて変更)
    static constexpr uint16_t ACPI_PM_TIMER_PORT = 0x0608;

    // 1ミリ秒あたりにCPUのTSCが進むカウント数
    static volatile uint64_t tsc_freq_per_ms;

    /**
     * @brief CPUのタイムスタンプカウンタ（TSC）の現在値を取得
     */
    static inline uint64_t rdtsc() {
        uint32_t low, high;
        __asm__ volatile("rdtsc" : "=a" (low), "=d" (high));
        return (static_cast<uint64_t>(high) << 32) | low;
    }
};

} // namespace Hardware

#endif // TIMER_HPPs
