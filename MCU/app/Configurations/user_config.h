#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

//#include "Compilers.h"
#include "Cpu/Std/Platform_Types.h"
#include <string.h>
#include "uart.h"

/************************UDS algthorim access config***************/
#define ASSERT(xValue)\
do{\
    if(xValue)\
    {\
        while(1){}\
    }\
}while(0)


/*AES seed length*/
#define AES_SEED_LEN                               (16u)

#define	RX_FUN_ID                                  (0x7FFu)   /*can tp rx function ID*/
#define	RX_PHY_ID                                  (0x784u)   /*can tp rx phy ID*/
#define	TX_ID                                      (0x7F0u)       /*can tp tx ID*/
/*Enable TX CAN FD or not. If enable CAN FD, CAN TP transmit SF message will over 8 Bytes*/
//#define EN_TX_CAN_FD

/***********************CRC config****************************/
/*enable CRC module with hardware*/
//#define EN_CRC_HARDWARE

/*enable CRC module with software*/
#define EN_CRC_SOFTWARE

/*FLASH address continue or not*/
#define FALSH_ADDRESS_CONTINUE (FALSE)
/***********************************************************/

/********************FIFO define*******************************/
/*RX message from BUS FIFO ID*/
#define RX_BUS_FIFO                                  ('r')  /*RX bus fifo*/
#define RX_BUS_FIFO_LEN                              (300u)     /*RX BUS FIFO length*/

/*TX message to BUS FIFO ID*/
#define TX_BUS_FIFO                                  ('t')  /*RX bus fifo*/
#define TX_BUS_FIFO_LEN                              (100u)     /*RX BUS FIFO length*/

/***********************************************************/

/**********************FOTA A/B config************************/
//#define EN_SUPPORT_APP_B
typedef enum
{
	APP_A_TYPE =                0u,         /*APP A type*/
	APP_INVLID_TYPE =           0xFFu,      /*APP invalid type*/
}tAPPType;

/***********************************************************/

/*******************Global interrupt define************************/
/*disable all interrupts*/
#define DisableAllInterrupts()     IfxCpu_disableInterrupts()
								
								

/*enable all interrupts*/
#define EnableAllInterrupts()     IfxCpu_enableInterrupts()
/***********************************************************/

/***********************************************************/

/******Jump to APP delay time when have not received uds message*******/
//#define EN_DELAY_TIME
#define DELAY_MAX_TIME_MS                              (5000u)

/***********************************************************/

#endif /*__USER_CONFIG_H__*/

/************************End file********************************/

