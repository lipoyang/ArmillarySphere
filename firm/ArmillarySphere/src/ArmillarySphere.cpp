#include <math.h>
#include "DRV8825.h"
#include "SunCalc.h"
#include "BleCommand.h"
#include "Debug.h"

// ピン番号
#define MOTOR1_DIR   D0
#define MOTOR1_STP   D1
#define MOTOR2_DIR   D2
#define MOTOR2_STP   D3
#define LED_SUN      D8
#define HALL_SENSOR1 D9
#define HALL_SENSOR2 D10

// 1ステップ=5.625°= 360°/64, ギア比1/32, フルステップ
#define STEP_PER_REV (64*32*1)

// ステッピングモータ
DRV8825 motor1(MOTOR1_DIR, MOTOR1_STP, 1, STEP_PER_REV);
DRV8825 motor2(MOTOR2_DIR, MOTOR2_STP, 1, STEP_PER_REV);

// 初期化フラグ
static bool initializing = false;
// モータの角度[deg]
static double theta1 = 0.0;
static double theta2 = 0.0;

// モータの回転速度上限[deg/sec]
const double V_MAX = 60.0;

// 太陽の位置計算クラス
SunCalc sun;

// BLEコマンドクラス
BleCommand bleCommand;

// -360～+360°の角度を-180°～+180°に変換する
static double deg_range180(double x)
{
    if(x >  180.0) x -= 360.0;
    if(x < -180.0) x += 360.0;
    return x;
}

// 初期位置出し(太陽が春分点で南中)
static void initPosition()
{
    bleCommand.setBusy(true);
    
    Serial.println("Motor position initializing...");
    
    int state[2]    = {0, 0};
    int position[2];
    int hall[2];
    DRV8825 *motor[2] = {&motor1, &motor2};
    
    // ホールセンサ入力の取得
    hall[0] = digitalRead(HALL_SENSOR1);
    hall[1] = digitalRead(HALL_SENSOR2);
    
    // もしホールセンサがHIGHなら少し逆転しておく
    if(hall[0] == HIGH) motor1.rotateT(-30, 0.5);
    if(hall[1] == HIGH) motor2.rotateT(-30, 0.5);
    while(!motor1.isIdle() || !motor2.isIdle()){
        motor1.update();
        motor2.update();
        delay(1);
    }
    // 正転開始
    motor1.rotateT(360, 6);
    motor2.rotateT(360, 6);
    
    while(true)
    {
        // ホールセンサ入力の取得
        hall[0] = digitalRead(HALL_SENSOR1);
        hall[1] = digitalRead(HALL_SENSOR2);
        
        // モータごとの制御
        for(int i=0; i<2; i++)
        {
            switch(state[i])
            {
            // 正転中(ホールセンサがHIGHになるまで進む)
            case 0:
                if (hall[i] == HIGH){
                    state[i] = 1;
                    position[i] = motor[i]->getPos();
                    motor[i]->rotateT(180, 12);
                }
                break;
            // 正転中(ホールセンサがLOWになるまでさらに進む)
            case 1:
                if (hall[i] == LOW){
                    state[i] = 2;
                    int step = (motor[i]->getPos() - position[i]) / 2;
                    motor[i]->setStepV(-step, 100);
                }
                break;
            // 逆転中(追加で進んだステップ数の半分だけ戻る)
            case 2:
                if(motor[i]->isIdle()){
                    state[i] = 3;
                }
                break;
            // 初期位置出し完了
            case 3:
                break;
            }
        }
        
        // 両方のモータの初期位置出しが完了したら終了
        if((state[0] == 3) && (state[1] == 3)){
            Serial.println("Motor position initialized!");
            break;
        }
        
        // モータの制御更新
        motor1.update();
        motor2.update();
        delay(1);
        
        // BLEタスクの実行 TODO
        bleCommand.task();
    }
    bleCommand.setBusy(false);
}

// 初期位置コマンドのとき
static void onCommandInit()
{
    Serial.println("onCommandInit");
    
    // モータのアイドル判定
    bool motors_idle = motor1.isIdle() && motor2.isIdle();
    if(!motors_idle){
        Serial.println("Motor Busy");
        return;
    }
    
    initializing = true;
}

// 停止コマンドのとき
static void onCommandStop()
{
    Serial.println("onCommandStop");
    
    motor1.stop();
    motor2.stop();
}

// 自転コマンドのとき
static void onCommandRotation()
{
    Serial.println("onCommandRotation");
    
    // モータのアイドル判定
    bool motors_idle = motor1.isIdle() && motor2.isIdle();
    if(!motors_idle){
        Serial.println("Motor Busy");
        return;
    }
    
    motor1.rotateV(V_MAX);
}

// 公転コマンドのとき
static void onCommandRevolution()
{
    Serial.println("onCommandRevolution");
    
    // モータのアイドル判定
    bool motors_idle = motor1.isIdle() && motor2.isIdle();
    if(!motors_idle){
        Serial.println("Motor Busy");
        return;
    }
    
    motor2.rotateV(V_MAX);
}

// デモ1コマンドのとき
static void onCommandDemo1()
{
    Serial.println("onCommandDemo1");
    
    // モータのアイドル判定
    bool motors_idle = motor1.isIdle() && motor2.isIdle();
    if(!motors_idle){
        Serial.println("Motor Busy");
        return;
    }
    
    motor1.rotateV(V_MAX);
    motor2.rotateV(V_MAX);
}

// デモ2コマンドのとき
static void onCommandDemo2()
{
    Serial.println("onCommandDemo2");
    
    // モータのアイドル判定
    bool motors_idle = motor1.isIdle() && motor2.isIdle();
    if(!motors_idle){
        Serial.println("Motor Busy");
        return;
    }
    
    motor1.rotateV(V_MAX);
    motor2.rotateV(V_MAX / 4);
}

// 経度・UTC日時の設定のとき
static void onCommandLonTime()
{
    Serial.println("onCommandLonTime");
    
    float lon = bleCommand.lonTime.lon;
    int y = bleCommand.lonTime.year;
    int m = bleCommand.lonTime.month;
    int d = bleCommand.lonTime.day;
    int h = bleCommand.lonTime.hour;
    int n = bleCommand.lonTime.min;

    // モータのアイドル判定
    bool motors_idle = motor1.isIdle() && motor2.isIdle();
    if(!motors_idle){
        Serial.println("Motor Busy");
        return;
    }
    
    // 日時と観測地点の緯度・経度を設定して太陽の位置を計算
    sun.setDate(y, m, d, h, n, 0);
    sun.setLocation(35, lon); // 第1引数の緯度は実際には未使用
    sun.calc();
    
    // 太陽黄経λ[deg]と地方恒星時LST[deg]
    DEBUG_PRINT("Lambda = %7.3f\n", RAD2DEG(sun.lambda));
    DEBUG_PRINT("LST    = %7.3f\n", RAD2DEG(sun.t_LST));
    
    // モータの角度に換算
    double _theta2 = RAD2DEG(sun.lambda);
    DEBUG_PRINT("theta2 = %7.2f\n", _theta2);
    double _theta1 = RAD2DEG(sun.t_LST);
    DEBUG_PRINT("theta1 = %7.2f\n", _theta1);
    
    // モータの回転量
    double d_theta1 = deg_range180( _theta1 - theta1 );
    double d_theta2 = deg_range180( _theta2 - theta2 );
    theta1 = _theta1;
    theta2 = _theta2;
    DEBUG_PRINT("d_theta1 = %7.2f\n", d_theta1);
    DEBUG_PRINT("d_theta2 = %7.2f\n", d_theta2);
    
    // 回転時間
    double T1 = fabs(d_theta1 / V_MAX);
    double T2 = fabs(d_theta2 / V_MAX);
    double T = (T1 > T2) ? T1 : T2;
    DEBUG_PRINT("T = %f\n", T);
    
    // モータの回転指令
    motor1.rotateT(d_theta1, T);
    motor2.rotateT(d_theta2, T);
}

// 初期化
void setup()
{
    Serial.begin(115200);
#if 1
    while(!Serial);
    delay(100);
    Serial.println("Start!");
#endif

    // モータの初期化
    motor1.begin();
    motor2.begin();
    
    // ホールセンサの初期化
    pinMode(HALL_SENSOR1, INPUT);
    pinMode(HALL_SENSOR2, INPUT);
    
    // LEDの点灯
    pinMode(LED_SUN,    OUTPUT);
    digitalWrite(LED_SUN,    HIGH);
    
    // BLEコマンドの初期化
    bleCommand.onCommandInit       = onCommandInit;
    bleCommand.onCommandStop       = onCommandStop;
    bleCommand.onCommandRotation   = onCommandRotation;
    bleCommand.onCommandRevolution = onCommandRevolution;
    bleCommand.onCommandDemo1      = onCommandDemo1;
    bleCommand.onCommandDemo2      = onCommandDemo2;
    bleCommand.onCommandLonTime    = onCommandLonTime;
    bleCommand.begin();

    // 初期位置出し(太陽が春分点で南中)
    initPosition();
}

// メインループ
void loop()
{
    // BLEタスク
    bleCommand.task();

    // モータがアイドル状態かチェック
    static bool motors_idle_old = true;
    bool motors_idle = motor1.isIdle() && motor2.isIdle();
    if(motors_idle && !motors_idle_old){
        Serial.println("Motors rotated!");
    }
    motors_idle_old = motors_idle;
    bleCommand.setBusy(!motors_idle);
    
    // 初期位置出し
    if(initializing){
        initializing = false;
        initPosition();
    }
    
    // モータの制御更新
    motor1.update();
    motor2.update();
}
