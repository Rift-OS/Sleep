// code is src/kernel.c
#include "timer.h"

// OSのメインエントリーポイント（関数名は環境に合わせてください）
void kernel_main(void) {
    
    // 1. グラフィックやメモリの初期化のあとにタイマーを初期化
    init_sleep_timer();

    // 2. 実際の使用例（画面に文字を出して1秒待つ、を繰り返す）
    while (1) {
        // 例: print("Hello OS World!\n");
        
        msleep(1000); // 1000ミリ秒（1秒）スリープ
        
        // 例: print("1 second passed.\n");
    }
}
