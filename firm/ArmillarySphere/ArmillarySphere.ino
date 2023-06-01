#include <math.h>
#include "DRV8825.h"
#include "SunCalc.h"
#include "BleCommand.h"
#include "Debug.h"

// ピン番号 TODO
#define MOTOR1_DIR   21
#define MOTOR1_STP   20
#define MOTOR2_DIR   9
#define MOTOR2_STP   8
#define LED_SUN      10

// 1ステップ=5.625°= 360°/64, ギア比1/32, フルステップ
#define STEP_PER_REV (64*32*1)

// ステッピングモータ
DRV8825 motor1(MOTOR1_DIR, MOTOR1_STP, 1, STEP_PER_REV);
DRV8825 motor2(MOTOR2_DIR, MOTOR2_STP, 1, STEP_PER_REV);

// モータの角度[deg]
static double theta1 = 0.0;
static double theta2 = 0.0;
// モータの回転量[deg]と回転時間[sec]の指令値
static double d_theta1 = 0.0;
static double d_theta2 = 0.0;
static double d_T = 0.0;
// モータの回転要求
static bool request_rotate = false;

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

// 初期位置コマンドのとき
void onCommandInit()
{
    Serial.println("onCommandInit");
}

// 停止コマンドのとき
void onCommandStop()
{
    Serial.println("onCommandStop");
}

// 自転コマンドのとき
void onCommandRotation()
{
    Serial.println("onCommandRotation");
}

// 公転コマンドのとき
void onCommandRevolution()
{
    Serial.println("onCommandRevolution");
}

// デモ1コマンドのとき
void onCommandDemo1()
{
    Serial.println("onCommandDemo1");
}

// デモ2コマンドのとき
void onCommandDemo2()
{
    Serial.println("onCommandDemo2");
}

// 経度・UTC日時の設定のとき
void onCommandLonTime()
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
    d_theta1 = deg_range180( _theta1 - theta1 );
    d_theta2 = deg_range180( _theta2 - theta2 );
    theta1 = _theta1;
    theta2 = _theta2;
    DEBUG_PRINT("d_theta1 = %7.2f\n", d_theta1);
    DEBUG_PRINT("d_theta2 = %7.2f\n", d_theta2);
    
    // 回転時間
    double T1 = fabs(d_theta1 / V_MAX);
    double T2 = fabs(d_theta2 / V_MAX);
    d_T = (T1 > T2) ? T1 : T2;
    DEBUG_PRINT("T = %f\n", d_T);
    
    request_rotate = true;
}

// 初期化
void setup()
{
    // LEDの初期化
    pinMode(LED_SUN,  OUTPUT);
    digitalWrite(LED_SUN, HIGH);
    
    // シリアル
    Serial.begin(115200);
    delay(100); // TODO
    while(!Serial){;} // TODO
    Serial.println("Hello!");
    
    // モータの初期化
    motor1.begin();
    motor2.begin();
    
    // BLEコマンドの初期化
    bleCommand.onCommandInit       = onCommandInit;
    bleCommand.onCommandStop       = onCommandStop;
    bleCommand.onCommandRotation   = onCommandRotation;
    bleCommand.onCommandRevolution = onCommandRevolution;
    bleCommand.onCommandDemo1      = onCommandDemo1;
    bleCommand.onCommandDemo2      = onCommandDemo2;
    bleCommand.onCommandLonTime    = onCommandLonTime;
    bleCommand.begin();
}

// メインループ
void loop()
{
    static int demo = 0;
    
    bleCommand.task();

    // モータがアイドル状態か？
    static bool motors_idle_old = true;
    bool motors_idle = motor1.isIdle() && motor2.isIdle();
    
    if(demo == 0){
        if(motors_idle){
            // デモモード要求？
            if(Serial.available() > 0){
                char c = Serial.read();
                if(c == 'd'){
                    demo = 1;
                    motor1.rotateV(360, V_MAX);
                }
            }
            // モータの回転要求があったか？
            else if(request_rotate){
                request_rotate = false;
                Serial.println("Motors rotating...");
                // モータの回転指令
                motor1.rotateT(d_theta1, d_T);
                motor2.rotateT(d_theta2, d_T);
            }
            if(!motors_idle_old){
                Serial.println("Motors rotated!");
            }
            //delay(1);
        }else{
            // モータの制御更新
            motor1.update();
            motor2.update();
            //delay(1);
        }
    }else{
        if(motors_idle){
            motor1.rotateV(360, V_MAX);
        }else{
            // モータの制御更新
            motor1.update();
            motor2.update();
            //delay(1);
        }
        // デモモード終了？
        if(Serial.available() > 0){
            char c = Serial.read();
            if(c == 's'){
                demo = 0;
            }
        }
    }
    motors_idle_old = motors_idle;
}
