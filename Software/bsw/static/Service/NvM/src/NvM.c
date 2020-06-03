/*******************************************************************************
**                                                                            **
**  FILENAME     : NvM.c                                                      **
**                                                                            **
**  VERSION      : 4.3.1                                                      **
**                                                                            **
**  DATE         : 2020-6-3                                                   **
**                                                                            **
**  PLATFORM     : TIVA C                                                     **
**                                                                            **
**  AUTHOR       : Yomna Mokhtar                                              **
**                                                                            **
**                                                                            **
*******************************************************************************/

#include "NvM.h"
#include "Det.h"

extern NvMBlockDescriptorType NvMBlockDescriptor[NUMBER_OF_NVM_BLOCKS];

/*******************************************************************************/
//			Enumeration Type Definitions
/*******************************************************************************/
typedef uint8 JOB_REQUEST_TYPE;
#define NO_JOB                    (JOB_REQUEST_TYPE)0
#define READ_BLOCK                (JOB_REQUEST_TYPE)1
#define WRITE_BLOCK   						(JOB_REQUEST_TYPE)2
#define RESTORE_BLOCK							(JOB_REQUEST_TYPE)3
#define ERASE_BLOCK								(JOB_REQUEST_TYPE)4
#define CANCEL_WRITE_ALL					(JOB_REQUEST_TYPE)5
#define INVALIDATE_BLOCK          (JOB_REQUEST_TYPE)6
#define READ_PRAM_BLOCK           (JOB_REQUEST_TYPE)7
#define WRITE_PRAM_BLOCK          (JOB_REQUEST_TYPE)8
#define RESTORE_PRAM_BLOCK        (JOB_REQUEST_TYPE)9

/*******************************************************************************/
//			Structure Type Definitions
/*******************************************************************************/

/* struct to hold the parameters for the job to push request into queue*/
typedef struct{
   JOB_REQUEST_TYPE Job_Type;
   NvM_BlockIdType Block_Id;
   void* RAM_Ptr;
   _Bool Immediate_Flag;
}Job_Request_Struct;

/*struct to hold the indeces which point to the queue*/
typedef struct{
  uint16 Head;
  uint16 Tail;
}Queue_Indeces_Struct;

/**************************Internal Functions' Prototypes**************************/
static Std_ReturnType Job_Enqueue(Job_Request_Struct Job_Parameters);
static Std_ReturnType Job_Dequeue(Job_Request_Struct* Job_Parameters);

/*****************************Local Variables*****************************/
// standard job queue
static Job_Request_Struct Standard_Job_Queue[NVM_SIZE_STANDARD_JOB_QUEUE];
static Queue_Indeces_Struct Stand_Queue_Indeces = {0, 0};

// immediate job queue
#if (NVM_JOB_PRIORITIZATION == STD_ON)
static Job_Request_Struct Immediate_Job_Queue[NVM_SIZE_IMMEDIATE_JOB_QUEUE];
static Queue_Indeces_Struct Immed_Queue_Indeces = {0, 0};
#endif

/**************Internal Flags**************/
static _Bool Standard_Queue_Empty = TRUE;
static _Bool Standard_Queue_FULL = FALSE;

#if (NVM_JOB_PRIORITIZATION == STD_ON)
static _Bool Immediate_Queue_Empty = TRUE;
static _Bool Immediate_Queue_FULL = FALSE;
#endif

/***************************************************************************/
/*********************Functions Implementation******************************/
/***************************************************************************/
static Std_ReturnType Job_Enqueue(Job_Request_Struct Job_Parameters)
{
	#if (NVM_JOB_PRIORITIZATION == STD_ON)
	 // Immediate Job
   if(Job_Parameters.Immediate_Flag == TRUE){

      if(Immediate_Queue_FULL == TRUE){
        return E_NOT_OK;
      }
      
			// enqueue the job request into immediate queue
      Immediate_Job_Queue[Immed_Queue_Indeces.Tail] = Job_Parameters;

      if(Immediate_Queue_Empty == TRUE){
         Immediate_Queue_Empty = FALSE;
      }

      Immed_Queue_Indeces.Tail++;

      //When Tail reaches queue end
      if(Immed_Queue_Indeces.Tail == NVM_SIZE_IMMEDIATE_JOB_QUEUE){
        Immed_Queue_Indeces.Tail = 0;
      }

      if(Immed_Queue_Indeces.Tail == Immed_Queue_Indeces.Head){
        Immediate_Queue_FULL = TRUE;
      }
      return E_OK;
   }
	 #endif
	 
  // Standard Job
	if(Standard_Queue_FULL == TRUE){
		return E_NOT_OK;
	}
  
	// enqueue the job request into standard queue
	Standard_Job_Queue[Stand_Queue_Indeces.Tail] = Job_Parameters;

	if(Standard_Queue_Empty == TRUE){
		 Standard_Queue_Empty = FALSE;
	}

	Stand_Queue_Indeces.Tail++;

	//When Tail reaches queue end
	if(Stand_Queue_Indeces.Tail == NVM_SIZE_STANDARD_JOB_QUEUE){
		Stand_Queue_Indeces.Tail = 0;
	}

	if(Stand_Queue_Indeces.Tail == Stand_Queue_Indeces.Head){
		Standard_Queue_FULL = TRUE;
	}
	return E_OK;
}

/***************************************************************************/

Std_ReturnType Job_Dequeue(Job_Request_Struct* Job_Parameters)
{
	#if (NVM_JOB_PRIORITIZATION == STD_ON)
   // If immediate queue is not empty, so dequeue immediate job
   if(Immediate_Queue_Empty == FALSE){
     *Job_Parameters = Immediate_Job_Queue[Immed_Queue_Indeces.Head];
     Immediate_Job_Queue[Immed_Queue_Indeces.Head].Block_Id = 0;
     Immediate_Job_Queue[Immed_Queue_Indeces.Head].Job_Type = NO_JOB;
     Immediate_Job_Queue[Immed_Queue_Indeces.Head].RAM_Ptr = NULL;
     Immediate_Job_Queue[Immed_Queue_Indeces.Head].Immediate_Flag = FALSE;

     if(Immediate_Queue_FULL == TRUE){
        Immediate_Queue_FULL = FALSE;
     }

     Immed_Queue_Indeces.Head++;

     if(Immed_Queue_Indeces.Head == NVM_SIZE_IMMEDIATE_JOB_QUEUE){
       Immed_Queue_Indeces.Head = 0;
     }

     if(Immed_Queue_Indeces.Head == Immed_Queue_Indeces.Tail){
       Immediate_Queue_Empty = TRUE;
     }
     return E_OK;

   }
	#endif
	 
  /* If immediate queue is empty and standard queue is empty
	 * Or there is no immediate queue and standard queue is empty
	 * So, return error
   */
  if(Standard_Queue_Empty == TRUE){
     return E_NOT_OK;
  }
	
  /*else if standard queue is not empty,
   *So, dequeue standard job
	 */
  else{
    *Job_Parameters = Standard_Job_Queue[Stand_Queue_Indeces.Head];
    Standard_Job_Queue[Stand_Queue_Indeces.Head].Block_Id = 0;
    Standard_Job_Queue[Stand_Queue_Indeces.Head].Job_Type = NO_JOB;
    Standard_Job_Queue[Stand_Queue_Indeces.Head].RAM_Ptr = NULL;
    Standard_Job_Queue[Stand_Queue_Indeces.Head].Immediate_Flag = FALSE;

    if(Standard_Queue_FULL == TRUE){
       Standard_Queue_FULL = FALSE;
    }

    Stand_Queue_Indeces.Head++;

    if(Stand_Queue_Indeces.Head == NVM_SIZE_STANDARD_JOB_QUEUE){
      Stand_Queue_Indeces.Head = 0;
    }

    if(Stand_Queue_Indeces.Head == Stand_Queue_Indeces.Tail){
      Standard_Queue_Empty = TRUE;
    }

    return E_OK;
  }
}
/***************************************************************************/



