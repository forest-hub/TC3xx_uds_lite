
#include "uds_cfc.h"
#include "can_tp.h"
#include "can_if.h"
#include "uds_server.h"
#include "fl_cfc.h"


/*erase memory routine cotnrol ID*/
const static uint8 gs_aEraseMemoryRoutineControlId[] = {0x31u, 0x01u, 0xFFu, 0x00u};
/*check sum routine control ID*/
const static uint8 gs_aCheckSumRoutineControlId[] = {0x31u, 0x01u, 0x02u, 0x02u};
/*check programming dependency*/
const static uint8 gs_aCheckProgrammingDependencyId[] = {0x31u, 0x01u, 0xFFu, 0x01u};

/*Get bootloader version*/
const static uint8 gs_aGetVersion[] = {0x31u, 0x01, 0x03, 0xFFu};



/*check routine control right?*/
static uint8 UDS_IsCheckRoutineControlRight(const tCheckRoutineCtlInfo i_eCheckRoutineCtlId,
                                        const tUdsAppMsgInfo *m_pstPDUMsg)
{
    uint8 Index = 0u;
    uint8 FindCnt = 0u;
    uint8 *pDestRoutineCltId = NULL_PTR;

    ASSERT(NULL_PTR == m_pstPDUMsg);

    switch(i_eCheckRoutineCtlId)
    {
    case ERASE_MEMORY_ROUTINE_CONTROL :
        pDestRoutineCltId = (uint8 *)&gs_aEraseMemoryRoutineControlId[0u];

        FindCnt = sizeof(gs_aEraseMemoryRoutineControlId);

        break;

    case CHECK_SUM_ROUTINE_CONTROL :
        pDestRoutineCltId = (uint8 *)&gs_aCheckSumRoutineControlId[0u];

        FindCnt = sizeof(gs_aCheckSumRoutineControlId);

        break;

    case CHECK_DEPENDENCY_ROUTINE_CONTROL :
        pDestRoutineCltId = (uint8 *)&gs_aCheckProgrammingDependencyId[0u];

        FindCnt = sizeof(gs_aCheckProgrammingDependencyId);

        break;

    case GET_VERSION:
        pDestRoutineCltId = (uint8 *)&gs_aGetVersion[0u];
        FindCnt = sizeof(gs_aGetVersion);

        break;

    default :

        return FALSE;

        /*This is not have break*/
    }

    if((NULL_PTR == pDestRoutineCltId) || (m_pstPDUMsg->xDataLen < FindCnt))
    {
        return FALSE;
    }

    while(Index < FindCnt)
    {
        if(m_pstPDUMsg->aDataBuf[Index] != pDestRoutineCltId[Index])
        {
            return FALSE;
        }

        Index++;
    }

    return TRUE;
}

/*do erase flash response*/
static void UDS_DoEraseFlashResponse(uint8 i_Status)
{
    uint8 Index = 0u;
    uint8 aResponseBuf[8u] = {0u};
    uint8 TxDataLen = 0u;
    uint32 UdsTxId = 0u;

    TxDataLen = sizeof(gs_aEraseMemoryRoutineControlId) / sizeof(gs_aEraseMemoryRoutineControlId[0u]);
    aResponseBuf[0u] = gs_aEraseMemoryRoutineControlId[0u] + 0x40u;

    for(Index = 0u; Index < TxDataLen - 1u; Index++)
    {
        aResponseBuf[Index + 1u] = gs_aEraseMemoryRoutineControlId[Index + 1u];
    }

    if(TRUE == i_Status)
    {
        aResponseBuf[TxDataLen] = 0u;
    }
    else
    {
        aResponseBuf[TxDataLen] = 1u;
    }

    TxDataLen++;

    UdsTxId = TP_GetConfigTxMsgID();

    (void)TP_WriteAFrameDataInTP(UdsTxId, NULL_PTR, TxDataLen, aResponseBuf);
}

/*do response checksum*/
static void UDS_DoResponseChecksum(uint8 i_Status)
{
    uint8 Index = 0u;
    uint8 aResponseBuf[8u] = {0u};
    uint8 TxDataLen = 0u;
    uint32 UdsTxId = 0u;

    TxDataLen = sizeof(gs_aCheckSumRoutineControlId) / sizeof(gs_aCheckSumRoutineControlId[0u]);
    aResponseBuf[0u] = gs_aCheckSumRoutineControlId[0u] + 0x40u;

    for(Index = 0u; Index < TxDataLen - 1u; Index++)
    {
        aResponseBuf[Index + 1u] = gs_aCheckSumRoutineControlId[Index + 1u];
    }

    if(TRUE == i_Status)
    {
        aResponseBuf[TxDataLen] = 0u;
    }
    else
    {
        aResponseBuf[TxDataLen] = 1u;

        print("%s: Check CRC faild!\n",__func__);
    }

    TxDataLen++;

    UdsTxId = TP_GetConfigTxMsgID();

    (void)TP_WriteAFrameDataInTP(UdsTxId, NULL_PTR, TxDataLen, aResponseBuf);
}

/*do erase flash*/
 void UDS_DoEraseFlash(uint8 TxStatus)
{
    if(TX_MSG_SUCCESSFUL == TxStatus)
    {
        /*do erase flash need request client delay timeout*/
        Flash_SetOperateFlashActiveJob(FLASH_ERASING, &UDS_DoEraseFlashResponse, 0x31, &UDS_RequestMoreTime);
    }
}



 /*do check sum. If check sum right return TRUE, else return FALSE.*/
 void UDS_DoCheckSum(uint8 TxStatus)
 {
     if(TX_MSG_SUCCESSFUL == TxStatus)
     {
         /*need request client delay time for flash checking flash data*/
         Flash_SetOperateFlashActiveJob(FLASH_CHECKING, &UDS_DoResponseChecksum, 0x31u, &UDS_RequestMoreTime);
     }
 }

 /*Is erase memory routine control?*/
 uint8 UDS_IsEraseMemoryRoutineControl(const tUdsAppMsgInfo *m_pstPDUMsg)
 {
     ASSERT(NULL_PTR == m_pstPDUMsg);

     return UDS_IsCheckRoutineControlRight(ERASE_MEMORY_ROUTINE_CONTROL, m_pstPDUMsg);
 }

 /*Is check sum routine control?*/
 uint8 UDS_IsCheckSumRoutineControl(const tUdsAppMsgInfo *m_pstPDUMsg)
 {
     ASSERT(NULL_PTR == m_pstPDUMsg);

     return UDS_IsCheckRoutineControlRight(CHECK_SUM_ROUTINE_CONTROL, m_pstPDUMsg);
 }

 /*Is check programming dependency?*/
 uint8 UDS_IsCheckProgrammingDependency(const tUdsAppMsgInfo *m_pstPDUMsg)
 {
     ASSERT(NULL_PTR == m_pstPDUMsg);

     return UDS_IsCheckRoutineControlRight(CHECK_DEPENDENCY_ROUTINE_CONTROL, m_pstPDUMsg);
 }

