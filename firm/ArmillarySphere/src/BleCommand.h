#pragma once
#include <ArduinoBLE.h>

// 状態通知
#define STA_INIT       0x80 // 初期位置出し
#define STA_STOP       0x81 // 停止
#define STA_ROTATION   0x82 // 自転
#define STA_REVOLUTOIN 0x83 // 公転
#define STA_DEMO1      0x84 // デモ1
#define STA_DEMO2      0x85 // デモ2
#define STA_INIT_DONE  0x86 // 初期位置出し完了

// 経度とUTC日時
struct LonTime
{
    float       lon;
    uint16_t    year;
    uint8_t     month;
    uint8_t     day;
    uint8_t     hour;
    uint8_t     min;
    int8_t      timezone;
};

// BLEコマンド制御クラス
class BleCommand
{
public:
    void begin();
    void task();
    void setBusy(bool busy);
    bool isBusy(){return _busy;}
    void notifyStatus(uint8_t status);
    
    bool (*onCommandInit)();
    bool (*onCommandStop)();
    bool (*onCommandRotation)();
    bool (*onCommandRevolution)();
    bool (*onCommandDemo1)();
    bool (*onCommandDemo2)();
    bool (*onCommandLonTime)();

    void (*onConnected)();
    void (*onDisconnected)();
    
    LonTime lonTime;
    
private:
    BLEDevice central;
    bool isConnected;
    bool _busy = false;
};
