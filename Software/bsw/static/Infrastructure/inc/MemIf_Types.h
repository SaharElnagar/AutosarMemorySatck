
/*
 * MemIf_Types.h
 *
 *  Created on: Nov 24, 2019
 *      Author: Sahar
 */

#ifndef MEMIF_TYPES_H_
#define MEMIF_TYPES_H_

#include "Platform_Types.h"

typedef uint8 MemIf_JobResultType ;
#define     MEMIF_JOB_OK                ((MemIf_JobResultType)0U)
#define     MEMIF_JOB_FAILED            ((MemIf_JobResultType)1U)
#define     MEMIF_JOB_PENDING           ((MemIf_JobResultType)2U)
#define     MEMIF_JOB_CANCELED          ((MemIf_JobResultType)3U)
#define     MEMIF_BLOCK_INCONSISTENT    ((MemIf_JobResultType)4U)
#define     MEMIF_BLOCK_INVALID         ((MemIf_JobResultType)5U)


typedef uint8 MemIf_StatusType ;
#define     MEMIF_UNINIT                ((MemIf_JobResultType)0U)
#define     MEMIF_IDLE                  ((MemIf_JobResultType)1U)
#define     MEMIF_BUSY                  ((MemIf_JobResultType)2U)
#define     MEMIF_BUSY_INTERNAL         ((MemIf_JobResultType)3U)

typedef uint8 MemIf_ModeType ;
#define MEMIF_MODE_SLOW                 ((MemIf_JobResultType)0U)
#define MEMIF_MODE_FAST                 ((MemIf_JobResultType)1U)




#endif /* MEMIF_TYPES_H_ */


