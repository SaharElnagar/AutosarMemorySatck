/*******************************************************************************
**                                                                            **
**  FILENAME     : Fee.c                                                      **
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



/*****************************************************************************************/
/*                                   Include Component headres                           */
/*****************************************************************************************/
#include "Fee.h"

/*****************************************************************************************/
/*                                   Include Other  headres                              */
/*****************************************************************************************/
#include "Det.h"

/*****************************************************************************************/
/*                                   Local Macro Definitions                             */
/*****************************************************************************************/
/* Memory read address calculation */
#define MEMORY_READ_ADRESS(Logical_Block_Num, Offset)			(((Logical_Block_Num - FIRST_BLOCK_NUM)*\
																														 FeeVirtualPageSize) + Offset)

/* Block memory address calculation */
#define MEMORY_BLOCK_ADRESS(Logical_Block_Num)				    ((Logical_Block_Num - FIRST_BLOCK_NUM)*\
																														FeeVirtualPageSize)
																															
/*Calculate number of virtual pages inside a block*/
#define VIRTUAL_PAGES_NUM(Block_Size)                ((Block_Size % FeeVirtualPageSize != 0) ?\
                                                     ((Block_Size / FeeVirtualPageSize) + 1) :\
																					           (Block_Size / FeeVirtualPageSize))


/*****************************************************************************************/
/*																	Type Definitions				     								         */
/*****************************************************************************************/

/*pending job type definition*/
typedef uint8 JOB_PENDING_TYPE;
#define NO_JOB										(JOB_PENDING_TYPE)0
#define READ_JOB									(JOB_PENDING_TYPE)1
#define WRITE_JOB   							(JOB_PENDING_TYPE)2
#define ERASE_JOB   							(JOB_PENDING_TYPE)3
#define INVALIDATE_JOB						(JOB_PENDING_TYPE)4


/*****************************************************************************************/
/*                                   Local Variable Definitions                          */
/*****************************************************************************************/

uint16 Blocks_Sizes[BLOCKS_NUM] = {FEE_BLOCK_0_SIZE, FEE_BLOCK_1_SIZE, FEE_BLOCK_2_SIZE, FEE_BLOCK_3_SIZE};
boolean ImmediateData_Flags[BLOCKS_NUM] = {FEE_BLOCK_0_IMMEDIATE_DATA, FEE_BLOCK_1_IMMEDIATE_DATA, FEE_BLOCK_2_IMMEDIATE_DATA, FEE_BLOCK_3_IMMEDIATE_DATA};

boolean Validation_Flags[BLOCKS_NUM] = {TRUE, TRUE, TRUE, TRUE};
boolean Consistency_Flags[BLOCKS_NUM] = {TRUE, TRUE, TRUE, TRUE};

/*Current job parameters copy*/
Fls_AddressType Job_Physical_Address;
uint8* Job_DataBufferPtr;
uint16 Job_Length;
uint16 Job_BlockCount;


/*****************************************************************************************/
/*                          Module Status variables                                      */
/*****************************************************************************************/
static MemIf_StatusType Module_Status = MEMIF_UNINIT;
static MemIf_JobResultType Job_Result;
static JOB_PENDING_TYPE Fee_PENDING_JOB;
static Std_ReturnType Function_Error;

/*****************************************************************************************/
/*                                   Local Function Declaration                          */
/*****************************************************************************************/

static void Fee_Main_Read(void);
static void Fee_Main_Write(void);
static void Fee_Main_Erase(void);
static void Fee_Main_Invalidate(void);

void Fee_Init( const Fee_ConfigType* ConfigPtr ){

	uint16 count;
	
	if(Module_Status == MEMIF_UNINIT){
		
		Module_Status = MEMIF_BUSY_INTERNAL;
		
		/*Clear Module status variables to their initial states*/
	  Job_Result = MEMIF_JOB_OK;
	  Fee_PENDING_JOB = NO_JOB;
	  Function_Error = E_OK;
	
	  /*Clear all internal variables*/
	  Job_Physical_Address = 0;
	  Job_DataBufferPtr = NULL;
	  Job_Length = 0;
	  Job_BlockCount = 0;
	
	  for(count = 0; count < BLOCKS_NUM; count++){
	    Validation_Flags[count] = TRUE;
		  Consistency_Flags[count] = TRUE;
	  }
		Module_Status = MEMIF_IDLE;
	}
	
}

#if (FeeSetModeSupported == STD_ON)
void Fee_SetMode( MemIf_ModeType Mode ){
		
	// We will see it later
	/*
	#if (FeeDevErrorDetect == STD_ON)
			if(Module_Status == MEMIF_UNINIT){
					Function_Error = Det_ReportError(FEE_MODULE_ID, FEE_INSTANCE_ID, FEE_SETMODE_API_ID, FEE_E_UNINIT);
					return;
			}
	#endif
	
	if(Module_Status == MEMIF_BUSY){
			Function_Error = Det_ReportRuntimeError(FEE_MODULE_ID, FEE_INSTANCE_ID, FEE_SETMODE_API_ID, FEE_E_BUSY);
			return;
	}
	
	if(Module_Status == MEMIF_IDLE || Module_Status == MEMIF_BUSY_INTERNAL){
			Fee_PENDING_JOB = MODE_CHANGE_JOB;
			Module_Status = MEMIF_BUSY;
			Job_Result = E_NOT_OK;
	}
*/
}

#endif


Std_ReturnType Fee_Read( uint16 BlockNumber, uint16 BlockOffset, uint8* DataBufferPtr, uint16 Length ){
  
  uint16 count = 0;
  uint16 Compare_Block_Number = FIRST_BLOCK_NUM;
  boolean Valid_Block_Num = FALSE;

  if(Module_Status == MEMIF_UNINIT){
	   #if (FeeDevErrorDetect == STD_ON)
		  	Function_Error = Det_ReportError(FEE_MODULE_ID, FEE_INSTANCE_ID, FEE_READ_API_ID, FEE_E_UNINIT);
	   #endif
	   return E_NOT_OK;
  }

  else if(Module_Status == MEMIF_BUSY){
	  	Function_Error = Det_ReportRuntimeError(FEE_MODULE_ID, FEE_INSTANCE_ID, FEE_READ_API_ID, FEE_E_BUSY);
		  return E_NOT_OK;
  }

	/*Loop over the configured blocks to find the desired BlockNumber*/
	for(count = 0; count < BLOCKS_NUM; count++){
	  if(BlockNumber == Compare_Block_Number){
		   Valid_Block_Num = TRUE; 
			 Job_BlockCount = count;
		   break;
	  }
		else{
			Compare_Block_Number += VIRTUAL_PAGES_NUM(Blocks_Sizes[count]);
	  }
  }


  #if (FeeDevErrorDetect == STD_ON)
		
    if(Valid_Block_Num == FALSE){
		  Function_Error = Det_ReportError(FEE_MODULE_ID, FEE_INSTANCE_ID, FEE_READ_API_ID, FEE_E_INVALID_BLOCK_NO);
			return E_NOT_OK;
		}
		
    if(BlockOffset > Blocks_Sizes[Job_BlockCount]){
      Function_Error = Det_ReportError(FEE_MODULE_ID, FEE_INSTANCE_ID, FEE_READ_API_ID, FEE_E_INVALID_BLOCK_OFS);
      return E_NOT_OK;
    }
		
    if(DataBufferPtr == NULL){
      Function_Error = Det_ReportError(FEE_MODULE_ID, FEE_INSTANCE_ID, FEE_READ_API_ID, FEE_E_PARAM_POINTER);
      return E_NOT_OK;
    }
		
    if((Length + BlockOffset) > Blocks_Sizes[Job_BlockCount]){
      Function_Error = Det_ReportError(FEE_MODULE_ID, FEE_INSTANCE_ID, FEE_READ_API_ID, FEE_E_INVALID_BLOCK_LEN);
      return E_NOT_OK;
    }
		
  #endif
		
  if(Module_Status == MEMIF_IDLE || Module_Status == MEMIF_BUSY_INTERNAL){
	  	Job_Physical_Address = MEMORY_READ_ADRESS(BlockNumber, BlockOffset);
		  Job_DataBufferPtr = DataBufferPtr;
		  Job_Length = Length;
	
		  Fee_PENDING_JOB = READ_JOB;
		  Module_Status = MEMIF_BUSY;
		  Job_Result = MEMIF_JOB_PENDING;			
  }
	return E_OK;

}



Std_ReturnType Fee_Write( uint16 BlockNumber, const uint8* DataBufferPtr ){
	
	uint16 count = 0;
  uint16 Compare_Block_Number = FIRST_BLOCK_NUM;
  boolean Valid_Block_Num = FALSE;
	
	if(Module_Status == MEMIF_UNINIT){
	   #if (FeeDevErrorDetect == STD_ON)
		  	Function_Error = Det_ReportError(FEE_MODULE_ID, FEE_INSTANCE_ID, FEE_WRITE_API_ID, FEE_E_UNINIT);
	   #endif
	   return E_NOT_OK;
  }

  if(Module_Status == MEMIF_BUSY){
	  	Function_Error = Det_ReportRuntimeError(FEE_MODULE_ID, FEE_INSTANCE_ID, FEE_WRITE_API_ID, FEE_E_BUSY);
		  return E_NOT_OK;
  }
	
	for(count = 0; count < BLOCKS_NUM; count++){
	  if(BlockNumber == Compare_Block_Number){
		   Valid_Block_Num = TRUE; 
			 Job_BlockCount = count;
		   break;
	  }
		else{
			Compare_Block_Number += VIRTUAL_PAGES_NUM(Blocks_Sizes[count]);
	  }
  }
	
	
	#if (FeeDevErrorDetect == STD_ON)
	
	  if(Valid_Block_Num == FALSE){
		  Function_Error = Det_ReportError(FEE_MODULE_ID, FEE_INSTANCE_ID, FEE_WRITE_API_ID, FEE_E_INVALID_BLOCK_NO);
			return E_NOT_OK;
		}
		
		if(DataBufferPtr == NULL){
      Function_Error = Det_ReportError(FEE_MODULE_ID, FEE_INSTANCE_ID, FEE_WRITE_API_ID, FEE_E_PARAM_POINTER);
      return E_NOT_OK;
    }
		
	#endif
	
	if(Module_Status == MEMIF_IDLE || Module_Status == MEMIF_BUSY_INTERNAL){
		
	  Job_Physical_Address = MEMORY_BLOCK_ADRESS(BlockNumber);
	  Job_DataBufferPtr = (uint8 *)DataBufferPtr;
		
		/*[SRS_MemHwAb_14006]                                          */
		/*you can not erase or write only parts of the configured block*/
		/*it’s either all or nothing.                                  */
		Job_Length = VIRTUAL_PAGES_NUM(Blocks_Sizes[Job_BlockCount]) * FeeVirtualPageSize;
	
		Fee_PENDING_JOB = WRITE_JOB;
		Module_Status = MEMIF_BUSY;
		Job_Result = MEMIF_JOB_PENDING;
  }
	return E_OK;
}


void Fee_Cancel( void ){
	
	#if (FeeDevErrorDetect == STD_ON)
	  
	  if(Module_Status == MEMIF_UNINIT){
		  Function_Error = Det_ReportError(FEE_MODULE_ID, FEE_INSTANCE_ID, FEE_CANCEL_API_ID, FEE_E_UNINIT);
			return;
		}
	#endif
	  
	if(Module_Status == MEMIF_BUSY){
		
		Fls_Cancel();
		
		if(Fee_PENDING_JOB == WRITE_JOB || Fee_PENDING_JOB == ERASE_JOB){
		  /*Mark the current block as inconsistent block*/
			Consistency_Flags[Job_BlockCount] = FALSE;
		}
		
		
		Job_Physical_Address = 0;
		Job_DataBufferPtr = NULL;
		Job_Length = 0;
		Job_BlockCount = 0;
		
		Module_Status = MEMIF_IDLE;
		Job_Result = MEMIF_JOB_CANCELED;
		Fee_PENDING_JOB = NO_JOB;
		
	} else{
		Function_Error = Det_ReportRuntimeError(FEE_MODULE_ID, FEE_INSTANCE_ID, FEE_CANCEL_API_ID, FEE_E_INVALID_CANCEL);
	}
	
}


MemIf_StatusType Fee_GetStatus( void ){
  
	return Module_Status;

}


MemIf_JobResultType Fee_GetJobResult( void ){

	#if (FeeDevErrorDetect == STD_ON)
	  if(Module_Status == MEMIF_UNINIT){
		  Function_Error = Det_ReportError(FEE_MODULE_ID, FEE_INSTANCE_ID, FEE_GETJOBRESULT_API_ID, FEE_E_UNINIT);
			return MEMIF_JOB_FAILED;
		}
	#endif
	
	return Job_Result;
}



Std_ReturnType Fee_InvalidateBlock( uint16 BlockNumber ){
  
  uint16 count = 0;
	uint16 Compare_Block_Number = FIRST_BLOCK_NUM;
  boolean Valid_Block_Num = FALSE;
	
	for(count = 0; count < BLOCKS_NUM; count++){
		if(BlockNumber == Compare_Block_Number){
			 Valid_Block_Num = TRUE; 
			 Job_BlockCount = count;
			 break;
		}
		else{
		  Compare_Block_Number += VIRTUAL_PAGES_NUM(Blocks_Sizes[count]);
		}
	}
	
	if(Module_Status == MEMIF_BUSY){
	 Function_Error = Det_ReportRuntimeError(FEE_MODULE_ID, FEE_INSTANCE_ID, FEE_INVALIDATEBLOCK_API_ID, FEE_E_BUSY);
	 return E_NOT_OK;
	}
	
	#if (FeeDevErrorDetect == STD_ON)
	
	  if(Module_Status == MEMIF_UNINIT){
     Function_Error = Det_ReportError(FEE_MODULE_ID, FEE_INSTANCE_ID, FEE_INVALIDATEBLOCK_API_ID, FEE_E_UNINIT);
     return E_NOT_OK;
		}
		
		if(Valid_Block_Num == FALSE){
		  Function_Error = Det_ReportError(FEE_MODULE_ID, FEE_INSTANCE_ID, FEE_INVALIDATEBLOCK_API_ID, FEE_E_INVALID_BLOCK_NO);
			return E_NOT_OK;
		}
	
	#endif
	
	if(Module_Status == MEMIF_IDLE || Module_Status == MEMIF_BUSY_INTERNAL){
		
	  Job_Physical_Address = MEMORY_BLOCK_ADRESS(BlockNumber);
		Job_DataBufferPtr = NULL;
		
		/*[SRS_MemHwAb_14006]                                          */
		/*you can not erase or write only parts of the configured block*/
		/*it’s either all or nothing.                                  */
		Job_Length = VIRTUAL_PAGES_NUM(Blocks_Sizes[Job_BlockCount]) * FeeVirtualPageSize;

	  Fee_PENDING_JOB = INVALIDATE_JOB;
	  Module_Status = MEMIF_BUSY;
	  Job_Result = MEMIF_JOB_PENDING;
	}
	return E_OK;
}


#if (FeeVersionInfoApi == STD_ON)
void Fee_GetVersionInfo( Std_VersionInfoType* VersionInfoPtr ){
	
  #if (FeeDevErrorDetect == STD_ON)
	  if(VersionInfoPtr == NULL){
		  Function_Error = Det_ReportError(FEE_MODULE_ID, FEE_INSTANCE_ID, FEE_GETVERSIONINFO_API_ID, FEE_E_PARAM_POINTER);
		}
	#endif
		
	VersionInfoPtr->moduleID = FEE_MODULE_ID;
	VersionInfoPtr->instanceID = FEE_INSTANCE_ID;
	/*******************************************************************************/
	//  Rest of version info
	/*******************************************************************************/

}
#endif



Std_ReturnType Fee_EraseImmediateBlock( uint16 BlockNumber ){

	uint16 count = 0;
	uint16 Compare_Block_Number = FIRST_BLOCK_NUM;
  boolean Valid_Block_Num = FALSE;
	
	for(count = 0; count < BLOCKS_NUM; count++){
	    if(BlockNumber == Compare_Block_Number){
		     Valid_Block_Num = TRUE; 
				 Job_BlockCount = count;
		     break;
	    }
		  else{
			  Compare_Block_Number += VIRTUAL_PAGES_NUM(Blocks_Sizes[count]);
		  }
    }
	
	#if (FeeDevErrorDetect == STD_ON)
	
	  if(Module_Status == MEMIF_UNINIT){
		  Function_Error = Det_ReportError(FEE_MODULE_ID, FEE_INSTANCE_ID, FEE_ERASEIMMEDIATEBLOCK_API_ID, FEE_E_UNINIT);
			return E_NOT_OK;
		}
		
		if(Valid_Block_Num == FALSE){
		  Function_Error = Det_ReportError(FEE_MODULE_ID, FEE_INSTANCE_ID, FEE_ERASEIMMEDIATEBLOCK_API_ID, FEE_E_INVALID_BLOCK_NO);
			return E_NOT_OK;
		}
		
		if(ImmediateData_Flags[Job_BlockCount] == FALSE){
		  Function_Error = Det_ReportError(FEE_MODULE_ID, FEE_INSTANCE_ID, FEE_ERASEIMMEDIATEBLOCK_API_ID, FEE_E_INVALID_BLOCK_NO);
			return E_NOT_OK;
		}
		
		
		
	#endif
		
	if(Module_Status == MEMIF_BUSY){
	  Function_Error = Det_ReportRuntimeError(FEE_MODULE_ID, FEE_INSTANCE_ID, FEE_ERASEIMMEDIATEBLOCK_API_ID, FEE_E_BUSY);
	  return E_NOT_OK;
	}
	else{
		
	  Job_Physical_Address = MEMORY_BLOCK_ADRESS(BlockNumber);
		Job_DataBufferPtr = NULL;
		
		/*[SRS_MemHwAb_14006]                                          */
		/*you can not erase or write only parts of the configured block*/
		/*it’s either all or nothing.                                  */
		Job_Length = VIRTUAL_PAGES_NUM(Blocks_Sizes[Job_BlockCount]) * FeeVirtualPageSize;
		
	  Fee_PENDING_JOB = ERASE_JOB;
	  Module_Status = MEMIF_BUSY;
	  Job_Result = MEMIF_JOB_PENDING;
	}
	
	return E_OK;
}


void Fee_JobEndNotification( void ){

	/*Call the function defined in the configuration parameter FeeNvmJobEndNotification */
	/* upon successful end of an asynchronous operation                                 */
	FeeNvmJobEndNotification();
	
	if(Job_Result == MEMIF_JOB_PENDING){
	  Job_Result = MEMIF_JOB_OK;
	}
}


void Fee_JobErrorNotification( void ){

	/*Call the function defined in the configuration parameter FeeNvmJobErrorNotification*/
	/*upon failure of an asynchronous operation                                          */
  FeeNvmJobErrorNotification();
	
	if(Job_Result == MEMIF_JOB_PENDING){
	  Job_Result = MEMIF_JOB_FAILED;
	}
}

void Fee_MainFunction( void ){

	/*If initialization is not finished return without processing any job*/
	if(Module_Status == MEMIF_UNINIT || Module_Status == MEMIF_BUSY_INTERNAL){
	  return;
	}
	
	if(Job_Result == MEMIF_JOB_PENDING){
	  
		switch (Fee_PENDING_JOB){
			  case READ_JOB :
					Fee_Main_Read();
				break;
				
				case WRITE_JOB:
					Fee_Main_Write();
				break;
				
				case ERASE_JOB :
					Fee_Main_Erase();
				break;
				
				case INVALIDATE_JOB :
					Fee_Main_Invalidate();
				break;
			}
		
			if(Job_Result == MEMIF_JOB_PENDING){
				Job_Result = MEMIF_JOB_OK;
			}
			
			Module_Status = MEMIF_IDLE;
			Fee_PENDING_JOB = NO_JOB;
	}
}



static void Fee_Main_Read(void){

	Std_ReturnType Return_Val = E_OK;
	
	if(Validation_Flags[Job_BlockCount] == FALSE){
	  Job_Result = MEMIF_BLOCK_INVALID;
		/*Call error notification routine of the upper layer if configured*/
		FeeNvmJobErrorNotification();
	}

	if(Consistency_Flags[Job_BlockCount] == FALSE){
	  Job_Result = MEMIF_BLOCK_INCONSISTENT;
		/*Call error notification routine of the upper layer if configured*/
		FeeNvmJobErrorNotification();
	}
	
	Return_Val = Fls_Read(Job_Physical_Address, Job_DataBufferPtr, Job_Length);
	
	if(Return_Val == E_NOT_OK){
	  Job_Result = MEMIF_JOB_FAILED;
		Fee_PENDING_JOB = NO_JOB;
		Module_Status = MEMIF_IDLE;
		
		/*Call error notification routine of the upper layer if configured*/
		FeeNvmJobErrorNotification();
	}
}


static void Fee_Main_Write(void){

	Std_ReturnType Return_Val = E_OK;
	Return_Val = Fls_Write(Job_Physical_Address, Job_DataBufferPtr, Job_Length);
	
	if(Return_Val == E_NOT_OK){
	  Job_Result = MEMIF_JOB_FAILED;
		Fee_PENDING_JOB = NO_JOB;
		Module_Status = MEMIF_IDLE;
		
		/*Call error notification routine of the upper layer if configured*/
		FeeNvmJobErrorNotification();
	}
}

static void Fee_Main_Erase(void){

	Std_ReturnType Return_Val = E_OK;
	Return_Val = Fls_Erase(Job_Physical_Address, Job_Length);
	
	if(Return_Val == E_NOT_OK){
	  Job_Result = MEMIF_JOB_FAILED;
		Fee_PENDING_JOB = NO_JOB;
		Module_Status = MEMIF_IDLE;
		
		/*Call error notification routine of the upper layer if configured*/
		FeeNvmJobErrorNotification();
	}
}

static void Fee_Main_Invalidate(void){

	Std_ReturnType Return_Val = E_OK;
	Return_Val = Fls_Erase(Job_Physical_Address, Job_Length);
	
	if(Return_Val == E_NOT_OK){
	  Job_Result = MEMIF_JOB_FAILED;
		Fee_PENDING_JOB = NO_JOB;
		Module_Status = MEMIF_IDLE;
		
		/*Call error notification routine of the upper layer if configured*/
		FeeNvmJobErrorNotification();
	} else{
	  Validation_Flags[Job_BlockCount] = FALSE;
	}
}

