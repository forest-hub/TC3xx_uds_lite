#ifndef __FLS_CFC_H__
#define __FLS_CFC_H__

#include <drv/dflash.h>
#include "string.h"
#include "user_config.h"
#include "uds_cfc.h"


#define FL_USE_GAP_FILL  0

#define FL_NUM_LOGICAL_BLOCKS                (uint8)11
/*reset handler information*/
#define EN_WRITE_RESET_HANDLER_IN_FLASH (FALSE)  /*enable write reset handler in flash or not*/
#define APP_VECTOR_TABLE_OFFSET (4096u)  /*vector table offset from gs_astBlockNumA/B*/
#define RESET_HANDLE_OFFSET     (4u)   /*from top vector table to reset handle 508 = 127 * 4*/
#define RESET_HANDLER_ADDR_LEN  (4u)    /*pointer length or reset hanlder length*/
#define FL_NVM_INFO_ADDRESS                  0xAF000000U


typedef void (*tpfResponse)(uint8);
typedef void (*tpfReuestMoreTime)(uint8, void (*)(uint8));

typedef enum
{
    FLASH_IDLE,           /*flash idle*/
    FLASH_ERASING,        /*erase flash */
    FLASH_PROGRAMMING,    /*program flash*/
    FLASH_CHECKING,       /*check flash*/
    FLASH_WAITTING       /*waitting transmitted message successful*/
}tFlshJobModle;


typedef struct
{
    uint32 xBlockStartLogicalAddr; /*block start logical addr*/
    uint32 xBlockEndLogicalAddr;   /*block end logical addr*/
}BlockInfo_t;

/* needed in the interface between flashloader runtime environment and security module */
typedef struct
{
    /* block start global address */
    const uint32 address;
    const uint32 endress;
    /* block length */
    const uint32 length;
    /* maximum program cycle */
    const uint16 maxProgAttempt;

} FL_BlockDescriptorType;
typedef struct
{
    /* flag if fingerprint has written */
    uint8 isFingerPrintWritten;

    /* flag if flash driver has downloaded */
    uint8 isFlashDrvDownloaded;

    /* error code for flash active job */
    uint8 errorCode;

    /*erase app type flag*/
    uint8 erasedAPPFlag;

    /*request active job UDS service ID*/
    uint8 requestActiveJobUDSSerID;

    /*storage program data buff*/
    uint8 aProgramDataBuff[MAX_FLASH_DATA_LEN];

    /* current procees start address */
    uint32 startAddr;

    /* current procees length */
    uint32 length;

    /*recieve data start address*/
    uint32 receivedDataStartAddr;

    /*received data length*/
    uint32 receivedDataLength;

    /*received CRC value*/
    uint32 receivedCRC;

    /*calculate CRC value*/
    uint32 calculateCRCValue;

    /*received program data length*/
    uint32 receiveProgramDataLength;

    /* flashloader download step */
    tFlDownloadStepType eDownloadStep;

    /* current job status */
    tFlshJobModle eActiveJob;

    /*interrupted job*/
    tFlshJobModle eInterruptedJob;

    /*active job finshed callback*/
    tpfResponse pfActiveJobFinshedCallBack;

    /*request more time from host*/
    tpfReuestMoreTime pfRequestMoreTime;

    /*point app flash status*/
    tAppFlashStatus *pstAppFlashStatus;

    /*opeate flash API*/
    tFlashOperateAPI stFlashOperateAPI;

}tFlsDownloadStateType;

void FL_InitState(void);
void Flash_EraseFlashDriverInRAM(void);
void Flash_SetNextDownloadStep(const tFlDownloadStepType i_donwloadStep);
boolean FLASH_HAL_GetAPPInfo(const tAPPType i_appType, uint32 *o_pAppInfoStartAddr, uint32 *o_pBlockSize);
void Flash_SetOperateFlashActiveJob(const tFlshJobModle i_activeJob, const tpfResponse i_pfActiveFinshedCallBack,
                                    const uint8 i_requestUDSSerID,const tpfReuestMoreTime i_pfRequestMoreTimeCallback);
#endif
