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
static
int ASSystem(void);
static
int LEDSystem(void);
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

  ret = ASSystem();
  if(ret) {
    return ret;
  }
  
  
  ret = LEDSystem();
  if(ret){
    return ret;
  }

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

static
int ASSystem(void) {
  static int BLOW_AIR;

  if((__RC_ISPRESSED_R2(g_rc_data)) && (__RC_ISPRESSED_L2(g_rc_data))) {
    BLOW_AIR = 0;
  }else if(BLOW_AIR == 1) {                 //BLOW_AIRに1が代入されていればそれを保持
    BLOW_AIR = 1;
  }else if((__RC_ISPRESSED_R1(g_rc_data)) && (__RC_ISPRESSED_L1(g_rc_data))) {  //R2を押せばBLOW_AIRに1を代入
    BLOW_AIR = 1;
  }
  
  switch(BLOW_AIR) {
  case 0:
    g_ab_h[0].dat &= ~ON_AB0;
    g_ab_h[0].dat &= ~ON_AB1;
    break;

  case 1:
    g_ab_h[0].dat |= ON_AB0;
    g_ab_h[0].dat |= ON_AB1;
    break;
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
