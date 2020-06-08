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

/*****************************************************************************************/
/*                                   Include headers                                     */
/*****************************************************************************************/
#include "NvM_Shared.h"

/*****************************************************************************************/
/*                       Local Functions Implementation                                  */
/*****************************************************************************************/

/****************************************************************************************/
/*    Function Name           : Init_Queue                                              */
/*    Function Description    : put queue in it's initialized state                     */
/*    Parameter in            : none                                                    */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : none                                                    */
/****************************************************************************************/

void Init_Queue(void)
{
    /*counter to loop over queue elements*/
    uint16 counter;

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

    for(counter = 0; counter < NVM_SIZE_STANDARD_JOB_QUEUE; counter++){
        Standard_Job_Queue[counter].Job_Type = NO_JOB;
        Standard_Job_Queue[counter].Block_Id = 0;
        Standard_Job_Queue[counter].RAM_Ptr = NULL;
    }
    #if(NVM_JOB_PRIORITIZATION == STD_ON)
        for(counter = 0; counter < NVM_SIZE_IMMEDIATE_JOB_QUEUE; counter++){
            Immediate_Job_Queue[counter].Job_Type = NO_JOB;
            Immediate_Job_Queue[counter].Block_Id = 0;
            Immediate_Job_Queue[counter].RAM_Ptr = NULL;
        }
    #endif
}

/****************************************************************************************/
/*    Function Name           : Search_Queue                                            */
/*    Function Description    : Search for a passed Block Id                            */
/*                              and find if it exists in the queue or not               */
/*    Parameter in            : BlockId                                                 */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : _Bool                                                   */
/****************************************************************************************/

Std_ReturnType Search_Queue(NvM_BlockIdType BlockId)
{
    /*counter to loop over queue elements*/
    uint16 counter;
    Std_ReturnType Return_Val = FALSE;

    #if(NVM_JOB_PRIORITIZATION == STD_ON)
        for(counter = 0; counter < NVM_SIZE_IMMEDIATE_JOB_QUEUE; counter++){
           if(Immediate_Job_Queue[counter].Block_Id == BlockId){
               Return_Val = TRUE;
               break;
           }
        }
    #endif
    for(counter = 0; counter < NVM_SIZE_STANDARD_JOB_QUEUE; counter++){
       if(Standard_Job_Queue[counter].Block_Id == BlockId){
           Return_Val = TRUE;
           break;
       }
    }

    return Return_Val;
}

/****************************************************************************************/
/*    Function Name           : Job_Enqueue                                             */
/*    Function Description    : Add jobs to the queue to be executed later              */
/*    Parameter in            : Job                                                     */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : Std_ReturnType                                          */
/****************************************************************************************/

Std_ReturnType Job_Enqueue(Job_Parameters Job)
{

  /*[SWS_NvM_00378]
   * In case of priority based job processing order,
   * the NvM module shall use two queues, one for immediate write jobs (crash data)
   * another for all other jobs
   */
  // Case1 : Immediate Job
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

  // Case2 : Standard Job
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

        uint16 counter;   //internal variable to store the loop index
        //intermediate variable to store ID of the compared job in each cycle
        NvM_BlockIdType Compared_Job_Id;

        /*insert the new job based on priority.
        *loop over queue elements starting from tail until you reach head,
        *or reach a higher priority job
        */
        for(counter = Stand_Queue_Indeces.Tail ; counter != Stand_Queue_Indeces.Head; counter--){
          // if counter = 0
          if(counter == 0){
            Compared_Job_Id = Standard_Job_Queue[NVM_SIZE_STANDARD_JOB_QUEUE -1].Block_Id;
            if(NvMBlockDescriptor[Job.Block_Id].NvMBlockJobPriority < NvMBlockDescriptor[Compared_Job_Id].NvMBlockJobPriority){
              Standard_Job_Queue[counter] = Standard_Job_Queue[NVM_SIZE_STANDARD_JOB_QUEUE -1];
              counter = NVM_SIZE_STANDARD_JOB_QUEUE;
            }
            else{
              break;
            }
          }
          // if counter != 0
          else{
            Compared_Job_Id = Standard_Job_Queue[counter-1].Block_Id;
            if(NvMBlockDescriptor[Job.Block_Id].NvMBlockJobPriority < NvMBlockDescriptor[Compared_Job_Id].NvMBlockJobPriority){
              Standard_Job_Queue[counter] = Standard_Job_Queue[counter-1];
            }
            else{
              break;
            }
          }
        }
        Standard_Job_Queue[counter] = Job;

    /*
     * [SWS_NvM_00379]
     * If priority based job processing is disabled via configuration,
     * the NvM module shall not support immediate write jobs. In this case,
     * the NvM module processes all jobs in FCFS order
     */
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

/****************************************************************************************/
/*    Function Name           : Job_Dequeue                                             */
/*    Function Description    : take a job out of the queue to be executed              */
/*    Parameter in            : none                                                    */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : Job                                                     */
/*    Return value            : Std_ReturnType                                          */
/****************************************************************************************/

Std_ReturnType Job_Dequeue(Job_Parameters* Job)
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

/*****************************************************************************************/
/*                              Global Functions Implementation                          */
/*****************************************************************************************/

/****************************************************************************************/
/*    Function Name           : NvM_WriteBlock                                          */
/*    Function Description    : Service to copy the data of the RAM block to its        */
/*                              corresponding NV block.                                 */
/*    Parameter in            : BlockId, NvM_SrcPtr                                     */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : Std_ReturnType                                          */
/*    Requirement              : SWS_NvM_00455                                          */
/*    Notes                   :                                                         */
/****************************************************************************************/
Std_ReturnType NvM_WriteBlock( NvM_BlockIdType BlockId, const void* NvM_SrcPtr )
{
    if(AdministrativeBlock[BlockId].WriteProtect == TRUE){
        /*Report error to the Dem Module*/
    }


}


