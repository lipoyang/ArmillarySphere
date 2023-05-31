#include <math.h>
#include "DRV8825.h"
#include "SunCalc.h"
#include "BleCommand.h"

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
            delay(1000);
        }else{
            // モータの制御更新
            motor1.update();
            motor2.update();
            delay(1);
        }
    }else{
        if(motors_idle){
            motor1.rotateV(360, V_MAX);
        }else{
            // モータの制御更新
            motor1.update();
            motor2.update();
            delay(1);
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

#if 0
void sample_direct_method_thread_entry(ULONG parameter)
{
    UCHAR loop = NX_TRUE;
    NX_PACKET *packet_ptr;
    UINT status = 0;
    USHORT method_name_length;
    const UCHAR *method_name_ptr;
    USHORT context_length;
    VOID *context_ptr;
    CHAR res_payload[100];
    const char *JSON_FORMAT = "{\"status\": 200, \"payload\": {\"Lambda\": %7.2f, \"Alpha\": %7.2f, \"Delta\": %7.2f, \"Error\": %d}}";
    int error_code = 0;
    bool _request_rotate = false;
    
    NX_PARAMETER_NOT_USED(parameter);
    
    // ダイレクトメソッドメッセージ受信(コマンド受信)ごとに回るループ
    while (loop)
    {
        // コマンド受信 (受信するまで待ち続ける)
        if ((status = nx_azure_iot_hub_client_direct_method_message_receive(
            &iothub_client,
            &method_name_ptr, &method_name_length,
            &context_ptr,     &context_length,
            &packet_ptr,
            NX_WAIT_FOREVER)))
        {
            printf("Direct method receive failed!: error code = 0x%08x\r\n", status);
            break;
        }
#if 0
        // ※ ここで表示するとパケットが破棄されて以降の処理ができないことに注意
        printf("Receive direct method :");
        printf_packet(packet_ptr);
        printf("\r\n");
#endif
        // コマンド名取得
        char method[10] = {0};
        strncpy(method, (CHAR *)method_name_ptr, method_name_length);
        printf("command:[%s]\n", method);
        
        // コマンド sun_calc の処理
        if(strcmp("sun_calc", method)==0)
        {
            // モータのアイドル判定
            bool motors_idle = motor1.isIdle() && motor2.isIdle();
            if(!motors_idle) error_code = 1;
            if(error_code == 0){
                // JSONのパース
                StaticJsonDocument<200> doc;
                char jsonstr[200];
                INT len = (INT)(packet_ptr -> nx_packet_append_ptr - packet_ptr -> nx_packet_prepend_ptr);
                memcpy(jsonstr, packet_ptr -> nx_packet_prepend_ptr, len);
                jsonstr[len] = 0x00;
                printf("Request JSON = %s\n", jsonstr);
                DeserializationError error =
                      deserializeJson(doc, jsonstr);
                if (error) {
                   Serial.print(F("deserializeJson() failed: "));
                   Serial.println(error.c_str());
                }
                // パラメータの取得
                // 位置(緯度, 経度)
                double lat = doc["Latitude"];
                double lon = doc["Longitude"];
                // UTC日時
                int y = doc["Year"];
                int m = doc["Month"];
                int d = doc["Day"];
                 int h = doc["Hour"];
                int n = doc["Min"];
                int s = doc["Sec"];
                // 範囲チェック
                bool err = false;
                if(!(-180 <= lat && lat <= 180)) error_code = 2;
                if(!(-180 <= lon && lon <= 180)) error_code = 3;
                if(!(1900 <= y && y <= 2099)) error_code = 4;
                if(!(   1 <= m && m <=   12)) error_code = 5;
                if(!(   1 <= d && d <=   31)) error_code = 6;
                if(!(   0 <= h && h <=   23)) error_code = 7;
                if(!(   0 <= n && n <=   59)) error_code = 8;
                if(!(   0 <= s && s <=   59)) error_code = 9;
                if(error_code == 0){
                    latitude  = lat;
                    longitude = lon;
                    printf("Latitude = %7.2f, Longitude = %7.2f\n",
                        latitude, longitude);
                    year  = y;
                    month = m;
                    day   = d;
                    hour  = h;
                    min   = n;
                    sec   = s;
                    printf("Time(UTC) = %4d/%02d/%02d %02d:%02d:%02d\n",
                        year, month, day, hour, min, sec);
                }
            }
            if(error_code == 0){
                // 日時と観測地点の緯度・経度を設定して太陽の位置を計算
                sun.setDate(year, month, day, hour, min, sec);
                sun.setLocation(latitude, longitude);
                sun.calc();
                
                // 太陽黄経λ[deg]と地方恒星時LST[deg]
                printf("Lambda = %7.3f\n", RAD2DEG(sun.lambda));
                printf("LST    = %7.3f\n", RAD2DEG(sun.t_LST));
                
                // モータの角度に換算
                double _theta2 = RAD2DEG(sun.lambda);
                printf("theta2 = %7.2f\n", _theta2);
                double _theta1 = RAD2DEG(sun.t_LST);
                printf("theta1 = %7.2f\n", _theta1);
                
                // モータの回転量
                d_theta1 = deg_range180( _theta1 - theta1 );
                d_theta2 = deg_range180( _theta2 - theta2 );
                theta1 = _theta1;
                theta2 = _theta2;
                printf("d_theta1 = %7.2f\n", d_theta1);
                printf("d_theta2 = %7.2f\n", d_theta2);
                
                // 回転時間
                double T1 = fabs(d_theta1 / V_MAX);
                double T2 = fabs(d_theta2 / V_MAX);
                d_T = (T1 > T2) ? T1 : T2;
                printf("T = %f\n", d_T);
                
                _request_rotate = true;
            }
            
            // 応答のJSON作成
            if(error_code == 0){
                sprintf(res_payload, JSON_FORMAT,
                    RAD2DEG(sun.lambda), RAD2DEG(sun.alpha), RAD2DEG(sun.delta), 0);
                printf("Response JSON = %s\n", res_payload);
            }else{
                sprintf(res_payload, JSON_FORMAT, 0, 0, 0, error_code);
                printf("Response (ERROR) = %d\n", error_code);
            }
        }
        
        packet_ptr = packet_ptr -> nx_packet_next; // ???
        
        // コマンド応答送信
        if ((status = nx_azure_iot_hub_client_direct_method_message_response(
            &iothub_client,
            200, // ステータスコード OK
            context_ptr, context_length,
            (UCHAR *)res_payload, strlen(res_payload),
            NX_WAIT_FOREVER)))
        {
            printf("Direct method response failed!: error code = 0x%08x\r\n", status);
            nx_packet_release(packet_ptr);
            break;
        }
        // 後始末
        nx_packet_release(packet_ptr);
        
        printf("Dirct method loop end\n");
        
        request_rotate = _request_rotate;
    }
}
#endif
