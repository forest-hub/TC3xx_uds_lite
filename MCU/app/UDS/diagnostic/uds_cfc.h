#ifndef __UDS__CFG_H__
#define __UDS__CFG_H__

/*********************Include headers************************/
#include "user_config.h"
#include "fls_app.h"
#include "dflash.h"
/**********************************************************/
#define BOOTLOADER_TYPE (0x0Fu)
#define BOOTLOADER_SW_VERSION {BOOTLOADER_TYPE, 0x02, 0x0, 0x00}
#define BOOTLOADER_HW_VERSION {BOOTLOADER_TYPE, 0x02, 0x0, 0x00}
#define MAX_FLASH_DATA_LEN (200u)



typedef struct
{
    uint32 xUdsId;
    uint32 xDataLen;
    uint8 aDataBuf[150u];
    /*tx message call back*/
    void (*pfUDSTxMsgServiceCallBack)(uint8);
} tUdsAppMsgInfo;


void  UDS_DoResetMCU(uint8 Txstatus);
void  Flash_SavedReceivedCheckSumCrc(uint32 i_receivedCrc);
void  UDS_DoEraseFlash(uint8 TxStatus);
void  UDS_DoCheckSum(uint8 TxStatus);
uint8 UDS_IsEraseMemoryRoutineControl(const tUdsAppMsgInfo *m_pstPDUMsg);
uint8 UDS_IsCheckSumRoutineControl(const tUdsAppMsgInfo *m_pstPDUMsg);
uint8 UDS_IsCheckProgrammingDependency(const tUdsAppMsgInfo *m_pstPDUMsg);
uint8 Flash_WriteFlashAppInfo(void);
uint8 UDS_DoCheckProgrammingDependency(void);
uint8 UDS_IsReceivedKeyRight(const uint8 *i_pReceivedKey,const uint8 *i_pTxSeed, const uint8 KeyLen);
uint8 UDS_IsGetVersion(const tUdsAppMsgInfo *m_pstPDUMsg);
#endif
