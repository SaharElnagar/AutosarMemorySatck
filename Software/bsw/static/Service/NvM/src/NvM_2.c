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


/*******************************************************************************/
//          Enumeration Type Definitions
/*******************************************************************************/
typedef uint8 JOB_REQUEST_TYPE;
#define NO_JOB                    ((JOB_REQUEST_TYPE)0U)
#define READ_BLOCK                ((JOB_REQUEST_TYPE)1U)
#define WRITE_BLOCK               ((JOB_REQUEST_TYPE)2U)
#define RESTORE_BLOCK             ((JOB_REQUEST_TYPE)3U)
#define ERASE_BLOCK               ((JOB_REQUEST_TYPE)4U)
#define CANCEL_WRITE_ALL          ((JOB_REQUEST_TYPE)5U)
#define INVALIDATE_BLOCK          ((JOB_REQUEST_TYPE)6U)
#define READ_PRAM_BLOCK           ((JOB_REQUEST_TYPE)7U)
#define WRITE_PRAM_BLOCK          ((JOB_REQUEST_TYPE)8U)
#define RESTORE_PRAM_BLOCK        ((JOB_REQUEST_TYPE)9U)

/*******************************************************************************/
//          Structure Type Definitions
/*******************************************************************************/

/* struct to hold the parameters for the job request*/
typedef struct{
   JOB_REQUEST_TYPE Job_Type;
   NvM_BlockIdType Block_Id;
   void* RAM_Ptr;
}Job_Parameters;

/*struct to hold the indeces which point to the queue*/
typedef struct{
  uint16 Head;
  uint16 Tail;
}Queue_Indeces_Struct;

/*****************************External Variables***********************************/
extern NvMBlockDescriptorType NvMBlockDescriptor[NUMBER_OF_NVM_BLOCKS];

/**************************Internal Functions' Prototypes**************************/
static Std_ReturnType Job_Enqueue(Job_Parameters Job);
static Std_ReturnType Job_Dequeue(Job_Parameters* Job);
static void Init_Queue(void);
static _Bool Search_Queue(NvM_BlockIdType BlockId);

/*****************************Local Variables*****************************/
// standard job queue
static Job_Parameters Standard_Job_Queue[NVM_SIZE_STANDARD_JOB_QUEUE];
static Queue_Indeces_Struct Stand_Queue_Indeces = {0, 0};

// immediate job queue
#if (NVM_JOB_PRIORITIZATION == STD_ON)
  static Job_Parameters Immediate_Job_Queue[NVM_SIZE_IMMEDIATE_JOB_QUEUE];
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

static void Init_Queue(void)
{
    uint16 i;

    Stand_Queue_Indeces.Head = 0;
    Stand_Queue_Indeces.Tail = 0;

    Standard_Queue_Empty = TRUE;
    Standard_Queue_FULL = FALSE;

    #if(NVM_JOB_PRIORITIZATION == STD_ON)
      Immed_Queue_Indeces.Head = 0;
      Immed_Queue_Indeces.Tail = 0;

      Immediate_Queue_Empty = TRUE;
      Immediate_Queue_FULL = FALSE;
    #endif

    for(i = 0; i < NVM_SIZE_STANDARD_JOB_QUEUE; i++){
        Standard_Job_Queue[i].Job_Type = NO_JOB;
        Standard_Job_Queue[i].Block_Id = 0;
        Standard_Job_Queue[i].RAM_Ptr = NULL;
    }
    #if(NVM_JOB_PRIORITIZATION == STD_ON)
        for(i = 0; i < NVM_SIZE_IMMEDIATE_JOB_QUEUE; i++){
            Immediate_Job_Queue[i].Job_Type = NO_JOB;
            Immediate_Job_Queue[i].Block_Id = 0;
            Immediate_Job_Queue[i].RAM_Ptr = NULL;
        }
    #endif
}

/***************************************************************************/

static _Bool Search_Queue(NvM_BlockIdType BlockId)
{
    uint16 i;
    _Bool Return_Val = FALSE;

    #if(NVM_JOB_PRIORITIZATION == STD_ON)
        for(i = 0; i < NVM_SIZE_IMMEDIATE_JOB_QUEUE; i++){
           if(Immediate_Job_Queue[i].Block_Id == BlockId){
               Return_Val = TRUE;
               break;
           }
        }
    #endif
    for(i = 0; i < NVM_SIZE_STANDARD_JOB_QUEUE; i++){
       if(Standard_Job_Queue[i].Block_Id == BlockId){
           Return_Val = TRUE;
           break;
       }
    }

    return Return_Val;
}

/***************************************************************************/

static Std_ReturnType Job_Enqueue(Job_Parameters Job)
{

  // Immediate Job
  if(NvMBlockDescriptor[Job.Block_Id].NvMBlockJobPriority == 0){
    #if (NVM_JOB_PRIORITIZATION == STD_ON)
      if(Immediate_Queue_FULL == TRUE){
        return E_NOT_OK;
      }

      Immediate_Job_Queue[Immed_Queue_Indeces.Tail] = Job;

      if(Immediate_Queue_Empty == TRUE){
        Immediate_Queue_Empty = FALSE;
      }

      Immed_Queue_Indeces.Tail++;

      //When Tail reaches queue end
      if(Immed_Queue_Indeces.Tail == NVM_SIZE_IMMEDIATE_JOB_QUEUE){
        Immed_Queue_Indeces.Tail = 0;
      }

      //When Tail reaches Head while enqueing, the queue is full
      if(Immed_Queue_Indeces.Tail == Immed_Queue_Indeces.Head){
        Immediate_Queue_FULL = TRUE;
      }
      return E_OK;
    #endif
  }

  // Standard Job
  else{

    if(Standard_Queue_FULL == TRUE){
      return E_NOT_OK;
    }

    // if queue is empty, so insert your job directly
    if(Standard_Queue_Empty == TRUE){
      Standard_Job_Queue[Stand_Queue_Indeces.Tail] = Job;
      Standard_Queue_Empty = FALSE;
    }
    else{ // Queue is not full and not empty

      #if (NVM_JOB_PRIORITIZATION == STD_ON)
          uint16 i;   //internal variable to store the loop index
          //intermediate variable to store ID of the compared job in each cycle
          NvM_BlockIdType Compared_Job_Id;

         /*insert the new job based on priority.
          *loop over queue elements starting from tail until you reach head,
          *or reach a higher priority job*/
        for(i = Stand_Queue_Indeces.Tail ; i != Stand_Queue_Indeces.Head; i--){
          // if i = 0
          if(i == 0){
            Compared_Job_Id = Standard_Job_Queue[NVM_SIZE_STANDARD_JOB_QUEUE -1].Block_Id;
            if(NvMBlockDescriptor[Job.Block_Id].NvMBlockJobPriority < NvMBlockDescriptor[Compared_Job_Id].NvMBlockJobPriority){
              Standard_Job_Queue[i] = Standard_Job_Queue[NVM_SIZE_STANDARD_JOB_QUEUE -1];
              i = NVM_SIZE_STANDARD_JOB_QUEUE;
            }
            else{
              break;
            }
          }
          // if i != 0
          else{
            Compared_Job_Id = Standard_Job_Queue[i-1].Block_Id;
            if(NvMBlockDescriptor[Job.Block_Id].NvMBlockJobPriority < NvMBlockDescriptor[Compared_Job_Id].NvMBlockJobPriority){
              Standard_Job_Queue[i] = Standard_Job_Queue[i-1];
            }
            else{
              break;
            }
          }
        }
        Standard_Job_Queue[i] = Job;
      #else
        Standard_Job_Queue[Stand_Queue_Indeces.Tail] = Job;
      #endif
    }

    Stand_Queue_Indeces.Tail++;

    //When Tail reaches queue end
    if(Stand_Queue_Indeces.Tail == NVM_SIZE_STANDARD_JOB_QUEUE){
      Stand_Queue_Indeces.Tail = 0;
    }

      //When Tail reaches Head while enqueing, the queue is full
    if(Stand_Queue_Indeces.Tail == Stand_Queue_Indeces.Head){
      Standard_Queue_FULL = TRUE;
    }
  }
  return E_OK;
}

/***************************************************************************/

static Std_ReturnType Job_Dequeue(Job_Parameters* Job)
{
 #if (NVM_JOB_PRIORITIZATION == STD_ON)
   //Immediate queue is not empty, so dequeue immediate job
   if(Immediate_Queue_Empty == FALSE){

     *Job = Immediate_Job_Queue[Immed_Queue_Indeces.Head];
     Immediate_Job_Queue[Immed_Queue_Indeces.Head].Block_Id = 0;
     Immediate_Job_Queue[Immed_Queue_Indeces.Head].Job_Type = NO_JOB;
     Immediate_Job_Queue[Immed_Queue_Indeces.Head].RAM_Ptr = NULL;

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
  //Immediate queue is empty and standard queue is empty, so return error
  if(Standard_Queue_Empty == TRUE){
    return E_NOT_OK;
  }
  //Immediate queue is empty and standard queue is not empty,
  //so dequeue standard jobs
  else{
    *Job = Standard_Job_Queue[Stand_Queue_Indeces.Head];
    Standard_Job_Queue[Stand_Queue_Indeces.Head].Block_Id = 0;
    Standard_Job_Queue[Stand_Queue_Indeces.Head].Job_Type = NO_JOB;
    Standard_Job_Queue[Stand_Queue_Indeces.Head].RAM_Ptr = NULL;

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

