#ifndef __CAN_TP_H__
#define __CAN_TP_H__

#include "can_tp_cfg.h"
#include "uds_can_fifo_list.h"

/*uds network man function*/
extern void CANTP_MainFun(void);

/*can tp system tick control*/
extern void CANTP_SytstemTickControl(void);

/*Init CAN TP list*/
extern void CANTP_Init(void);

#endif /*#ifndef __CAN_TP_H__*/

/***************************End file********************************/

