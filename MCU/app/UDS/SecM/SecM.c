/*============================================================================*/
/** Copyright (C) 2009-2018, 10086 INFRASTRUCTURE SOFTWARE CO.,LTD.
 *
 *  All rights reserved. This software is 10086 property. Duplication
 *  or disclosure without 10086 written authorization is prohibited.
 *
 *  @file       <SecM.c>
 *  @brief      <Security set>
 *
 *              <seed and key generate for UDS>
 *
 *  <Compiler: HighTec4.6    MCU:TC27x>
 *
 *  @author     <ccl>
 *  @date       <2013-09-13>
 */
/*============================================================================*/

/*=======[R E V I S I O N   H I S T O R Y]====================================*/
/** <VERSION>  <DATE>  <AUTHOR>     <REVISION LOG>
 *  V1.0    20121109    Gary       Initial version
 *
 *  V1.1    20130517    liuyp      modify the generate key algorithm
 *
 *  V1.2    20130913    ccl        update
 *
 *  V1.3    20180511    CChen      update
 */
/*============================================================================*/

/*=======[I N C L U D E S]====================================================*/
#include "SecM.h"
#include "stdlib.h"


uint8 SecM_CrcDataBuffer[SECM_CRC_BUFFER_LEN];

static uint8 SecM_ProcessCrc(SecM_CRCParamType *crcParam,uint32 Address, uint32 Length);


const uint32 Cal_Crc32Tab[256] =
{
    0x00000000uL, 0x77073096uL, 0xee0e612cuL, 0x990951bauL,
    0x076dc419uL, 0x706af48fuL, 0xe963a535uL, 0x9e6495a3uL,
    0x0edb8832uL, 0x79dcb8a4uL, 0xe0d5e91euL, 0x97d2d988uL,
    0x09b64c2buL, 0x7eb17cbduL, 0xe7b82d07uL, 0x90bf1d91uL,
    0x1db71064uL, 0x6ab020f2uL, 0xf3b97148uL, 0x84be41deuL,
    0x1adad47duL, 0x6ddde4ebuL, 0xf4d4b551uL, 0x83d385c7uL,
    0x136c9856uL, 0x646ba8c0uL, 0xfd62f97auL, 0x8a65c9ecuL,
    0x14015c4fuL, 0x63066cd9uL, 0xfa0f3d63uL, 0x8d080df5uL,
    0x3b6e20c8uL, 0x4c69105euL, 0xd56041e4uL, 0xa2677172uL,
    0x3c03e4d1uL, 0x4b04d447uL, 0xd20d85fduL, 0xa50ab56buL,
    0x35b5a8fauL, 0x42b2986cuL, 0xdbbbc9d6uL, 0xacbcf940uL,
    0x32d86ce3uL, 0x45df5c75uL, 0xdcd60dcfuL, 0xabd13d59uL,
    0x26d930acuL, 0x51de003auL, 0xc8d75180uL, 0xbfd06116uL,
    0x21b4f4b5uL, 0x56b3c423uL, 0xcfba9599uL, 0xb8bda50fuL,
    0x2802b89euL, 0x5f058808uL, 0xc60cd9b2uL, 0xb10be924uL,
    0x2f6f7c87uL, 0x58684c11uL, 0xc1611dabuL, 0xb6662d3duL,
    0x76dc4190uL, 0x01db7106uL, 0x98d220bcuL, 0xefd5102auL,
    0x71b18589uL, 0x06b6b51fuL, 0x9fbfe4a5uL, 0xe8b8d433uL,
    0x7807c9a2uL, 0x0f00f934uL, 0x9609a88euL, 0xe10e9818uL,
    0x7f6a0dbbuL, 0x086d3d2duL, 0x91646c97uL, 0xe6635c01uL,
    0x6b6b51f4uL, 0x1c6c6162uL, 0x856530d8uL, 0xf262004euL,
    0x6c0695eduL, 0x1b01a57buL, 0x8208f4c1uL, 0xf50fc457uL,
    0x65b0d9c6uL, 0x12b7e950uL, 0x8bbeb8eauL, 0xfcb9887cuL,
    0x62dd1ddfuL, 0x15da2d49uL, 0x8cd37cf3uL, 0xfbd44c65uL,
    0x4db26158uL, 0x3ab551ceuL, 0xa3bc0074uL, 0xd4bb30e2uL,
    0x4adfa541uL, 0x3dd895d7uL, 0xa4d1c46duL, 0xd3d6f4fbuL,
    0x4369e96auL, 0x346ed9fcuL, 0xad678846uL, 0xda60b8d0uL,
    0x44042d73uL, 0x33031de5uL, 0xaa0a4c5fuL, 0xdd0d7cc9uL,
    0x5005713cuL, 0x270241aauL, 0xbe0b1010uL, 0xc90c2086uL,
    0x5768b525uL, 0x206f85b3uL, 0xb966d409uL, 0xce61e49fuL,
    0x5edef90euL, 0x29d9c998uL, 0xb0d09822uL, 0xc7d7a8b4uL,
    0x59b33d17uL, 0x2eb40d81uL, 0xb7bd5c3buL, 0xc0ba6caduL,
    0xedb88320uL, 0x9abfb3b6uL, 0x03b6e20cuL, 0x74b1d29auL,
    0xead54739uL, 0x9dd277afuL, 0x04db2615uL, 0x73dc1683uL,
    0xe3630b12uL, 0x94643b84uL, 0x0d6d6a3euL, 0x7a6a5aa8uL,
    0xe40ecf0buL, 0x9309ff9duL, 0x0a00ae27uL, 0x7d079eb1uL,
    0xf00f9344uL, 0x8708a3d2uL, 0x1e01f268uL, 0x6906c2feuL,
    0xf762575duL, 0x806567cbuL, 0x196c3671uL, 0x6e6b06e7uL,
    0xfed41b76uL, 0x89d32be0uL, 0x10da7a5auL, 0x67dd4accuL,
    0xf9b9df6fuL, 0x8ebeeff9uL, 0x17b7be43uL, 0x60b08ed5uL,
    0xd6d6a3e8uL, 0xa1d1937euL, 0x38d8c2c4uL, 0x4fdff252uL,
    0xd1bb67f1uL, 0xa6bc5767uL, 0x3fb506dduL, 0x48b2364buL,
    0xd80d2bdauL, 0xaf0a1b4cuL, 0x36034af6uL, 0x41047a60uL,
    0xdf60efc3uL, 0xa867df55uL, 0x316e8eefuL, 0x4669be79uL,
    0xcb61b38cuL, 0xbc66831auL, 0x256fd2a0uL, 0x5268e236uL,
    0xcc0c7795uL, 0xbb0b4703uL, 0x220216b9uL, 0x5505262fuL,
    0xc5ba3bbeuL, 0xb2bd0b28uL, 0x2bb45a92uL, 0x5cb36a04uL,
    0xc2d7ffa7uL, 0xb5d0cf31uL, 0x2cd99e8buL, 0x5bdeae1duL,
    0x9b64c2b0uL, 0xec63f226uL, 0x756aa39cuL, 0x026d930auL,
    0x9c0906a9uL, 0xeb0e363fuL, 0x72076785uL, 0x05005713uL,
    0x95bf4a82uL, 0xe2b87a14uL, 0x7bb12baeuL, 0x0cb61b38uL,
    0x92d28e9buL, 0xe5d5be0duL, 0x7cdcefb7uL, 0x0bdbdf21uL,
    0x86d3d2d4uL, 0xf1d4e242uL, 0x68ddb3f8uL, 0x1fda836euL,
    0x81be16cduL, 0xf6b9265buL, 0x6fb077e1uL, 0x18b74777uL,
    0x88085ae6uL, 0xff0f6a70uL, 0x66063bcauL, 0x11010b5cuL,
    0x8f659effuL, 0xf862ae69uL, 0x616bffd3uL, 0x166ccf45uL,
    0xa00ae278uL, 0xd70dd2eeuL, 0x4e048354uL, 0x3903b3c2uL,
    0xa7672661uL, 0xd06016f7uL, 0x4969474duL, 0x3e6e77dbuL,
    0xaed16a4auL, 0xd9d65adcuL, 0x40df0b66uL, 0x37d83bf0uL,
    0xa9bcae53uL, 0xdebb9ec5uL, 0x47b2cf7fuL, 0x30b5ffe9uL,
    0xbdbdf21cuL, 0xcabac28auL, 0x53b39330uL, 0x24b4a3a6uL,
    0xbad03605uL, 0xcdd70693uL, 0x54de5729uL, 0x23d967bfuL,
    0xb3667a2euL, 0xc4614ab8uL, 0x5d681b02uL, 0x2a6f2b94uL,
    0xb40bbe37uL, 0xc30c8ea1uL, 0x5a05df1buL, 0x2d02ef8duL
};

/******************************************************************************/
/**
 * @brief               <CRC32 initialize>
 *
 * <This Function Initializes the CRC algorithm> .
 * Service ID   :       <NONE>
 * Sync/Async   :       <Synchronous>
 * Reentrancy           <Reentrant>
 * @param[in]           <NONE>
 * @param[out]          <NONE>
 * @param[in/out]       <curCrc (IN/OUT)>
 * @return              <NONE>
 */
/******************************************************************************/
void Cal_CrcInit(uint32 *curCrc)
{
#if (CAL_CRC32 == CAL_METHOD)
    *curCrc = 0xFFFFFFFFU;
#else
    /* CRC16 */
    *curCrc = 0xFFFFU;
#endif

    return;
}

/******************************************************************************/
/**
 * @brief               <CRC32 compute>
 *
 * <This Function computes the CRC value> .
 * Service ID   :       <NONE>
 * Sync/Async   :       <Synchronous>
 * Reentrancy           <Reentrant>
 * @param[in]           <buf (IN), size (IN)>
 * @param[out]          <NONE>
 * @param[in/out]       <curCrc (IN/OUT)>
 * @return              <NONE>
 */
/******************************************************************************/

void Cal_CrcCal(uint32 *curCrc, const uint8 *buf, const uint32 size)
{
    uint32 i;
#ifdef BootloaderVersion
    uint8 idx;
#endif
#if (CAL_CRC32 == CAL_METHOD)
    for (i = 0UL; i < size; i++)
    {
#if BootloaderVersion
        *curCrc ^= (uint32)buf[i];
        for (idx = 0; idx < 8; idx++)
        {
            *curCrc = (*curCrc & 0x00000001) ? ((*curCrc >> 1) ^ DAI_CRC32_POLYNOMIAL) : (*curCrc >> 1);
        }
#else
        *curCrc = Cal_Crc32Tab[(*curCrc ^ (uint32)buf[i]) & 0xffu] ^ (*curCrc >> 8);
#endif
    }
#else
    /* CRC16 */
    for (i = 0UL; i < size; i++)
    {
        *curCrc = Cal_Crc16Tab[(*curCrc >> 8 ^ (uint16)buf[i]) & 0xffu] ^ (*curCrc << 8);
    }
#endif

    return;
}


/******************************************************************************/
/**
 * @brief               <CRC32 finish>
 *
 * <This Function finish the CRC compute.> .
 * Service ID   :       <NONE>
 * Sync/Async   :       <Synchronous>
 * Reentrancy           <Reentrant>
 * @param[in]           <NONE>
 * @param[out]          <NONE>
 * @param[in/out]       <curCrc (IN/OUT)>
 * @return              <NONE>
 */
/******************************************************************************/

void Cal_CrcFinalize(uint32 *curCrc)
{
#if (CAL_CRC32 == CAL_METHOD)
    *curCrc ^= 0xFFFFFFFFU;
#endif

    return;
}

/******************************************************************************/
/**
 * @brief               <generate seed for UDS>
 *
 * <Needed by the UDS module,generate seed> .
 * Service ID   :       <NONE>
 * Sync/Async   :       <Synchronous>
 * Reentrancy           <Reentrant>
 * @param[in]           <NONE>
 * @param[out]          <seed (OUT)>
 * @param[in/out]       <NONE>
 * @return              <SecM_StatusType>
 */
/******************************************************************************/
uint8 SecM_GenerateSeed(uint32 *seed)
{
    uint32 secM_Seed;

    /* random seed */
   // srand(FL_NvmInfo.blockInfo[0].blkProgAttempt);
    secM_Seed = rand();
    secM_Seed = ((secM_Seed << 16) + rand());

    /* seed is non zero */
    if (0UL == secM_Seed)
    {
        secM_Seed++;
    }

    *seed = secM_Seed;

    return SECM_OK;
}

/******************************************************************************/
/**
 * @brief               <compute key>
 *
 * <compute key before compute key> .
 * Service ID   :       <NONE>
 * Sync/Async   :       <Synchronous>
 * Reentrancy           <Reentrant>
 * @param[in]           <seed (IN), k (IN)>
 * @param[out]          <key (OUT)>
 * @param[in/out]       <NONE>
 * @return              <SecM_StatusType>
 */
/******************************************************************************/
uint8 SecM_ComputeKey(uint32 seed,uint32 mask,uint32 *key)
{
    uint32 seed2;
    uint32 key1;
    uint32 key2;

    /* calculate seed2 */
    seed2 = ((seed << (uint8)16) & 0xFFFF0000U) + ((seed >> (uint8)16) & 0xFFFFU);

    /* calculate key1 */
    key1 = seed ^ mask;

    /* calculate key2 */
    key2 = seed2 ^ mask;

    /* calculate key */
    *key = (key1 + key2);

    return SECM_OK;
}

/******************************************************************************/
/**
 * @brief               <compare key>
 *
 * <Needed by the UDS module,compare seed> .
 * Service ID   :       <NONE>
 * Sync/Async   :       <Synchronous>
 * Reentrancy           <Reentrant>
 * @param[in]           <key (IN), seed (IN)>
 * @param[out]          <NONE>
 * @param[in/out]       <NONE>
 * @return              <SecM_StatusType>
 */
/******************************************************************************/
uint8 SecM_CompareKey(uint32 key, uint32 seed)
{
    uint8 ret = SECM_OK;
    uint32 getkey;

    ret = SecM_ComputeKey(seed, SECM_ECU_KEY, &getkey);

    if (getkey != key)
    {
        // ret = SECM_NOT_OK; /*10086 mask SeedKey veritfy function*/
    }
    else
    {
        /*empty */
    }

    return ret;
}

/******************************************************************************/
/**
 * @brief               <compute CRC>
 *
 * <process CRC compute,include init,compute and finish> .
 * Service ID   :       <NONE>
 * Sync/Async   :       <Synchronous>
 * Reentrancy           <Reentrant>
 * @param[in]           <NONE>
 * @param[out]          <NONE>
 * @param[in/out]       <crcParam (IN/OUT)>
 * @return              <SecM_StatusType>
 */
/******************************************************************************/
uint8 SecM_ComputeCRC(SecM_CRCParamType *crcParam)
{
    uint8 ret = SECM_OK;

    switch (crcParam->crcState)
    {
    case SECM_CRC_INIT:
        /* CRC value initialize */
        Cal_CrcInit(&crcParam->currentCRC);
        break;

    case SECM_CRC_COMPUTE:
        /* CRC value compute */
        Cal_CrcCal(&crcParam->currentCRC,
                   crcParam->crcSourceBuffer,
                   (uint32)crcParam->crcByteCount);
        break;

    case SECM_CRC_FINALIZE:
        /* CRC value finish */
        Cal_CrcFinalize(&crcParam->currentCRC);
        break;

    default:
        ret = SECM_NOT_OK;
        break;
    }

    return ret;
}

/******************************************************************************/
/**
 * @brief               <verificate CRC value>
 *
 * <verificate if transfered CRC is equal to computed CRC> .
 * Service ID   :       <NONE>
 * Sync/Async   :       <Synchronous>
 * Reentrancy           <Reentrant>
 * @param[in]           <NONE>
 * @param[out]          <NONE>
 * @param[in/out]       <verifyParam (IN/OUT)>
 * @return              <SecM_StatusType>
 */
/******************************************************************************/
uint8 SecM_Verification(SecM_VerifyParamType *verifyParam)
{
    uint8 ret = SECM_OK;
    SecM_CRCParamType crcParam;
    uint8 segmentIndex = (uint8)0;
    uint32 transferedCrc;

    if (NULL_PTR == verifyParam->segmentList)
    {
        /* if segment list is NULL, verification is failed */
        ret = SECM_NOT_OK;
    }
    else
    {
        /* initialize CRC */
        crcParam.currentCRC = 0UL;
        crcParam.crcState = (uint8)SECM_CRC_INIT;
        ret = SecM_ComputeCRC(&crcParam);
    }

    for (segmentIndex = (uint8)0;
         (segmentIndex < verifyParam->segmentList->nrOfSegments) && (SECM_OK == ret);
         segmentIndex++)
    {
        /* compute each segment CRC */
        ret = SecM_ProcessCrc(&crcParam,
                              verifyParam->segmentList->segmentInfo[segmentIndex].address,
                              verifyParam->segmentList->segmentInfo[segmentIndex].length);
    }

    if (SECM_OK == ret)
    {
        /* finish compute CRC */
        crcParam.crcState = (uint8)SECM_CRC_FINALIZE;
        ret = SecM_ComputeCRC(&crcParam);
    }
    else
    {
        /* empty */
    }

    if (SECM_OK == ret)
    {
        /* get CRC transfered from client */
#if (CAL_CRC32 == CAL_METHOD)
        transferedCrc = (((uint8)verifyParam->verificationData[0]) << (uint8)24);
        transferedCrc += (((uint8)verifyParam->verificationData[1]) << (uint8)16);
        transferedCrc += (((uint8)verifyParam->verificationData[2]) << (uint8)8);
        transferedCrc += ((uint8)verifyParam->verificationData[3]);
#else
        /* CRC16 */
        transferedCrc = ((SecM_CRCType)verifyParam->verificationData[0] << (uint8)8);
        transferedCrc += (SecM_CRCType)verifyParam->verificationData[1];
#endif

        /* compare CRC */
        if (transferedCrc != crcParam.currentCRC)
        {
            // ret = SECM_NOT_OK; /* 10086 do */
        }
        else
        {
            /* empty */
        }

        /* save CRC */
        verifyParam->crcTotle = crcParam.currentCRC;
    }
    else
    {
        /* empty */
    }

    return ret;
}

/******************************************************************************/
/**
 * @brief               <process CRC compute>
 *
 * <CRC compute> .
 * @param[in]           <Address (IN), Length (IN)>
 * @param[out]          <NONE>
 * @param[in/out]       <crcParam (IN/OUT)>
 * @return              <SecM_StatusType>
 */
/******************************************************************************/
static uint8 SecM_ProcessCrc(SecM_CRCParamType *crcParam,
                                       uint32 Address, uint32 Length)
{
    uint8 ret = SECM_OK;
    uint32 readLength = 0UL;

    /* set CRC compute step */
    crcParam->crcState = (uint8)SECM_CRC_COMPUTE;

    while ((Length > 0UL) && (SECM_OK == ret))
    {
        /* read maximum length is SECM_CRC_BUFFER_LEN */
        if (Length > (uint32)SECM_CRC_BUFFER_LEN)
        {
            readLength = SECM_CRC_BUFFER_LEN;
        }
        else
        {
            readLength = Length;
        }

        /* get source data from memory */
        crcParam->crcByteCount = Hal_Flash_Read(Address, SecM_CrcDataBuffer, readLength);

        Length -= readLength;
        Address += readLength;
        crcParam->crcSourceBuffer = SecM_CrcDataBuffer;

        /* update watch dog */
        //Appl_UpdateTriggerCondition();

        /* compute CRC */
        ret = SecM_ComputeCRC(crcParam);
    }

    return ret;
}

/*=======[E N D   O F   F I L E]==============================================*/
