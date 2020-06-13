#ifndef MEM_IF_H_
#define MEM_IF_H_

//#include "Fee.h"

Std_ReturnType MemIf_Write( uint8 DeviceIndex, uint16 BlockNumber, const uint8* DataBufferPtr ) ;
MemIf_StatusType MemIf_GetStatus( uint8 DeviceIndex ) ;
MemIf_JobResultType MemIf_GetJobResult( uint8 DeviceIndex ) ;
Std_ReturnType MemIf_InvalidateBlock( uint8 DeviceIndex, uint16 BlockNumber ) ;


#endif
