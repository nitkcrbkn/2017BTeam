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
int suspensionSystem(void);
//static 
//int ABSystem(void);
static
int LEDSystem(void);
static
int SwingArm(void);
static
int changeOpeMode(void);
static
ope_mode_t g_ope_mode = OPE_MODE_A;
static
int suspensionSystem_fast(void);
//static
//int WorkLock(void);
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

  /*
    ret = ABSystem();
    if(ret){
    return ret;
    }
  */
  
  ret = LEDSystem();
  if(ret){
    return ret;
  }

  ret = SwingArm();
  if(ret){
    return ret;
  }

  /*
    ret = WorkLock();
    if(ret) {
    return ret;
    }
  */

  ret = changeOpeMode();
  if(ret) {
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

/*
  static 
  int ABSystem(void){
  int i;
  const int NUM_OF_AB = 5;

  for(i=0; i<NUM_OF_AB; i++) {
  switch(i) {
  case 0: //竿１
  if((__RC_ISPRESSED_CIRCLE(g_rc_data)) && (__RC_ISPRESSED_TRIANGLE(g_rc_data)) && !(__RC_ISPRESSED_CROSS(g_rc_data)) && !(__RC_ISPRESSED_SQARE(g_rc_data))) {
  g_ab_h[0].dat |= ON_AB0;
  } else {
  g_ab_h[0].dat &= ~ON_AB0;
  }
  break;

  case 1: //竿２
  if((__RC_ISPRESSED_SQARE(g_rc_data)) && (__RC_ISPRESSED_CROSS(g_rc_data)) && !(__RC_ISPRESSED_CIRCLE(g_rc_data)) && !(__RC_ISPRESSED_TRIANGLE(g_rc_data))) {
  g_ab_h[0].dat |= ON_AB1;
  } else {
  g_ab_h[0].dat &= ~ON_AB1;
  }
  break;
      
  case 2: //シリンダ１
  if((__RC_ISPRESSED_R1(g_rc_data)) && (__RC_ISPRESSED_L1(g_rc_data))){
  g_ab_h[0].dat |= ON_AB2;
  } else {
  g_ab_h[0].dat &= ~ON_AB2;
  }
  break;
      
  case 3: //シリンダ２
  if((__RC_ISPRESSED_R2(g_rc_data)) && (__RC_ISPRESSED_L2(g_rc_data))){
  g_ab_h[0].dat |= ON_AB3;
  } else {
  g_ab_h[0].dat &= ~ON_AB3;
  }
  break;
      
  case 4: //剣を振る
  if((__RC_ISPRESSED_UP(g_rc_data)) && !(__RC_ISPRESSED_CIRCLE(g_rc_data)) && !(__RC_ISPRESSED_CROSS(g_rc_data)) && !(__RC_ISPRESSED_SQARE(g_rc_data)) && !(__RC_ISPRESSED_TRIANGLE(g_rc_data))) {
  g_ab_h[0].dat |= ON_AB4;
  } else {
  g_ab_h[0].dat &= ~ON_AB4;
  }
  break;
      
  default :
  return EXIT_FAILURE;
  }
  }
  return EXIT_SUCCESS;
  }
*/

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
  int i,m,x,w;//y,w;
  x = -DD_RCGetLY(g_rc_data);
  //y = -DD_RCGetLX(g_rc_data);
  w = -DD_RCGetRX(g_rc_data);
  
  //for each motor

  for (i=0; i<num_of_motor; i++) {
    switch (i) {
    case 0:
      idx = MECHA1_MD0;
      m = x -w;
      break;

    case 1:
      idx = MECHA1_MD1;
      m = -x -w;
      break;

      /*４輪オムニ用のプログラム
	case 0:
	idx = MECHA1_MD0;
	m = x -y -w;
	break;

	case 1:
	idx = MECHA1_MD1;
	m = x +y -w;
	break;

	case 2:
	idx = MECHA1_MD2;
	m = -x +y -w;
	break;

	case 3:
	idx = MECHA1_MD3;
	m = -x -y -w;
	break;
      */

    default:
      return EXIT_FAILURE;
    }
    m *= 75;//モータの出力不足を補う
    trapezoidCtrl(m,&g_md_h[idx],&tc);
  }
  return EXIT_SUCCESS;
}

static
int SwingArm(void){

  int target;
  const tc_const_t tc= {
    .inc_con = 1000,  //duty上昇時の傾き
    .dec_con = 1000,  //duty下降時の傾き
  };
  
  unsigned int idx;//インデックス
  idx = MECHA1_MD2;
  target = 0;
  
  if((__RC_ISPRESSED_RIGHT(g_rc_data)) && (_IS_PRESSED_RIGHT_LIMITSW())) {
    target = 4000;
  }
  if((__RC_ISPRESSED_LEFT(g_rc_data)) && (_IS_PRESSED_LEFT_LIMITSW())) {
    target = -4000;
  }

  if((target > 0) && !(_IS_PRESSED_RIGHT_LIMITSW())) {
    target = 0;
  }
  if((target < 0) && !(_IS_PRESSED_LEFT_LIMITSW())) {
    target = 0;
  }

  trapezoidCtrl(target,&g_md_h[idx],&tc);
    
  return EXIT_SUCCESS;
}

/*
  static //サーボを動かすプログラム
  int WorkLock(void){
  if(((__RC_ISPRESSED_CIRCLE(g_rc_data))) && ((__RC_ISPRESSED_CROSS(g_rc_data))) && ((__RC_ISPRESSED_SQARE(g_rc_data))) && ((__RC_ISPRESSED_TRIANGLE(g_rc_data))) && ((__RC_ISPRESSED_DOWN(g_rc_data)))) {
  g_sv_h.val[0] = 475;
  }
  
  if(((__RC_ISPRESSED_CIRCLE(g_rc_data))) && ((__RC_ISPRESSED_CROSS(g_rc_data))) && ((__RC_ISPRESSED_SQARE(g_rc_data))) && ((__RC_ISPRESSED_TRIANGLE(g_rc_data))) && ((__RC_ISPRESSED_UP(g_rc_data)))) {
  g_sv_h.val[0] = 300;
  }
  
  return EXIT_SUCCESS;
  }
*/

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
    .inc_con = 400,  //duty上昇時の傾き
    .dec_con = 400  //duty下降時の傾き
  };
  
  const int num_of_motor = 2;//モータの個数
  //int rc_analogdata;//アナログデータ
  unsigned int idx;//インデックス
  int i,m,x,w,adjust;
  x = -DD_RCGetLY(g_rc_data);
  //y = -DD_RCGetLX(g_rc_data);
  w = -DD_RCGetRX(g_rc_data);
  
  for (i=0; i<num_of_motor; i++) {
    switch (i) {
    case 0:
      idx = MECHA1_MD0;
      m = x -w;
      m *= 75;//モータの出力不足を補う
      if(abs(m) <= 4800) {//dutyが低かったら引き上げ
	m *= 2;
      } else if(abs(m) >= 9500) {//dutyが9500を超えたら9500以下になるよう調整
	adjust = abs(m) - 9500;
	if(m > 0) {
	  m -= adjust;
	} else if(m < 0) {
	  m += adjust;
	}
      }
      break;
    case 1:
      idx = MECHA1_MD1;
      m = -x -w;
      m *= 75;//モータの出力不足を補う
      if(abs(m) <= 4800) {
	m *= 2;
      } else if(abs(m) >= 9500) {
	adjust = abs(m) - 9500;
	if(m > 0) {
	  m -= adjust;
	} else if(m < 0) {
	  m += adjust;
	}
      }
      break;

    default:
      return EXIT_FAILURE;
    }

    trapezoidCtrl(m,&g_md_h[idx],&tc);
  }

  return EXIT_SUCCESS;
}
