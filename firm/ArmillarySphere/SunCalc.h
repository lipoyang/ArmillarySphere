#pragma once
#include <stdlib.h>

// 度°分′秒″
struct DegMinSec
{
    int sign;
    int deg;
    int min;
    double sec;

    DegMinSec(int d, int m, double s) {
        sign = (d >= 0) ? 1 : -1;
        deg = abs(d);
        min = m;
        sec = s;
    }
    DegMinSec(){
    	sign = 1;
    	deg = 0;
    	min = 0;
    	sec = 0;
    }
};

// 時:分:秒
struct HourMinSec
{
    int sign;
    int hour;
    int min;
    double sec;

    HourMinSec(int h, int m, double s) {
        sign = (h >= 0) ? 1 : -1;
        hour = abs(h);
        min = m;
        sec = s;
    }
    HourMinSec(){
    	sign = 1;
    	hour = 0;
    	min = 0;
    	sec = 0;
    }
};

// 太陽の位置計算クラス
class SunCalc
{
public:
    //********** 公開メソッド **********
    // UTC日時を設定する
    void setDate(int year, int month, int day, int hour, int min, int sec);
    // 観測地点の位置を設定する
    void setLocation(double latitude, double longitude);
    void setLocation(DegMinSec latitude, DegMinSec longitude);
    // 太陽位置を計算する
    void calc();
    
    //********** 静的メソッド **********
    // J2000通日を計算する
    static double J2000(int year, int month, int day, int hour, int min, int sec);
    // 黄経を計算する
    static double ecliptic_longitude(double d, double& L, double& g);
    // 単位を換算する
    static double rad_0to2PI(double x);
    static DegMinSec  rad2dms(double rad);
    static HourMinSec rad2hms(double rad);
    static double dms2rad(DegMinSec  dms);
    static double hms2rad(HourMinSec hms);
    // 文字列に変換する
    static char* dms2str(DegMinSec  dms);
    static char* hms2str(HourMinSec hms);

    //********** 定数 **********
    static const double DEGS;
    static const double RADS;
    
    //********** 変数 **********
    // UTC日時
    int year;   // 年
    int month;  // 月
    int day;    // 日
    int hour;   // 時
    int min;    // 分
    int sec;    // 秒
    
    // 地球上の観測地点の位置 (単位が[rad]であることに注意)
    double lat; // 緯度[rad]
    double lon; // 経度[rad]
    
    // J2000通日 (小数部に時刻を含む)
    double d;
    
    // 太陽の位置 (すべて単位が[rad]であることに注意)
    double L;           // 平均黄経(mean longitude)
    double g;           // 平均近点角(mean anomaly)
    double lambda;      // 視黄経(ecliptic longitude) λ
    double obliq;       // 赤道傾斜角(obliquity) ε
    double alpha;       // 赤経(right ascension) α
    double delta;       // 赤緯(declination) δ
    double equation;    // 均時差(equation of time)
    
    // 時刻 (単位が[rad]であることに注意)
    double t_UTC;   // 協定世界時[rad]   UTC (Coordinated Universal Time)
    double t_LMT;   // 地方平均時[rad]   LMT (Local Mean Time)
    double t_LTST;  // 地方視太陽時[rad] LTST (Local True Solar Time)
    double t_LST;   // 地方恒星時[rad]   LST (Local Sidereal Time)
};

// 単位換算マクロ
#define RAD2DMS(x) SunCalc::rad2dms(x)
#define RAD2HMS(x) SunCalc::rad2hms(x)
#define DMS2RAD(x) SunCalc::dms2rad(x)
#define HMS2RAD(x) SunCalc::hms2rad(x)
#define RAD2DEG(x) ((x) * SunCalc::DEGS)
#define DEG2RAD(x) ((x) * SunCalc::RADS)

// 文字列への変換マクロ
#define DMS2STR(x) SunCalc::dms2str(x)
#define HMS2STR(x) SunCalc::hms2str(x)
