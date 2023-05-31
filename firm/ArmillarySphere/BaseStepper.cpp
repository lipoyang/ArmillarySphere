#include <stdlib.h>
#include <stdint.h>
#include "BaseStepper.h"

// 変数の初期化
void BaseStepper::init()
{
  pos_now = 0;
  pos_target = 0;
}

// 更新(じゅうぶん短い周期で呼ぶ)
void BaseStepper::update()
{
  if(pos_now != pos_target)
  {
    int now = getTime();
    int elapse = now - t0;
    if(elapse >= tn)
    {
      takeStep((dir * POL > 0) ? 1 : 0, n % 2);
      if((n % 2) == 0) pos_now += dir;
      n++;
      tn = (int)((int64_t)T * (int64_t)n / (int64_t)N);
    }
  }
}

// 停止
void BaseStepper::stop()
{
  pos_target = pos_now;
}

// 停止中か？
bool BaseStepper::isIdle()
{
  return (pos_now == pos_target);
}

// ステップ数だけ回転
// step: ステップ数
// msec: 時間[msec]
void BaseStepper::setStepT(int step, int msec)
{
  this->pos_target = this->pos_now + step;
  this->dir = (step>=0) ? 1 : -1;
  this->n = 1;
  this->N = abs(step) * 2;
  this->T = msec * 1000;
  this->t0 = getTime();
  this->tn = T / N;
}

// ステップ数だけ回転
// step: ステップ数
// sps: 速度[step/sec]
void BaseStepper::setStepV(int step, int sps)
{
  this->pos_target = this->pos_now + step;
  this->dir = (step>=0) ? 1 : -1;
  this->n = 1;
  this->N = abs(step) * 2;
  this->T = (int)((int64_t)abs(step) * 1000000 / (int64_t)sps);
  this->t0 = getTime();
  this->tn = T / N;
}

// 目標位置まで回転
// pos: 目標位置
// msec: 時間[msec]
void BaseStepper::setPosT(int pos, int msec)
{
  this->pos_target = pos;
  int step = this->pos_target - this->pos_now;
  this->dir = (step>=0) ? 1 : -1;
  this->n = 1;
  this->N = abs(step) * 2;
  this->T = msec * 1000;
  this->t0 = getTime();
  this->tn = T / N;
}

// 目標位置まで回転
// pos: 目標位置
// sps: 速度[step/sec]
void BaseStepper::setPosV(int pos, int sps)
{
  this->pos_target = pos;
  int step = this->pos_target - this->pos_now;
  this->dir = (step>=0) ? 1 : -1;
  this->n = 1;
  this->N = abs(step) * 2;
  this->T = (int)((int64_t)abs(step) * 1000000 / (int64_t)sps);
  this->t0 = getTime();
  this->tn = T / N;
}

// 指定角度だけ回転
// deg: 角度[deg]
// sec: 時間[sec]
void BaseStepper::rotateT(double deg, double sec)
{
  int step = SPR * deg / 360;
  setStepT(step, (int)(sec*1000));
}

// 指定角度だけ回転
// deg: 角度[deg]
// dps: 速度[deg/sec]
void BaseStepper::rotateV(double deg, double dps)
{
  int step  = SPR * deg / 360;
  int V     = SPR * dps / 360;
  setStepV(step, V);
}

// 目標角度まで回転
// deg: 角度[deg]
// sec: 時間[sec]
void BaseStepper::moveT(double deg, double sec)
{
  int pos = SPR * deg / 360;
  setPosT(pos, (int)(sec*1000));
}

// 目標角度まで回転
// deg: 角度[deg]
// dps: 速度[deg/sec]
void BaseStepper::moveV(double deg, double dps)
{
  int pos = SPR * deg / 360;
  int V   = SPR * dps / 360;
  setPosV(pos, V);
}


