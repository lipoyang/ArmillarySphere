#pragma once

#include "BaseStepper.h"

// ステッピングモータドライバDRV8825
class DRV8825 : public BaseStepper
{
private:
    const int PIN_DIR;  // DIRピン番号
    const int PIN_STP;  // STPピン番号
    const int PIN_EN;   // ENABLEピン番号 (負論理)
    
public:
    // コンストラクタ
    DRV8825(int dir, int stp, int pol, int spr, int en=-1) : 
        BaseStepper(pol, spr), PIN_DIR(dir), PIN_STP(stp), PIN_EN(en){}

    // モータドライバICごとに実装するメソッド
    void begin();                    // 初期化
    void takeStep(int dir, int stp); // ステップ動作
    int getTime();                   // システム時刻の取得
    void enable();                   // モータ出力有効化
    void disable();                  // モータ出力無効化
};
