# sleep - Kernel Timer Module

組織名 void-start-OS が開発する、x86_64自作OS向けのミリ秒単位高精度スリープタイマーモジュールです。

コンパイル最適化で消滅する空ループではなく、ACPI PMタイマーとCPUのタイムスタンプカウンタ（TSC）を組み合わせることで、実機およびエミュレータ（QEMU）上で正確なウェイト処理を実現します。

## ファイル構成

*   src/timer.h - タイマーモジュールの共通設定・関数宣言
*   src/timer.c - TSCキャリブレーションおよびスリープの本体ロジック
*   src/kernel.c - カーネルメイン処理での組み込みサンプル
*   README.md - このドキュメント

## 組み込み方法

### 1. ソースコードの配置
ソースファイルをプロジェクトの `src/` ディレクトリに配置します。

### 2. ACPIポートの確認
`src/timer.h` 内の `ACPI_PM_TIMER_PORT` を、お使いのOS環境（またはQEMUの引数）に合わせて調整してください。
```c
// 例: QEMU環境の一般的なデフォルトポート
#define ACPI_PM_TIMER_PORT 0x0608 
```

### 3. ビルドへの追加
Makefile等を使用している場合、コンパイル対象に `src/timer.c` を追加してください。
```makefile
# Makefileの記述例
SRCS = src/kernel.c src/timer.c
OBJS = \$(SRCS:.c=.o)

kernel.elf: \$(OBJS)
	\((LD)\)(LDFLAGS) -o \(@ \)^
```

## 使い方

カーネルの初期化シーケンス（メモリや画面出力の初期化後）で一度だけ `init_sleep_timer()` を呼び出し、その後任意の場所で `msleep()` を使用します。

```c
#include "timer.h"

void kernel_main(void) {
    // 起動時に1回だけ実行（TSCの周波数を計測）
    init_sleep_timer();

    while(1) {
        // 1000ミリ秒（1秒）スリープ
        msleep(1000);
    }
}
```

## 技術仕様

1.  初期化 (init_sleep_timer): 周波数が 3.579545 MHz に固定されているACPI PMタイマーを利用し、1ミリ秒間ループを回します。その間に進んだCPUのTSCカウント数を計測し、1ミリ秒あたりの周波数を確定させます。
2.  スリープ (msleep): 引数（ミリ秒）から目標の総TSCカウント数を計算し、現在のTSCが目標値に達するまで while ループで監視します。ループ内では pause 命令を発行し、CPUの消費電力と発熱を抑えています。

## ライセンス

This project is licensed under the GNU General Public License v3.0 (GPL-3.0) - see the LICENSE file for details.

---
Copyright (c) 2026 void-start-OS
