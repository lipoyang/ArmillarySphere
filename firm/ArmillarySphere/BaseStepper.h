#pragma once
#include <stdbool.h>

// 非同期処理版ステッピングモータ基底クラス
class BaseStepper
{
public:
    // コンストラクタ
    BaseStepper(int pol, int spr) : POL(pol), SPR(spr){
        dir = 0;
        n = N = 0;
        pos_now = pos_target = 0;
        T = t0 = tn = 0;
    }
    // デストラクタ
    virtual ~BaseStepper() {}
    
    // モータドライバICごとに実装するメソッド
    virtual void begin()                    = 0; // 初期化
    virtual void takeStep(int dir, int stp) = 0; // ステップ動作
    virtual int getTime()                   = 0; // システム時刻の取得
    
    // 変数の初期化
    void init();
    // 更新(じゅうぶん短い周期で呼ぶ)
    void update();
    // 停止
    void stop();
    // 停止中か？
    bool isIdle();
    
    // ステップ数ベースの位置・速度制御
    void setStepT(int step, int msec);
    void setStepV(int step, int sps);
    void setPosT (int pos,  int msec);
    void setPosV (int pos,  int sps);
    
    // 角度ベースの位置・速度制御
    void rotateT(double deg, double sec);
    void rotateV(double deg, double dps);
    void moveT  (double deg, double sec);
    void moveV  (double deg, double dps);
    
    // 位置の取得
    int getPos();
    int getAngle();

private:
    const int POL;  // 回転の極性(1/-1)
    const int SPR;  // ステップ数/回転
    int pos_now;    // ステップ位置の現在値
    int pos_target; // ステップ位置の目標値
    int dir;        // 回転方向(1/-1)
    int n;          // ステップカウント値
    int N;          // ステップカウントの目標値
    int T;          // 始点から目標位置までの移動時間[usec]
    int t0;         // 始点の時刻[usec]
    int tn;         // 次のステップ時刻[usec]
};
