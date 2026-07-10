#pragma once

enum OpMode
{
    MODE_NORMAL = 0,
    MODE_INIT,
    MODE_ROTATION,
    MODE_REVOLUTION,
    MODE_DEMO1,
    MODE_DEMO2,
    MAX_MODE
};

class M5UI
{
public:
    void begin();
    void task();
    void setConnected(bool _connected) {connected = _connected; toUpdate = true;}
    void setLongitude(float _lon) {lon = _lon; toUpdate = true;}
    void setDate(int _y, int _m, int _d) {y = _y; m = _m; d= _d; toUpdate = true;}
    void setTime(int _h, int _n) {h = _h; n = _n; toUpdate = true;}
    void setMode(OpMode mode);

    void (*onSetMode)(OpMode mode);
private:
    float lon = 135.0f;
    int y = 2026;
    int m = 3;
    int d = 20;
    int h = 12;
    int n = 0;
    bool connected = false;
    bool toUpdate = false;
};