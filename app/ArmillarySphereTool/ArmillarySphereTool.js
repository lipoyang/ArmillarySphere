/********** UIの要素 ***********/
// 表示領域
const panel_connect = document.getElementById('panel_connect');
const panel_main    = document.getElementById('panel_main');
// タブ
const tab_season  = document.getElementById('tab_season');  // 季節
const tab_time    = document.getElementById('tab_time');    // 時刻
const tab_setting = document.getElementById('tab_setting'); // 設定
// キャンバスとコンテキスト
const canvasSeason = document.getElementById('canvasSeason'); // 季節
const canvasTime   = document.getElementById('canvasTime');   // 時刻
const contextSeason = canvasSeason.getContext('2d');
const contextTime   = canvasTime.getContext('2d');
// 接続時のメッセージ
const text_connect = document.getElementById('text_connect');
// 日時の設定/表示
const text_datetime = document.getElementById('text_datetime');
// タイムゾーン
const select_timezone = document.getElementById('select_timezone');
const text_timezone   = document.getElementById('text_timezone');
// 緯度・経度
const select_latitude  = document.getElementById('select_latitude');
const number_latitude  = document.getElementById('number_latitude');
const select_longitude = document.getElementById('select_longitude');
const number_longitude = document.getElementById('number_longitude');
const text_location    = document.getElementById('text_location');
// ボタン
const btn_connect    = document.getElementById('btn_connect');
const btn_disconnect = document.getElementById('btn_disconnect');
const btn_init       = document.getElementById('btn_init');
const btn_stop       = document.getElementById('btn_stop');
const btn_rotation   = document.getElementById('btn_rotation');
const btn_revolution = document.getElementById('btn_revolution');
const btn_demo1      = document.getElementById('btn_demo1');
const btn_demo2      = document.getElementById('btn_demo2');
// 太陽の位置
const text_lambda = document.getElementById('text_lambda');
const text_alpha  = document.getElementById('text_alpha');
const text_delta  = document.getElementById('text_delta');

/********** BLEの定数 ***********/

// BLEサービスのUUID
const UUID_ArmillarySphere   = "220eb65b-d64d-e553-f51e-a1048818dc96";
// BLEキャラクタリスティックのUUID
const UUID_Command    = "2d3f5dde-42b2-fbfa-3e3f-7673832c7db4"; // コマンド
const UUID_LonTime    = "328e1678-11b5-5d34-aad2-ef1a4a2957eb"; // 経度とUTC日時を設定
const UUID_Busy       = "31180989-2299-a86f-c856-b2154171c07b"; // ビジー状態か？
// コマンド
const CMD_INIT       = 0x80; // 初期位置
const CMD_STOP       = 0x81; // 停止
const CMD_ROTATION   = 0x82; // 自転
const CMD_REVOLUTOIN = 0x83; // 公転
const CMD_DEMO1      = 0x84; // デモ1
const CMD_DEMO2      = 0x85; // デモ2

/********** BLEの変数 ***********/
// BLEデバイス
let bleDevice = null;
// BLEキャラクタリスティック
let chrCommand;    // コマンド
let chrLonTime;    // 経度とUTC日時の設定
let chrBusy;       // ビジー状態か？

let last_time = 0;    // 前回送信時刻
let pending = false;  // 送信保留フラグ
const SEND_INTERVAL = 50; // [msec] これ未満の間隔での送信は保留する
const SEND_DELAY    = 50; // [msec] これだけ待って再送信する  

/********** その他の定数・変数 ***********/

// 色
const MY_BLUE       = '#0d6efd';
const MY_LIGHT_BLUE = '#d5e6ff';
const MY_RED        = '#dc3545';

// キャンバス
let canvas_R = 0;             // 内接円の半径
let canvas_isTouched = false; // タッチ中フラグ

/********** 初期化処理 ***********/

// 読み込み時の処理
window.onload = function() {

  // キャンバスサイスの調整
  const divSeason = document.getElementById('tab_content1'); // 季節キャンバスのラッパ
  const divTime   = document.getElementById('tab_content2'); // 時刻キャンバスのラッパ
  // 季節キャンバス
  const canvas_size = divSeason.offsetWidth;
  divSeason.style.height = canvas_size + 'px';
  canvasSeason.width = canvas_size;
  canvasSeason.height = canvas_size;
  // 時刻キャンバス
  divTime.style.height = canvas_size + 'px';
  canvasTime.width = canvas_size;
  canvasTime.height = canvas_size;
  // キャンバスの内接円の半径
  canvas_R = canvasSeason.width / 2;

  // 日時の設定
  text_datetime.value = "2026-03-20T12:00";

  // 季節キャンバスの初期表示
  draw_season();
  draw_sun(0 * Math.PI); // 春分
  // 時刻キャンバスの初期表示
  draw_time();
  draw_clock(1 * Math.PI); // 正午

  // タイムゾーン
  const timezone = 9; // UTC+9 (JST)
  select_timezone.options[12 - timezone].selected = true;
  show_timezone();
  // 緯度・経度
  const north_south = 0; // 北緯
  select_latitude.options[north_south].selected = true;
  number_latitude.value = '35.6894';
  const east_west = 0; // 東経
  select_longitude.options[east_west].selected = true;
  number_longitude.value = '139.6917';
  show_location();
  // 太陽の位置の計算
  calc_sun_pos();
}

/********** UIのイベントハンドラ ***********/

// 季節キャンバス(ホロスコープ)のマウス/タッチイベント
canvasSeason.addEventListener("mousedown", function (e) {
  canvas_isTouched = true;
  canvasSeason_zodiac(e);
}); 
canvasSeason.addEventListener("mouseup", function (e) {
  canvas_isTouched = false;
  canvasSeason_zodiac(e);
  canvasSeason_button(e);
}); 
canvasSeason.addEventListener("mousemove", function (e) {
  if(canvas_isTouched){
    canvasSeason_zodiac(e);
  }
}); 
canvasSeason.addEventListener("touchstart",  function (e) {
  touch2mouse(e);
  canvas_isTouched = true;
  canvasSeason_zodiac(e);
}); 
canvasSeason.addEventListener("touchend" ,  function (e) {
  touch2mouse(e);
  canvas_isTouched = false;
  canvasSeason_zodiac(e);
  canvasSeason_button(e);
}); 
canvasSeason.addEventListener("touchmove",  function (e) {
  touch2mouse(e);
  if(canvas_isTouched){
    canvasSeason_zodiac(e);
  }
});

// 時刻キャンバス(時計)のマウス/タッチイベント
canvasTime.addEventListener("mousedown", function (e) {
  canvas_isTouched = true;
  canvasTime_clock(e);
}); 
canvasTime.addEventListener("mouseup", function (e) {
  canvas_isTouched = false;
  canvasTime_clock(e);
  canvasTime_button(e);
}); 
canvasTime.addEventListener("mousemove", function (e) {
  if(canvas_isTouched){
    canvasTime_clock(e);
  }
}); 
canvasTime.addEventListener("touchstart",  function (e) {
  touch2mouse(e);
  canvas_isTouched = true;
  canvasTime_clock(e);
}); 
canvasTime.addEventListener("touchend" ,  function (e) {
  touch2mouse(e);
  canvas_isTouched = false;
  canvasTime_clock(e);
  canvasTime_button(e);
}); 
canvasTime.addEventListener("touchmove",  function (e) {
  touch2mouse(e);
  if(canvas_isTouched){
    canvasTime_clock(e);
  }
}); 

// タブ:季節 が開いたとき
tab_season.addEventListener('shown.bs.tab', function (e) {
    panel_date_time.style.display = "block";
    panel_sun_position.style.display = "block";
});
// タブ:時刻 が開いたとき
tab_time.addEventListener('shown.bs.tab', function (e) {
  panel_date_time.style.display = "block";
  panel_sun_position.style.display = "block";
});
// タブ:設定 が開いたとき
tab_setting.addEventListener('shown.bs.tab', function (e) {
  panel_date_time.style.display = "none";
  panel_sun_position.style.display = "none";
});

// 日時が変更されたとき
text_datetime.addEventListener('change', function (e) {
  datetime_onchanged();
});

// タイムゾーンが変更されたとき
select_timezone.addEventListener('change', function (e) {
  show_timezone();
  calc_sun_pos();
});
// 経度・緯度が変更されたとき
select_latitude.addEventListener('change', function (e) {
  show_location();
  calc_sun_pos();
});
select_longitude.addEventListener('change', function (e) {
  show_location();
  calc_sun_pos();
});
number_latitude.addEventListener('change', function (e) {
  show_location();
  calc_sun_pos();
});
number_longitude.addEventListener('change', function (e) {
  show_location();
  calc_sun_pos();
});

// 接続ボタン
btn_connect.addEventListener('click', async function () {
  try {
    // デバイスを取得 (サービスのUUIDでフィルタ)
    console.log("Requesting Bluetooth Device...");
    bleDevice = await navigator.bluetooth.requestDevice({
        filters: [{ services: [UUID_ArmillarySphere] }],
    });
    // 切断時イベントハンドラの登録
    bleDevice.addEventListener('gattserverdisconnected', onDisconnected);
    // デバイスに接続
    text_connect.innerText = "接続中...";
    console.log("Connecting to GATT Server...");
    const server = await bleDevice.gatt.connect();
    // サービスを取得
    text_connect.innerText = "デバイス情報取得中...";
    console.log("Getting Service...");
    const service = await server.getPrimaryService(UUID_ArmillarySphere);
    // キャラクタリスティックを取得
    console.log("Getting Characteristics...");
    chrCommand   = await service.getCharacteristic(UUID_Command);
    chrLonTime   = await service.getCharacteristic(UUID_LonTime);
    chrBusy      = await service.getCharacteristic(UUID_Busy);
    // 画面表示切替
    panel_connect.style.display = "none";
    panel_main.style.display = "block";

  } catch (error) {
    error_toast(error);
    bleDevice = null;
  }
});

// 切断ボタン
btn_disconnect.addEventListener('click', async function () {
  if(bleDevice != null){
    await bleDevice.gatt.disconnect();
    bleDevice = null; // TODO
  }  
});

// 初期位置ボタン
btn_init.addEventListener('click', async function () {
  const busy = await readBusy();
  if(!busy) sendCommand(CMD_INIT);
});

// 停止ボタン
btn_stop.addEventListener('click', async function () {
  sendCommand(CMD_STOP);
});

// 自転ボタン
btn_rotation.addEventListener('click', async function () {
  const busy = await readBusy();
  if(!busy) sendCommand(CMD_ROTATION);
});

// 公転ボタン
btn_revolution.addEventListener('click', async function () {
  const busy = await readBusy();
  if(!busy) sendCommand(CMD_REVOLUTOIN);
});

// デモ1ボタン
btn_demo1.addEventListener('click', async function () {
  const busy = await readBusy();
  if(!busy) sendCommand(CMD_DEMO1);
});

// デモ2ボタン
btn_demo2.addEventListener('click', async function () {
  const busy = await readBusy();
  if(!busy) sendCommand(CMD_DEMO2);
});

/********** BLEのイベントハンドラ ***********/
// 切断時
function onDisconnected(event) {
  const device = event.target;
  console.log(`Device ${device.name} is disconnected.`);
  bleDevice = null;
  // 画面表示切替
  text_connect.innerText = "";
  panel_main.style.display = "none";
  panel_connect.style.display = "block";
}

/********** キャンバスの描画処理 ***********/

// 円の線と塗りつぶし
function drawCircle(gc, x, y, r, fillColor, strokeColor, width){
  gc.beginPath();
  gc.arc(x, y, r, 0, 2 * Math.PI, false);
  if(fillColor != null){
    gc.fillStyle = fillColor;
    gc.fill();
  }
  if(strokeColor != null){
    gc.strokeStyle = strokeColor;
    gc.lineWidth = width;
    gc.stroke();
  }
}
// 円の塗りつぶし
function fillCircle(gc, x, y, r, color){
  drawCircle(gc, x, y, r, color, null, null);
}
// 円の線
function strokeCircle(gc, x, y, r, color, width){
  drawCircle(gc, x, y, r, null, color, width);
}
// 直線の描画
function drawLine(gc, x1, y1, x2, y2, color, width)
{
  gc.strokeStyle = color;
  gc.lineWidth = width;
  gc.beginPath();
  gc.moveTo(x1, y1);
  gc.lineTo(x2, y2);
  gc.stroke();
}
// 文字描画の設定
function setFont(gc, font, color){
  gc.font = font;
  gc.fillStyle = color;
  gc.textAlign = "center";
  gc.textBaseline = "middle";
}

// 季節のグラフィックの表示 (ホロスコープ)
function draw_season()
{
  // キャンバスの中心を原点にする
  const gc = contextSeason;
  gc.translate(canvas_R, canvas_R);

  // 円の半径
  const r1 = canvas_R * 0.7;    // 黄道の外枠
  const r2 = canvas_R * 0.5;    // 黄道の内枠
  const r3 = canvas_R * 0.83;   // 春夏秋冬のボタンの中心
  const r4 = canvas_R * 0.1;    // 春夏秋冬のボタンの半径
  const r5 = canvas_R * 0.6;    // 十二宮のシンボルの中心

  // 十二宮の記号
  const zodiac = ["♈","♉","♊","♋","♌","♍","♎","♏","♐","♑","♒","♓"];

  // 黄道の円を描画
  drawCircle(gc, 0, 0, r1, MY_LIGHT_BLUE, MY_BLUE, 1);
  drawCircle(gc, 0, 0, r2, "white",       MY_BLUE, 1);

  // 春夏秋冬のボタンを描画
  fillCircle(gc,  r3,   0, r4, MY_BLUE);
  fillCircle(gc, -r3,   0, r4, MY_BLUE);
  fillCircle(gc,   0,  r3, r4, MY_BLUE);
  fillCircle(gc,   0, -r3, r4, MY_BLUE);

  // 春夏秋冬の文字を描画
  setFont(gc, "1.2em sans-serif", "white");
  gc.fillText( "春", -r3,   0 );
  gc.fillText( "夏",   0,  r3 );
  gc.fillText( "秋",  r3,   0 );
  gc.fillText( "冬",   0, -r3 );

  // 黄道十二宮の目盛りとシンボルを描画
  setFont(gc, "1.5em sans-serif", MY_BLUE);
  for(let i = 0; i < 12; i++){
    // 目盛り
    const rad = i * Math.PI / 6;
    const cos = Math.cos(rad);
    const sin = Math.sin(rad);
    drawLine(gc, -r1 * cos, r1 * sin, -r2 * cos, r2 * sin, MY_BLUE, 1);
    // シンボル
    const rad2 = (i + 0.5) * Math.PI / 6;
    const cos2 = Math.cos(rad2);
    const sin2 = Math.sin(rad2);
    gc.fillText( zodiac[i], -r5 * cos2 , r5 * sin2);
  }
}

// 太陽の表示
function draw_sun(rad)
{
  const gc = contextSeason;

  // 円の半径
  const r2 = canvas_R * 0.49;   // 黄道の内枠 - わずかに余白
  const r6 = canvas_R * 0.4;    // 太陽のシンボルの中心
  const r7 = canvas_R * 0.08;   // 太陽のシンボルの半径
  const r8 = canvas_R * 0.02;   // 太陽のシンボルの中心点の半径

  // 黄道の内側を塗りつぶす
  fillCircle(gc, 0, 0, r2, "white");

  // 太陽の描画
  const x = -r6 * Math.cos(rad);
  const y =  r6 * Math.sin(rad);
  strokeCircle(gc, x, y, r7, MY_RED, 2);
  fillCircle  (gc, x, y, r8, MY_RED);
}

// 時刻のグラフィックの表示 (時計)
function draw_time()
{
  // キャンバスの中心を原点にする
  const gc = contextTime;
  gc.translate(canvas_R, canvas_R);

  // 円の半径
  const r1 = canvas_R * 0.7;    // 時計の外枠
  const r2 = canvas_R * 0.65;   // 時計の内枠
  const r3 = canvas_R * 0.83;   // 東西南北のボタンの中心
  const r4 = canvas_R * 0.1;    // 東西南北のボタンの半径
  const r5 = canvas_R * 0.63;   // 目盛りの外側
  const r6 = canvas_R * 0.6;    // 目盛りの内側
  const r7 = canvas_R * 0.52;   // 数字の中心

  // 時計の円を描画
  fillCircle(gc, 0, 0, r1, MY_BLUE);
  fillCircle(gc, 0, 0, r2, "white");

  // 春夏秋冬のボタンを描画
  fillCircle(gc,  r3,   0, r4, MY_BLUE);
  fillCircle(gc, -r3,   0, r4, MY_BLUE);
  fillCircle(gc,   0,  r3, r4, MY_BLUE);
  fillCircle(gc,   0, -r3, r4, MY_BLUE);

  // 春夏秋冬の文字を描画
  setFont(gc, "1.2em sans-serif", "white");
  gc.fillText( "西", -r3,   0 );
  gc.fillText( "南",   0,  r3 );
  gc.fillText( "東",  r3,   0 );
  gc.fillText( "北",   0, -r3 );

  // 時刻の目盛りと数字を描画
  setFont(gc, "1.2em sans-serif", MY_BLUE);
  for(let i = 0; i < 24; i++){
    // 目盛り
    const rad = i * Math.PI / 12;
    const cos = Math.cos(rad);
    const sin = Math.sin(rad);
    drawLine(gc, r5 * sin, -r5 * cos, r6 * sin, -r6 * cos, MY_BLUE, 2);
    // 数字
    if(i % 2 == 0){
      gc.fillText( i.toString(), r7 * sin , -r7 * cos);
    }
  }
}

// 時計の針の表示
function draw_clock(rad)
{
  const gc = contextTime;

  // 円の半径
  const r7 = canvas_R * 0.46;   // 文字盤の内側
  const r8 = canvas_R * 0.45;   // 針の外側
  const r9 = canvas_R * 0.1;    // 針の内側
  const r10 = canvas_R * 0.05;  // 時計の中心

  // 文字盤の内側を塗りつぶす
  fillCircle(gc, 0, 0, r7, "white");

  // 針の描画
  const x1 =  r8 * Math.sin(rad);
  const y1 = -r8 * Math.cos(rad);
  const x2 = -r9 * Math.sin(rad);
  const y2 =  r9 * Math.cos(rad);
  drawLine(gc, x1, y1, x2, y2, MY_RED, 4);
  fillCircle(gc, 0, 0, r10, MY_BLUE);
}

/********** キャンバスのイベント処理 ***********/

// タッチイベントをマウスイベントに変換
function touch2mouse(e){
  e.preventDefault(); // デフォルトイベントをキャンセル
  if(e.touches.length > 1) return; // マルチタッチ非対応

  const bcr = e.target.getBoundingClientRect();
  e.offsetX = e.changedTouches[0].clientX - bcr.x;
  e.offsetY = e.changedTouches[0].clientY - bcr.y;
}

// 季節表示(ホロスコープ)の黄道判定
function canvasSeason_zodiac(e) {

  const x = e.offsetX - canvas_R;
  const y = e.offsetY - canvas_R;
  
  const r1 = canvas_R * 0.7;    // 黄道の外枠
  const r2 = canvas_R * 0.3;    // 太陽の内側

  // 黄道の範囲判定
  const rr = x*x + y*y;
  if(rr > r1*r1) return;
  if(rr < r2*r2) return;

  const th = Math.atan2(y, -x);

  // 太陽を表示
  draw_sun(th);
  // 角度から日付を表示
  theta_to_date(th);
  // 太陽の位置の計算
  calc_sun_pos();
  // 渾天儀に指令
  moveArmillarySphere();
}

// 季節表示(ホロスコープ)の春夏秋冬ボタン判定
function canvasSeason_button(e) {

  const x = e.offsetX - canvas_R;
  const y = e.offsetY - canvas_R;

  const r3 = canvas_R * 0.83;   // 春夏秋冬のボタンの中心
  const r4 = canvas_R * 0.1;    // 春夏秋冬のボタンの半径

  // 春夏秋冬ボタンの範囲判定
  const X = [-r3,   0,  r3,   0];
  const Y = [  0,  r3,   0, -r3];
  for(let i = 0; i < 4; i++){
    const dx = x - X[i];
    const dy = y - Y[i];
    if(dx*dx + dy*dy <= r4*r4){
      if(i == 0) set_date_spring_equinox();   // 春分
      if(i == 1) set_date_summer_solstice();  // 夏至
      if(i == 2) set_date_autumn_equinox();   // 秋分
      if(i == 3) set_date_winter_solstice();  // 冬至
      datetime_onchanged();
/*
      const th = i * Math.PI / 2;
      // 太陽を表示
      draw_sun(th);
      // 角度から日付を表示
      theta_to_date(th);
*/
    }
  }
}

// 時刻表示(時計)の時計判定
function canvasTime_clock(e) {

  const x = e.offsetX - canvas_R;
  const y = e.offsetY - canvas_R;
  
  const r1 = canvas_R * 0.7;    // 時計の外枠
  const r8 = canvas_R * 0.3;    // 針の外側より少し内側

  // 黄道の範囲判定
  const rr = x*x + y*y;
  if(rr > r1*r1) return;
  if(rr < r8*r8) return;

  const th = Math.atan2(x, -y);

  // 時計の針を表示
  draw_clock(th);
  // 角度から時刻を設定
  theta_to_time(th);
  // 太陽の位置の計算
  calc_sun_pos();
  // 渾天儀に指令
  moveArmillarySphere();
}

// 時刻表示(時計)の東西南北ボタン判定
function canvasTime_button(e) {

  const x = e.offsetX - canvas_R;
  const y = e.offsetY - canvas_R;

  const r3 = canvas_R * 0.83;   // 東西南北のボタンの中心
  const r4 = canvas_R * 0.1;    // 東西南北のボタンの半径

  // 東西南北ボタンの範囲判定
  const X = [  0,  r3,   0, -r3];
  const Y = [-r3,   0,  r3,   0];
  for(let i = 0; i < 4; i++){
    const dx = x - X[i];
    const dy = y - Y[i];
    if(dx*dx + dy*dy <= r4*r4){
      const th = i * Math.PI / 2;
      // 時計の針を表示
      draw_clock(th);
      // 角度から時刻を設定
      theta_to_time(th);
      // 太陽の位置の計算
      calc_sun_pos();
      // 渾天儀に指令
      moveArmillarySphere();
      return;
    }
  }
}

/********** 日時の処理 ***********/

// 日時が変更されたときの表示更新
function datetime_onchanged()
{
  let datetime = datetime_get();
  const year  = datetime.year;
  const month = datetime.month;
  const day   = datetime.day;
  const hour  = datetime.hour;
  const min   = datetime.min;
  if(year < 1901) return;
  if(year > 2099) return;

  // 太陽を表示
  const from = new Date(year, 0, 1); // その年の元旦
  const to   = new Date(year, month-1, day);
  const ms = to.getTime() - from.getTime();
  const d = Math.floor(ms / (1000*60*60*24)) - calc_days_spring_equinox(year);
  const th1 = d * 2 * Math.PI / calc_days_in_year(year);
  draw_sun(th1);

  // 時計の針を表示
  const t = hour * 60 + min;
  const th2 = t * 2 * Math.PI / (24 * 60);
  draw_clock(th2);

  // 太陽の位置の計算
  calc_sun_pos();
  // 渾天儀に指令
  moveArmillarySphere();
}

// 日時の取得
function datetime_get() {
  let sDateTime = text_datetime.value;
  let datetime = new Object();
  datetime.year  = parseInt(sDateTime.slice(0,4));
  datetime.month = parseInt(sDateTime.slice(5,7));
  datetime.day   = parseInt(sDateTime.slice(8,10));
  datetime.hour  = parseInt(sDateTime.slice(11,13));
  datetime.min   = parseInt(sDateTime.slice(14,16));
  return datetime;
}

// 日時の設定
function datetime_set(datetime) {
  const sYear  = ( '0000' + datetime.year).slice( -4 );
  const sMonth = ( '00' + datetime.month ).slice( -2 );
  const sDay   = ( '00' + datetime.day   ).slice( -2 );
  const sHour  = ( '00' + datetime.hour  ).slice( -2 );
  const sMin   = ( '00' + datetime.min   ).slice( -2 );
  const sDateTime = sYear + "-" + sMonth + "-" + sDay + "T" + sHour + ":" + sMin;
  text_datetime.value = sDateTime;
}

// UTC時刻の取得
function utc_datetime_get(local_datetime, timezone)
{
  const local_ms = new Date(
    local_datetime.year,
    local_datetime.month - 1,
    local_datetime.day,
    local_datetime.hour,
    local_datetime.min).getTime();
  
  const utc_ms = local_ms - (timezone * 60 * 60 * 1000);
  const utc = new Date(utc_ms);
  
  let utc_datetime = new Object();
  utc_datetime.year  = utc.getFullYear();
  utc_datetime.month = utc.getMonth() + 1;
  utc_datetime.day   = utc.getDate();
  utc_datetime.hour  = utc.getHours();
  utc_datetime.min   = utc.getMinutes();
  
  return utc_datetime;
}

// 角度から時刻を設定
function theta_to_time(th)
{
  if(th < 0) th += 2 * Math.PI;

  const t = Math.floor(th * (24 * 60) / (2 * Math.PI)); // 正子からの通算分数
  const h = Math.floor(t / 60);
  const m = t % 60;

  let datetime = datetime_get();
  datetime.hour = h;
  datetime.min  = m;
  datetime_set(datetime);
}

// 角度から日付を設定
function theta_to_date(th)
{
  if(th < 0) th += 2 * Math.PI;

  let datetime = datetime_get();
  
  const year = datetime.year;
  const days_in_year = calc_days_in_year(year); // その年の日数
  const spring_day = calc_days_spring_equinox(year); // その年の春分の日が元旦から何日後かを計算

  let d = Math.floor(th * days_in_year / (2 * Math.PI)); // 春分からの日数
  d = (d + spring_day) % days_in_year; // 元旦からの日数

  const new_year = new Date(year, 0, 1); // その年の元旦
  const one_day  = 24 * 60 * 60 * 1000; // 1日のミリ秒数
  const day_ms = new_year.getTime() + d * one_day;
  const date = new Date(day_ms);

  const month = date.getMonth() + 1;
  const day   = date.getDate();

  datetime.month = month;
  datetime.day   = day;
  datetime_set(datetime);
}

// その年の日数を計算
function calc_days_in_year(year)
{
  const days = ( (year % 4 === 0 && year % 100 !== 0) || year % 400 === 0) ? 366 : 365;
  return days;
}

// その年の春分の日が元旦から何日後かを計算
function calc_days_spring_equinox(year)
{
  const days_in_year = calc_days_in_year(year);

  const date = Math.floor( 20.8431 + 0.242194 * ( year - 1980 ) - Math.floor( ( year - 1980 ) / 4 ) );
  const days = date - 20 + 78 + ((days_in_year == 365) ? 0 : 1); // 閏年以外では 3/20 は 1/1 の 78日後
  return days;
}

// 日付を春分に設定 (1901-2099年)
function set_date_spring_equinox()
{
  let datetime = datetime_get();
  const year = datetime.year;
  const date = Math.floor( 20.8431 + 0.242194 * ( year - 1980 ) - Math.floor( ( year - 1980 ) / 4 ) );
  datetime.month = 3;
  datetime.day = date;
  datetime_set(datetime);
}
// 日付を夏至に設定 (1901-2099年)
function set_date_summer_solstice()
{
  let datetime = datetime_get();
  const year = datetime.year;
  const date = Math.floor( 22.2747 + 0.24162603 * ( year - 1900 ) - Math.floor( ( year - 1900 ) / 4 ) );
  datetime.month = 6;
  datetime.day = date;
  datetime_set(datetime);
}
// 日付を秋分に設定 (1901-2099年)
function set_date_autumn_equinox()
{
  let datetime = datetime_get();
  const year = datetime.year;
  const date = Math.floor( 23.2488 + 0.242194 * ( year - 1980 ) - Math.floor( ( year - 1980 ) / 4 ) )
  datetime.month = 9;
  datetime.day = date;
  datetime_set(datetime);
}
// 日付を冬至に設定 (1901-2099年)
function set_date_winter_solstice()
{
  let datetime = datetime_get();
  const year = datetime.year;
  const date = Math.floor( 22.6587 + 0.24274049 * ( year - 1900 ) - Math.floor( ( year - 1900 ) / 4 ) )
  datetime.month = 12;
  datetime.day = date;
  datetime_set(datetime);
}

/********** タイムゾーンと位置の処理 ***********/

// タイムゾーンの表示
function show_timezone()
{
  const timezone = Number(select_timezone.value);
  if(timezone >= 0){
    text_timezone.innerText = "UTC+" + timezone;
  }else{
    text_timezone.innerText = "UTC" + timezone;
  }
}

// 位置の表示
function show_location()
{
  const north_south = select_latitude.options[0].selected ? "N" : "S";
  const latitude = number_latitude.value;
  const east_west = select_longitude.options[0].selected ? "E" : "W";
  const longitude = number_longitude.value;
  text_location.innerText = "位置 " + latitude + "°" + north_south
                          + ", " + longitude +  "°" + east_west;
}

/********** BLE通信 ***********/

// 渾天儀に経度とUTC日時を指令
async function moveArmillarySphere()
{
  if(bleDevice == null) return;

  // 前回送信からSEND_INTERVAL[ms]未満なら送信保留
  const now = Date.now();
  const elapsed = now - last_time;
  if(elapsed < SEND_INTERVAL){
    // 既に保留中でなければ、SEND_DELAY[ms]後にリトライ予約
    if(pending == false) setTimeout(moveArmillarySphere, SEND_DELAY);
    pending= true; // 保留セット
    return;
  }
  last_time = now;
  pending = false; // 保留クリア

  // Busy確認
  const busy = await readBusy();
  if(!busy)
  {
    // 経度とUTC日時
    const longitude = Number(number_longitude.value);
    const timezone = Number(select_timezone.value);
    const local_datetime = datetime_get();
    const utc_datetime = utc_datetime_get(local_datetime, timezone);

    // 送信
    sendLonTime(
      longitude,
      utc_datetime.year, utc_datetime.month, utc_datetime.day,
      utc_datetime.hour, utc_datetime.min);
  }
}

// コマンドの送信
async function sendCommand(command) {
  if(bleDevice == null) return;

  await chrCommand.writeValue(new Uint8Array([command])).then(() => {
    console.log('chrCommand:' + command);
  }).catch(()=>{
    error_toast('ERROR! chrCommand:' + command);
  });
}

// 経度とUTC日時の送信
async function sendLonTime(longitude, year, month, day, hour, min) {
  if(bleDevice == null) return;

  const aBuffer = new ArrayBuffer(10);
  const dView = new DataView(aBuffer);
  const LITTLE_ENDIAN = true;
  dView.setFloat32(0, longitude, LITTLE_ENDIAN);
  dView.setUint16 (4, year,      LITTLE_ENDIAN);
  dView.setUint8  (6, month,     LITTLE_ENDIAN);
  dView.setUint8  (7, day,       LITTLE_ENDIAN);
  dView.setUint8  (8, hour,      LITTLE_ENDIAN);
  dView.setUint8  (9, min,       LITTLE_ENDIAN);
  const bArray = new Uint8Array(aBuffer);

  await chrLonTime.writeValue(bArray).then(() => {
    console.log('chrLonTime:' + bArray);
  }).catch(()=>{
    error_toast('ERROR! chrLonTime');
  });
}

// Busy状態確認
async function readBusy(){
  if(bleDevice == null) return;

  await chrBusy.readValue().then(value => {
    const busy = value.getUint8(0);
    console.log("chrBusy = " + busy);
    if(busy != 0){
      return true;
    }else{
      return false;
    }
  }).catch(()=>{
    error_toast('ERROR! chrBusy');
    return true;
  });
}

/********** 太陽の位置の計算 ***********/
function calc_sun_pos()
{
    // 経度とUTC日時
    const longitude = Number(number_longitude.value);
    const timezone = Number(select_timezone.value);
    const local_datetime = datetime_get();
    const utc_datetime = utc_datetime_get(local_datetime, timezone);
    const year  = utc_datetime.year;
    const month = utc_datetime.month;
    const day   = utc_datetime.day;
    const hour  = utc_datetime.hour;
    const min   = utc_datetime.min;

    // 太陽の位置の計算
    const sunCalc = new SunCalc();
    sunCalc.setDate(year, month, day, hour, min, 0);
    sunCalc.setLocation(0, longitude);
    sunCalc.calc();
    const lambda = sunCalc.lambda;
    const alpha  = sunCalc.alpha;
    const delta  = sunCalc.delta;

    // 表示
//  text_lambda.innerHTML = lambda * DEGS + "°";
//  text_alpha.innerHTML  = alpha * DEGS + "°";
//  text_delta.innerHTML  = delta * DEGS + "°";
    text_lambda.innerHTML = Math.floor(lambda * DEGS * 100) / 100 + "°";
    const alpha_deg = alpha * DEGS;
    const alpha_hour = Math.floor(alpha_deg * 24 / 360);
    const alpha_min = Math.floor((alpha_deg * 24 / 360 - alpha_hour)*60);
    const alpha_deg_F2 = Math.floor(alpha_deg * 100) / 100;
    text_alpha.innerHTML  = alpha_hour + " 時 " + alpha_min + " 分 (" + alpha_deg_F2 + "°)";
    text_delta.innerHTML  = Math.floor(delta * DEGS * 100) / 100 + "°";
}

/********** トースト表示(エラー) ***********/
function error_toast(message)
{
  console.log(message);

  const jsFrame = new JSFrame();
  jsFrame.showToast({
    html: message, align: 'top', duration: 5000
  });
}
