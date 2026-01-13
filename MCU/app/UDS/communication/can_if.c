#include "can_if.h"
#include "can_tp_cfg.h"

/*
    description: the enum have not init all member. Because, different project maybe have different MAX_TP_NO.
    The enum is a global enum and value.
*/

static tpfUDSTxMsgCallBack gs_pfUDSTxMsgCallBack = NULL_PTR; /*Tx message call back*/


/*Get TP config TX message ID*/
uint32 TP_GetConfigTxMsgID(void)
{
    uint32 txMsgID = 0u;
    txMsgID = CANTP_GetConfigTxMsgID();

    return txMsgID;
}

/*Get TP config recieve Funcation ID*/
uint32 TP_GetConfigRxMsgFUNID(void)
{
    uint32 rxMsgFUNID = 0u;
    rxMsgFUNID = CANTP_GetConfigRxMsgFUNID();

    return rxMsgFUNID;
}

/*Get TP config receive pyhiscal ID*/
uint32 TP_GetConfigRxMsgPHYID(void)
{
    uint32 rxMsgPHYID = 0u;

    rxMsgPHYID = CANTP_GetConfigRxMsgPHYID();

    return rxMsgPHYID;
}


/*register transmit a frame message call back*/
void TP_RegisterTransmittedAFrmaeMsgCallBack(const tpfUDSTxMsgCallBack i_pfTxMsgCallBack)
{
    gs_pfUDSTxMsgCallBack = (tpfUDSTxMsgCallBack)i_pfTxMsgCallBack;
}

/*do transmited a frame message call back*/
void TP_DoTransmittedAFrameMsgCallBack(const uint8 i_result)
{
    if(NULL_PTR != gs_pfUDSTxMsgCallBack)
    {
        (gs_pfUDSTxMsgCallBack)(i_result);
        gs_pfUDSTxMsgCallBack = NULL_PTR;
    }
}

/*Driver write data in TP for read message from BUS*/
boolean TP_DriverWriteDataInTP(const uint32 i_RxID, const uint32 i_RxDataLen, const uint8 *i_pRxDataBuf)
{
    boolean result = FALSE;

    ASSERT(NULL_PTR == i_pRxDataBuf);
    ASSERT(0u == i_RxDataLen);

    result = CANTP_DriverWriteDataInCANTP(i_RxID, i_RxDataLen, i_pRxDataBuf);

    return result;
}

/*Driver read data from TP for Tx message to BUS*/
boolean TP_DriverReadDataFromTP(const uint32 i_readDataLen, uint8 * o_pReadDatabuf, uint32 *o_pTxMsgID, uint32 *o_pTxMsgLength)
{
    boolean result = FALSE;
    tTPTxMsgHeader TPTxMsgHeader;


    ASSERT(0u == i_readDataLen);
    ASSERT(NULL_PTR == o_pReadDatabuf);
    ASSERT(NULL_PTR == o_pTxMsgID);
    ASSERT(NULL_PTR == o_pTxMsgLength);

    result = CANTP_DriverReadDataFromCANTP(i_readDataLen, o_pReadDatabuf, &TPTxMsgHeader);

    if(TRUE == result)
    {
        *o_pTxMsgID = TPTxMsgHeader.TxMsgID;
        *o_pTxMsgLength = TPTxMsgHeader.TxMsgLength;
    }

    return result;
}

/*register abort tx message*/
void TP_RegisterAbortTxMsg(void (*i_pfAbortTxMsg)(void))
{
    CANTP_RegisterAbortTxMsg((const tpfAbortTxMsg)i_pfAbortTxMsg);

}

/*do TP tx message successful callback*/
void TP_DoTxMsgSuccesfulCallback(void)
{
    CANTP_DoTxMsgSuccessfulCallBack();

}

/*FUNCTION**********************************************************************
 *
 * Function Name : TP_Init
 * Description   : This function initial this module.
 *
 * Implements : TP_Init_Activity
 *END**************************************************************************/
void TP_Init(void)
{

    CANTP_Init();
}

/*FUNCTION**********************************************************************
 *
 * Function Name : TP_mainFunc
 * Description   : This function main function this module.
 *
 * Implements : TP_Init_Activity
 *END**************************************************************************/
void TP_MainFun(void)
{
    CANTP_MainFun();

}

/*TP system tick control*/
void TP_SystemTickCtl(void)
{
    CANTP_SytstemTickControl();

}


/*read a frame from TP Rx FIFO. If no data can read return FALSE, else return TRUE*/
boolean TP_ReadAFrameDataFromTP(uint32 *o_pRxMsgID,
                                      uint32 *o_pxRxDataLen,
                                      uint8 *o_pDataBuf)
{
    tErroCode eStatus;
    uint16 xReadDataLen = 0u;

    tUDSAndTPExchangeMsgInfo exchangeMsgInfo;

    ASSERT(NULL_PTR == o_pRxMsgID);
    ASSERT(NULL_PTR == o_pDataBuf);
    ASSERT(NULL_PTR == o_pxRxDataLen);

    /*can read data from buf*/
    GetCanReadLen(RX_TP_QUEUE_ID, &xReadDataLen, &eStatus);
    if(ERRO_NONE != eStatus || (xReadDataLen < sizeof(tUDSAndTPExchangeMsgInfo)))
    {
        return FALSE;
    }

    /*read receive ID and data len*/
    ReadDataFromFifo(RX_TP_QUEUE_ID,
                    sizeof(exchangeMsgInfo),
                    (uint8 *)&exchangeMsgInfo,
                    &xReadDataLen,
                    &eStatus);
    if(ERRO_NONE != eStatus || sizeof(exchangeMsgInfo) != xReadDataLen)
    {
        print("Read data len erro!\n");
        return FALSE;
    }

    /*read data from fifo*/
    ReadDataFromFifo(RX_TP_QUEUE_ID,
                    exchangeMsgInfo.dataLen,
                    o_pDataBuf,
                    &xReadDataLen,
                    &eStatus);
    if(ERRO_NONE != eStatus || (exchangeMsgInfo.dataLen != xReadDataLen))
    {
        print("Read data erro!\r\n");
        return FALSE;
    }

    *o_pRxMsgID = exchangeMsgInfo.msgID;
    *o_pxRxDataLen = exchangeMsgInfo.dataLen;

    return TRUE;
}

/*write a frame data  to tp TX FIFO*/
boolean TP_WriteAFrameDataInTP(const uint32 i_TxMsgID,
                                     const tpfUDSTxMsgCallBack i_pfUDSTxMsgCallBack,
                                     const uint32 i_xTxDataLen,
                                     const uint8 *i_pDataBuf)
{
    tErroCode eStatus;
    uint16 xCanWriteLen = 0u;
    uint16 xWritDataLen = (uint16)i_xTxDataLen;

    tUDSAndTPExchangeMsgInfo exchangeMsgInfo;
    uint32 totalWriteDataLen = i_xTxDataLen + sizeof(tUDSAndTPExchangeMsgInfo);

    exchangeMsgInfo.msgID = (uint32)i_TxMsgID;
    exchangeMsgInfo.dataLen = (uint32)i_xTxDataLen;
    exchangeMsgInfo.pfCallBack = (tpfUDSTxMsgCallBack)i_pfUDSTxMsgCallBack;

    ASSERT(NULL_PTR == i_pDataBuf);

    /*check transmit ID*/
    if(i_TxMsgID != TP_GetConfigTxMsgID())
    {
        return FALSE;
    }

    if(0u == xWritDataLen)
    {
        return FALSE;
    }

    /*check can wirte data len*/
    GetCanWriteLen(TX_TP_QUEUE_ID, &xCanWriteLen, &eStatus);
    if(ERRO_NONE != eStatus || xCanWriteLen < totalWriteDataLen)
    {
        return FALSE;
    }

    /*write uds transmitt ID*/
    WriteDataInFifo(TX_TP_QUEUE_ID, (uint8 *)&exchangeMsgInfo, sizeof(tUDSAndTPExchangeMsgInfo), &eStatus);
    if(ERRO_NONE != eStatus)
    {
        return FALSE;
    }

    /*write data in fifo*/
    WriteDataInFifo(TX_TP_QUEUE_ID, (uint8 *)i_pDataBuf, xWritDataLen, &eStatus);
    if(ERRO_NONE != eStatus)
    {
        return FALSE;
    }

    return TRUE;
}



/*FUNCTION**********************************************************************
 *
 * Function Name : TP_Deinit
 * Description   : This function initial this module.
 *
 * Implements : TP_Deinit_Activity
 *END**************************************************************************/
void TP_Deinit(void)
{

}
