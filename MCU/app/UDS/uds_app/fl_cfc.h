#ifndef __FLS_CFC_H__
#define __FLS_CFC_H__

#include <drv/dflash.h>
#include "string.h"
#include "user_config.h"



#define FL_USE_GAP_FILL  0

#define FL_NUM_LOGICAL_BLOCKS                (uint8)11


#define FL_NVM_INFO_ADDRESS                  0xAF000000U

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

void FL_InitState(void);
#endif
