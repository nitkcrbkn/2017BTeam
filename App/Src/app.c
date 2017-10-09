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
int moveAB(void);

static
int missileAB(void);

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

  ret = suspensionSystem();
    if(ret){
      return ret;
    }
 
    ret = armAB();
    if(ret){
      return ret;
    }
    
    ret = moveAB();
    if(ret){
      return ret;
    }
    
    ret = missileAB();
    if(ret){
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

/*アーム移動機構*/  

static
int moveAB(void){
  static int switch_AB = 0;

  if(__RC_ISPRESSED_R1(g_rc_data)){
    if(switch_AB == 0){
      g_ab_h[DRIVER_AB].dat ^= ARM_MOVE;
      switch_AB = 1;
    }
  }
  else{
    switch_AB = 0;
  }
  
  return EXIT_SUCCESS;
}

/*ミサイル*/

static
int missileAB(void){
    
  if(__RC_ISPRESSED_R2(g_rc_data)){
    g_ab_h[DRIVER_AB].dat |= MISSILE_AB_0;
  }
  else{
    g_ab_h[DRIVER_AB].dat &= ~MISSILE_AB_0;
  }
  /*
    if(__RC_ISPRESSED_L2(g_rc_data)){
    g_ab_h[DRIVER_AB].dat |= MISSILE_AB_1;
    }
    else{
    g_ab_h[DRIVER_AB].dat &= ~MISSILE_AB_1;
    }

    if(__RC_ISPRESSED_L1(g_rc_data)){
    g_ab_h[DRIVER_AB].dat |= MISSILE_AB_2;
    }
    else{
    g_ab_h[DRIVER_AB].dat &= ~MISSILE_AB_2;
    }
  */
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
      m*=95;
      if(abs(m)<=4800){
	m*=2;
      }else if(abs(m)>=9500){
	adjust = abs(m) - 9500;
	if(m>0){
	  m-=adjust;
	}else if(m<0){
	  m+=adjust;
	}
      }
      break;
    case 1:
      idx = MECHA1_MD2;
      m = -1*1/SR_TWO*x + 1*1/SR_SIX*y - 1*1/SR_THREE*w;
      m*=95;
      if(abs(m)<=4800){
	m*=2;
      }else if(abs(m)>9500){
	adjust = abs(m) - 9500;
	if(m>0){
	  m-=adjust;
	}else if(m<0){
	  m+=adjust;
	}
      }
      break;
    case 2:
      idx = MECHA1_MD3;
      m = 1*1/SR_TWO*x + 1*1/SR_SIX*y - 1*1/SR_THREE*w;
      m*=95;
      if(abs(m)<=4800){
	m*=2;
      }else if(abs(m)>9600){
	adjust = abs(m) - 9600;
	if(m>0){
	  m-=adjust;
	}else if(m<0){
	  m+=adjust;
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
