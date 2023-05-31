#pragma once

#include "BaseStepper.h"

// ステッピングモータドライバDRV8825
class DRV8825 : public BaseStepper
{
private:
    const int PIN_DIR;  // DIRピン番号
    const int PIN_STP;  // STPピン番号
    
public:
    // コンストラクタ
    DRV8825(int dir, int stp, int pol, int spr) : 
        PIN_DIR(dir), PIN_STP(stp), BaseStepper(pol, spr){}

    // モータドライバICごとに実装するメソッド
    void begin();                    // 初期化
    void takeStep(int dir, int stp); // ステップ動作
    int getTime();                   // システム時刻の取得
};
