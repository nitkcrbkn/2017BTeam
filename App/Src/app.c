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
#include "trapezoid_ctrl.h"
/*suspensionSystem*/

static
int suspensionSystem(void);

static
int LEDSystem(void);

static
int armAB(void);

static
int rotationarm(void);

static
int changeOpeMode(void);

static
ope_mode_t g_ope_mode = OPE_MODE_N;

static
int transamSystem(void);

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
  /*途中必ず定数回で終了ること。*/
 
  switch(g_ope_mode){
  case OPE_MODE_N:
    ret = suspensionSystem();
    if(ret){
      return ret;
    }
    break;
    
  case OPE_MODE_T:
    ret = transamSystem();
    if(ret){
      return ret;
    }
    break;
  }

  ret = changeOpeMode();
  if(ret){
    return ret;
  }
   
  ret = armAB();
  if(ret){
    return ret;
  }
  
  ret = LEDSystem();
  if(ret){
    return ret;
  }
  
  ret = rotationarm();
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

/*アーム展開機構*/

static
int armAB(void){
  
  static int open_count = ARM_AB_MAX_COUNT;
   
  if(__RC_ISPRESSED_TRIANGLE(g_rc_data)){
    open_count = 0;
  }

  if(ARM_AB_MAX_COUNT > open_count){
    g_ab_h[DRIVER_AB].dat |= ARM_AB;
    open_count++;
  }else{
    g_ab_h[DRIVER_AB].dat &= ~ARM_AB;
  }
  
  return EXIT_SUCCESS;
}

/*プライベート 足回りシステム*/
static
int suspensionSystem(void){
  const tc_const_t tc ={
    .inc_con = 300,//DUTY上限時の傾き
    .dec_con = 400//　　下限時
  };
  const int num_of_motor = 3;/*モータの個数*/
  unsigned int idx;/*インデックス*/
  int m,x,y,w;
  int i;

  if(abs(DD_RCGetLX(g_rc_data))<CENTRAL_THRESHOLD){
    y = 0;
  }else{
    y = DD_RCGetLX(g_rc_data);
  }

  if(abs(DD_RCGetLY(g_rc_data))<CENTRAL_THRESHOLD){
    x = 0;
  }else{
    x = -DD_RCGetLY(g_rc_data);
  }

  if(abs(DD_RCGetRX(g_rc_data))<CENTRAL_THRESHOLD){
    w = 0;
  }else{
    w = -DD_RCGetRX(g_rc_data);
  }

  

  /*for each motor*/
  for(i=0;i<num_of_motor;i++){
    /*それぞれの差分*/
    switch(i){
    case 0:
      idx = MECHA1_MD1;
      m = -2*1/SR_SIX*y - 1*1/SR_THREE*w;
      break;
    case 1:
      idx = MECHA1_MD2;
      m = -1*1/SR_TWO*x + 1*1/SR_SIX*y - 1*1/SR_THREE*w;
      break;
    case 2:
      idx = MECHA1_MD3;
      m = 1*1/SR_TWO*x + 1*1/SR_SIX*y - 1*1/SR_THREE*w;
      break;
    default:
      return EXIT_FAILURE;
    }
    m *= 95;
    trapezoidCtrl(m,&g_md_h[idx],&tc);
  }

  return EXIT_SUCCESS;

}


/*竿回転機構*/
static 
int rotationarm(void){
  
  const tc_const_t tc ={
    .inc_con = 500, //DUTY上限時の傾き
    .dec_con = 500, //　　下限時
  };

  int target;
  unsigned int idx;

  idx = MECHA1_MD4;

  if((__RC_ISPRESSED_L1(g_rc_data)) && (__RC_ISPRESSED_L2(g_rc_data))){
    target = 0;
  }else if((__RC_ISPRESSED_L1(g_rc_data)) && !(__RC_ISPRESSED_L2(g_rc_data))){
    target = 8000;
  }else if((__RC_ISPRESSED_L2(g_rc_data)) && !(__RC_ISPRESSED_L1(g_rc_data))){
    target = -8000;
  }else{
    target = 0;
  }
  trapezoidCtrl(target,&g_md_h[idx],&tc);

  return EXIT_SUCCESS;
}

/*モード変更*/
static
int changeOpeMode(void){

  if(__RC_ISPRESSED_CIRCLE(g_rc_data)){
    g_ope_mode = OPE_MODE_N;
  }
  else if(__RC_ISPRESSED_CROSS(g_rc_data)){
    g_ope_mode = OPE_MODE_T;
  }
  
  return EXIT_SUCCESS;
}

/*トランザムシステム*/
static
int transamSystem(void){
  const tc_const_t tc_bC1 ={
    .inc_con = 100,//DUTY上限時の傾き
    .dec_con = 200//　　下限時
  };

  const tc_const_t tc_bC2 ={
    .inc_con = 200,//DUTY上限時の傾き
    .dec_con = 300//　　下限時
  };

  const tc_const_t tc_bC3 ={
    .inc_con = 300,//DUTY上限時の傾き
    .dec_con = 400//　　下限時
  };

  const int num_of_motor = 3;/*モータの個数*/
  unsigned int idx;/*インデックス*/
  int m,x,y,w,adjust;
  int i;

  if(abs(DD_RCGetLX(g_rc_data))<CENTRAL_THRESHOLD){
    y = 0;
  }else{
    y = DD_RCGetLX(g_rc_data);
  }

  if(abs(DD_RCGetLY(g_rc_data))<CENTRAL_THRESHOLD){
    x = 0;
  }else{
    x = -DD_RCGetLY(g_rc_data);
  }

  if(abs(DD_RCGetRX(g_rc_data))<CENTRAL_THRESHOLD){
    w = 0;
  }else{
    w = -DD_RCGetRX(g_rc_data);
  }
  

  /*for each motor*/
  for(i=0;i<num_of_motor;i++){
    /*それぞれの差分*/
    switch(i){
    case 0:
      idx = MECHA1_MD1;
      m = -2*1/SR_SIX*y - 1*1/SR_THREE*w;
      m *= 85;
      if(abs(m) <= 4800){
	m *= 2;
      }else if(abs(m) >= 9500){
	adjust = abs(m) - 9500;
	if(m > 0){
	  m -= adjust;
	}else if(m < 0){
	  m += adjust;
	}
      }
      break;
    case 1:
      idx = MECHA1_MD2;
      m = -1*1/SR_TWO*x + 1*1/SR_SIX*y - 1*1/SR_THREE*w;
      m *= 85;
      if(abs(m) <= 4800){
	m *= 2;
      }else if(abs(m) >= 9500){
	adjust = abs(m) - 9500;
	if(m > 0){
	  m -= adjust;
	}else if(m < 0){
	  m += adjust;
	}
      }
      break;
    case 2:
      idx = MECHA1_MD3;
      m = 1*1/SR_TWO*x + 1*1/SR_SIX*y - 1*1/SR_THREE*w;
      m *= 85;
      if(abs(m) <= 4800){
	m *= 2;
      }else if(abs(m) >= 9500){
	adjust = abs(m) - 9500;
	if(m > 0){
	  m -= adjust;
	}else if(m < 0){
	  m += adjust;
	}
      }
      break;
    default:
      return EXIT_FAILURE;
    }
    
    if(abs(m) <= 3000){
      trapezoidCtrl(m,&g_md_h[idx],&tc_bC1);
    }
    else if(abs(m) > 3000 && abs(m) <= 6000){
      trapezoidCtrl(m,&g_md_h[idx],&tc_bC2);
    }
    else if(abs(m) > 6000 && abs(m) <= 9500){
      trapezoidCtrl(m,&g_md_h[idx],&tc_bC3);
    }
 
  }
  return EXIT_SUCCESS;
  
}

