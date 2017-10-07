#include "app.h"
#include "DD_Gene.h"
#include "DD_RCDefinition.h"
#include "SystemTaskManager.h"
#include <stdlib.h>
#include "MW_GPIO.h"
#include "MW_IWDG.h"
#include "message.h"
#include "MW_flash.h"
#include "constManager.h"
#include <trapezoid_ctrl.h>

static
int ASSystem(void);
static
int LEDSystem(void);
static
int suspensionSystem(void);
static
int changeOpeMode(void);
static
ope_mode_t g_ope_mode = OPE_MODE_A;
static
int suspensionSystem_fast(void);
/*メモ
 *g_ab_h...ABのハンドラ
 *g_md_h...MDのハンドラ
 *
 *g_rc_data...RCのデータ
 */

int appInit(void){

  ad_init();

  /*GPIO の設定などでMW,GPIOではHALを叩く*/
  return EXIT_SUCCESS;
}

/*application tasks*/
int appTask(void){
  int ret=0;

  if(__RC_ISPRESSED_R1(g_rc_data)&&__RC_ISPRESSED_R2(g_rc_data)&&
     __RC_ISPRESSED_L1(g_rc_data)&&__RC_ISPRESSED_L2(g_rc_data)){
    while(__RC_ISPRESSED_R1(g_rc_data)||__RC_ISPRESSED_R2(g_rc_data)||
	  __RC_ISPRESSED_L1(g_rc_data)||__RC_ISPRESSED_L2(g_rc_data))
      SY_wait(10);
    ad_main();
  }
  
  /*それぞれの機構ごとに処理をする*/
  /*途中必ず定数回で終了すること。*/
  switch(g_ope_mode){
  case OPE_MODE_A:
    ret = suspensionSystem();
    if(ret){
      return ret;
    }
    break;

  case OPE_MODE_B:
    ret = suspensionSystem_fast();
    if(ret){
      return ret;
    }
    break;
  }

  ret = changeOpeMode();
  if(ret) {
    return ret;
  }

  ret = ASSystem();
  if(ret) {
    return ret;
  }  
  
  ret = LEDSystem();
  if(ret){
    return ret;
  }
  
  return EXIT_SUCCESS;
}

static int LEDSystem(void){
  if(__RC_ISPRESSED_UP(g_rc_data)){
    g_led_mode = lmode_1;
  }
  if(__RC_ISPRESSED_DOWN(g_rc_data)){
    g_led_mode = lmode_2;
  }
  if(__RC_ISPRESSED_RIGHT(g_rc_data)){
    g_led_mode = lmode_3;
  }

  return EXIT_SUCCESS;
}

static
int ASSystem(void) {
  static int BLOW_SYLINDER_FRONT = 0;
  static int BLOW_SYLINDER_BACK = 0;

  if((__RC_ISPRESSED_R1(g_rc_data)) && (__RC_ISPRESSED_L1(g_rc_data))) {
    if(BLOW_SYLINDER_FRONT == 0) {
      g_ab_h[0].dat ^= ON_AB0;
      BLOW_SYLINDER_FRONT = 1;
    }
  } else {
    BLOW_SYLINDER_FRONT = 0;
  }

  if((__RC_ISPRESSED_R2(g_rc_data)) && (__RC_ISPRESSED_L2(g_rc_data))) {
    if(BLOW_SYLINDER_BACK == 0) {
      g_ab_h[0].dat ^= ON_AB1;
      BLOW_SYLINDER_BACK = 1;
    }
  } else {
    BLOW_SYLINDER_BACK = 0;
  }

  return EXIT_SUCCESS;
}

/*プライベート 足回りシステム*/
static
int suspensionSystem(void){
  
  const tc_const_t tc= {
    .inc_con = 400,  //duty上昇時の傾き
    .dec_con = 400  //duty下降時の傾き
  };
  
  const int num_of_motor = 2;//モータの個数
  //int rc_analogdata;//アナログデータ
  unsigned int idx;//インデックス
  int i,duty,x,w;//y,w;
  x = -DD_RCGetLY(g_rc_data);
  w = -DD_RCGetRX(g_rc_data);
  
  //for each motor

  for (i=0; i<num_of_motor; i++) {
    switch (i) {
    case 0:
      idx = MECHA1_MD0;
      duty = x -w;
      break;

    case 1:
      idx = MECHA1_MD1;
      duty = -x -w;
      break;

    default:
      return EXIT_FAILURE;
    }
    duty *= 75;//モータの出力不足を補う
    trapezoidCtrl(duty,&g_md_h[idx],&tc);
  }
  return EXIT_SUCCESS;
}


static
int changeOpeMode(void){

  if(__RC_ISPRESSED_CIRCLE(g_rc_data)){
    g_ope_mode = OPE_MODE_A;
  }else if(__RC_ISPRESSED_CROSS(g_rc_data)){
    g_ope_mode = OPE_MODE_B;
  }
  
  return EXIT_SUCCESS;
}

static
int suspensionSystem_fast(void){
  
  const tc_const_t tc= {
    .inc_con = 150,  //duty上昇時の傾き
    .dec_con = 150  //duty下降時の傾き
  };
  
  const int num_of_motor = 2;//モータの個数
  //int rc_analogdata;//アナログデータ
  unsigned int idx;//インデックス
  int i,duty,x,w,adjust;
  x = -DD_RCGetLY(g_rc_data); //左スティックの前後の座標
  w = -DD_RCGetRX(g_rc_data); //右スティックの左右の座標
  
  for (i=0; i<num_of_motor; i++) { 
    switch (i) {
    case 0:
      idx = MECHA1_MD0; //左の駆動
      duty = x -w;
      duty *= 75; //モータの出力不足を補う
      if(abs(duty) <= 4800) { //dutyが低かったら引き上げ
	duty *= 2;
      } else if(abs(duty) >= 9500) { //dutyが9500を超えたら9500以下になるよう調整
	adjust = abs(duty) - 9500;
	if(duty > 0) {
	  duty -= adjust;
	} else if(duty < 0) {
	  duty += adjust;
	}
      }
      break;
    case 1:
      idx = MECHA1_MD1; //右の駆動
      duty = -x -w;
      duty *= 75; //モータの出力不足を補う
      if(abs(duty) <= 4800) {
	duty *= 2;
      } else if(abs(duty) >= 9500) {
	adjust = abs(duty) - 9500;
	if(duty > 0) {
	  duty -= adjust;
	} else if(duty < 0) {
	  duty += adjust;
	}
      }
      break;

    default:
      return EXIT_FAILURE;
    }
    trapezoidCtrl(duty,&g_md_h[idx],&tc);
  }

  return EXIT_SUCCESS;
}
