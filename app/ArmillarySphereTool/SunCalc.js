// 太陽の位置計算クラス

// このコードは下記のコードを参考にしました
// https://github.com/jarmol/suncalcs/blob/master/rscalc.c
// Jarmo Lammi 1999 - 2001

// 定数
const PI   = Math.PI;
const DEGS = 180.0 / Math.PI;
const RADS = Math.PI / 180.0;

// 太陽の位置計算クラス
function SunCalc()
{
    return this;
}

//********** メソッド **********

// UTC日時を設定する
SunCalc.prototype.setDate = function(year, month, day, hour, min, sec)
{
    // UTC日時
    this.year  = year;  // 年
    this.month = month; // 月
    this.day   = day;   // 日
    this.hour  = hour;  // 時
    this.min   = min;   // 分
    this.sec   = sec;   // 秒
};

// 観測地点の位置を設定する
SunCalc.prototype.setLocation = function(latitude, longitude)
{
    // 地球上の観測地点の位置 (単位が[rad]であることに注意)
    this.lat = latitude  * RADS;    // 緯度[rad]
    this.lon = longitude * RADS;    // 経度[rad]
};

// 太陽位置を計算する
SunCalc.prototype.calc = function()
{
    // UTC日時
    const year  = this.year;  // 年
    const month = this.month; // 月
    const day   = this.day;   // 日
    const hour  = this.hour;  // 時
    const min   = this.min;   // 分
    const sec   = this.sec;   // 秒
    // 地球上の観測地点の位置
    const lat = this.lat;    // 緯度[rad]
    const lon = this.lon;    // 経度[rad]

    // UTC日時をJ2000通日に換算
    const d = J2000(year, month, day, hour, min, sec);
    // 太陽黄経λ[rad]
    const [lambda, L, g] = ecliptic_longitude(d);
    // 赤道傾斜角ε[rad]
    const obliq = 23.439 * RADS - .0000004 * RADS * d;
    // 太陽赤経α[rad] と 太陽赤緯δ[rad]
    const alpha = rad_0to2PI( Math.atan2(Math.cos(obliq) * Math.sin(lambda), Math.cos(lambda)));
    const delta =  Math.asin(Math.sin(obliq) * Math.sin(lambda));
    // 均時差[rad]
    const equation = alpha - L;

    // UTC時刻[rad]
    const t_UTC  = hms2rad(hour, min, sec);
    // 地方平均時[rad]
    const t_LMT  = rad_0to2PI(t_UTC + lon);
    // 地方視太陽時[rad]
    const t_LTST = rad_0to2PI(t_LMT - equation);
    // 地方恒星時[rad]
    const t_LST  = rad_0to2PI(t_LTST + PI + alpha);

    // 太陽の位置 (すべて単位が[rad]であることに注意)
    this.L = L;                 // 平均黄経(mean longitude)
    this.g = g;                 // 平均近点角(mean anomaly)
    this.lambda = lambda;       // 視黄経(ecliptic longitude) λ
    this.obliq  = obliq;        // 赤道傾斜角(obliquity) ε
    this.alpha  = alpha;        // 赤経(right ascension) α
    this.delta  = delta;        // 赤緯(declination) δ
    this.equation = equation;   // 均時差(equation of time)
    
    // 時刻 (単位が[rad]であることに注意)
    this.t_UTC  = t_UTC;        // 協定世界時[rad]   UTC (Coordinated Universal Time)
    this.t_LMT  = t_LMT;        // 地方平均時[rad]   LMT (Local Mean Time)
    this.t_LTST = t_LTST;       // 地方視太陽時[rad] LTST (Local True Solar Time)
    this.t_LST  = t_LST;        // 地方恒星時[rad]   LST (Local Sidereal Time)
};

//********** サブルーチン **********

// UTCの日時をJ2000通日に換算する
// (閏年の都合上、適用範囲はグレゴリオ暦1901～2099年)
// year  : 年 (1901～2099)
// month : 月 (1～12)
// day   : 日 (1～28/29/30/31)
// hour:min:sec : UTCでの時刻 (時:分:秒)
// return : J2000通日
function J2000(year, month, day, hour, min, sec)
{
    const i_j2000 =
        367 * year
        - Math.floor(7 * (year + Math.floor((month + 9) / 12)) / 4)
        + Math.floor(275 * month / 9)
        + day;
    const f_j2000 = 
        i_j2000
        + (hour + min/60.0 + sec/3600.0) / 24.0
        - 730531.5;
    return f_j2000;
}

// 太陽黄経を求める
// d: J2000通日
// L: 平均黄経(mean longitude)[rad]
// g: 平均近点角(mean anomaly)[rad]
// return: 太陽黄経[rad]
function ecliptic_longitude(d)
{
    // 平均黄経 (mean longitude)
    const L = rad_0to2PI(280.460 * RADS + .9856474 * RADS * d);
    // 平均近点角 (mean anomaly)
    const g = rad_0to2PI(357.528 * RADS + .9856003 * RADS * d);
    // 視黄経 (true longitude)
    const lambda =  rad_0to2PI(L + 1.915 * RADS * Math.sin(g) + .02 * RADS * Math.sin(2 * g));

    return [lambda, L, g];
}

// 角度[rad]を0～2πの範囲の値に変換する
// x: 角度[rad]
// return: 変換後の角度[rad]
function rad_0to2PI(x)
{
    x = x % (2.0*PI);
    if (x < 0) x += 2.0*PI;
    return x;
}

// 時:分:秒をラジアンに換算
// hms: 時:分:秒
// return: ラジアン
function hms2rad(hour, min, sec)
{
    const f_hour = hour + min/60.0 + sec/3600.0;
    const rad = f_hour * PI / 12.0;
    return rad;
}
