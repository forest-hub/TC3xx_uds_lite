/*****************************************************************
**    File Name    	: multi_cyc_fifo.h
**    Description  	: multi cycy fifo
**    Created Date : 2013-3-26
**    Author       	: Tomlin
*****************************************************************/
#ifndef __MULTI_CYC_FIFO_H__
#define __MULTI_CYC_FIFO_H__

#include "user_config.h"

/*default open check input parameters*/
#define SAFE_LEVEL_O3
#define FIFO_NUM                            (4u)           /*Fifo num*/
#define TOTAL_FIFO_BYTES                    (800u)         /*config total bytes*/

/*define erro code */
typedef enum
{
    ERRO_NONE = 0u,         /*no erro*/
	ERRO_LEES_MIN,		    /*less than min*/
	ERRO_NO_NODE,
	ERRO_OVER_MAX,          /*over max*/
	ERRO_POINTER_NULL,      /*pointer null*/
	ERRO_REGISTERED_SECOND, /*timer registered*/
	ERRO_TIME_TYPE_ERRO,    /*time type erro*/
	ERRO_TIME_USEING,
	ERRO_TIMEOUT,           /*timeout*/
	ERRO_WRITE_ERRO,
	ERRO_READ_ERRO
}tErroCode;

typedef enum
{
    FIFO_EMPTY,           /*fifo empty*/
    FIFO_USING,           /*fifo using*/
    FIFO_FULL             /*fifo full */
}tFifoStatus;

typedef struct
{
    uint16 xOwnerId;                 /*owner fifo id*/
    uint16 xFifoLen;                 /*fifo len*/
    uint16 xReadAddr;                /*read fifo addr*/
    uint16 xWriteAddr;               /*write fifo addr*/
    tFifoStatus eFifoStatus;         /*fifo status*/
    unsigned char *pDataFifoAddr;    /*data addr*/
    void *pvNextFifoList;            /*next fifo list*/
}tFifoInfo;

/**********************************************************
**	Function Name	:	ApplyFifo
**	Description		:	Apply a fifo
**	Input Parameter	:	i_xApplyFifoLen need apply fifo len
						i_xFifoId fifo id. Use find this fifo.
**	Modify Parameter	:	none
**	Output Parameter	:	o_peApplyStatus apply status. If apply success ERRO_NONE, else ERRO_XXX
**	Return Value		:	none
**	Version			:	v00.00.01
**	Author			:	Tomlin
**	Created Date		:	2013-3-27
**********************************************************/
 void ApplyFifo(uint16 i_xApplyFifoLen, uint16 i_xFifoId, tErroCode *o_peApplyStatus);

/**********************************************************
**	Function Name	:	WriteDataInFifo
**	Description		:	write data in fifo.
**	Input Parameter	:	i_xFifoId	fifo id
						i_pucWriteDataBuf Need write data buf
						i_xWriteDatalen  write data len
**	Modify Parameter	:	none
**	Output Parameter	:	o_peWriteStatus write data status. If successfull ERRO_NONE, else ERRO_XX
**	Return Value		:	none
**	Version			:	v00.00.01
**	Author			:	Tomlin
**	Created Date		:	2013-3-27
**********************************************************/
 void WriteDataInFifo(uint16 i_xFifoId,
					   		  unsigned char *i_pucWriteDataBuf, 
					   		uint32 i_xWriteDatalen,
					          tErroCode *o_peWriteStatus);

/**********************************************************
**	Function Name	:	ReadDataFromFifo
**	Description		:	Read data from fifo.
**	Input Parameter	:	i_xFifoId need read fifo
						i_xNeedReadDataLen read data len
**	Modify Parameter	:	none
**	Output Parameter	:	o_pucReadDataBuf need read data buf.
						o_pxReadLen need read data len
						o_peReadStatus read status. If read successfull ERRO_NONE, else ERRO_XXX
**	Return Value		:	none
**	Version			:	v00.00.01
**	Author			:	Tomlin
**	Created Date		:	2013-3-27
**********************************************************/
 void ReadDataFromFifo(uint16 i_xFifoId, uint32 i_xNeedReadDataLen,
						   		  unsigned char *o_pucReadDataBuf,
						  		  uint32 *o_pxReadLen,
						   		  tErroCode *o_peReadStatus);

/**********************************************************
**	Function Name	:	GetCanReadLen
**	Description		:	Get fifo have data.
**	Input Parameter	:	i_xFifoId fifo id
**	Modify Parameter	:	none
**	Output Parameter	:	o_pxCanReadLen how much data can read.
						o_peGetStatus get status. If get successfull ERRO_NONE, else ERRO_XXX
**	Return Value		:	none
**	Version			:	v00.00.01
**	Author			:	Tomlin
**	Created Date		:	2013-3-27
**********************************************************/
 void GetCanReadLen(uint16 i_xFifoId, uint32 *o_pxCanReadLen, tErroCode *o_peGetStatus);

/**********************************************************
**	Function Name	:	GetCanWriteLen
**	Description		:	Get can write data.
**	Input Parameter	:	i_xFifoId fifo id
**	Modify Parameter	:	none
**	Output Parameter	:	o_pxCanWriteLen how much data can write.
						o_peGetStatus get data status. If get successfull ERRO_NONE, esle ERRO_XX
**	Return Value		:	none
**	Version			:	v00.00.01
**	Author			:	Tomlin
**	Created Date		:	2013-3-27
**********************************************************/
 void GetCanWriteLen(uint16 i_xFifoId, uint32 *o_pxCanWriteLen, tErroCode *o_peGetStatus);

/**********************************************************
**	Function Name	:	ClearFIFO
**	Description		:	Clear FIFO, set read pointer equal write pointer
**	Input Parameter	:	i_xFifoId fifo id
**	Modify Parameter	:	none
**	Output Parameter	:	o_peGetStatus get data status. If get successfull ERRO_NONE, esle ERRO_XX
**	Return Value		:	none
**	Version			:	v00.00.01
**	Author			:	Tomlin
**	Created Date		:	2019-6-18
**********************************************************/
 void ClearFIFO(uint16 i_xFifoId, tErroCode *o_peGetStatus);


#endif /*#ifndef __MULTI_CYC_FIFO_H__*/

/**************************End file******************************/
