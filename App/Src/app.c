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
int rodAB(void);

static
int missileAB(void);

static
int rotationright(void);

static
int rotationleft(void);

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
  
  ret = rodAB();
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
  
  ret = rotationright();
  if(ret){
    return ret;
  }
  
  ret = rotationleft();
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

/*ティラルス・プレシルス竿展開機構*/
static
int rodAB(void){

  static int open_count = ROD_AB_MAX_COUNT;
   
  if((__RC_ISPRESSED_CIRCLE(g_rc_data)) && (__RC_ISPRESSED_TRIANGLE(g_rc_data))){
    open_count = 0;
  }

  if(ROD_AB_MAX_COUNT > open_count){
    g_ab_h[DRIVER_AB_0].dat |= ROD_AB_0;
    g_ab_h[DRIVER_AB_0].dat |= ROD_AB_1;
    open_count++;
  }else{
    g_ab_h[DRIVER_AB_0].dat &= ~ROD_AB_0;
    g_ab_h[DRIVER_AB_0].dat &= ~ROD_AB_1;
  }
  
  return EXIT_SUCCESS;
}
  
/*プレシルスミサイル*/

static
int missileAB(void){

  /*全弾*/  
  if((__RC_ISPRESSED_R1(g_rc_data)) && (__RC_ISPRESSED_L1(g_rc_data)) && (__RC_ISPRESSED_UP(g_rc_data))){
    g_ab_h[DRIVER_AB_1].dat |= MISSILE_AB_0;
    g_ab_h[DRIVER_AB_1].dat |= MISSILE_AB_1;
    g_ab_h[DRIVER_AB_1].dat |= MISSILE_AB_2;
    g_ab_h[DRIVER_AB_1].dat |= MISSILE_AB_3;
    g_ab_h[DRIVER_AB_1].dat |= MISSILE_AB_4;
    g_ab_h[DRIVER_AB_1].dat |= MISSILE_AB_5;
    g_ab_h[DRIVER_AB_2].dat |= MISSILE_AB_6;
    g_ab_h[DRIVER_AB_2].dat |= MISSILE_AB_7;
    g_ab_h[DRIVER_AB_2].dat |= MISSILE_AB_8;
    g_ab_h[DRIVER_AB_2].dat |= MISSILE_AB_9;
    g_ab_h[DRIVER_AB_2].dat |= MISSILE_AB_10;
    g_ab_h[DRIVER_AB_2].dat |= MISSILE_AB_11;
    g_ab_h[DRIVER_AB_3].dat |= MISSILE_AB_12;
    g_ab_h[DRIVER_AB_3].dat |= MISSILE_AB_13;
  }

  /*21弾ミサイル*/
  else if((__RC_ISPRESSED_R1(g_rc_data)) && (__RC_ISPRESSED_CROSS(g_rc_data))){
    g_ab_h[DRIVER_AB_1].dat |= MISSILE_AB_0;
    g_ab_h[DRIVER_AB_1].dat |= MISSILE_AB_1;
    g_ab_h[DRIVER_AB_1].dat |= MISSILE_AB_2;
    g_ab_h[DRIVER_AB_1].dat |= MISSILE_AB_3;
    g_ab_h[DRIVER_AB_1].dat |= MISSILE_AB_4;
    g_ab_h[DRIVER_AB_1].dat |= MISSILE_AB_5;
    g_ab_h[DRIVER_AB_2].dat |= MISSILE_AB_6;
  }
  
  /*19弾ミサイル*/
  else if((__RC_ISPRESSED_L1(g_rc_data)) && (__RC_ISPRESSED_CROSS(g_rc_data))){
    g_ab_h[DRIVER_AB_2].dat |= MISSILE_AB_7;
    g_ab_h[DRIVER_AB_2].dat |= MISSILE_AB_8;
    g_ab_h[DRIVER_AB_2].dat |= MISSILE_AB_9;
    g_ab_h[DRIVER_AB_2].dat |= MISSILE_AB_10;
    g_ab_h[DRIVER_AB_2].dat |= MISSILE_AB_11;
    g_ab_h[DRIVER_AB_3].dat |= MISSILE_AB_12;
    g_ab_h[DRIVER_AB_3].dat |= MISSILE_AB_13;
  } 

  /*9弾ミサイル*/
  else if((__RC_ISPRESSED_R1(g_rc_data)) && (__RC_ISPRESSED_CIRCLE(g_rc_data))){
    g_ab_h[DRIVER_AB_1].dat |= MISSILE_AB_0;
    g_ab_h[DRIVER_AB_1].dat |= MISSILE_AB_1;
    g_ab_h[DRIVER_AB_1].dat |= MISSILE_AB_2;
  }
  else if((__RC_ISPRESSED_R1(g_rc_data)) && (__RC_ISPRESSED_TRIANGLE(g_rc_data))){
    g_ab_h[DRIVER_AB_1].dat |= MISSILE_AB_3;
    g_ab_h[DRIVER_AB_1].dat |= MISSILE_AB_4;
    g_ab_h[DRIVER_AB_1].dat |= MISSILE_AB_5;
  }
  else if((__RC_ISPRESSED_R1(g_rc_data)) && (__RC_ISPRESSED_SQARE(g_rc_data))){
    g_ab_h[DRIVER_AB_2].dat |= MISSILE_AB_6;
    g_ab_h[DRIVER_AB_2].dat |= MISSILE_AB_7;
    g_ab_h[DRIVER_AB_2].dat |= MISSILE_AB_8;
    g_ab_h[DRIVER_AB_2].dat |= MISSILE_AB_9;
  }
  else if((__RC_ISPRESSED_R2(g_rc_data)) && (__RC_ISPRESSED_CIRCLE(g_rc_data))){
    g_ab_h[DRIVER_AB_2].dat |= MISSILE_AB_10;
    g_ab_h[DRIVER_AB_2].dat |= MISSILE_AB_11;
    g_ab_h[DRIVER_AB_3].dat |= MISSILE_AB_12;
    g_ab_h[DRIVER_AB_3].dat |= MISSILE_AB_13;
  }
  
  else{
    g_ab_h[DRIVER_AB_1].dat &= ~MISSILE_AB_0;
    g_ab_h[DRIVER_AB_1].dat &= ~MISSILE_AB_1;
    g_ab_h[DRIVER_AB_1].dat &= ~MISSILE_AB_2;
    g_ab_h[DRIVER_AB_1].dat &= ~MISSILE_AB_3;
    g_ab_h[DRIVER_AB_1].dat &= ~MISSILE_AB_4;
    g_ab_h[DRIVER_AB_1].dat &= ~MISSILE_AB_5;
    g_ab_h[DRIVER_AB_2].dat &= ~MISSILE_AB_6;
    g_ab_h[DRIVER_AB_2].dat &= ~MISSILE_AB_7;
    g_ab_h[DRIVER_AB_2].dat &= ~MISSILE_AB_8;
    g_ab_h[DRIVER_AB_2].dat &= ~MISSILE_AB_9;
    g_ab_h[DRIVER_AB_2].dat &= ~MISSILE_AB_10;
    g_ab_h[DRIVER_AB_2].dat &= ~MISSILE_AB_11;
    g_ab_h[DRIVER_AB_3].dat &= ~MISSILE_AB_12;
    g_ab_h[DRIVER_AB_3].dat &= ~MISSILE_AB_13;
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
      // case 3:
      // idx = MECHA1_MD4;
      // m = -x+y-w;
      // break;
    default:
      return EXIT_FAILURE;
    }
    m *= 95;
    trapezoidCtrl(m,&g_md_h[idx],&tc);
    /*これは中央か?*/
    //  if(m==0){
    //  g_md_h[idx].mode = D_MMOD_FREE;
    //  g_md_h[idx].duty = 0;
    // }else{
    // if(m > 0){
    /*前後の向き判定*/
    //	g_md_h[idx].mode = D_MMOD_FORWARD;
    // }
    // else{
    //	g_md_h[idx].mode = D_MMOD_BACKWARD;
    // }
    /*絶対値を取りDutyに格納*/
    // g_md_h[idx].duty = abs(m) * MD_GAIN;
    // }
  }

  return EXIT_SUCCESS;

}

/*ティラルス竿回転機構右*/

static 
int rotationright(void){
  
  const tc_const_t tc ={
    .inc_con = 500,//DUTY上限時の傾き
    .dec_con = 500,//　　下限時
  };
  int target;
  
    
  unsigned int idx;/*インデックス*/
  
  idx = MECHA1_MD4;
  if((__RC_ISPRESSED_R1(g_rc_data))){
    target = 500;
  }else if((__RC_ISPRESSED_R2(g_rc_data))){
    target = -500;
  }else{
    target = 0;
  }
  trapezoidCtrl(target,&g_md_h[idx],&tc);

  return EXIT_SUCCESS;
}    

/*ティラルス竿回転機構左*/

static 
int rotationleft(void){
  
  const tc_const_t tc ={
    .inc_con = 500, //DUTY上限時の傾き
    .dec_con = 500, //　　下限時
  };
  int target;
  unsigned int idx;

  idx = MECHA1_MD5;
  if((__RC_ISPRESSED_L1(g_rc_data))){
    target = 500;
  }else if((__RC_ISPRESSED_L2(g_rc_data))){
    target = -500;
  }else{
    target = 0;
  }
  trapezoidCtrl(target,&g_md_h[idx],&tc);

  return EXIT_SUCCESS;
}

