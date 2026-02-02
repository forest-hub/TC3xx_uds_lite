#ifndef __FLS_CFC_H__
#define __FLS_CFC_H__

#include <drv/dflash.h>
#include "string.h"
#include "user_config.h"
#include "uds_cfc.h"


/*reset handler information*/
#define EN_WRITE_RESET_HANDLER_IN_FLASH   (FALSE)  /*enable write reset handler in flash or not*/
#define APP_VECTOR_TABLE_OFFSET           (4096u)  /*vector table offset from gs_astBlockNumA/B*/
#define RESET_HANDLE_OFFSET               (4u)     /*from top vector table to reset handle 508 = 127 * 4*/
#define RESET_HANDLER_ADDR_LEN            (4u)      /*pointer length or reset hanlder length*/


typedef void (*tpfResponse)(uint8);
typedef void (*tpfReuestMoreTime)(uint8, void (*)(uint8));

typedef enum
{
    FLASH_IDLE,                  /*flash idle*/
    FLASH_ERASING,               /*erase flash */
    FLASH_PROGRAMMING,           /*program flash*/
    FLASH_CHECKING,              /*check flash*/
    FLASH_WAITTING               /*waitting transmitted message successful*/
}tFlshJobModle;


typedef struct
{
    uint32 xBlockStartLogicalAddr;      /*block start logical addr*/
    uint32 xBlockEndLogicalAddr;        /*block end logical addr*/
}BlockInfo_t;

/* needed in the interface between flashloader runtime environment and security module */
typedef struct
{

    const uint32 address;              /* block start global address */
    const uint32 endress;
    const uint32 length;               /* block length */
    const uint16 maxProgAttempt;       /* maximum program cycle */
} FL_BlockDescriptorType;

typedef struct
{
    uint8 isFingerPrintWritten;                    /* flag if fingerprint has written */
    uint8 isFlashDrvDownloaded;                    /* flag if flash driver has downloaded */
    uint8 errorCode;                               /* error code for flash active job */
    uint8 erasedAPPFlag;                           /*erase app type flag*/
    uint8 requestActiveJobUDSSerID;                /*request active job UDS service ID*/
    uint8 aProgramDataBuff[MAX_FLASH_DATA_LEN];    /*storage program data buff*/
    uint32 startAddr;                              /* current procees start address */
    uint32 length;                                 /* current procees length */
    uint32 receivedDataStartAddr;                  /*recieve data start address*/
    uint32 receivedDataLength;                     /*received data length*/
    uint32 receivedCRC;                            /*received CRC value*/
    uint32 calculateCRCValue;                      /*calculate CRC value*/
    uint32 receiveProgramDataLength;               /*received program data length*/
    tFlDownloadStepType eDownloadStep;             /* flashloader download step */
    tFlshJobModle eActiveJob;                      /* current job status */
    tFlshJobModle eInterruptedJob;                 /*interrupted job*/
    tpfResponse pfActiveJobFinshedCallBack;        /*active job finshed callback*/
    tpfReuestMoreTime pfRequestMoreTime;           /*request more time from host*/
    tAppFlashStatus *pstAppFlashStatus;            /*point app flash status*/
    tFlashOperateAPI stFlashOperateAPI;            /*opeate flash API*/

}tFlsDownloadStateType;

tFlshJobModle Flash_GetOperateFlashActiveJob(void);
void   Flash_EraseFlashDriverInRAM(void);
uint32 FLASH_HAL_GetEraseFlashASectorMaxTimeMs(void);
uint32 FLASH_HAL_GetTotalSectors(const tAPPType i_appType);
tFlDownloadStepType Flash_GetCurDownloadStep(void);
uint8  Flash_ProgramRegion(const uint32 i_addr,const uint8 *i_pDataBuf,const uint32 i_dataLen);
void   Flash_SetNextDownloadStep(const tFlDownloadStepType i_donwloadStep);
uint8  UDS_IsDownloadDataAddrValid(const uint32 i_dataAddr, const uint32 i_dataLen);
void   Flash_SaveDownloadDataInfo(const uint32 i_dataStartAddr, const uint32 i_dataLen);
boolean FLASH_HAL_GetAPPInfo(const tAPPType i_appType, uint32 *o_pAppInfoStartAddr, uint32 *o_pBlockSize);
void   Flash_SetOperateFlashActiveJob(const tFlshJobModle i_activeJob, const tpfResponse i_pfActiveFinshedCallBack,
                                    const uint8 i_requestUDSSerID,const tpfReuestMoreTime i_pfRequestMoreTimeCallback);
#endif
