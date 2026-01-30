#include "uds_app.h"
#include "can_tp.h"
#include "uds_cfc.h"
/*********************************************************/

/*UDS init*/
void UDS_Init(void)
{

}

/*uds main function. ISO14229*/
void UDS_MainFun(void)
{
    uint8 UDSSerIndex = 0u;
    uint8 UDSSerNum = 0u;
    tUdsAppMsgInfo stUdsAppMsg = {0u, 0u, {0u}, NULL_PTR};
    uint8 isFindService = FALSE;
    uint8 SupSerItem = 0u;
    tUDSService *pstUDSService = NULL_PTR;

    if(TRUE == UDS_IsS3ServerTimeout())
    {
        /*If s3 server timeout, back default session mode.*/
        UDS_SetCurrentSession(DEFALUT_SESSION);

        /*set security level. If S3server timeout, clear current security.*/
        UDS_SetSecurityLevel(NONE_SECURITY);
    }

    /*read data from can tp*/
    if(TRUE == TP_ReadAFrameDataFromTP(&stUdsAppMsg.xUdsId,
                                        &stUdsAppMsg.xDataLen,
                                        stUdsAppMsg.aDataBuf))
    {
        UDS_SetIsRxUdsMsg(TRUE);

        if(TRUE != UDS_IsCurDefaultSession())
        {
            /*restart s3server time*/
            UDS_RestartS3Server();
        }

        /*save request id type.*/
        UDS_SaveRequestIdType(stUdsAppMsg.xUdsId);
        print("[%s]:RX read TP fifo . id: %x - data: %02x%02x%02x%02x %02x%02x%02x%02x \r\n",__func__,
                stUdsAppMsg.xUdsId,stUdsAppMsg.aDataBuf[0],stUdsAppMsg.aDataBuf[1],stUdsAppMsg.aDataBuf[2],
                stUdsAppMsg.aDataBuf[3],stUdsAppMsg.aDataBuf[4],stUdsAppMsg.aDataBuf[5],stUdsAppMsg.aDataBuf[6],stUdsAppMsg.aDataBuf[7]);
    }
    else
    {
        return;
    }

    /*get UDS service Information, start PTR and service item*/
    pstUDSService = UDS_GetUDSServiceInfo(&SupSerItem);

    /*get UDS service ID*/
    //can 数据长度在can_tp 写到RX_tp_fifo去掉了
    UDSSerNum = stUdsAppMsg.aDataBuf[0u];

    while((UDSSerIndex < SupSerItem) && (NULL_PTR != pstUDSService))
    {
        if(UDSSerNum == pstUDSService[UDSSerIndex].serNum)
        {
            isFindService = TRUE;
            if(TRUE != UDS_IsCurRxIdCanRequest(pstUDSService[UDSSerIndex].supReqMode))
            {
                /*received ID cann't request this service.*/
                UDS_SetNegativeErroCode(stUdsAppMsg.aDataBuf[0u], SNS, &stUdsAppMsg);
                break;
            }

            if(TRUE != UDS_IsCurSessionCanRequest(pstUDSService[UDSSerIndex].sessionMode))
            {
                /*currnet session mode cann't request ths service.*/
                UDS_SetNegativeErroCode(stUdsAppMsg.aDataBuf[0u], SNS, &stUdsAppMsg);
                break;
            }

            if(TRUE != UDS_IsCurSecurityLevelRequest(pstUDSService[UDSSerIndex].reqLevel))
            {
                /*current security level cann't request this service.*/
                UDS_SetNegativeErroCode(stUdsAppMsg.aDataBuf[0u], SNS, &stUdsAppMsg);
                break;
            }

            stUdsAppMsg.pfUDSTxMsgServiceCallBack = NULL_PTR;

            /*find service, and do it.*/
            if(NULL_PTR != pstUDSService[UDSSerIndex].pfSerNameFun)
            {
                pstUDSService[UDSSerIndex].pfSerNameFun((tUDSService *)&pstUDSService[UDSSerIndex], &stUdsAppMsg);
            }
            else
            {
                /*current security level cann't request this service.*/
                UDS_SetNegativeErroCode(stUdsAppMsg.aDataBuf[0u], SNS, &stUdsAppMsg);
            }

            break;
        }

        UDSSerIndex++;
    }

    if(TRUE != isFindService)
    {
        /*response not support service.*/
        UDS_SetNegativeErroCode(stUdsAppMsg.aDataBuf[0u], SNS, &stUdsAppMsg);
    }

     //数据如何发送出去的
     //在这里把数据写入TX_TP_FIFO
     //然后在 CANTP_DoCanTpIdle空闲任务里面，如果数据是空闲的，这个里面会调用 CANTP_CopyAFrameFromFifoToBuf 读取 TX_TP_FIFO
     //读取成功 m_peNextStatus  状态变成SF(单帧)或者FF，
     //然后 CANTP_MainFun 里面轮询状态机（m_peNextStatus）,把数据发送出去（调用对应 SF发送函数）

    if(0u != stUdsAppMsg.xDataLen)
    {
        stUdsAppMsg.xUdsId = TP_GetConfigTxMsgID();

        (void)TP_WriteAFrameDataInTP(stUdsAppMsg.xUdsId,
                                     stUdsAppMsg.pfUDSTxMsgServiceCallBack,
                                     stUdsAppMsg.xDataLen,
                                     stUdsAppMsg.aDataBuf);
    }
}


