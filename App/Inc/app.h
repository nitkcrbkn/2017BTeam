#ifndef __APP_H
#define __APP_H

/*NO Device mode*/
#define _NO_DEVICE 0

int appTask(void);
int appInit(void);

#define DD_NUM_OF_MD 3
#define DD_NUM_OF_AB 3

#define DD_USE_ENCODER1 0
#define DD_USE_ENCODER2 0
#define DD_NUM_OF_SV 0

#include "DD_RC.h"
#include "DD_MD.h"
#include "DD_SV.h"

#define MECHA1_MD1 0
#define MECHA1_MD2 1
#define MECHA1_MD3 2

#define CENTRAL_THRESHOLD 4

#define SR_TWO 1.4142 //ルート2
#define SR_THREE 1.7320 //ルート3
#define SR_SIX 2.4494 //ルート6

/*モード変更*/
typedef enum{
  OPE_MODE_N,
  OPE_MODE_F,
  OPE_MODE_M,
} ope_mode_t;

#define DRIVER_AB_0 0
#define DRIVER_AB_1 1
#define DRIVER_AB_2 2

/*アーム展開機構*/
#define ARM_AB (1<<0)

#define ARM_AB_MAX_COUNT 300

/*アーム用シリンダ*/
#define ARM_MOVE (1<<1)

/*アーム制御用*/
#define CHAIN_ARM_AB (1<<2)

/*ミサイル*/
#define MISSILE_AB_0 (1<<0)
#define MISSILE_AB_1 (1<<1)
#define MISSILE_AB_2 (1<<0)
#define MISSILE_AB_3 (1<<1)

#define DRIVE_MD_MAX_COUNT 20

#define MD_GAIN ( DD_MD_MAX_DUTY / DD_RC_ANALOG_MAX/3 )

#endif
