#ifndef TIMER_HPP
#define TIMER_HPP

#include <cstdint>

namespace Hardware {

/**
 * @brief 高精度スリープ用タイマー（静的クラス）
 *
 * 想定環境:
 * - x86/x86_64 のブートローダまたはカーネル初期化フェーズでの利用を想定
 * - inl (I/O ポート読み出し)、rdtsc、cpuid、pause 命令が利用可能であること
 *
 * 注意:
 * - Timer はインスタンス化を禁止（static メンバのみ）
 * - init() を起動時に一度呼んでから msleep() を使用すること
 * - msleep() はスピン待ち方式のため長時間待機やスケジューラ有り環境では
 *   別のブロッキング手段を推奨
 */
class Timer {
public:
    // インスタンス化を禁止
    Timer() = delete;

    /**
     * @brief 初期化: ACPI PM タイマーを基準にして TSC の 1ms あたりカウントを測定する
     *
     * 実装ノート:
     * - ACPI PM タイマーは 24-bit（ラップ処理が必要）
     * - PM タイマーの理論周波数は約 3,579,545 Hz（1ms ≒ 3579.545 カウント）
     * - 実装側では丸めて 3580 カウントを 1ms として測定する
     */
    static void init();

    /**
     * @brief 指定ミリ秒だけスリープ（スピン待ち）
     * @param ms スリープするミリ秒数
     *
     * 実装ノート:
     * - 高精度だが CPU を消費する（短時間待ち向け）
     * - tsc_freq_per_ms が 0 の場合は即座に戻る（未初期化保護）
     */
    static void msleep(uint32_t ms);

private:
    // ACPI PM タイマーの I/O ポート（環境に応じて変更してください）
    // 例: FADT に従う場合は実行時に読み替える必要があります。
    static constexpr uint16_t ACPI_PM_TIMER_PORT = 0x0608;

    // 1ミリ秒あたりの TSC カウント（init() で設定）
    // volatile にしているが、マルチスレッド/割り込み競合に対しては
    // atomic に変更することを検討してください。
    static volatile uint64_t tsc_freq_per_ms;

    /**
     * @brief rdtsc を使って TSC を読み取る（簡易版、シリアライズなし）
     *
     * 注意:
     * - 精度を高めるためには CPUID を使ったシリアライズ版を
     *   cpp 側で用意しています（rdtsc_serialized）。
     * - ヘッダ内での簡易読み出しを必要とする場合に備え残してありますが、
     *   測定時はシリアライズ版を使用してください。
     */
    static inline uint64_t rdtsc() {
        uint32_t low, high;
        __asm__ volatile("rdtsc" : "=a"(low), "=d"(high));
        return (static_cast<uint64_t>(high) << 32) | low;
    }
};

} // namespace Hardware

#endif // TIMER_HPP
