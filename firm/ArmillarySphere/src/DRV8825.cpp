// ステッピングモータドライバDRV8825

#include <Arduino.h>
#include "DRV8825.h"

// 初期化
void DRV8825::begin()
{
	init();
    pinMode(PIN_DIR, OUTPUT);
    pinMode(PIN_STP, OUTPUT);
    digitalWrite(PIN_DIR, LOW);
    digitalWrite(PIN_STP, LOW);
    if(PIN_EN != -1){
        pinMode(PIN_EN, OUTPUT);
        digitalWrite(PIN_EN, HIGH); // 負論理
    }
}

// ステップ動作
// dir: 方向
// stp: ステップ
void DRV8825::takeStep(int dir, int stp)
{
    digitalWrite(PIN_DIR, dir);
    digitalWrite(PIN_STP, stp);
}

// システム時刻の取得
// return: usec単位の通算時間
int DRV8825::getTime()
{
    return micros();
}

// モータ出力有効化
void DRV8825::enable()
{
    if(PIN_EN != -1){
        digitalWrite(PIN_EN, LOW); // 負論理
    }
}

// モータ出力無効化
void DRV8825::disable()
{
    if(PIN_EN != -1){
        digitalWrite(PIN_EN, HIGH); // 負論理
    }
}
