#pragma once
#include <ArduinoBLE.h>

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
};
