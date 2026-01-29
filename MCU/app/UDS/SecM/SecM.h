/*============================================================================*/
/** Copyright (C) 2009-2018, 10086 INFRASTRUCTURE SOFTWARE CO.,LTD.
 *
 *  All rights reserved. This software is 10086 property. Duplication
 *  or disclosure without 10086 written authorization is prohibited.
 *
 *
 *  @file       <SecM.h>
 *  @brief      <Macros,Types defines and function declarations for Security
 *              Module>
 *
 *  <Compiler: HighTec4.6    MCU:TC27x>
 *
 *  @author     <10086>
 *  @date       <2012-11-09>
 */
/*============================================================================*/
#ifndef SECM_H
#define SECM_H
#include "user_config.h"
#include "fls_app.h"
/*=======[R E V I S I O N   H I S T O R Y]====================================*/
/** <VERSION>  <DATE>  <AUTHOR>     <REVISION LOG>
 *  V1.0    20121109    Gary       Initial version
 *
 *  V1.1    20130913    ccl        update
 *
 *  V1.2    20180511    CChen      update
 */
/*============================================================================*/

/*=======[I N C L U D E S]====================================================*/


/*=======[M A C R O S]========================================================*/
#define CAL_CRC16 0x00u
#define CAL_CRC32 0x01u
#define CAL_METHOD CAL_CRC32

/** value k for security access */
/* @type:uint32 range:0x00000000~0xFFFFFFFF note:NONE */
#define SECM_ECU_KEY 0x8704162BU
/** CRC buffer length */
#define SECM_CRC_BUFFER_LEN 100

#define SECM_OK (uint8)0x00U
#define SECM_NOT_OK (uint8)0x01U

/** CRC step */
#define SECM_CRC_INIT 0x00u
#define SECM_CRC_COMPUTE 0x01u
#define SECM_CRC_FINALIZE 0x02u

#if (CAL_CRC32 == CAL_METHOD)
#define SECM_CRC_LENGTH 0x04u
#else
#define SECM_CRC_LENGTH 0x02u
#endif

/* frc base address */
#define FRC_REG_BASE (0x40u)
#define FRC_TCNTH (0x04u)
#define FRC_TCNTL (0x05u)
#define FRC_TSCR1 (0x06u)

/*=======[T Y P E   D E F I N I T I O N S]====================================*/
/** struct type for Crc */
typedef struct
{
    /* CRC step */
    uint8 crcState;
    /* current CRC value */
    uint32 currentCRC;

    /* CRC buffer point */
    const uint8 *crcSourceBuffer;

    /* CRC length */
    uint32 crcByteCount;

} SecM_CRCParamType;

/** struct type for verify parameter list */
typedef struct
{
    /* segment list for block */
    FL_SegmentListType *segmentList;

    /* Crc value transfered by UDS */
    const uint8 *verificationData;

    /* Crc value totle */
    uint32 crcTotle;

} SecM_VerifyParamType;

/*=======[E X T E R N A L   D A T A]==========================================*/
/** CRC buffer */


/*=======[E X T E R N A L   F U N C T I O N   D E C L A R A T I O N S]========*/
 uint8 SecM_GenerateSeed(uint32 *seed);

 uint8 SecM_ComputeKey(uint32 seed, uint32 mask, uint32 *key);

 uint8 SecM_CompareKey(uint32 key, uint32 seed);

 uint8 SecM_ComputeCRC(SecM_CRCParamType *crcParam);

 uint8 SecM_Verification(SecM_VerifyParamType *verifyParam);

#endif /* endof SECM_H */

/*=======[E N D   O F   F I L E]==============================================*/

