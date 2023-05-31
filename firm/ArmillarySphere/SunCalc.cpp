// 太陽の位置計算クラス

// このコードは下記のコードを参考にしました
// https://github.com/jarmol/suncalcs/blob/master/rscalc.c
// Jarmo Lammi 1999 - 2001

#define _USE_MATH_DEFINES // for M_PI
#define _CRT_SECURE_NO_WARNINGS // for VC++

#include <stdio.h>
#include <math.h>
#include "SunCalc.h"

//********** 定数 **********
const double PI   = M_PI;
const double SunCalc::DEGS = 180.0 / M_PI;
const double SunCalc::RADS = M_PI / 180.0;

//********** 公開メソッド **********

// UTC日時を設定する
// year/month/day : 日付
// hour:min/sec : 時刻
void SunCalc::setDate(int year, int month, int day, int hour, int min, int sec)
{
    this->year  = year;
    this->month = month;
    this->day   = day;
    this->hour  = hour;
    this->min   = min;
    this->sec   = sec;
}

// 観測地点の位置を設定する
// latitude:  緯度
// longitude: 経度
void SunCalc::setLocation(double latitude, double longitude)
{
    this->lat = latitude * RADS;
    this->lon = longitude * RADS;
}

// 観測地点の位置を設定する
// latitude:  緯度
// longitude: 経度
void SunCalc::setLocation(DegMinSec latitude, DegMinSec longitude)
{
    this->lat = dms2rad(latitude);
    this->lon = dms2rad(longitude);
}

// 太陽位置を計算する
void SunCalc::calc()
{
    // UTC日時をJ2000通日に換算
    d = J2000(year, month, day, hour, min, sec);
    // 太陽黄経λ[rad]
    lambda = ecliptic_longitude(d, L, g);
    // 赤道傾斜角ε[rad]
    obliq = 23.439 * RADS - .0000004 * RADS * d;
    // 太陽赤経α[rad] と 太陽赤緯δ[rad]
    alpha = atan2(cos(obliq) * sin(lambda), cos(lambda));
    delta =  asin(sin(obliq) * sin(lambda));
    alpha = rad_0to2PI(alpha);
    // 均時差[rad]
    equation = alpha - L;

    // UTC時刻[rad]
    HourMinSec hms_utc = HourMinSec(hour, min, (double)sec);
    t_UTC  = hms2rad(hms_utc);
    // 地方平均時[rad]
    t_LMT  = rad_0to2PI(t_UTC + lon);
    // 地方視太陽時[rad]
    t_LTST = rad_0to2PI(t_LMT - equation);
    // 地方恒星時[rad]
    t_LST  = rad_0to2PI(t_LTST + PI + alpha);
}

//********** 静的メソッド **********

// UTCの日時をJ2000通日に換算する
// (閏年の都合上、適用範囲はグレゴリオ暦1901～2099年)
// year  : 年 (1901～2099)
// month : 月 (1～12)
// day   : 日 (1～28/29/30/31)
// hour:min:sec : UTCでの時刻 (時:分:秒)
// return : J2000通日
double SunCalc::J2000(int year, int month, int day, int hour, int min, int sec)
{
    int i_j2000 =
        367 * year
        - 7 * (year + (month + 9) / 12) / 4
        + 275 * month / 9
        + day;
    double f_j2000 = 
        (double)i_j2000
        + ((double)hour + ((double)min)/60.0 + ((double)sec)/3600.0) / 24.0
        - 730531.5;
    return f_j2000;
}

// 太陽黄経を求める
// d: J2000通日
// L: 平均黄経(mean longitude)[rad]
// g: 平均近点角(mean anomaly)[rad]
// return: 太陽黄経[rad]
double SunCalc::ecliptic_longitude(double d, double &L, double &g)
{
    // 平均黄経 (mean longitude)
    L = rad_0to2PI(280.460 * RADS + .9856474 * RADS * d);
    // 平均近点角 (mean anomaly)
    g = rad_0to2PI(357.528 * RADS + .9856003 * RADS * d);
    // 視黄経 (true longitude)
    return rad_0to2PI(L + 1.915 * RADS * sin(g) + .02 * RADS * sin(2 * g));
}

// 角度[rad]を0～2πの範囲の値に変換する
// x: 角度[rad]
// return: 変換後の角度[rad]
double SunCalc::rad_0to2PI(double x)
{
    x = fmod(x, 2.0*PI);
    if (x < 0) x += 2.0*PI;
    return x;
}

// ラジアンを度°分′秒″に換算
// rad: ラジアン
// return: 度°分′秒″
DegMinSec  SunCalc::rad2dms(double rad)
{
    DegMinSec x;
    x.sign = (rad >= 0) ? 1 : -1;
    rad = fabs(rad);
    
    double f_deg = RAD2DEG(rad);
    x.deg = (int)f_deg;
    double f_min = (f_deg - (double)(x.deg)) * 60.0;
    x.min = (int)f_min;
    x.sec = (f_min - (double)(x.min)) * 60.0;
    return x;
}

// ラジアンを時:分:秒に換算
// rad: ラジアン
// return: 時:分:秒
HourMinSec SunCalc::rad2hms(double rad)
{
    HourMinSec x;
    x.sign = (rad >= 0) ? 1 : -1;
    rad = fabs(rad);

    double f_hour = rad * 12.0 / PI;
    x.hour = (int)f_hour;
    double f_min = (f_hour - (double)x.hour) * 60.0;
    x.min = (int)f_min;
    x.sec = (f_min - (double)x.min) * 60.0;
    return x;
}

// 度°分′秒″をラジアンに換算
// dms: 度°分′秒″
// return: ラジアン
double SunCalc::dms2rad(DegMinSec  dms)
{
    double f_deg = (double)dms.deg;
    f_deg += (double)dms.min / 60.0;
    f_deg += (double)dms.sec / 3600.0;

    if (dms.sign < 0) f_deg = -f_deg;
    double rad = DEG2RAD(f_deg);
    return rad;
}

// 時:分:秒をラジアンに換算
// hms: 時:分:秒
// return: ラジアン
double SunCalc::hms2rad(HourMinSec hms)
{
    double f_hour = (double)hms.hour;
    f_hour += (double)hms.min / 60.0;
    f_hour += (double)hms.sec / 3600.0;
    
    if (hms.sign < 0) f_hour = -f_hour;
    double rad = f_hour * PI / 12.0;
    return rad;
}

// 度°分′秒″を文字列に変換する
// dms: 度°分′秒″
// return: 文字列
// ※この関数はスレッドセーフでない
char* SunCalc::dms2str(DegMinSec  dms)
{
    int d = dms.deg;
    int m = dms.min;
    double s = dms.sec;
    if ((int)(s * 1000) % 6000 >= 5995) {
        s = 0;
        if (++m == 60) {
            m = 0;
            if (++d == 360) d = 0;
        }
    }
    static char buff[16];
    sprintf(buff, "%c%3d,%02d,%05.2f",
        (dms.sign) > 0 ? ' ' : '-', d, m, s);
    return buff;
}

// 時:分:秒を文字列に変換する
// hms: 時:分:秒
// return: 文字列
// ※この関数はスレッドセーフでない
char* SunCalc::hms2str(HourMinSec hms)
{
    int h = hms.hour;
    int m = hms.min;
    double s = hms.sec;
    if ((int)(s * 1000) % 6000 >= 5995) {
        s = 0;
        if (++m == 60) {
            m = 0;
            if (++h == 24) h = 0;
        }
    }
    static char buff[16];
    sprintf(buff, "%c%2d:%02d:%05.2f",
        (hms.sign) > 0 ? ' ' : '-', h, m, s);
    return buff;
}
