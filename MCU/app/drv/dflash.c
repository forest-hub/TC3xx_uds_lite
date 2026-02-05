#include <drv/dflash.h>
#include <stdio.h>
#include <string.h>
#include <IfxCpu.h>
#include "Ifx_Assert.h"
#include "IfxCpu.h"

#include "IfxScuWdt.h"
#include "Ifx_Types.h"




#define PFLASH_BLANK_SIZE        0x4000
#define DFLASH_BLANK_SIZE        0x1000
#define UCBFLASH_BLANK_SIZE      0x200

#define FLASHOPER                   0x00000001      /* Flash Operation Error    */
#define FLASHSQER                   0x00000002      /* Command Sequence Error   */
#define FLASH_MASK_ERROR (FLASHOPER | FLASHSQER)

#define FLASH_PROTECTION_ERROR_MASK 0x00000004      /* Protection Error         */
#define FLASH_PROGRAM_VERIFY_ERROR  0x00000008      /* Program Verify Error     */
#define FLASH_ERASE_VERIFY_ERROR    0x00000010      /* Erase Verify Error       */
#define FLASH_ADER_ERROR            0x00000020      /* SRI Bus Address ECC Error*/
#define FLASH_ORIER_ERROR           0x00000040      /* Original Error           */



#define FLASH_RELOC_START_ADDR   (0xC0000000U) /**< \brief Program & Erase routines relocation address */

/** \brief Length of Program & Erase routines
 */
#define FLASH_ERASESECTOR_LEN    (100)
#define FLASH_WAITUNBUSY_LEN     (100)
#define FLASH_ENTERPAGEMODE_LEN  (100)
#define FLASH_LOADPAGE2X32_LEN   (100)
#define FLASH_WRITEPAGE_LEN      (100)
#define FLASH_PFLASHERASE_LEN    (0x200)
#define FLASH_PFLASHPROGRAM_LEN  (0x200)

/** \brief Start addresses of Program & Erase routines
 */
#define FLASH_ERASESECTOR_ADDR   (FLASH_RELOC_START_ADDR)
#define FLASH_WAITUNBUSY_ADDR    (FLASH_ERASESECTOR_ADDR + FLASH_ERASESECTOR_LEN)
#define FLASH_ENTERPAGEMODE_ADDR (FLASH_WAITUNBUSY_ADDR + FLASH_WAITUNBUSY_LEN)
#define FLASH_LOADPAGE2X32_ADDR  (FLASH_ENTERPAGEMODE_ADDR + FLASH_ENTERPAGEMODE_LEN)
#define FLASH_WRITEPAGE_ADDR     (FLASH_LOADPAGE2X32_ADDR + FLASH_LOADPAGE2X32_LEN)
#define FLASH_PFLASHERASE_ADDR   (FLASH_WRITEPAGE_ADDR + FLASH_WRITEPAGE_LEN)
#define FLASH_PFLASHPROGRAM_ADDR (FLASH_PFLASHERASE_ADDR + FLASH_PFLASHERASE_LEN)

static PflashFun_t sg_tPflashFun;

static int __PFlashErase(uint32 flash, uint32 sector_addr, IfxFlash_FlashType flashType);
static int __ProgramPFlashPerPage(uint32 pageAddr, IfxFlash_FlashType flashType, const void *pBuf, uint32 length);

/* 得到Flash的类型 */
IfxFlash_FlashType __GetFlashType(uint32 programAddr)
{
    if (programAddr >= 0xA0000000 && programAddr <= 0xA02FFFFF)
    {
        return IfxFlash_FlashType_P0;
    }
    else if (programAddr >= 0xA0300000 && programAddr <= 0xA05FFFFF)
    {
        return IfxFlash_FlashType_P1;
    }
    else if (programAddr >= 0xAF000000 && programAddr <= 0xAF03FFFF)
    {
        return IfxFlash_FlashType_D0;
    }
    else if (programAddr >= 0xAF400000 && programAddr <= 0xAF405FFF)
    {
        return IfxFlash_FlashType_D1;
    }

    return IfxFlash_FlashType_Fa;
}

/* 判断DFlash是否可写 */
static int __IsDFlashWritable(uint32 startAddr, uint32 length)
{
    uint32 pageAddr = startAddr / IFXFLASH_DFLASH_PAGE_LENGTH * IFXFLASH_DFLASH_PAGE_LENGTH;
    uint32 pageNum = (length + (startAddr - pageAddr)) / IFXFLASH_DFLASH_PAGE_LENGTH;

    if ((length + (startAddr - pageAddr)) % IFXFLASH_DFLASH_PAGE_LENGTH != 0)
    {
        pageNum += 1;
    }

    sint64 *p = (sint64 *)pageAddr;

    for (int i = 0; i < pageNum; i++)
    {
        if (p[i] != 0)
        {
            return 0;
        }
    }

    return 1;
}

/* 封装PFlash的擦除函数实现，可复制到PSPR存储器中调用 */
static int __PFlashErase(uint32 flash, uint32 sector_addr, IfxFlash_FlashType flashType)
{
    uint16 endinitSfty_pw;
    endinitSfty_pw = IfxScuWdt_getSafetyWatchdogPasswordInline();

    /* Erase the sector */
    IfxScuWdt_clearSafetyEndinitInline(endinitSfty_pw);
    sg_tPflashFun.pfnEraseSector(sector_addr);
    IfxScuWdt_setSafetyEndinitInline(endinitSfty_pw);

    /* wait until unbusy */
    sg_tPflashFun.pfnWaitUnbusy(flash, flashType);

    return 0;
}

/* 擦除PFlash */
static int __ErasePFlash(uint32 sectorAddr)
{
    uint32  flash       = 0;
    IfxFlash_FlashType flashType = __GetFlashType(sectorAddr);

    /* disable interrupts */
    boolean interruptState = IfxCpu_disableInterrupts();

    /* erase flash (execute from relocated memory)*/
    sg_tPflashFun.pfnFlashErase(flash, sectorAddr, flashType);

    /* enable interrupts again */
    IfxCpu_restoreInterrupts(interruptState);

    return 0;
}

/* 擦除DFlash扇区 */
static int __EraseDFlash(uint32 sectorAddr)
{
    uint32 flash       = 0;
    uint16 endinitSfty_pw = IfxScuWdt_getSafetyWatchdogPasswordInline();//IfxScuWdt_getSafetyWatchdogPassword();
    IfxFlash_FlashType flashType = __GetFlashType(sectorAddr);


    // clear status register
    IfxFlash_clearStatus(0);

    /* erase program flash */
    IfxScuWdt_clearSafetyEndinit(endinitSfty_pw);
    IfxFlash_eraseSector(sectorAddr);
    IfxScuWdt_setSafetyEndinit(endinitSfty_pw);

    /* wait until unbusy */
    IfxFlash_waitUnbusy(flash, flashType);

    int uvResult = DMU_HF_ERRSR.U;

    if (uvResult & FLASH_MASK_ERROR)
    {
        return -1;
    }
    else if (uvResult & FLASH_PROTECTION_ERROR_MASK)
    {
        return -1;
    }
    else
    {
        if (uvResult & FLASH_ERASE_VERIFY_ERROR)
        {
            IfxFlash_clearStatus(0);
            return -1;
        }
    }

    return 0;
}
/* 封装PFlash的每页编程函数实现，可复制到PSPR存储器中调用 */
static int __ProgramPFlashPerPage(uint32 pageAddr, IfxFlash_FlashType flashType, const void *pBuf, uint32 length)
{
    union
    {
        uint8 u8Data[32];
        uint32 u32Data[8];
    } flashBuf;
    uint32 offset;
    uint32 flash = 0;
    uint16 endinitSfty_pw = IfxScuWdt_getSafetyWatchdogPasswordInline();

    boolean interruptState = IfxCpu_disableInterrupts();

    for (offset = 0; offset < 8; offset++)
    {
        flashBuf.u32Data[offset] = 0;
    }

    for (offset = 0; offset < length; offset++)
    {
        flashBuf.u8Data[offset] = *((char *)pBuf + offset);
    }

    /* program the given no of pages */
    sg_tPflashFun.pfnEnterPageMode(pageAddr);

    /* wait until unbusy */
    sg_tPflashFun.pfnWaitUnbusy(flash, flashType);

    /* write 32 bytes (8 doublewords) into assembly buffer */
    for (offset = 0; offset < IFXFLASH_PFLASH_PAGE_LENGTH / 8; offset++)
    {
        sg_tPflashFun.pfnLoadPage2X32(pageAddr, flashBuf.u32Data[offset * 2], flashBuf.u32Data[offset * 2 + 1]);
    }

    /* write page */
    IfxScuWdt_clearSafetyEndinitInline(endinitSfty_pw);
    sg_tPflashFun.pfnWritePage(pageAddr);
    IfxScuWdt_setSafetyEndinitInline(endinitSfty_pw);

    /* wait until unbusy */
    sg_tPflashFun.pfnWaitUnbusy(flash, flashType);

    IfxCpu_restoreInterrupts(interruptState);

    return 0;
}


/* 对PFlash编程 */
static int __ProgramPFlash(uint32 programAddr, const void *pBuf, uint32 length)
{
    if (length > 512)
    {
        return -1;
    }

    uint32 pageNum;
    uint32 writeLength = 0;
    uint8 flashBuf[IFXFLASH_PFLASH_PAGE_LENGTH];
    IfxFlash_FlashType flashType = __GetFlashType(programAddr);

    if (writeLength < length)
    {
        pageNum = (length - writeLength) / IFXFLASH_PFLASH_PAGE_LENGTH;


        /* program the given no of pages */
        for (uint32 page = 0; page < pageNum; ++page)
        {
            uint32 pageAddr    = programAddr + writeLength;

            if (sg_tPflashFun.pfnFlashProgram(pageAddr, flashType, (char *)pBuf + writeLength, IFXFLASH_PFLASH_PAGE_LENGTH) != 0)
            {
                return -1;
            }

            writeLength += IFXFLASH_PFLASH_PAGE_LENGTH;
        }
    }

    // 最后不足 IFXFLASH_DFLASH_PAGE_LENGTH 的长度
    if (writeLength < length)
    {
        uint32 lastLength = length - writeLength;


        uint32 pageAddr   = programAddr + writeLength;

        memset(flashBuf, 0, 8);
        memcpy(flashBuf, (char *)pBuf + writeLength, lastLength);

        if (sg_tPflashFun.pfnFlashProgram(pageAddr, flashType, flashBuf, IFXFLASH_PFLASH_PAGE_LENGTH) != 0)
        {
            return -1;
        }
    }

    return 0;
}

/* 对DFlash每页编程 */
static int __ProgramDFlashPerPage(uint32 pageAddr, uint16 endinitSfty_pw, IfxFlash_FlashType flashType, uint32 data1, uint32 data2)
{
    uint32 flash = 0;

    if (IfxFlash_enterPageMode(pageAddr) != 0)
    {
        return -1;
    }

    /* wait until unbusy */
    IfxFlash_waitUnbusy(flash, flashType);

    IfxFlash_loadPage2X32(pageAddr, data1, data2);
    /* write page */
    IfxScuWdt_clearSafetyEndinit(endinitSfty_pw);
    IfxFlash_writePage(pageAddr);
    IfxScuWdt_setSafetyEndinit(endinitSfty_pw);

    /* wait until unbusy */
    IfxFlash_waitUnbusy(flash, flashType);

    int uvResult = DMU_HF_ERRSR.U;

    if (uvResult & FLASH_MASK_ERROR)
    {
        return -1;
    }
    else if (uvResult & FLASH_PROTECTION_ERROR_MASK)
    {
        return -1;
    }
    else if (uvResult & FLASH_PROGRAM_VERIFY_ERROR)
    {
        IfxFlash_clearStatus(0);
        return -1;
    }

    return 0;
}

/* 对DFlash编程 */
static int __ProgramDFlash(uint32 programAddr, const void *pBuf, uint32 length)
{
    union
    {
        uint8 u8Data[8];
        uint32 u32Data[2];
    } flashBuf;
    uint32 writeLength = 0;
    uint16 endinitSfty_pw = IfxScuWdt_getSafetyWatchdogPassword();
    IfxFlash_FlashType flashType = __GetFlashType(programAddr);

    if (programAddr % IFXFLASH_DFLASH_PAGE_LENGTH != 0)
    {
        uint32 pageAddr = (programAddr / IFXFLASH_DFLASH_PAGE_LENGTH) * IFXFLASH_DFLASH_PAGE_LENGTH;
        uint32 headDiffLen = programAddr - pageAddr;

        memset(flashBuf.u8Data, 0, 8);

        // 超出该页
        if ((programAddr + length) > (pageAddr + IFXFLASH_DFLASH_PAGE_LENGTH))
        {
            memcpy(&flashBuf.u8Data[headDiffLen], pBuf, IFXFLASH_DFLASH_PAGE_LENGTH - headDiffLen);
            writeLength += (IFXFLASH_DFLASH_PAGE_LENGTH - headDiffLen);
        }
        else
        {
            memcpy(&flashBuf.u8Data[headDiffLen], pBuf, length);
            writeLength = length;
        }

        if (__ProgramDFlashPerPage(pageAddr, endinitSfty_pw, flashType, flashBuf.u32Data[0], flashBuf.u32Data[1]) != 0)
        {
            return -1;
        }
    }

    if (writeLength < length)
    {
        uint32 pageNum = (length - writeLength) / IFXFLASH_DFLASH_PAGE_LENGTH;


        /* program the given no of pages */
        for (uint32 page = 0; page < pageNum; ++page)
        {
            uint32 pageAddr    = programAddr + writeLength;

            memcpy(flashBuf.u8Data, (char *)pBuf + writeLength, IFXFLASH_DFLASH_PAGE_LENGTH);

            if (__ProgramDFlashPerPage(pageAddr, endinitSfty_pw, flashType, flashBuf.u32Data[0], flashBuf.u32Data[1]) != 0)
            {
                return -1;
            }

            writeLength += IFXFLASH_DFLASH_PAGE_LENGTH;
        }
    }

    // 最后不足 IFXFLASH_DFLASH_PAGE_LENGTH 的长度
    if (writeLength < length)
    {
        uint32 lastLength = length - writeLength;


        uint32 pageAddr   = programAddr + writeLength;

        memset(flashBuf.u8Data, 0, 8);
        memcpy(flashBuf.u8Data, (char *)pBuf + writeLength, lastLength);

        if (__ProgramDFlashPerPage(pageAddr, endinitSfty_pw, flashType, flashBuf.u32Data[0], flashBuf.u32Data[1]) != 0)
        {
            return -1;
        }
    }

    return 0;
}

/* 根据地址范围得到需要操作 Flash 的信息 */
uint8 HAL_GetFlashOperationInfo(uint32 startAddr, uint32 endAddr, FlashOpera_t *pflashInfo)
{
    if (startAddr >= endAddr)
    {
        return -1;
    }

    if ((startAddr >= 0xA0000000 && startAddr <= 0xA05FFFFF) && endAddr <= 0xA05FFFFF)
    {
        pflashInfo->eFlashType      = FLASH_TYPE_P_FLASH;
        pflashInfo->pfnWriteFlash   = __ProgramPFlash;
        pflashInfo->pfnEraseFlash   = __ErasePFlash;
        pflashInfo->ptflashSector   = (IfxFlash_flashSector *)IfxFlash_pFlashTableLog;
        pflashInfo->sectorNum       = IFXFLASH_PFLASH_NUM_LOG_SECTORS;
        pflashInfo->sectorSize      = PFLASH_BLANK_SIZE;
    }
    else if (startAddr >= 0xAF000000 && startAddr <= 0xAF03FFFF && endAddr <= 0xAF03FFFF)
    {
        pflashInfo->eFlashType      = FLASH_TYPE_D_FLASH;
        pflashInfo->pfnWriteFlash   = __ProgramDFlash;
        pflashInfo->pfnEraseFlash   = __EraseDFlash;
        pflashInfo->ptflashSector   = (IfxFlash_flashSector *)IfxFlash_dFlashTableEepLog;
        pflashInfo->sectorNum       = IFXFLASH_DFLASH_NUM_LOG_SECTORS;
        pflashInfo->sectorSize      = DFLASH_BLANK_SIZE;
    }
    else if (startAddr >= 0xAF400000 && startAddr <= 0xAF405FFF && endAddr <= 0xAF405FFF)
    {
        pflashInfo->eFlashType      = FLASH_TYPE_UCB_FLASH;
        pflashInfo->pfnWriteFlash   = __ProgramDFlash;
        pflashInfo->pfnEraseFlash   = __EraseDFlash;
        pflashInfo->ptflashSector   = (IfxFlash_flashSector *)IfxFlash_dFlashTableUcbLog;
        pflashInfo->sectorNum       = IFXFLASH_DFLASH_NUM_UCB_LOG_SECTORS;
        pflashInfo->sectorSize      = UCBFLASH_BLANK_SIZE;
    }
    else
    {
        return -1;
    }

    pflashInfo->startSector = -1;
    pflashInfo->endSector = -1;

    for (int i = 0; i < pflashInfo->sectorNum; i++)
    {
        if (startAddr >= pflashInfo->ptflashSector[i].start && startAddr <= pflashInfo->ptflashSector[i].end)
        {
            pflashInfo->startSector = i;

            for (int j = i; j < pflashInfo->sectorNum; j++)
            {
                if (endAddr >= pflashInfo->ptflashSector[j].start && endAddr <= pflashInfo->ptflashSector[j].end)
                {
                    pflashInfo->endSector = j;
                    break;
                }
            }
        }
    }

    if (pflashInfo->startSector < 0 || pflashInfo->endSector < 0)
    {
        return -1;
    }

    return 0;
}

/**
  * @brief      Flash 驱动初始化
  *
  */
void Hal_Flash_Init(void)
{
    /* 将擦除和编程复制到PSPR存储器中 */
    memcpy((void *)FLASH_ERASESECTOR_ADDR, (const void *)IfxFlash_eraseSector, FLASH_ERASESECTOR_LEN);
    sg_tPflashFun.pfnEraseSector = (void *)FLASH_RELOC_START_ADDR;

    memcpy((void *)FLASH_WAITUNBUSY_ADDR, (const void *)IfxFlash_waitUnbusy, FLASH_WAITUNBUSY_LEN);
    sg_tPflashFun.pfnWaitUnbusy = (void *)FLASH_WAITUNBUSY_ADDR;

    memcpy((void *)FLASH_ENTERPAGEMODE_ADDR, (const void *)IfxFlash_enterPageMode, FLASH_ENTERPAGEMODE_LEN);
    sg_tPflashFun.pfnEnterPageMode = (void *)FLASH_ENTERPAGEMODE_ADDR;

    memcpy((void *)FLASH_LOADPAGE2X32_ADDR, (const void *)IfxFlash_loadPage2X32, FLASH_LOADPAGE2X32_LEN);
    sg_tPflashFun.pfnLoadPage2X32 = (void *)FLASH_LOADPAGE2X32_ADDR;

    memcpy((void *)FLASH_WRITEPAGE_ADDR, (const void *)IfxFlash_writePage, FLASH_WRITEPAGE_LEN);
    sg_tPflashFun.pfnWritePage = (void *)FLASH_WRITEPAGE_ADDR;

    memcpy((void *)FLASH_PFLASHERASE_ADDR, (const void *)__PFlashErase, FLASH_PFLASHERASE_LEN);
    sg_tPflashFun.pfnFlashErase = (void *)FLASH_PFLASHERASE_ADDR;

    memcpy((void *)FLASH_PFLASHPROGRAM_ADDR, (const void *)__ProgramPFlashPerPage, FLASH_PFLASHPROGRAM_LEN);
    sg_tPflashFun.pfnFlashProgram = (void *)FLASH_PFLASHPROGRAM_ADDR;
}

/**
  * @brief      擦除包含指定地址+长度所涉及的扇区数据
  *
  * @param[in]  eraseAddr 擦除地址
  * @param[in]  length    擦除长度
  * @return     0,成功; -1,失败
  */
#if 0
int Hal_Flash_Erase(uint32 eraseAddr, uint32 length)
{
    int error = 0;
    FlashOpera_t tFlashOpera;

    if (HAL_GetFlashOperationInfo(eraseAddr, eraseAddr + length - 1, &tFlashOpera) != 0)
    {
        return -1;
    }

    for (uint32 sec = tFlashOpera.startSector; sec <= tFlashOpera.endSector; sec++)
    {
        //一次擦除dplash 4KB大小，pflash 16kb
        error += tFlashOpera.pfnEraseFlash(tFlashOpera.ptflashSector[sec].start);
    }

    return (error != 0 ? -1 : 0);
}
#else

uint8 Hal_Flash_Erase(FlashOpera_t *pflashInfo,uint32 erasesec)
{
    int error = 0;
     //一次擦除dplash 4KB大小，pflash 16kb
     error = pflashInfo->pfnEraseFlash(pflashInfo->ptflashSector[erasesec].start);

    return (error != 0 ? -1 : 0);
}

#endif

uint8 Hal_Flash_write(uint32 Addr, const void *pBuf, uint32 length)
{
    FlashOpera_t tFlashOpera;
    uint32 writeLength = 0;
    uint32 needWriteLength = 0;
    uint32 currProgramAddr = 0;

    if (HAL_GetFlashOperationInfo(Addr, Addr + length - 1, &tFlashOpera) != 0)
    {
       return -1;
    }

    switch(tFlashOpera.eFlashType){

       case FLASH_TYPE_P_FLASH :
         //写pflash
         if ((Addr - tFlashOpera.ptflashSector[tFlashOpera.startSector].start) % IFXFLASH_PFLASH_PAGE_LENGTH != 0)
         {
            return -1;
         }

         for (uint32 sec = tFlashOpera.startSector; sec <= tFlashOpera.endSector; sec++)
         {
             if (writeLength >= length)
             {
                 break;
             }

             currProgramAddr = Addr + writeLength;

             if ((currProgramAddr + (length - writeLength) - 1) > tFlashOpera.ptflashSector[sec].end)
             {
                 needWriteLength = tFlashOpera.ptflashSector[sec].end - currProgramAddr + 1;
             }
             else
             {
                 needWriteLength = length - writeLength;
             }

             if (tFlashOpera.pfnWriteFlash(currProgramAddr, ((char *)pBuf + writeLength), needWriteLength) != 0)
             {
                 return -1;
             }

             writeLength += needWriteLength;
         }
         /* Verify the programmed data */
         if (memcmp((uint8 *)Addr, pBuf, length))
         {
             return -1;
         }

            return 0;
         break;
       case FLASH_TYPE_D_FLASH :
           //写Dflash
        for (uint32 sec = tFlashOpera.startSector; sec <= tFlashOpera.endSector; sec++)
        {
            if (writeLength >= length)
            {
               break;
            }
             currProgramAddr = Addr + writeLength;
            if ((currProgramAddr + (length - writeLength) - 1) > tFlashOpera.ptflashSector[sec].end)
            {
               needWriteLength = tFlashOpera.ptflashSector[sec].end - currProgramAddr + 1;
            }
            else
            {
               needWriteLength = length - writeLength;
            }
               //判断是否擦除过
            if (__IsDFlashWritable(currProgramAddr, needWriteLength))
            {
               if(tFlashOpera.pfnWriteFlash(currProgramAddr, ((char *)pBuf + writeLength), needWriteLength) != 0)
               {
                   return -1;
               }
                 writeLength += needWriteLength;
            }
            else
            {
                return -1;
            }
         }
          /* Verify the programmed data */
         if (memcmp((uint8 *)Addr, pBuf, length))
         {
             return -1;
         }
             return 0;
           break;

        case FLASH_TYPE_UCB_FLASH :

           return -1;
         break;

        default :
            return -1;
         break;
      }

    return 0;
}

#if 0
/**
  * @brief      在指定地址编程数据
  *             Pflash  一个page 16K
  * @note       使用前需要先调用 Hal_Flash_Erase 擦除该区域, 才能确保正常写入
  * @param[in]  programAddr 编程地址
  * @param[in]  programAddr 校验地址
  * @param[in]  pBuf        编程数据
  * @param[in]  length      编程数据长度
  * @return     0,成功; -1,失败
  */
int Hal_Flash_Program(uint32 programAddr, uint32 verifyAddr, const void *pBuf, uint32 length)
{
    FlashOpera_t tFlashOpera;

    if (HAL_GetFlashOperationInfo(programAddr, programAddr + length - 1, &tFlashOpera) != 0)
    {
        return -1;
    }

    if (tFlashOpera.eFlashType == FLASH_TYPE_P_FLASH)
    {
        if ((programAddr - tFlashOpera.ptflashSector[tFlashOpera.startSector].start) % IFXFLASH_PFLASH_PAGE_LENGTH != 0)
        {
            return -1;
        }
    }

    uint32 writeLength = 0;
    uint32 needWriteLength = 0;
    uint32 currProgramAddr = 0;

    for (uint32 sec = tFlashOpera.startSector; sec <= tFlashOpera.endSector; sec++)
    {
        if (writeLength >= length)
        {
            break;
        }

        currProgramAddr = programAddr + writeLength;

        if ((currProgramAddr + (length - writeLength) - 1) > tFlashOpera.ptflashSector[sec].end)
        {
            needWriteLength = tFlashOpera.ptflashSector[sec].end - currProgramAddr + 1;
        }
        else
        {
            needWriteLength = length - writeLength;
        }

        if (tFlashOpera.pfnWriteFlash(currProgramAddr, ((char *)pBuf + writeLength), needWriteLength) != 0)
        {
            return -1;
        }

        writeLength += needWriteLength;
    }

    /* Verify the programmed data */
    if (memcmp((uint8 *)verifyAddr, pBuf, length))
    {
        return -1;
    }

    return 0;
}

/**
  * @brief      写指定长度数据至 DFlash 中, 不影响地址长度以外的数据
  *             Dflash  一个page 4K
  * @param[in]  writeAddr   写入地址
  * @param[in]  pBuf        写入数据
  * @param[in]  length      写入数据长度
  * @return     0,成功; -1,失败
  */
int Hal_Flash_Write(uint32 writeAddr, const void *pBuf, uint32 length)
{
    FlashOpera_t tFlashOpera;


    if (HAL_GetFlashOperationInfo(writeAddr, writeAddr + length - 1, &tFlashOpera) != 0)
    {
        return -1;
    }

    if (tFlashOpera.eFlashType == FLASH_TYPE_P_FLASH)
    {
        return -1;
    }

    uint32 writeLength = 0;
    uint32 needWriteLength = 0;
    uint32 currProgramAddr = 0;

    for (uint32 sec = tFlashOpera.startSector; sec <= tFlashOpera.endSector; sec++)
    {
        if (writeLength >= length)
        {
            break;
        }

        currProgramAddr = writeAddr + writeLength;

        if ((currProgramAddr + (length - writeLength) - 1) > tFlashOpera.ptflashSector[sec].end)
        {
            needWriteLength = tFlashOpera.ptflashSector[sec].end - currProgramAddr + 1;
        }
        else
        {
            needWriteLength = length - writeLength;
        }
         //判断是否擦除过
        if (__IsDFlashWritable(currProgramAddr, needWriteLength))
        {
            if (tFlashOpera.pfnWriteFlash(currProgramAddr, ((char *)pBuf + writeLength), needWriteLength) != 0)
            {
                return -1;
            }
            writeLength += needWriteLength;
        }
        else{
           return -1;
       }
    }

    /* Verify the programmed data */
    if (memcmp((uint8 *)writeAddr, pBuf, length))
    {
        return -1;
    }

    return 0;
}
#endif
/**
  * @brief      从 DFlash 读取指定长度得数据内存
  *
  * @param[in]  writeAddr   读取地址
  * @param[in]  pBuf        需要读取的buf
  * @param[in]  length      需要读取数据长度
  * @return     0,成功; -1,失败
  */
uint32 Hal_Flash_Read(uint32 readAddr, void *pBuf, uint32 length)
{
    uint32 endAddr = readAddr + length - 1;

    if (readAddr >= 0xAF000000 && readAddr <= 0xAF03FFFF && endAddr <= 0xAF03FFFF)
    {
        memcpy(pBuf, (uint8 *)readAddr, length);
        return length;
    }
    else if (readAddr >= 0xAF400000 && readAddr <= 0xAF405FFF && endAddr <= 0xAF405FFF)
    {
        memcpy(pBuf, (uint8 *)readAddr, length);
        return length;
    }

    return 0;
}

