
/*
 * MemIf_Types.h
 *
 *  Created on: Nov 24, 2019
 *      Author: Sahar
 */

#ifndef MEMIF_TYPES_H_
#define MEMIF_TYPES_H_

#include "Platform_Types.h"

typedef uint8 MemIf_JobResultType;
#define     MEMIF_JOB_OK            (0U)
#define     MEMIF_JOB_FAILED        (1U)
#define     MEMIF_JOB_PENDING       (2U)
#define     MEMIF_JOB_CANCELED      (3U)


typedef uint8 MemIf_StatusType;
#define     MEMIF_UNINIT            (0U)
#define     MEMIF_IDLE              (1U)
#define     MEMIF_BUSY              (2U)
#define     MEMIF_BUSY_INTERNAL     (3U)

typedef uint8 MemIf_ModeType;
#define MEMIF_MODE_SLOW             (0U)
#define MEMIF_MODE_FAST             (1U)






#endif /* MEMIF_TYPES_H_ */


