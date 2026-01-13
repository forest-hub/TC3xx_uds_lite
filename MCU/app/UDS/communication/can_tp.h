#ifndef __CAN_TP_H__
#define __CAN_TP_H__

#include "user_config.h"
#include "can_tp_cfg.h"
#include "uds_can_fifo_list.h"
#include "can_if.h"


/*CAN/CAN FD define*/
typedef enum
{
    CANTP_STANDARD,  /*standard CAN*/
    CANTP_FD,        /*CAN FD*/
}tCANType;

/*********************************************************
**  SF  --  signle frame
**  FF  --  first frame
**  FC  --  flow control
**  CF  --  consective frame
*********************************************************/

typedef enum
{
    IDLE,      /*idle*/
    RX_SF,   /*wait signle frame*/
    RX_FF,   /*wait first frame*/
    RX_FC,   /*wait flow control frame*/
    RX_CF,   /*wait consective frame*/

    TX_SF,     /*tx signle frame*/
    TX_FF,     /*tx first frame*/
    TX_FC,     /*tx flow control*/
    TX_CF,     /*tx consective frame*/

    WAITTING_TX, /*watting tx message*/

    WAIT_CONFIRM /*wait confrim*/
}tCanTpWorkStatus;

typedef enum
{
    SF,        /*signle frame value*/
    FF,        /*first frame value*/
    CF,        /*consective frame value*/
    FC         /*flow control value*/
}tNetWorkFrameType;

typedef enum
{
    CONTINUE_TO_SEND, /*continue to send*/
    WAIT_FC,          /*wait flow control*/
    OVERFLOW_BUF      /*overflow buf*/
}tFlowStatus;

typedef enum
{
    N_OK = 0,    /*This value means that the service execution has completed successfully; it can be issued to a service user on both the sender and receiver side*/
    N_TIMEOUT_A, /*This value is issued to the protocol user when the timer N_Ar/N_As has passed its time-out
                            value N_Asmax/N_Armax; it can be issued to service user on both the sender and receiver side.*/
    N_TIMEOUT_Bs, /*This value is issued to the service user when the timer N_Bs has passed its time-out value
                                N_Bsmax; it can be issued to the service user on the sender side only.*/
    N_TIMEOUT_Cr, /*This value is issued to the service user when the timer N_Cr has passed its time-out value
                    N_Crmax; it can be issued to the service user on the receiver side only.*/
    N_WRONG_SN,   /*This value is issued to the service user upon reception of an unexpected sequence number
                    (PCI.SN) value; it can be issued to the service user on the receiver side only.*/
    N_INVALID_FS, /*This value is issued to the service user when an invalid or unknown FlowStatus value has
                    been received in a flow control (FC) N_PDU; it can be issued to the service user on the sender side only.*/
    N_UNEXP_PDU,  /*This value is issued to the service user upon reception of an unexpected protocol data unit;
                    it can be issued to the service user on the receiver side only.*/
    N_WTF_OVRN,   /*This value is issued to the service user upon reception of flow control WAIT frame that
                    exceeds the maximum counter N_WFTmax.*/
    N_BUFFER_OVFLW, /*This value is issued to the service user upon reception of a flow control (FC) N_PDU with
                    FlowStatus = OVFLW. It indicates that the buffer on the receiver side of a segmented
                    message transmission cannot store the number of bytes specified by the FirstFrame
                    DataLength (FF_DL) parameter in the FirstFrame and therefore the transmission of the
                    segmented message was aborted. It can be issued to the service user on the sender side
                    only.*/
    N_ERROR       /*This is the general error value. It shall be issued to the service user when an error has been
                    detected by the network layer and no other parameter value can be used to better describe
                    the error. It can be issued to the service user on both the sender and receiver side.*/
}tN_Result;

typedef enum{
    CANTP_TX_MSG_IDLE = 0, /*CAN TP tx message idle*/
    CANTP_TX_MSG_SUCC,     /*CAN TP tx message successful*/
    CANTP_TX_MSG_FAIL,     /*CAN TP tx message fail*/
    CANTP_TX_MSG_WAITTING /*CAN TP waitting tx message*/
}tCanTPTxMsgStatus;

typedef struct
{
    uint32 xCanTpId;                    /*can tp message id*/
    uint16 xPduDataLen;                 /*pdu data len(Rx/Tx data len)*/
    uint16 xFFDataLen;                  /*Rx/Tx FF data len*/
    uint8  aDataBuf[MAX_CF_DATA_LEN];   /*Rx/Tx data buf*/
}tCanTpDataInfo;

typedef struct
{
    uint8 ucSN;          /*SN*/
    uint8 ucBlockSize;   /*Block size*/
    uint16 xSTmin;             /*STmin*/
    uint16 xMaxWatiTimeout;    /*timeout time*/
    tCanTpDataInfo stCanTpDataInfo;
}tCanTpInfo;

typedef struct
{
    uint8 isFree;            /*rx message status. TRUE = not received messag.*/
    uint32 xMsgId;                     /*received message id*/
    uint8 msgLen;            /*received message len*/
    uint8 aMsgBuf[MAX_CAN_DATA_LEN]; /*message data buf*/
}tCanTpMsg;

typedef tN_Result (*tpfCanTpFun)(tCanTpMsg *, tCanTpWorkStatus *);
typedef struct
{
    tCanTpWorkStatus eCanTpStaus;
    tpfCanTpFun pfCanTpFun;
}tCanTpFunInfo;

/*uds network man function*/
void CANTP_MainFun(void);

/*can tp system tick control*/
void CANTP_SytstemTickControl(void);

/*Init CAN TP list*/
void CANTP_Init(void);

#endif /*#ifndef __CAN_TP_H__*/

/***************************End file********************************/

