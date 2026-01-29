#ifndef __FLS_APP_H__
#define __FLS_APP_H__

#include <drv/dflash.h>
#include "string.h"
#include "user_config.h"

/* macro of ECU Hardware Number length */
#define HW_VER_LENGTH       (16)
/* macro of Boot Software Identification length */
#define SW_REFNUM_LENGTH    (10)
/* the length of programming counter */
#define PROG_COUNTER_LENGTH  (1)
#define FL_NUM_LOGICAL_BLOCKS                 (uint8)11
/* the length of programming attempt counter */
#define PROG_ATTEMPT_LENGTH 1
#define FL_FINGER_PRINT_LENGTH          (0x09U)
#define FL_FINGER_PRINT_OFFSET          (0x04U)
#define FL_MAX_SEGMENTS                 (60)
#define kFlashOk 0x00u
#define kFlashFailed 0x01u
#define FL_OK 0x00U
#define FL_FAILED 0x01U
#define FL_ERR_SEQUENCE 0x02U
#define FL_NO_FINGERPRINT 0x03U
#define FL_NO_FLASHDRIVER 0x04U
#define FL_ERR_ADDR_LENGTH 0x05U
#define FL_INVALID_DATA 0x06U
#define FL_UPDATING_NVM 0x07U
#define FLS_USED STD_OFF
#define FL_GAP_FILL_VALUE                    0xFFU
#define FL_PROGRAM_SIZE    0x100U
#define      FL_FLASH_ALIGN_SIZE   (4)
/** Motorola Star12 */
#define TFLASH_DRIVER_VERSION_MCUTYPE 0x12u

/** some mask number */
#define TFLASH_DRIVER_VERSION_MASKTYPE 0xabu

/** interface version number */
#define TFLASH_DRIVER_VERSION_INTERFACE 0x01u

/** major version number / interface */
#define FLASH_DRIVER_VERSION_MAJOR 0x01u

/** minor version number / internal */
#define FLASH_DRIVER_VERSION_MINOR 0x01u

/** bugfix / patchlevel */
#define FLASH_DRIVER_VERSION_PATCH 0x00u

/** config if flash driver is compiled */
#define TFLASH_COMPILED STD_OFF
/*=======[T Y P E   D E F I N I T I O N S]====================================*/
/** flashloader job status */
typedef enum
{
    FL_JOB_IDLE,

    FL_JOB_ERASING,

    FL_JOB_PROGRAMMING,

    FL_JOB_CHECKING

} FL_ActiveJobType;

/** flashloader download step */
typedef enum
{
    FL_REQUEST_STEP,

    FL_TRANSFER_STEP,

    FL_EXIT_TRANSFER_STEP,

    FL_CHECKSUM_STEP

} FL_DownloadStepType;

/** Segment list information of the block */
typedef struct
{
    /* Start global address of the segment in flash */
    uint32 address;

    /* Length of the segment */
    uint32 length;

} FL_SegmentInfoType;

/** Needed in interface to the security module. */
typedef struct
{
    /* number of segment */
    uint8 nrOfSegments;

    /* segments information */
    FL_SegmentInfoType segmentInfo[FL_MAX_SEGMENTS];

} FL_SegmentListType;

/** flashloader status information */
typedef struct
{
    /* flag if finger print has written to NVM */
    boolean fingerPrintWrittenFlag;
    /* repair shop code buffer */
    /* flag if finger print has written */
    boolean fingerPrintWritten;

    /* flag if finger print buffer */
    uint8 fingerPrint[FL_FINGER_PRINT_LENGTH];

    /* flag if flash driver has downloaded */
    boolean flDrvDownloaded;

    /* error code for flash active job */
    uint8 errorCode;

    /* flag if current block is erased */
    boolean blockErased;

    /* current process block index */
    uint8 blockIndex;

    /* current process start address */
    uint32 startAddr;

    /* current process length */
    uint32 downLength;

    /* current process buffer point, point to buffer supplied from DCM */
    const uint8 *dataBuff;

    /* segment list of current process block */
    FL_SegmentListType segmentList;

    /* flashloader download step */
    FL_DownloadStepType downloadStep;

    /* current job status */
    FL_ActiveJobType activeJob;

} FL_DownloadStateType;

/* handle the two segments in one page */
typedef struct
{
    /* current process start address */
    uint32 remainAddr;

    /* current process length */
    uint32 remainLength;

} FL_RemainDataType;

typedef void (*tWDTriggerFct)(void);

/** initialization: input parameters */
typedef struct
{
    /* flash driver patch level version */
        uint8 patchLevel;

    /* flash driver minor version number */
        uint8 minorNumber;

    /* flash driver major version number */
        uint8 majorNumber;

    /* reserved for future use, set to 0x00 for now */
        uint8 reserved1;

    /* retrun value / error code: output parameters */
        uint8 errorCode;

    /* reserved for future use, set to 0x0000 for now */
        uint16 reserved2;

    /* erase / write: input parameters */
    /* logical target address */
        uint32 address;

    /* lenght information (in bytes) */
        uint32 length;

    /* pointer to data buffer */
    const uint8 *data;

    /* pointer to watchdog trigger routine */
    tWDTriggerFct wdTriggerFct;

} tFlashParam;


/** Each logical block information */
typedef struct
{
    boolean blkValid;

    uint8 blkProgAttempt;

    uint8 blkProgCounter;

    uint8 reserved;

    uint8 fingerPrint[FL_FINGER_PRINT_LENGTH];

    uint32 blkChecksum;

} FL_blockInfoType;

typedef struct
{
    FL_blockInfoType blockInfo[FL_NUM_LOGICAL_BLOCKS];

    /* Security access error counter */
    uint8 secAccessErr;

    uint8 reserved1;

    uint8 reserved2;

    uint8 reserved3;

    /* NVM information checksum */
    uint32 infoChecksum;

} FL_NvmInfoType;
void FLASH_APP_Init(void);
void Flash_InitDowloadInfo(void);
void Flash_OperateMainFunction(void);
uint8 Flash_Read(uint32 readAddr, void *pBuf, uint32 length);
uint8 Flash_Erase(void);
uint8 Flash_Write(void);


#endif
