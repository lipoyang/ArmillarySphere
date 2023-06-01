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
};

// BLEコマンド制御クラス
class BleCommand
{
public:
    void begin();
    void task();
    
    void (*onCommandInit)();
    void (*onCommandStop)();
    void (*onCommandRotation)();
    void (*onCommandRevolution)();
    void (*onCommandDemo1)();
    void (*onCommandDemo2)();
    void (*onCommandLonTime)();
    
    LonTime lonTime;
    
private:
    BLEDevice central;
    bool isConnected;
};
