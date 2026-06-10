// code is src/kernel.cpp
#include "timer.h"

// Cリンケージを指定してC言語やブートローダー（アセンブリ）からの呼び出しを可能にする
extern "C" {
    void kernel_main();
}

// OSのメインエントリーポイント
void kernel_main() {
    
    // 1. グラフィックやメモリの初期化のあとにタイマーを初期化
    init_sleep_timer();

    // 2. 実際の使用例（画面に文字を出して1秒待つ、を繰り返す）
    while (true) {        
        msleep(1000); // 1000ミリ秒（1秒）スリープ
        
    }
}
