/*
 * MemIf.h
 *
 *  Created on: Jun 9, 2020
 *      Author: Sahar
 */

#ifndef BSP_MEMIF_MEMIF_H_
#define BSP_MEMIF_MEMIF_H_

#include "MemIf_Types.h"

#define MemIf_Write(DeviceIndex, BlockNumber, DataPtr) \
Fee_Write(BlockNumber, DataPtr)

#define MemIf_Read( DeviceIndex,BlockNumber, DataBufferPtr, Length)\
		Fee_Read( BlockNumber,DataBufferPtr, Length )

#define MemIf_GetStatus( DeviceIndex)\
		Fee_GetStatus()

#define MemIf_GetJobResult(DeviceIndex) \
		Fee_GetJobResult()



#endif /* BSP_MEMIF_MEMIF_H_ */
