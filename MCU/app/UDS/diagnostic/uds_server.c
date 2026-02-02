#include "uds_server.h"
#include "uds_cfc.h"
#include "fl_cfc.h"
#define DOWLOAD_DATA_ADDR_LEN    (4u)         /*dowload data addr len*/
#define DOWLOAD_DATA_LEN         (4u)         /*dowload data len*/

/*support function/physical ID request*/
#define ERRO_REQUEST_ID          (0u)          /*received ID failled*/
#define SUPPORT_PHYSICAL_ADDR    (1u << 0u)    /*support physical ID request */
#define SUPPORT_FUNCTION_ADDR    (1u << 1u)    /*support function ID request*/
/***********************UDS Information Global function************************/
/*set current request id  SUPPORT_PHYSICAL_ADDR/SUPPORT_FUNCTION_ADDR */
#define UDS_SetRequestIdType(xRequestIDType)   (gs_stUdsInfo.requsetIdMode = (xRequestIDType))
/*uds app time to count*/
#define UdsAppTimeToCount(xTime)    ((xTime) / gs_stUdsAppCfg.CalledPeriod)
/*********************************************************/
/*download data info*/
static tDowloadDataInfo gs_stDowloadDataInfo = {0u, 0u};

/* received block number */
static uint8 gs_RxBlockNum = 0u;
/*Get bootloader version*/


/* UDS time control information config table*/
const static uint16Info gs_stUdsAppCfg =
{
    1u,
    3u,
    10000u,
    5000u
};

static tUdsInfo gs_stUdsInfo =
{
    DEFALUT_SESSION,
    ERRO_REQUEST_ID,
    NONE_SECURITY,
    0u,
    0u,
};

/*get UDS s3 watermark timer. return s3 * S3_TIMER_WATERMARK_PERCENT / 100*/
uint32 UDS_GetUDSS3WatermarkTimerMs(void)
{
    const uint32 watermarkTimerMs = (gs_stUdsAppCfg.xS3Server * S3_TIMER_WATERMARK_PERCENT) / 100u;

    return (uint32)watermarkTimerMs;
}

#ifdef EN_DELAY_TIME
typedef struct
{
    boolean isReceiveUDSMsg;
    uint32 jumpToAPPDelayTime;
} tJumpAppDelayTimeInfo;

static tJumpAppDelayTimeInfo gs_stJumpAPPDelayTimeInfo = {FALSE, 0u};
#endif

/**********************UDS Information Static function************************/
static uint16 UDS_GetUdsS3ServerTime(void);
static void   UDS_SubUdsS3ServerTime(uint16 i_SubTime);
static uint16 UDS_GetUdsSecurityReqLockTime(void);
static void   UDS_SubUdsSecurityReqLockTime(uint16 i_SubTime);

/******************************UDS service main function define***************************************/
static void UDS_DigSession_0x10(struct UDSServiceInfo* i_pstUDSServiceInfo, tUdsAppMsgInfo *m_pstPDUMsg);
static void UDS_SecurityAccess_0x27(struct UDSServiceInfo* i_pstUDSServiceInfo, tUdsAppMsgInfo *m_pstPDUMsg);
static void UDS_ControlDTCSetting_0x85(struct UDSServiceInfo* i_pstUDSServiceInfo, tUdsAppMsgInfo *m_pstPDUMsg);
static void UDS_WriteDataByIdentifier_0x2E(struct UDSServiceInfo* i_pstUDSServiceInfo, tUdsAppMsgInfo *m_pstPDUMsg);
static void UDS_CommunicationControl_0x28(struct UDSServiceInfo* i_pstUDSServiceInfo, tUdsAppMsgInfo *m_pstPDUMsg);
static void UDS_RequestDownload_0x34(struct UDSServiceInfo* i_pstUDSServiceInfo, tUdsAppMsgInfo *m_pstPDUMsg);
static void UDS_TransferData_0x36(struct UDSServiceInfo* i_pstUDSServiceInfo, tUdsAppMsgInfo *m_pstPDUMsg);
static void UDS_RequestTransferExit_0x37(struct UDSServiceInfo* i_pstUDSServiceInfo, tUdsAppMsgInfo *m_pstPDUMsg);
static void UDS_RoutineControl_0x31(struct UDSServiceInfo* i_pstUDSServiceInfo, tUdsAppMsgInfo *m_pstPDUMsg);
static void UDS_TesterPresent_0x3E(struct UDSServiceInfo* i_pstUDSServiceInfo, tUdsAppMsgInfo *m_pstPDUMsg);
static void UDS_ResetECU_0x11(struct UDSServiceInfo* i_pstUDSServiceInfo, tUdsAppMsgInfo *m_pstPDUMsg);
/***********************UDS service Static Global value************************/
/*dig serverice config table*/
const static tUDSService gs_astUDSService[] =
{
    /*diagnose mode control*/
    {
        0x10u,
        DEFALUT_SESSION | PROGRAM_SESSION | EXTEND_SESSION,
        SUPPORT_PHYSICAL_ADDR | SUPPORT_FUNCTION_ADDR,
        NONE_SECURITY,
        UDS_DigSession_0x10
    },
    /*reset ECU*/
    {
        0x11u,
        PROGRAM_SESSION,
        SUPPORT_PHYSICAL_ADDR | SUPPORT_FUNCTION_ADDR,
        SECURITY_LEVEL_1,
        UDS_ResetECU_0x11
    },
    /*security access*/
    {
        0x27u,
        PROGRAM_SESSION,
        SUPPORT_PHYSICAL_ADDR,
        NONE_SECURITY,
        UDS_SecurityAccess_0x27
    },

    /*communication control*/
   {
        0x28u,
        DEFALUT_SESSION | PROGRAM_SESSION | EXTEND_SESSION,
        SUPPORT_PHYSICAL_ADDR | SUPPORT_FUNCTION_ADDR,
        NONE_SECURITY,
        UDS_CommunicationControl_0x28
   },

    /*control DTC setting*/
   {
        0x85u,
        DEFALUT_SESSION | PROGRAM_SESSION | EXTEND_SESSION,
        SUPPORT_PHYSICAL_ADDR | SUPPORT_FUNCTION_ADDR,
        NONE_SECURITY,
        UDS_ControlDTCSetting_0x85
   },

    /*write data by identifier*/
   {
        0x2Eu,
        PROGRAM_SESSION,
        SUPPORT_PHYSICAL_ADDR,
        SECURITY_LEVEL_1,
        UDS_WriteDataByIdentifier_0x2E
   },

    /*routine control*/
   {
        0x31u,
        DEFALUT_SESSION | PROGRAM_SESSION | EXTEND_SESSION,
        SUPPORT_PHYSICAL_ADDR | SUPPORT_FUNCTION_ADDR,
        NONE_SECURITY,
        UDS_RoutineControl_0x31
   },

    /*request download data */
   {
        0x34u,
        PROGRAM_SESSION,
        SUPPORT_PHYSICAL_ADDR,
        SECURITY_LEVEL_1,
        UDS_RequestDownload_0x34
   },

    /*transter data*/
   {
        0x36u,
        PROGRAM_SESSION,
        SUPPORT_PHYSICAL_ADDR,
        SECURITY_LEVEL_1,
        UDS_TransferData_0x36
   },
    /*request exit transfer data*/
   {
        0x37u,
        PROGRAM_SESSION,
        SUPPORT_PHYSICAL_ADDR,
        SECURITY_LEVEL_1,
        UDS_RequestTransferExit_0x37
   },

    /*diagnose mode control*/
   {
        0x3Eu,
        DEFALUT_SESSION | PROGRAM_SESSION | EXTEND_SESSION,
        SUPPORT_PHYSICAL_ADDR | SUPPORT_FUNCTION_ADDR,
        NONE_SECURITY,
        UDS_TesterPresent_0x3E
   },
};


/***********************UDS Information Static Global value************************/
/* UDS support Session mode?��RequestId and Security level config */


static uint16 UDS_GetUdsS3ServerTime(void)
{
    return (gs_stUdsInfo.xUdsS3ServerTime);
}

static void UDS_SubUdsS3ServerTime(uint16 i_SubTime)
{
    gs_stUdsInfo.xUdsS3ServerTime -= i_SubTime;
}

static uint16 UDS_GetUdsSecurityReqLockTime(void)
{
    return (gs_stUdsInfo.xSecurityReqLockTime);
}

static void UDS_SubUdsSecurityReqLockTime(uint16 i_SubTime)
{
    gs_stUdsInfo.xSecurityReqLockTime -= i_SubTime;
}

/*Is security request lock timeout?*/
static uint8 UDS_IsSecurityRequestLockTimeout(void)
{
    uint8 status = 0u;

    if(gs_stUdsInfo.xSecurityReqLockTime)
    {
        status = TRUE;
    }
    else
    {
        status = FALSE;
    }

    return status;
}

/*restart s3server time*/
void UDS_RestartS3Server(void)
{
    gs_stUdsInfo.xUdsS3ServerTime = UdsAppTimeToCount(gs_stUdsAppCfg.xS3Server);
}

/*set currrent session mode. DEFAULT_SESSION/PROGRAM_SESSION/EXTEND_SESSION */
void UDS_SetCurrentSession(const uint8 i_setSessionMode)
{
    gs_stUdsInfo.curSessionMode = i_setSessionMode;
}


/**********************UDS service correlation main function realizing************************/
/*dig session*/
static void UDS_DigSession_0x10(struct UDSServiceInfo* i_pstUDSServiceInfo, tUdsAppMsgInfo *m_pstPDUMsg)
{
    uint8 requestSubfunction = 0u;

    ASSERT(NULL_PTR == m_pstPDUMsg);
    ASSERT(NULL_PTR == i_pstUDSServiceInfo);

    requestSubfunction = m_pstPDUMsg->aDataBuf[1u];

    /*set send postive message*/
    m_pstPDUMsg->aDataBuf[0u] = i_pstUDSServiceInfo->serNum + 0x40u;
    m_pstPDUMsg->aDataBuf[1u] = requestSubfunction;
    m_pstPDUMsg->xDataLen = 2u;

    /*sub function*/
    switch(requestSubfunction)
    {
    case 0x01u :  /*default mode*/
    case 0x81u :
        UDS_SetCurrentSession(DEFALUT_SESSION);

        if(0x81u == requestSubfunction)
        {
            m_pstPDUMsg->xDataLen = 0u;
        }

        break;

    case 0x02u :  /*program mode*/
    case 0x82u :
        //进入0x10 编程会话，会调用重启MCU,并把升级标记 写进flash,然后看门狗重启MCU,MCU重启起来进入boot
        //会发0x10 编程会话的肯定响应。
        //我们这里不这么干，我们有其他事要做，不止升级
        UDS_SetCurrentSession(PROGRAM_SESSION);

        if(0x82u == requestSubfunction)
        {
           m_pstPDUMsg->xDataLen = 0u;
        }

        /*request more time*/
       // UDS_RequestMoreTime(i_pstUDSServiceInfo->serNum, &UDS_DoResetMCU);
        UDS_RestartS3Server();
        break;

    case 0x03u :  /*extend mode*/
    case 0x83u :
        UDS_SetCurrentSession(EXTEND_SESSION);

        if(0x83u == requestSubfunction)
        {
            m_pstPDUMsg->xDataLen = 0u;
        }

        /*restart s3server time*/
        UDS_RestartS3Server();
        break;

    default :
        UDS_SetNegativeErroCode(i_pstUDSServiceInfo->serNum, SFNS, m_pstPDUMsg);
        break;
    }
}

/*reset ECU*/
static void UDS_ResetECU_0x11(struct UDSServiceInfo* i_pstUDSServiceInfo, tUdsAppMsgInfo *m_pstPDUMsg)
{
    ASSERT(NULL_PTR == m_pstPDUMsg);
    ASSERT(NULL_PTR == i_pstUDSServiceInfo);

    /*If program data in flash successfull, set Bootloader will jump to application flag*/
   // Flash_EraseFlashDriverInRAM();

    /*If invalid application software in flash, then this step set application jump to bootloader flag*/
   // Boot_SetDownloadAppSuccessful();

    m_pstPDUMsg->pfUDSTxMsgServiceCallBack = &UDS_DoResetMCU;

    /*request client timeout time*/
    UDS_SetNegativeErroCode(i_pstUDSServiceInfo->serNum, RCRRP, m_pstPDUMsg);
}

/*security access*/
static void UDS_SecurityAccess_0x27(struct UDSServiceInfo* i_pstUDSServiceInfo, tUdsAppMsgInfo *m_pstPDUMsg)
{
    uint8 requestSubfunction = 0u;
    static uint8 s_aSeedBuf[AES_SEED_LEN] = {0u};
   // boolean ret = FALSE;

    ASSERT(NULL_PTR == m_pstPDUMsg);
    ASSERT(NULL_PTR == i_pstUDSServiceInfo);

    /*get subfunction*/
    requestSubfunction = m_pstPDUMsg->aDataBuf[1u];

    switch(requestSubfunction)
    {
        case 0x01u :
             //调试用
             m_pstPDUMsg->aDataBuf[0u] = i_pstUDSServiceInfo->serNum + 0x40u;
             m_pstPDUMsg->aDataBuf[1u]=requestSubfunction;
             m_pstPDUMsg->xDataLen = 4u;
             m_pstPDUMsg->aDataBuf[2u]=0x11;
             m_pstPDUMsg->aDataBuf[3u]=0x12;
             memcpy(&m_pstPDUMsg->aDataBuf[2u], s_aSeedBuf, 2);
            /*get random and put in m_pstPDUMsg->aDataBuf[2u] ~ 17u byte*/
           // ret = UDS_ALG_HAL_GetRandom(AES_SEED_LEN, s_aSeedBuf);

           // if(TRUE == ret)
           // {
           //     memcopy(s_aSeedBuf,&m_pstPDUMsg->aDataBuf[2u], AES_SEED_LEN);
           //     m_pstPDUMsg->xDataLen = 2u + AES_SEED_LEN;
          //  }
          //  else
          //  {
          //      UDS_SetNegativeErroCode(i_pstUDSServiceInfo->serNum, IK, m_pstPDUMsg);
          //  }

            break;

        case 0x02u :

            /*count random to key and check received key right?*/
           // if(TRUE == UDS_IsReceivedKeyRight(&m_pstPDUMsg->aDataBuf[2u], s_aSeedBuf, AES_SEED_LEN))
           // {
           //     m_pstPDUMsg->aDataBuf[0u] = i_pstUDSServiceInfo->serNum + 0x40u;

            //    m_pstPDUMsg->xDataLen = 2u;

            //    memset(0x1u,s_aSeedBuf, sizeof(s_aSeedBuf));

            //    UDS_SetSecurityLevel(SECURITY_LEVEL_1);
           // }
            //调试用
            if(memcmp(s_aSeedBuf,&m_pstPDUMsg->aDataBuf[2u],2)==0)
            {
                m_pstPDUMsg->aDataBuf[0u] = i_pstUDSServiceInfo->serNum + 0x40u;
                m_pstPDUMsg->xDataLen = 2u;
                UDS_SetSecurityLevel(SECURITY_LEVEL_1);
            }
            else
            {
                UDS_SetNegativeErroCode(i_pstUDSServiceInfo->serNum, IK, m_pstPDUMsg);
             }

            break;

        default :

            break;
    }
}
/*control DTC setting*/
static void UDS_ControlDTCSetting_0x85(struct UDSServiceInfo* i_pstUDSServiceInfo, tUdsAppMsgInfo *m_pstPDUMsg)
{
    uint8 requestSubfunction = 0u;

    ASSERT(NULL_PTR == m_pstPDUMsg);
    ASSERT(NULL_PTR == i_pstUDSServiceInfo);

    requestSubfunction = m_pstPDUMsg->aDataBuf[1u];

    switch(requestSubfunction)
    {
        case 0x01u :
        case 0x02u :
            m_pstPDUMsg->aDataBuf[0u] = i_pstUDSServiceInfo->serNum + 0x40u;
            m_pstPDUMsg->aDataBuf[1u] = requestSubfunction;
            m_pstPDUMsg->xDataLen = 2u;
            break;

        case 0x81u :
        case 0x82u :
            m_pstPDUMsg->xDataLen = 0u;
            break;

        default :
            UDS_SetNegativeErroCode(i_pstUDSServiceInfo->serNum, SFNS, m_pstPDUMsg);
            break;
    }
}

/*write data by identifier*/
static void UDS_WriteDataByIdentifier_0x2E(struct UDSServiceInfo* i_pstUDSServiceInfo, tUdsAppMsgInfo *m_pstPDUMsg)
{
    ASSERT(NULL_PTR == m_pstPDUMsg);
    ASSERT(NULL_PTR == i_pstUDSServiceInfo);

    /*Is write fingerprint id right?*/
    if(TRUE )//== UDS_IsWriteFingerprintRight(m_pstPDUMsg))
    {
        /*do write fingerprint*/
        //Flash_SavePrintfigner(&m_pstPDUMsg->aDataBuf[3u], (m_pstPDUMsg->xDataLen - 3u));

        m_pstPDUMsg->aDataBuf[0u] = i_pstUDSServiceInfo->serNum + 0x40u;
        m_pstPDUMsg->aDataBuf[1u] = 0xF1u;
        m_pstPDUMsg->aDataBuf[2u] = 0x5Au;
        m_pstPDUMsg->xDataLen = 3u;
    }
    else
    {
        /*don't have this routine control ID*/
        UDS_SetNegativeErroCode(i_pstUDSServiceInfo->serNum, SFNS, m_pstPDUMsg);
    }
}

/*communication control*/
static void UDS_CommunicationControl_0x28(struct UDSServiceInfo* i_pstUDSServiceInfo, tUdsAppMsgInfo *m_pstPDUMsg)
{
    uint8 requestSubfunction = 0u;

    ASSERT(NULL_PTR == m_pstPDUMsg);
    ASSERT(NULL_PTR == i_pstUDSServiceInfo);

    requestSubfunction = m_pstPDUMsg->aDataBuf[1u];

    switch(requestSubfunction)
    {
    case 0x0u  :
    case 0x03u :
        m_pstPDUMsg->aDataBuf[0u] = i_pstUDSServiceInfo->serNum + 0x40u;
        m_pstPDUMsg->aDataBuf[1u] = requestSubfunction;
        m_pstPDUMsg->xDataLen = 2u;

        break;

    case 0x80u :
    case 0x83u :
        /*don't transmit uds message.*/
        m_pstPDUMsg->aDataBuf[0u] = 0u;
        m_pstPDUMsg->xDataLen = 0u;

        break;

    default :
        UDS_SetNegativeErroCode(i_pstUDSServiceInfo->serNum, SFNS, m_pstPDUMsg);

        break;
    }
}

/*routine control*/
static void UDS_RoutineControl_0x31(struct UDSServiceInfo* i_pstUDSServiceInfo, tUdsAppMsgInfo *m_pstPDUMsg)
{
       uint8 ret = FALSE;
       uint32 ReceivedCrc = 0u;
       uint8 aSWVersion[] = BOOTLOADER_SW_VERSION;
       uint8 aHWVersion[] = BOOTLOADER_HW_VERSION;
       uint32 offset = 0u;

       ASSERT(NULL_PTR == m_pstPDUMsg);
       ASSERT(NULL_PTR == i_pstUDSServiceInfo);

       UDS_RestartS3Server();

       /*Is erase memory routine control?*/
       if(TRUE == UDS_IsEraseMemoryRoutineControl(m_pstPDUMsg))
       {
           /*request client timeout time*/
           UDS_SetNegativeErroCode(i_pstUDSServiceInfo->serNum, RCRRP, m_pstPDUMsg);

           m_pstPDUMsg->pfUDSTxMsgServiceCallBack = &UDS_DoEraseFlash;
       }
       /*Is check sum routine control?*/
       else if(TRUE == UDS_IsCheckSumRoutineControl(m_pstPDUMsg))
       {
           ReceivedCrc = m_pstPDUMsg->aDataBuf[4u];
           ReceivedCrc = (ReceivedCrc << 8u) | m_pstPDUMsg->aDataBuf[5u];
           ReceivedCrc = (ReceivedCrc << 8u) | m_pstPDUMsg->aDataBuf[6u];
           ReceivedCrc = (ReceivedCrc << 8u) | m_pstPDUMsg->aDataBuf[7u];
           Flash_SavedReceivedCheckSumCrc(ReceivedCrc);

           /*request client timeout time*/
           UDS_SetNegativeErroCode(i_pstUDSServiceInfo->serNum, RCRRP, m_pstPDUMsg);

           m_pstPDUMsg->pfUDSTxMsgServiceCallBack = &UDS_DoCheckSum;
       }

       /*Is check programming dependency?*/
       else if(TRUE == UDS_IsCheckProgrammingDependency(m_pstPDUMsg))
       {
           /*Fill response*/
           m_pstPDUMsg->aDataBuf[0u] = i_pstUDSServiceInfo->serNum + 0x40u;
           m_pstPDUMsg->xDataLen = 5u;

           /*write application information in flash.*/
           ret = Flash_WriteFlashAppInfo();
           if(TRUE == ret)
           {
               /*do check programming dependency*/
               ret = UDS_DoCheckProgrammingDependency();
           }

           if(TRUE == ret)
           {
               m_pstPDUMsg->aDataBuf[4u] = 0u;
           }
           else
           {
               m_pstPDUMsg->aDataBuf[4u] = 1u;

               print("%s: Write APP info or check dependency failed!\n", __func__);
           }
       }

       /*Is get version*/
       else if(TRUE == UDS_IsGetVersion(m_pstPDUMsg))
       {
           m_pstPDUMsg->aDataBuf[0u] = i_pstUDSServiceInfo->serNum + 0x40u;
           m_pstPDUMsg->xDataLen = 4u;

           /*fill sofware information*/
           offset = m_pstPDUMsg->xDataLen;
           m_pstPDUMsg->aDataBuf[offset] = sizeof(aSWVersion);
           offset += 1u;

           memcpy(&m_pstPDUMsg->aDataBuf[offset], aSWVersion, sizeof(aSWVersion));
           offset += sizeof(aSWVersion);

           /*fill hardware version*/
           m_pstPDUMsg->aDataBuf[offset] = sizeof(aHWVersion);
           offset += 1u;

           memcpy(&m_pstPDUMsg->aDataBuf[offset], aHWVersion, sizeof(aHWVersion));
           offset += sizeof(aHWVersion);

           m_pstPDUMsg->xDataLen = offset;
       }
       else
       {
           /*don't have this routine control ID*/
           UDS_SetNegativeErroCode(i_pstUDSServiceInfo->serNum, SFNS, m_pstPDUMsg);
       }
}

/*Tester present service*/
static void UDS_TesterPresent_0x3E(struct UDSServiceInfo* i_pstUDSServiceInfo, tUdsAppMsgInfo *m_pstPDUMsg)
{
    uint8 requestSubfunction = 0u;

    ASSERT(NULL_PTR == m_pstPDUMsg);
    ASSERT(NULL_PTR == i_pstUDSServiceInfo);

    requestSubfunction = m_pstPDUMsg->aDataBuf[1u];

    /*sub function*/
    switch(requestSubfunction)
    {
        case 0x00u :  /*zero subFunction*/
            /*set send postive message*/
            m_pstPDUMsg->aDataBuf[0u] = i_pstUDSServiceInfo->serNum + 0x40u;
            m_pstPDUMsg->aDataBuf[1u] = requestSubfunction;
            m_pstPDUMsg->xDataLen = 2u;
            break;

        case 0x80u :  /*program mode*/
            m_pstPDUMsg->xDataLen = 0u;
            break;

        default :
            UDS_SetNegativeErroCode(i_pstUDSServiceInfo->serNum, SFNS, m_pstPDUMsg);
            break;
    }
}

/*request download*/
static void UDS_RequestDownload_0x34(struct UDSServiceInfo* i_pstUDSServiceInfo, tUdsAppMsgInfo *m_pstPDUMsg)
{
   uint8 Index = 0u;
    uint8 Ret = TRUE;

    ASSERT(NULL_PTR == m_pstPDUMsg);
    ASSERT(NULL_PTR == i_pstUDSServiceInfo);

    if(m_pstPDUMsg->xDataLen < (DOWLOAD_DATA_ADDR_LEN + DOWLOAD_DATA_LEN + 1u + 2u))
    {
        Ret = FALSE;

        UDS_SetNegativeErroCode(i_pstUDSServiceInfo->serNum, IMLOIF, m_pstPDUMsg);
    }

    if(TRUE == Ret)
    {
        /*get data addr */
        gs_stDowloadDataInfo.startAddr = 0u;
        for(Index = 0u; Index < DOWLOAD_DATA_ADDR_LEN; Index++)
        {
            gs_stDowloadDataInfo.startAddr <<= 8u;
            /* 3u = N_PCI(1) + SID34(1) + dataFormatldentifier(1) */
            gs_stDowloadDataInfo.startAddr |= m_pstPDUMsg->aDataBuf[Index + 3u];
        }

        /*get data len*/
        gs_stDowloadDataInfo.dataLen = 0u;
        for(Index = 0u; Index < DOWLOAD_DATA_LEN; Index++)
        {
            gs_stDowloadDataInfo.dataLen <<= 8u;
            gs_stDowloadDataInfo.dataLen |= m_pstPDUMsg->aDataBuf[Index + 7u];
        }
    }

    /*Is download data  addr  and len valid?*/
    if(((TRUE != UDS_IsDownloadDataAddrValid(gs_stDowloadDataInfo.startAddr, gs_stDowloadDataInfo.dataLen))) ||
		(TRUE != Ret))
    {
        UDS_SetNegativeErroCode(i_pstUDSServiceInfo->serNum, ROOR, m_pstPDUMsg);

        Ret = FALSE;
    }

    if(TRUE == Ret)
    {
        /*set wait transfer data step(0x34 service)*/
        Flash_SetNextDownloadStep(FL_TRANSFER_STEP);

        /*save received program addr and data len*/
        Flash_SaveDownloadDataInfo(gs_stDowloadDataInfo.startAddr, gs_stDowloadDataInfo.dataLen);

        /*fill postive message*/
        m_pstPDUMsg->aDataBuf[0u] = i_pstUDSServiceInfo->serNum + 0x40u;
        m_pstPDUMsg->aDataBuf[1u] = 0x10u;
        m_pstPDUMsg->aDataBuf[2u] = 0x80u;
        m_pstPDUMsg->xDataLen = 3u;

        /*set wait received block number*/
        gs_RxBlockNum = 1u;
    }
    else
    {
        Flash_InitDowloadInfo();

        /*set request transfer data step(0x34 service)*/
        Flash_SetNextDownloadStep(FL_REQUEST_STEP);
    }
}

/*transfer data*/
static void UDS_TransferData_0x36(struct UDSServiceInfo* i_pstUDSServiceInfo, tUdsAppMsgInfo *m_pstPDUMsg)
{
    uint8 Ret = TRUE;

    ASSERT(NULL_PTR == m_pstPDUMsg);
    ASSERT(NULL_PTR == i_pstUDSServiceInfo);

    /*request sequence erro*/
    if((FL_TRANSFER_STEP != Flash_GetCurDownloadStep()) && (TRUE == Ret))
    {
        Ret = FALSE;

        UDS_SetNegativeErroCode(i_pstUDSServiceInfo->serNum, RSE, m_pstPDUMsg);
    }

    if((gs_RxBlockNum != m_pstPDUMsg->aDataBuf[1u]) && (TRUE == Ret))
    {
        Ret = FALSE;

        /*received data is not wait block number*/
        UDS_SetNegativeErroCode(i_pstUDSServiceInfo->serNum, RSE, m_pstPDUMsg);
    }

    gs_RxBlockNum++;

    /*copy flash data in flash area*/
    if((TRUE != Flash_ProgramRegion(gs_stDowloadDataInfo.startAddr,
                                    &m_pstPDUMsg->aDataBuf[2u], 
                                    (m_pstPDUMsg->xDataLen - 2u))) && (TRUE == Ret))
    {
        Ret = FALSE;

        /*saved data and information failled!*/
        UDS_SetNegativeErroCode(i_pstUDSServiceInfo->serNum, CNC, m_pstPDUMsg);
    }
    else
    {
        gs_stDowloadDataInfo.startAddr += (m_pstPDUMsg->xDataLen - 2u);
        gs_stDowloadDataInfo.dataLen -= (m_pstPDUMsg->xDataLen - 2u);
    }

    /*received all data*/
    if((0u == gs_stDowloadDataInfo.dataLen) && (TRUE == Ret))
    {
        gs_RxBlockNum = 0u;

        /*set wait exit transfer step(0x37 service)*/
        Flash_SetNextDownloadStep(FL_EXIT_TRANSFER_STEP);
    }

    if(TRUE == Ret)
    {
        /*tranmitted postive message.*/
        m_pstPDUMsg->aDataBuf[0u] = i_pstUDSServiceInfo->serNum + 0x40u;
        m_pstPDUMsg->xDataLen = 4u;
    }
    else
    {
        Flash_InitDowloadInfo();

        /*set request transfer data step(0x34 service)*/
        Flash_SetNextDownloadStep(FL_REQUEST_STEP);

        gs_RxBlockNum = 0u;
    }
}
/*request transfer exit*/
static void UDS_RequestTransferExit_0x37(struct UDSServiceInfo* i_pstUDSServiceInfo, tUdsAppMsgInfo *m_pstPDUMsg)
{
    uint8 Ret = TRUE;

    ASSERT(NULL_PTR == m_pstPDUMsg);
    ASSERT(NULL_PTR == i_pstUDSServiceInfo);

    if(FL_EXIT_TRANSFER_STEP != Flash_GetCurDownloadStep())
    {
        Ret = FALSE;

        UDS_SetNegativeErroCode(i_pstUDSServiceInfo->serNum, RSE, m_pstPDUMsg);
    }

    if(TRUE == Ret)
    {
        Flash_SetNextDownloadStep(FL_CHECKSUM_STEP);

        /*tranmitted postive message.*/
        m_pstPDUMsg->aDataBuf[0u] = i_pstUDSServiceInfo->serNum + 0x40u;
        m_pstPDUMsg->xDataLen = 1u;
    }
    else
    {
        Flash_InitDowloadInfo();
    }
}


/**********************UDS service correlation other function realizing************************/
/* get uds service config information */
tUDSService* UDS_GetUDSServiceInfo(uint8 *m_pSupServItem)
{
    ASSERT(NULL_PTR == m_pSupServItem);

    *m_pSupServItem = sizeof(gs_astUDSService) / sizeof(gs_astUDSService[0u]);

    return (tUDSService*) &gs_astUDSService[0u];
}

/* If Rx UDS msg, set UDS layer received message TURE */
void UDS_SetIsRxUdsMsg(const uint8 i_setValue)
{

}

uint8 UDS_IsRxUdsMsg(void)
{
    return TRUE;
}

/*set negative erro code*/
void UDS_SetNegativeErroCode(const uint8 i_UDSServiceNum,
                         const uint8 i_erroCode,
                         tUdsAppMsgInfo *m_pstPDUMsg)
{
    ASSERT(NULL_PTR == m_pstPDUMsg);

    m_pstPDUMsg->aDataBuf[0u] = NEGTIVE_ID;
    m_pstPDUMsg->aDataBuf[1u] = i_UDSServiceNum;
    m_pstPDUMsg->aDataBuf[2u] = i_erroCode;
    m_pstPDUMsg->xDataLen = 3u;
}

/*Is current session DEFAULT return TRUE, else return FALSE.*/
uint8 UDS_IsCurDefaultSession(void)
{
    uint8 isCurDefaultSessionStatus = FALSE;

    if(DEFALUT_SESSION == gs_stUdsInfo.curSessionMode)
    {
        isCurDefaultSessionStatus = TRUE;
    }
    else
    {
        isCurDefaultSessionStatus = FALSE;
    }

    return isCurDefaultSessionStatus;
}

/*Is S3server timeout?*/
uint8 UDS_IsS3ServerTimeout(void)
{
    uint8 TimeoutStatus = FALSE;

    if(0u == gs_stUdsInfo.xUdsS3ServerTime)
    {
        TimeoutStatus = TRUE;
    }
    else
    {
        TimeoutStatus = FALSE;
    }

    return TimeoutStatus;
}

/*Is current session can request?*/
uint8 UDS_IsCurSessionCanRequest(uint8 i_sessionMode)
{
    uint8 status = FALSE;

    if((i_sessionMode & gs_stUdsInfo.curSessionMode) == gs_stUdsInfo.curSessionMode)
    {
        status = TRUE;
    }
    else
    {
        status = FALSE;
    }

    return status;
}

/*save received request id. If receved physical/function/none phy and function ID set rceived physicali/function/erro ID.*/
void UDS_SaveRequestIdType(const uint32 i_serRequestID)
{
    if(i_serRequestID == TP_GetConfigRxMsgPHYID())
    {
        UDS_SetRequestIdType(SUPPORT_PHYSICAL_ADDR);
    }
    else if(i_serRequestID == TP_GetConfigRxMsgFUNID())
    {
        UDS_SetRequestIdType(SUPPORT_FUNCTION_ADDR);
    }
    else
    {
        UDS_SetRequestIdType(ERRO_REQUEST_ID);
    }
}

/*Is current received id can request?*/
uint8 UDS_IsCurRxIdCanRequest(uint8 i_serRequestIdMode)
{
    uint8 status = 0u;
    if((i_serRequestIdMode & gs_stUdsInfo.requsetIdMode) == gs_stUdsInfo.requsetIdMode)
    {
        status = TRUE;
    }
    else
    {
        status = FALSE;
    }

    return status;
}

/*set security level*/
void UDS_SetSecurityLevel(const uint8 i_setSecurityLevel)
{
    gs_stUdsInfo.securityLevel = i_setSecurityLevel;
}

/*Is current security level can request?*/
uint8 UDS_IsCurSecurityLevelRequest(uint8 i_securityLevel)
{
    uint8 status = 0u;

   //
   // if((i_securityLevel & gs_stUdsInfo.securityLevel) == gs_stUdsInfo.securityLevel)
    if((i_securityLevel & gs_stUdsInfo.securityLevel) == i_securityLevel)
    {
        status = TRUE;
    }
    else
    {
        status = FALSE;
    }

    return status;
}

/*uds time control*/
void UDS_SystemTickCtl(void)
{
    if(UDS_GetUdsS3ServerTime())
    {
        UDS_SubUdsS3ServerTime(1u);
    }

    if(UDS_GetUdsSecurityReqLockTime())
    {
        UDS_SubUdsSecurityReqLockTime(1u);
    }
}

/***************************End file********************************/


