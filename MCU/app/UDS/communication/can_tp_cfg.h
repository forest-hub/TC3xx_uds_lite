#ifndef __CAN_TP_CFG_H__
#define __CAN_TP_CFG_H__

#include "user_config.h"
#include <uds_can_fifo_list.h>
#include "can_if.h"
//#include "can_if.h"
/*******************************************************
**  Description : ISO 15765-2 config file
*******************************************************/
typedef void (*tpfNetTxCallBack)(void);
typedef uint8 (*tNetTxMsg)(const uint32, const uint16, const uint8 *, const tpfNetTxCallBack, const uint32);
typedef uint8 (*tNetRx)(uint32 *, uint8 *, uint8 *);


/*abort tx message*/
typedef void (*tpfAbortTxMsg)(void);

#define MAX_CF_DATA_LEN (150u) /*max first frame data len */

#define MAX_CAN_DATA_LEN (64u)  /*max CAN data len*/

#define SF_CANFD_DATA_MAX_LEN (62u)  /*max CAN FD signle frame data len*/
#define SF_CAN_DATA_MAX_LEN (7u)    /*max CAN2.0 signle frame data len*/

/*RX message less length*/
#define RX_FF_DATA_LESS_LEN (6u)

#ifndef EN_TX_CAN_FD /*standard CAN*/
#define DATA_LEN (8u)

#define TX_SF_DATA_MAX_LEN (SF_CAN_DATA_MAX_LEN) /*RX support CAN FD, TX message is unspport CAN FD */

#define TX_FF_DATA_MIN_LEN (SF_CAN_DATA_MAX_LEN + 1u)  /*min fiirst frame data len*/

#define TX_CF_DATA_MAX_LEN (SF_CAN_DATA_MAX_LEN)  /*signle conective frame max data len*/
#else/*CAN FD*/
#define DATA_LEN (64u)

#define TX_SF_DATA_MAX_LEN (SF_CANFD_DATA_MAX_LEN) /*RX support CAN FD, TX message is unspport CAN FD */

#define TX_FF_DATA_MIN_LEN (SF_CANFD_DATA_MAX_LEN + 1u)  /*min fiirst frame data len*/

#define TX_CF_DATA_MAX_LEN (SF_CANFD_DATA_MAX_LEN + 1u)  /*single conective frame max data len*/
#endif

/*standard CAN message length for CAN TP*/
#define STANDARD_CAN_DL (8u)          

#define NORMAL_ADDRESSING (0u) /*normal addressing*/
#define MIXED_ADDRESSING (1u)  /*mixed addressing*/

typedef struct
{
    uint8 ucCalledPeriod;/*called CAN tp main function period*/
    uint32 xRxFunId;             /*rx function ID*/
    uint32 xRxPhyId;             /*Rx phy ID*/
    uint32 xTxId;                /*Tx ID*/
    uint16 xBlockSize;       /*BS*/
    uint16 xSTmin;             /*STmin*/
    uint16 xNAs;               /*N_As*/
    uint16 xNAr;               /*N_Ar*/
    uint16 xNBs;               /*N_Bs*/
    uint16 xNBr;               /*N_Br*/
    uint16 xNCs;               /*N_Cs*/
    uint16 xNCr;               /*N_Cr*/
    uint32 txBlockingMaxTimeMs;  /*TX message blocking max time (MS)*/
    tNetTxMsg pfNetTxMsg;/*net tx message with non blocking*/
    tNetRx pfNetRx;              /*net rx*/
    tpfAbortTxMsg pfAbortTXMsg;  /*abort tx message*/
}tUdsCANNetLayerCfg;


/*uds netwrok layer cfg info */
extern const tUdsCANNetLayerCfg g_stCANUdsNetLayerCfgInfo;


/*get config CAN TP tx ID*/
uint32 CANTP_GetConfigTxMsgID(void);


/*get config CAN TP recevie function message ID*/
uint32 CANTP_GetConfigRxMsgFUNID(void);

/*get config CAN TP recevie physical message ID*/
uint32 CANTP_GetConfigRxMsgPHYID(void);

/*Get CAN TP config Tx  handle*/
tNetTxMsg CANTP_GetConfigTxHandle(void);

/*Get CAN TP config Rx  handle*/
 tNetRx CANTP_GetConfigRxHandle(void);

/*Is received message valid?*/
 boolean CANTP_IsReceivedMsgIDValid(const uint32 i_receiveMsgID);

/*write data in CAN TP*/
 boolean CANTP_DriverWriteDataInCANTP(const uint32 i_RxID, const uint32 i_dataLen, const uint8 *i_pDataBuf);

/*register abort tx message to BUS*/
 void CANTP_RegisterAbortTxMsg(const tpfAbortTxMsg i_pfAbortTxMsg);

/*do tx message successful callback*/
 void CANTP_DoTxMsgSuccessfulCallBack(void);

/*Driver read data from LINTP*/
 boolean CANTP_DriverReadDataFromCANTP(const uint32 i_readDataLen, uint8 *o_pReadDataBuf, tTPTxMsgHeader *o_pstTxMsgHeader);


#endif /*#ifndef __CAN_TP_CFG_H__*/
/***************************End file********************************/

