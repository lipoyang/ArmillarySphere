#include <M5Unified.h>
#include "M5UI.h"

// スプライト
static LGFX_Sprite sprite1(&M5.Display);
static LGFX_Sprite sprite2(&M5.Display);
static LGFX_Sprite sprite3(&M5.Display);
static LGFX_Sprite sprite4(&M5.Display);

// 動作モード
enum OpMode
{
    MODE_NORMAL = 0,
    MODE_INIT,
    MODE_ROTATION,
    MODE_REVOLUTOIN,
    MODE_DEMO1,
    MODE_DEMO2,
    MAX_MODE
};
OpMode opMode = MODE_NORMAL;
bool isSelecting = false;
static int modeSelect;

// 指定した矩形領域の中央にテキストを描画する関数
static void drawCenteredText(
    LGFX_Sprite* sprite,
    const char* text,
    int x, int y,
    const lgfx::GFXfont *font, int size,
    int color, bool reverse = false)
{
    if(reverse){
        sprite->fillScreen(color);
        sprite->setTextColor(BLACK);
    }else{
        sprite->fillScreen(BLACK);
        sprite->setTextColor(color);
    }
    sprite->setFont(font);
    sprite->setTextSize(size);
    sprite->setTextDatum(MC_DATUM);
    sprite->drawString(text, sprite->width() / 2, sprite->height() / 2);
    sprite->pushSprite(x, y);
}

static void showDate(int y, int m, int d)
{
    static char buff[11]; // "YYYY/MM/DD" + null terminator
    snprintf(buff, sizeof(buff), "%04d/%02d/%02d", y, m, d);
    drawCenteredText(&sprite1, buff, 0, 0, &FreeSans24pt7b, 1, 0x4FF); // 00A0FF
}

static void showTime(int h, int n)
{
    static char buff[6]; // "HH:MM" + null terminator
    snprintf(buff, sizeof(buff), "%02d:%02d", h, n);
    drawCenteredText(&sprite2, buff, 0, 50, &FreeSans18pt7b, 2, 0x5FF); // 00C0FF
}

static void showLongitude(float lon)
{
    static char buff[12]; // "E123.4567" + null terminator
    if (lon < 0) {
        lon = -lon;
        snprintf(buff, sizeof(buff), "W%07.4f", lon);
    } else {
        snprintf(buff, sizeof(buff), "E%07.4f", lon);
    }
    drawCenteredText(&sprite3, buff, 0, 140, &FreeSans24pt7b, 1, 0x073C); // 00E8E8
}

static void showStatus(OpMode opMode, bool isOnline, bool isSelecting)
{
    // 動作モードの文字列
    static const char* OpModeStr[] = {
        "NORMAL",
        "INIT",
        "ROTATION",
        "REVOLUTOIN",
        "DEMO1",
        "DEMO2"
    };
    const char* status;
    bool reverse;
    if(isSelecting){
        status = OpModeStr[opMode];
        reverse = true;
        Serial.println("HOGE1");
    }else{
        if(opMode == MODE_NORMAL){
            status = (isOnline) ? "ONLINE" : "OFFLINE";
        }else{
            status = OpModeStr[opMode];
        }
        reverse = false;
    }

    drawCenteredText(&sprite4, status, 0, 190, &FreeSans18pt7b, 1, 0x737, reverse); // 00E8C0
}

void M5UI::begin()
{
    auto cfg = M5.config();
    M5.begin(cfg);

    // 画面のクリア（黒背景）
    M5.Display.fillScreen(BLACK);

    // スプライトの作成 (幅, 高さ)
    // 画面幅を320と仮定
    sprite1.createSprite(320, 50);
    sprite2.createSprite(320, 90);
    sprite3.createSprite(320, 50);
    sprite4.createSprite(320, 50);

    showDate(2026, 3, 20);
    showTime(12, 0);
    showLongitude(135.0000);
    showStatus(opMode, connected, isSelecting);
}

void M5UI::task()
{
    M5.update();

    bool redraw = false;

    // [SELECT] ボタン
    if (M5.BtnA.wasPressed()) {
        Serial.println("Button A pressed");
        if(isSelecting == false){
            isSelecting = true;
            modeSelect = (int)opMode;
        }else{
            modeSelect++;
            if(modeSelect >= (int)MAX_MODE) modeSelect = 0;
        }
        redraw = true;
    }
    // [SET] ボタン
    if (M5.BtnB.wasPressed()) {
        Serial.println("Button B is holding");
        if(isSelecting){
            opMode = (OpMode)modeSelect;
            isSelecting = false;
        }
        redraw = true;
    }
    // [CANCEL] ボタン
    if (M5.BtnC.wasPressed()) {
        Serial.println("Button C released");
        if(isSelecting){
            isSelecting = false;
        }
        redraw = true;
    }
    if(redraw || toUpdate){
        if(isSelecting){
            showStatus((OpMode)modeSelect, connected, isSelecting);
        }else{
            showStatus(opMode, connected, isSelecting);
        }
    }

    if(toUpdate){
        showDate(y, m, d);
        showTime(h, n);
        showLongitude(lon);
        toUpdate = false;
    }
}