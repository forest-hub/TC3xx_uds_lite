#ifndef __FLS_APP_H__
#define __FLS_APP_H__


#include "string.h"
#include "user_config.h"

#define FL_NUM_LOGICAL_BLOCKS                (11)
/* the length of programming attempt counter */

#define FL_FINGER_PRINT_LENGTH               (0x09U)
#define FL_MAX_SEGMENTS                      (60)

/*invalid UDS services ID*/
#define INVALID_UDS_SERVICES_ID              (0xFFu)
/*=======[T Y P E   D E F I N I T I O N S]====================================*/


/** flashloader download step */
typedef enum
{
    FL_REQUEST_STEP,             /*flash request step*/
    FL_TRANSFER_STEP,            /*flash transfer data step*/
    FL_EXIT_TRANSFER_STEP,       /*exit transfter data step*/
    FL_CHECKSUM_STEP             /*check sum step*/

}tFlDownloadStepType;

/*Erase flash status control*/
typedef enum
{
    START_ERASE_FLASH,            /*start erase flash*/
    DO_ERASING_FLASH,             /*Do erase flash*/
    END_ERASE_FLASH               /*end erase flash*/
}tEraseFlashStep;

/** Segment list information of the block */
typedef struct
{
    uint32 address;              /* Start global address of the segment in flash */
    uint32 length;               /* Length of the segment */
} FL_SegmentInfoType;

/** Needed in interface to the security module. */
typedef struct
{
    uint8 nrOfSegments;                                   /* number of segment */
    FL_SegmentInfoType segmentInfo[FL_MAX_SEGMENTS];      /* segments information */
} FL_SegmentListType;


/* handle the two segments in one page */
typedef struct
{
    uint32 remainAddr;            /* current process start address */
    uint32 remainLength;          /* current process length */
} FL_RemainDataType;

typedef void (*tWDTriggerFct)(void);

/** initialization: input parameters */
typedef struct
{
     uint8 patchLevel;                   /* flash driver patch level version */
     uint8 minorNumber;                  /* flash driver minor version number */
     uint8 majorNumber;                  /* flash driver major version number */
     uint8 reserved1;                    /* reserved for future use, set to 0x00 for now */
     uint8 errorCode;                    /* retrun value / error code: output parameters */
     uint16 reserved2;                   /* reserved for future use, set to 0x0000 for now */
     uint32 address;                     /* logical target address */
     uint32 length;                      /* lenght information (in bytes) */
    const uint8 *data;                   /* pointer to data buffer */
    tWDTriggerFct wdTriggerFct;          /* pointer to watchdog trigger routine */
} tFlashParam;


void  FLASH_APP_Init(void);
void  Flash_InitDowloadInfo(void);
void  Flash_OperateMainFunction(void);
void Flash_InitDowloadInfo(void);

#endif
