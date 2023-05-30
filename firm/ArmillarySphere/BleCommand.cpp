#include <stdint.h>
#include "BleCommand.h"

// 経度とUTC日時
struct LonTime
{
    float       lon;
    uint16_t    year;
    uint8_t     month;
    uint8_t     day;
    uint8_t     hour;
    uint8_t     min;
};
static LonTime lonTime;

// BLEサービス
BLEService svcArmillarySphere("220EB65B-D64D-E553-F51E-A1048818DC96");

// BLEキャラクタリスティック
class BLELonTimeCharacteristic : public BLETypedCharacteristic<LonTime> {
public:
  BLELonTimeCharacteristic(const char* uuid, unsigned int permissions):
    BLETypedCharacteristic<LonTime>(uuid, permissions) { }
};

typedef BLELonTimeCharacteristic        CHR_LT;
typedef BLEByteCharacteristic           CHR_U8;

// コマンド
CHR_U8  chrCommand   ("2D3F5DDE-42B2-FBFA-3E3F-7673832C7DB4", BLEWrite);
// 経度とUTC日時を設定
CHR_LT  chrLonTime   ("328E1678-11B5-5D34-AAD2-EF1A4A2957EB", BLERead | BLEWrite);
// ビジー状態か？
CHR_U8  chrBusy      ("31180989-2299-A86F-C856-B2154171C07B", BLERead);

// コマンド定数
const uint8_t CMD_INIT       = 0x80; // 初期位置
const uint8_t CMD_STOP       = 0x81; // 停止
const uint8_t CMD_ROTATION   = 0x82; // 自転
const uint8_t CMD_REVOLUTOIN = 0x83; // 公転
const uint8_t CMD_DEMO1      = 0x84; // デモ1
const uint8_t CMD_DEMO2      = 0x85; // デモ2

// 初期化
void BleCommand::begin()
{
    isConnected = false;
    
    // BLEの開始
    if (!BLE.begin()) {
        Serial.println("ERROR: starting BLE module failed!");
        while (1);
    }
    // Connection Intervalの設定
    BLE.setConnectionInterval(6, 80); // 7.25ms - 100ms
    
    // アドバタイズするローカル名とサービスを設定
    BLE.setLocalName("Armillary Sphere");
    BLE.setAdvertisedService(svcArmillarySphere);

    // サービスにキャラクタリスティックを追加
    svcArmillarySphere.addCharacteristic(chrCommand);
    svcArmillarySphere.addCharacteristic(chrLonTime);
    svcArmillarySphere.addCharacteristic(chrBusy);
    
    // サービスを追加
    BLE.addService(svcArmillarySphere);

    // キャラクタリスティックの初期値を設定
    lonTime.lon   = 135;
    lonTime.year  = 2023;
    lonTime.month = 1;
    lonTime.day   = 1;
    lonTime.hour  = 0;
    lonTime.min   = 0;
    chrLonTime.writeValue(lonTime);
    chrBusy.writeValue(0);
    
    // アドバタイズ開始
    BLE.advertise();
}

// タスク
void BleCommand::task()
{
    if(!isConnected){
        central = BLE.central();
        if (central)
        {
            isConnected = true;
            Serial.print("BLE Connected to central: ");
            Serial.println(central.address());
        }
    }else{
        if(central.connected())
        {
            // キャラクタリスティックへのWRITEがあれば処理
            
            // コマンド
            if (chrCommand.written())
            {
                uint8_t command = chrCommand.value();
                switch(command)
                {
                    case CMD_INIT:
                        Serial.println("CMD_INIT");
                        break;
                    case CMD_STOP:
                        Serial.println("CMD_STOP");
                        break;
                    case CMD_ROTATION:
                        Serial.println("CMD_ROTATION");
                        break;
                    case CMD_REVOLUTOIN:
                        Serial.println("CMD_REVOLUTOIN");
                        break;
                    case CMD_DEMO1:
                        Serial.println("CMD_DEMO1");
                        break;
                    case CMD_DEMO2:
                        Serial.println("CMD_DEMO2");
                        break;
                    default:
                        Serial.print("Unknown Command: ");
                        Serial.println(command, HEX);
                        break;
                }
            }
            // 経度とUTC日時
            if (chrLonTime.written())
            {
                LonTime lonTime = chrLonTime.value();
                static char buff[128];
                sprintf(buff, "Lon = %.4f\n", lonTime.lon);
                Serial.print(buff);
                sprintf(buff, "UTC = %4d/%02d/%02d %02d:%02d\n",
                    lonTime.year, lonTime.month, lonTime.day,
                    lonTime.hour, lonTime.min);
                Serial.print(buff);
            }
            
        }else{
            isConnected = false;
            Serial.print(F("BLE Disconnected from central: "));
            Serial.println(central.address());
        }
    }
}
