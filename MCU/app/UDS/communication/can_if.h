/*
 * TP_cf.h
 *
 *  Created on: 2019Äê1ÔÂ24ÈÕ
 *      Author: nxf10035
 */

#ifndef _CAN_IF_H_
#define _CAN_IF_H_

#include "user_config.h"

#define MAX_MESSAGE_LEN (64u)
#define RX_TP_QUEUE_ID ('R')  /*TP RX  FIFO ID*/
#define TX_TP_QUEUE_ID ('T')  /*TP TX  FIFO ID*/

/*defined FIFO length*/
#define TX_TP_QUEUE_LEN (50u)  /*UDS send message to  TP max length*/
#define RX_TP_QUEUE_LEN (150)  /*UDS read message from TP max length*/

/*tx message call back*/
typedef void (*tpfUDSTxMsgCallBack)(uint8);

/*a signle message buf len*/


typedef struct
{
    uint32 rxDataLen;      /*RX can harware data len*/
    uint32 rxDataId;      /*RX data len*/
    uint8 aucDataBuf[MAX_MESSAGE_LEN];   /*RX data buf*/
}tRxMsgInfo;

typedef struct
{
    uint32 msgID;                   /*message ID*/
    uint32 dataLen;                 /*data length*/
    tpfUDSTxMsgCallBack pfCallBack; /*call back*/
}tUDSAndTPExchangeMsgInfo;

typedef enum
{
    TX_MSG_SUCCESSFUL = 0u,
    TX_MSG_FAILD,
    TX_MSG_TIMEOUT
}tTxMsgStatus;

typedef struct
{
    uint32 TxMsgID;       /*Tx message ID*/
    uint32 TxMsgLength;   /*TX message length*/
    uint32 TxMsgCallBack; /*Tx message callback*/
}tTPTxMsgHeader;

/*!
 * @brief To initial this module.
 *
 * This function returns the state of the initial.
 *
 * @return none.
 */
 void TP_Init(void);

 void TP_MainFun(void);

/*TP system tick control*/
 void TP_SystemTickCtl(void);


/*read a frame  tp data  from UDS to Tp TxFIFO. If no data can read return FALSE, else return TRUE*/
 boolean TP_ReadAFrameDataFromTP(uint32 *o_pRxMsgID,
                                      uint32 *o_pxRxDataLen,
                                      uint8 *o_pDataBuf);

/*write a frame data from  Tp to UDS RxFIFO*/
 boolean TP_WriteAFrameDataInTP(const uint32 i_TxMsgID,
                                     const tpfUDSTxMsgCallBack i_pfUDSTxMsgCallBack,
                                     const uint32 i_xTxDataLen,
                                     const uint8 *i_pDataBuf);

 void TP_Deinit(void);

/*Get TP config TX message ID*/
 uint32 TP_GetConfigTxMsgID(void);

/*Get TP config recieve Funcation ID*/
 uint32 TP_GetConfigRxMsgFUNID(void);

/*Get TP config receive pyhiscal ID*/
 uint32 TP_GetConfigRxMsgPHYID(void);

/*register transmit a frame message call back*/
 void   TP_RegisterTransmittedAFrmaeMsgCallBack(const tpfUDSTxMsgCallBack i_pfTxMsgCallBack);

/*do transmited a frame message call back*/
 void   TP_DoTransmittedAFrameMsgCallBack(const uint8 i_result);

/*Driver write data in TP*/
 boolean TP_DriverWriteDataInTP(const uint32 i_RxID, const uint32 i_RxDataLen, const uint8 *i_pRxDataBuf);

/*Driver read data from TP for Tx message to BUS*/
 boolean TP_DriverReadDataFromTP(const uint32 i_readDataLen, uint8 * o_pReadDatabuf, uint32 *o_pTxMsgID, uint32 *o_pTxMsgLength);

/*register abort tx message*/
 void TP_RegisterAbortTxMsg(void (*i_pfAbortTxMsg)(void));

/*do TP tx message successful callback*/
 void TP_DoTxMsgSuccesfulCallback(void);

#endif /* UDS_STACK_TP_INC_TP_CF_H_ */

