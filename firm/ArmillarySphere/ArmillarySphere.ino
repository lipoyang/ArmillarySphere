#include "BleCommand.h"

BleCommand bleCommand;

// 初期化
void setup()
{
    Serial.begin(115200);
    delay(100); // TODO
    while(!Serial){;} // TODO
    Serial.println("Hello!");
    
    bleCommand.begin();
}

// メインループ
void loop()
{
    bleCommand.task();
}
