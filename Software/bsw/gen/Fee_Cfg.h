/*******************************************************************************
**                                                                            **
**  FILENAME     : Fee_Cfg.h                                                  **
**                                                                            **
**  VERSION      : 4.3.1                                                      **
**                                                                            **
**  DATE         : 2020-3-12                                                  **
**                                                                            **
**  PLATFORM     : TIVA C                                                     **
**                                                                            **
**  AUTHOR       : Yomna Mokhtar                                              **
**                                                                            **
**                                                                            **
*******************************************************************************/


#ifndef FEE_CFG_H_
#define FEE_CFG_H_

#include "MemIf_Types.h"
#include "Fee_Types.h"
#include "Fls.h"
#include "NvM_Cbk.h"


#define FeeDevErrorDetect					STD_ON

#define FeeMainFunctionPeriod				1

#define FeeNvmJobEndNotification			NvM_JobEndNotification

#define FeeNvmJobErrorNotification			NvM_JobErrorNotification

#define FeePollingMode						STD_OFF

#define FeeSetModeSupported					STD_OFF

#define FeeVersionInfoApi					STD_ON

//The size in bytes to which logical blocks shall be aligned = 1KB = 1024 BYTE
#define FeeVirtualPageSize				    0x00000400

/*****************************************************************************************/
/*FeeBlockConfiguration                  																								 */
/*Configuration of block specific parameters for the Flash EEPROM Emulation module       */
/*****************************************************************************************/

#define BLOCKS_NUM							(4U)

#define FEE_BLOCK_0_NUMBER					(1U)
#define FEE_BLOCK_1_NUMBER					(2U)
#define FEE_BLOCK_2_NUMBER					(3U)
#define FEE_BLOCK_3_NUMBER					(4U)

#define FEE_BLOCK_0_SIZE					(5120U)
#define FEE_BLOCK_1_SIZE                    (4096U)
#define FEE_BLOCK_2_SIZE                    (11200U)
#define FEE_BLOCK_3_SIZE                    (2048U)

#define MAX_CONFIGURED_BLOCK_SIZE           (11200U)
#define BLOCKS_SIZE                         (FEE_BLOCK_0_SIZE + FEE_BLOCK_1_SIZE + FEE_BLOCK_2_SIZE + FEE_BLOCK_3_SIZE)

#define FEE_BLOCK_0_IMMEDIATE_DATA			FALSE			
#define FEE_BLOCK_1_IMMEDIATE_DATA			FALSE
#define FEE_BLOCK_2_IMMEDIATE_DATA			FALSE
#define FEE_BLOCK_3_IMMEDIATE_DATA			FALSE

#define FEE_BLOCK_0_WRITE_CYCLES		    (1000U)
#define FEE_BLOCK_1_WRITE_CYCLES			(1000U)
#define FEE_BLOCK_2_WRITE_CYCLES			(1000U)
#define FEE_BLOCK_3_WRITE_CYCLES			(1000U)

#define FeeDeviceIndex						(1U)


/* Macro to return -1 if the Blocksize is not aligned to FlsPageSize */
#define SizeOfBlockAligned(Blocksize)             ((Blocksize % FlsPageSize) != 0 )? -1 : 1

/* struct to raise compilation error if a block configured size is not aligned to FlsPageSize */
typedef struct
{
    uint8 Block_0_Size : SizeOfBlockAligned(FEE_BLOCK_0_SIZE) ;
    uint8 Block_1_Size : SizeOfBlockAligned(FEE_BLOCK_1_SIZE) ;
    uint8 Block_2_Size : SizeOfBlockAligned(FEE_BLOCK_2_SIZE) ;
    uint8 Block_3_Size : SizeOfBlockAligned(FEE_BLOCK_3_SIZE) ;

}CheckBlockSize ;


#endif
