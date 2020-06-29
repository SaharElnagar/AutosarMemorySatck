/*
 * MemIf.c
 *
 *  Created on: Jun 22, 2020
 *      Author: Sahar
 */

#include "Std_Types.h"
#include "MemIf_Types.h"
#include "MemIf_Cfg.h"

Std_ReturnType      (*MemIf_WriteFctPtr[MEMIF_NUMBER_OF_DEVICES])(uint16 , const uint8* );
Std_ReturnType      (*MemIf_ReadFctPtr [MEMIF_NUMBER_OF_DEVICES])(uint16 ,       uint8* );
MemIf_StatusType    (*MemIf_GetStatusFctPtr[MEMIF_NUMBER_OF_DEVICES])(void );
MemIf_JobResultType (*MemIf_GetResultFctPtr[MEMIF_NUMBER_OF_DEVICES])(void );
Std_ReturnType      (*MemIf_InvalidateFctPtr[MEMIF_NUMBER_OF_DEVICES])(uint16 );
