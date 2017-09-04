#ifndef __APP_H
#define __APP_H

/*NO Device mode*/
#define _NO_DEVICE 1

int appTask(void);
int appInit(void);

#define DD_NUM_OF_MD 3
#define DD_NUM_OF_AB 4

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

#define DRIVER_AB_0 0
#define DRIVER_AB_1 1
#define DRIVER_AB_2 2
#define DRIVER_AB_3 3
/*竿展開機構*/
#define ROD_AB_0 (1<<0)
#define ROD_AB_1 (1<<1)

#define ROD_AB_MAX_COUNT 300

/*プレシルスミサイル*/
#define MISSILE_AB_0 (1<<0)
#define MISSILE_AB_1 (1<<1)
#define MISSILE_AB_2 (1<<2)
#define MISSILE_AB_3 (1<<3)
#define MISSILE_AB_4 (1<<4)
#define MISSILE_AB_5 (1<<5)
#define MISSILE_AB_6 (1<<0)
#define MISSILE_AB_7 (1<<1)
#define MISSILE_AB_8 (1<<2)
#define MISSILE_AB_9 (1<<3)
#define MISSILE_AB_10 (1<<4)
#define MISSILE_AB_11 (1<<5)
#define MISSILE_AB_12 (1<<0)
#define MISSILE_AB_13 (1<<1)

#define MD_GAIN ( DD_MD_MAX_DUTY / DD_RC_ANALOG_MAX/3 )

#endif
