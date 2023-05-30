#pragma once
#include <ArduinoBLE.h>

// BLEコマンド制御クラス
class BleCommand
{
public:
    void begin();
    void task();
    
private:
    BLEDevice central;
    bool isConnected;
};
