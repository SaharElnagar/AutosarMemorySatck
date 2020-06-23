/*******************************************************************************
**                                                                            **
**  FILENAME     : Fee_Cfg.c                                                  **
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

#include "Fee_Cfg.h"

/**Variables Definition**/

FeeBlockConfiguration Block0 = {FEE_BLOCK_0_NUMBER, FEE_BLOCK_0_SIZE, FEE_BLOCK_0_IMMEDIATE_DATA, FEE_BLOCK_0_WRITE_CYCLES};
FeeBlockConfiguration Block1 = {FEE_BLOCK_1_NUMBER, FEE_BLOCK_1_SIZE, FEE_BLOCK_1_IMMEDIATE_DATA, FEE_BLOCK_1_WRITE_CYCLES};
FeeBlockConfiguration Block2 = {FEE_BLOCK_2_NUMBER, FEE_BLOCK_2_SIZE, FEE_BLOCK_2_IMMEDIATE_DATA, FEE_BLOCK_2_WRITE_CYCLES};
FeeBlockConfiguration Block3 = {FEE_BLOCK_3_NUMBER, FEE_BLOCK_3_SIZE, FEE_BLOCK_3_IMMEDIATE_DATA, FEE_BLOCK_3_WRITE_CYCLES};

FeeBlockConfiguration Fee_BlockConfig[BLOCKS_NUM] = {
                                                     {FEE_BLOCK_0_NUMBER, FEE_BLOCK_0_SIZE, FEE_BLOCK_0_IMMEDIATE_DATA, FEE_BLOCK_0_WRITE_CYCLES},
                                                     {FEE_BLOCK_1_NUMBER, FEE_BLOCK_1_SIZE, FEE_BLOCK_1_IMMEDIATE_DATA, FEE_BLOCK_1_WRITE_CYCLES},
                                                     {FEE_BLOCK_2_NUMBER, FEE_BLOCK_2_SIZE, FEE_BLOCK_2_IMMEDIATE_DATA, FEE_BLOCK_2_WRITE_CYCLES},
                                                     {FEE_BLOCK_3_NUMBER, FEE_BLOCK_3_SIZE, FEE_BLOCK_3_IMMEDIATE_DATA, FEE_BLOCK_3_WRITE_CYCLES}
                                                    };




