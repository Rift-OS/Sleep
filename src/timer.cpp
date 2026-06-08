// timer.cpp
#include "timer.hpp"
#include <cstdint>

// ACPI PM タイマーの I/O ポート定義（必要なら timer.hpp 側と合わせてください）
#ifndef ACPI_PM_TIMER_PORT
#define ACPI_PM_TIMER_PORT 0x408U
#endif

namespace Hardware {

// static メンバ実体
volatile uint64_t Timer::tsc_freq_per_ms = 0;

// ----- inl: 32-bit I/O ポート読み出し（環境依存） -----
// 低レベル環境（ブートローダ・カーネル初期化）で inl 命令を用いる実装。
// ユーザ空間や別環境では別実装に差し替えてください。
inline uint32_t Timer::inl(uint16_t port) {
    uint32_t val;
    asm volatile ("inl %1, %0" : "=a"(val) : "Nd"(port));
    return val;
}

// ----- rdtsc_serialized: CPUID でシリアライズしてから RDTSC を実行 -----
// CPUID によるシリアライズで命令再順序を防ぎ、TSC 測定の安定性を高めます。
// CPUID はいくつかのレジスタを変更するため clobber を適切に宣言しています。
inline uint64_t Timer::rdtsc_serialized() {
    uint32_t hi, lo;
    asm volatile (
        "cpuid\n\t"          // EAX=0 を与えて実行（入力は a レジスタで指定）
        "rdtsc\n\t"          // TSC を読み出す
        : "=d"(hi), "=a"(lo) // 出力: EDX:EAX に TSC
        : "a"(0)             // 入力: EAX=0
        : "rbx", "rcx", "memory"
    );
    return (static_cast<uint64_t>(hi) << 32) | lo;
}

// ----- init(): ACPI PM タイマーで TSC 周波数（/ms）を測定 -----
// - ACPI PM タイマーは 24-bit（ラップに注意）
// - pm_clicks_per_ms は 3580（3,579,545 Hz を 1ms 単位に丸め）
// - 測定はブート時に一度行うことを想定
void Timer::init() {
    constexpr uint32_t pm_clicks_per_ms = 3580;
    constexpr uint32_t pm_mask = 0xFFFFFF; // 24-bit マスク
    constexpr uint32_t pm_range = pm_mask + 1U;

    // 開始値取得（24-bit にマスク）
    uint32_t start_pm = inl(static_cast<uint16_t>(ACPI_PM_TIMER_PORT)) & pm_mask;
    uint64_t start_tsc = rdtsc_serialized();

    // PMタイマーが所定分だけ進むまで待つ（ラップを考慮）
    while (true) {
        uint32_t current_pm = inl(static_cast<uint16_t>(ACPI_PM_TIMER_PORT)) & pm_mask;

        // 差分計算（ラップ対応）
        uint32_t diff = (current_pm >= start_pm)
                        ? (current_pm - start_pm)
                        : (pm_range - start_pm + current_pm);

        if (diff >= pm_clicks_per_ms) {
            break;
        }
        // PAUSE 命令でループの CPU 使用を若干軽減（ハイパスレッド向け）
        asm volatile("pause" ::: "memory");
    }

    uint64_t end_tsc = rdtsc_serialized();

    // 測定結果を設定（1ms あたりの TSC カウント）
    // end_tsc >= start_tsc のはずだが、念のため saturate（下限 1）
    uint64_t measured = (end_tsc > start_tsc) ? (end_tsc - start_tsc) : 1ULL;
    tsc_freq_per_ms = measured;
}

// ----- msleep(ms): スピン待ち方式で ms 単位の待機 -----
// - tsc_freq_per_ms が未初期化(0)なら即戻る
// - 内部で rdtsc_serialized を使い精度を確保
// - 長時間待機やスケジューラ有り環境では別手段を推奨
void Timer::msleep(uint32_t ms) {
    // 初期化確認
    uint64_t per_ms = tsc_freq_per_ms;
    if (per_ms == 0) {
        return;
    }

    // overflow 対策: ms * per_ms を uint64_t 演算で安全に計算
    uint64_t total_ticks = static_cast<uint64_t>(ms) * per_ms;

    // 開始時刻
    uint64_t start = rdtsc_serialized();

    // スピンループ（PAUSE を挟む）
    while ((rdtsc_serialized() - start) < total_ticks) {
        asm volatile("pause" ::: "memory");
    }
}

} // namespace Hardware
