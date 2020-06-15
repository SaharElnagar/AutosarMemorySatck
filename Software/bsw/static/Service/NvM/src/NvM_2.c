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
/*Those must be replaced by "MemIf.h" */
#include "MemIf_Types.h"


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

void Init_Queues(void)
{
    /*counter to loop over queue elements*/
    NvM_BlockIdType counter ;

    #if(NVM_JOB_PRIORITIZATION == STD_ON)
      Immed_Queue_Indeces.Head = 0 ;
      Immed_Queue_Indeces.Tail = 0 ;

      Immediate_Queue_Empty = TRUE ;
      Immediate_Queue_FULL = FALSE ;
    #endif

    Stand_Queue_Indeces.Head = 0 ;
    Stand_Queue_Indeces.Tail = 0 ;

    Standard_Queue_Empty = TRUE ;
    Standard_Queue_FULL = FALSE ;

    CRC_Queue_Indeces.Head = 0 ;
    CRC_Queue_Indeces.Tail = 0 ;

    CRC_Queue_Empty = TRUE ;
    CRC_Queue_Full = FALSE ;


    #if(NVM_JOB_PRIORITIZATION == STD_ON)
        for(counter = 0; counter < NVM_SIZE_IMMEDIATE_JOB_QUEUE; counter++)
        {
            Immediate_Job_Queue[counter].ServiceId = NVM_INIT_API_ID ;
            Immediate_Job_Queue[counter].Block_Id = 0 ;
            Immediate_Job_Queue[counter].RAM_Ptr = NULL ;
        }
    #endif

    for(counter = 0; counter < NVM_SIZE_STANDARD_JOB_QUEUE; counter++)
    {
        Standard_Job_Queue[counter].ServiceId = NVM_INIT_API_ID ;
        Standard_Job_Queue[counter].Block_Id = 0 ;
        Standard_Job_Queue[counter].RAM_Ptr = NULL ;
    }

    for(counter = 0; counter < NVM_SIZE_CRC_JOB_QUEUE ; counter++)
    {
        CRC_Job_Queue[counter] = 0 ;
    }
}

/****************************************************************************************/
/*    Function Name           : Search_Queue                                            */
/*    Function Description    : Search for a passed Block Id in the queue,              */
/*                              and find if it exists or not.                           */
/*                              If existed -> return E_OK                               */
/*                              If not existed -> return E_NOT_OK                       */
/*    Parameter in            : BlockId                                                 */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : Std_ReturnType                                          */
/****************************************************************************************/

Std_ReturnType Search_Queue(NvM_BlockIdType BlockId)
{
    /*counter to loop over queue elements*/
    uint16 counter ;
    /*Variable to save return value*/
    Std_ReturnType Return_Val = E_NOT_OK ;

    #if(NVM_JOB_PRIORITIZATION == STD_ON)
        for(counter = 0; counter < NVM_SIZE_IMMEDIATE_JOB_QUEUE; counter++){
           if(Immediate_Job_Queue[counter].Block_Id == BlockId){
               Return_Val = E_OK ;
               break ;
           }
        }
    #endif
    for(counter = 0; counter < NVM_SIZE_STANDARD_JOB_QUEUE; counter++){
       if(Standard_Job_Queue[counter].Block_Id == BlockId){
           Return_Val = E_OK ;
           break ;
       }
    }

    return Return_Val ;
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
        return E_NOT_OK ;
      }

      Immediate_Job_Queue[Immed_Queue_Indeces.Tail] = Job ;

      if(Immediate_Queue_Empty == TRUE){
        Immediate_Queue_Empty = FALSE ;
      }

      Immed_Queue_Indeces.Tail++ ;

      //When Tail reaches queue end
      if(Immed_Queue_Indeces.Tail == NVM_SIZE_IMMEDIATE_JOB_QUEUE){
        /*Go to index 0 of the queue again
         *As the queue is circular
         */
        Immed_Queue_Indeces.Tail = 0 ;
      }

      //When Tail reaches Head while enqueing, the queue is full
      if(Immed_Queue_Indeces.Tail == Immed_Queue_Indeces.Head){
        Immediate_Queue_FULL = TRUE ;
      }
      return E_OK ;
    #endif
  }

  // Case2 : Standard Job
  else{

    if(Standard_Queue_FULL == TRUE){
      return E_NOT_OK ;
    }

    // if queue is empty, so insert your job directly
    if(Standard_Queue_Empty == TRUE){
      Standard_Job_Queue[Stand_Queue_Indeces.Tail] = Job ;
      Standard_Queue_Empty = FALSE ;
    }
    else{ // Queue is not full and not empty


      #if (NVM_JOB_PRIORITIZATION == STD_ON)

        uint16 counter ;   //internal variable to store the loop index
        //intermediate variable to store ID of the compared job in each cycle
        NvM_BlockIdType Compared_Job_Id ;

        /*insert the new job based on priority.
        *loop over queue elements starting from tail until you reach head,
        *or reach a higher priority job
        */
        for(counter = Stand_Queue_Indeces.Tail ; counter != Stand_Queue_Indeces.Head; counter--){
          // if counter = 0
          if(counter == 0){
            Compared_Job_Id = Standard_Job_Queue[NVM_SIZE_STANDARD_JOB_QUEUE -1].Block_Id ;
            if(NvMBlockDescriptor[Job.Block_Id].NvMBlockJobPriority < NvMBlockDescriptor[Compared_Job_Id].NvMBlockJobPriority){
              Standard_Job_Queue[counter] = Standard_Job_Queue[NVM_SIZE_STANDARD_JOB_QUEUE -1] ;
              counter = NVM_SIZE_STANDARD_JOB_QUEUE ;
            }
            else{
              break ;
            }
          }
          // if counter != 0
          else{
            Compared_Job_Id = Standard_Job_Queue[counter-1].Block_Id ;
            if(NvMBlockDescriptor[Job.Block_Id].NvMBlockJobPriority < NvMBlockDescriptor[Compared_Job_Id].NvMBlockJobPriority){
              Standard_Job_Queue[counter] = Standard_Job_Queue[counter-1] ;
            }
            else{
              break ;
            }
          }
        }
        Standard_Job_Queue[counter] = Job ;

    /*
     * [SWS_NvM_00379]
     * If priority based job processing is disabled via configuration,
     * the NvM module shall not support immediate write jobs. In this case,
     * the NvM module processes all jobs in FCFS order
     */
      #else
        Standard_Job_Queue[Stand_Queue_Indeces.Tail] = Job ;
      #endif
    }

    Stand_Queue_Indeces.Tail++ ;

    //When Tail reaches queue end
    if(Stand_Queue_Indeces.Tail == NVM_SIZE_STANDARD_JOB_QUEUE){
      Stand_Queue_Indeces.Tail = 0 ;
    }

      //When Tail reaches Head while enqueing, the queue is full
    if(Stand_Queue_Indeces.Tail == Stand_Queue_Indeces.Head){
      Standard_Queue_FULL = TRUE ;
    }
  }
  return E_OK;
}

/****************************************************************************************/
/*    Function Name           : Job_Dequeue                                             */
/*    Function Description    : Remove a single job from the queue                      */
/*    Parameter in            : none                                                    */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : Std_ReturnType                                          */
/****************************************************************************************/

Std_ReturnType Job_Dequeue(void)
{
 #if (NVM_JOB_PRIORITIZATION == STD_ON)
   //Immediate queue is not empty, so dequeue immediate job
   if(Immediate_Queue_Empty == FALSE){

     Immediate_Job_Queue[Immed_Queue_Indeces.Head].Block_Id = 0 ;
     Immediate_Job_Queue[Immed_Queue_Indeces.Head].ServiceId = NVM_INIT_API_ID ;
     Immediate_Job_Queue[Immed_Queue_Indeces.Head].RAM_Ptr = NULL ;

     if(Immediate_Queue_FULL == TRUE){
       Immediate_Queue_FULL = FALSE ;
     }

     Immed_Queue_Indeces.Head++ ;

     if(Immed_Queue_Indeces.Head == NVM_SIZE_IMMEDIATE_JOB_QUEUE){
       Immed_Queue_Indeces.Head = 0 ;
     }

     if(Immed_Queue_Indeces.Head == Immed_Queue_Indeces.Tail){
       Immediate_Queue_Empty = TRUE ;
     }
     return E_OK ;
  }
 #endif

  //Immediate queue is empty and standard queue is empty, so return error
  if(Standard_Queue_Empty == TRUE){
    return E_NOT_OK ;
  }
  //Immediate queue is empty and standard queue is not empty,
  //so dequeue standard jobs
  else{

    Standard_Job_Queue[Stand_Queue_Indeces.Head].Block_Id = 0 ;
    Standard_Job_Queue[Stand_Queue_Indeces.Head].ServiceId = NVM_INIT_API_ID ;
    Standard_Job_Queue[Stand_Queue_Indeces.Head].RAM_Ptr = NULL ;

    if(Standard_Queue_FULL == TRUE){
      Standard_Queue_FULL = FALSE ;
    }

    Stand_Queue_Indeces.Head++ ;

    if(Stand_Queue_Indeces.Head == NVM_SIZE_STANDARD_JOB_QUEUE){
      Stand_Queue_Indeces.Head = 0 ;
    }

    if(Stand_Queue_Indeces.Head == Stand_Queue_Indeces.Tail){
      Standard_Queue_Empty = TRUE ;
    }

    return E_OK ;
  }

}

/****************************************************************************************/
/*    Function Name           : Get_SingleJob                                           */
/*    Function Description    : copy a single job parameters from queue to be executed  */
/*    Parameter in            : none                                                    */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : none                                                    */
/****************************************************************************************/

void Get_SingleJob(Job_Parameters* Job)
{
    #if(NVM_JOB_PRIORITIZATION == STD_ON)

        if(Immediate_Queue_Empty == FALSE){

            *Job = Immediate_Job_Queue[Immed_Queue_Indeces.Head] ;
        }
        else if(Standard_Queue_Empty == FALSE){

            *Job = Standard_Job_Queue[Stand_Queue_Indeces.Head] ;
        }
        else {

            Job->ServiceId = NVM_INIT_API_ID ;
            Job->Block_Id = 0 ;
            Job->RAM_Ptr = NULL ;
        }
        #else

             if(Standard_Queue_Empty == FALSE){

                *Job = Standard_Job_Queue[Stand_Queue_Indeces.Head] ;
             }
             else {

                 Job->ServiceId = NVM_INIT_API_ID ;
                 Job->Block_Id = 0 ;
                 Job->RAM_Ptr = NULL ;
             }

         #endif

}

/****************************************************************************************/
/*    Function Name           : CRCJob_Enqueue                                          */
/*    Function Description    : Enqueue CRC Jobs to be processed inside main function.  */
/*    Parameter in            : BlockId                                                 */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : Std_ReturnType                                          */
/*    Requirement             :                                                         */
/*    Notes                   :                                                         */
/****************************************************************************************/

Std_ReturnType CRCJob_Enqueue(NvM_BlockIdType BlockId)
{
    Std_ReturnType Return_Val ;

    if(CRC_Queue_Full == TRUE)
    {
        Return_Val = E_NOT_OK ;
    }
    else
    {
        /* insert the new CRC job*/
        CRC_Job_Queue[CRC_Queue_Indeces.Tail] = BlockId ;
        /* increment the tail pointer */
        CRC_Queue_Indeces.Tail++ ;

        /* When Tail reaches queue end */
        if(CRC_Queue_Indeces.Tail == NVM_SIZE_CRC_JOB_QUEUE)
        {
            /* Return the tail pointer to the queue start again */
            CRC_Queue_Indeces.Tail = 0 ;
        }

        /* if the queue was empty, mark it as not empty */
        if(CRC_Queue_Empty == TRUE)
        {
            CRC_Queue_Empty = FALSE ;
        }

        /* When Tail reaches Head while Enqueuing, the queue is full */
        if(CRC_Queue_Indeces.Tail == CRC_Queue_Indeces.Head)
        {
            CRC_Queue_Full = TRUE ;
        }
        Return_Val = E_OK ;
    }

    return Return_Val ;

}

/****************************************************************************************/
/*    Function Name           : CRCJob_Dequeue                                          */
/*    Function Description    : Dequeue CRC Jobs                                        */
/*    Parameter in            : none                                                    */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : Std_ReturnType                                          */
/*    Requirement             :                                                         */
/*    Notes                   :                                                         */
/****************************************************************************************/

 Std_ReturnType CRCJob_Dequeue( void )
{
     Std_ReturnType Return_Val ;


     if(CRC_Queue_Empty == TRUE)
     {
         Return_Val = E_NOT_OK ;
     }
     else
     {
         CRC_Job_Queue[CRC_Queue_Indeces.Head] = 0 ;
         CRC_Queue_Indeces.Head++ ;

         /* if the queue was full, mark it as not full */
         if(CRC_Queue_Full == TRUE)
         {
             CRC_Queue_Full = FALSE ;
         }

         /* When Head reaches queue end */
         if(CRC_Queue_Indeces.Head == NVM_SIZE_CRC_JOB_QUEUE)
         {
             /* Return the head pointer to the queue start again */
             CRC_Queue_Indeces.Head = 0 ;
         }

         /* When Head reaches Tail while Dequeuing, the queue is empty */
         if(CRC_Queue_Indeces.Head == CRC_Queue_Indeces.Tail)
         {
             CRC_Queue_Empty = TRUE ;
         }

         Return_Val = E_OK ;
     }

     return Return_Val ;
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
    /*Variable to save return value*/
    Std_ReturnType Return_Val = E_OK ;


    /* [SWS_NvM_00619] If development error detection is enabled for NvM module,
     * the function NvM_WriteBlock shall report the DET error NVM_E_NOT_INITIALIZED
     * when NVM not yet initialized.
     */
    if(ModuleState == MODULE_UNINITIALIZED){

       #if(NVM_DEV_ERROR_DETECT == STD_ON)
           Det_ReportError(NVRAM_MANAGER_ID, NVRAM_MANAGER_INSTANCE, NVM_WRITEBLOCK_API_ID, NVM_E_NOT_INITIALIZED) ;
       #endif
       Return_Val = E_NOT_OK ;

    }

    /* [SWS_NvM_00620]
     * If development error detection is enabled for NvM module,
     * the function NvM_WriteBlock shall report the DET error NVM_E_BLOCK_PENDING
     * when NVRAM block identifier is already queued or currently in progress.
     * */
    else if(Search_Queue(BlockId) == E_OK){

        #if(NVM_DEV_ERROR_DETECT == STD_ON)
            Det_ReportError(NVRAM_MANAGER_ID, NVRAM_MANAGER_INSTANCE, NVM_WRITEBLOCK_API_ID, NVM_E_BLOCK_PENDING) ;
        #endif
        Return_Val = E_NOT_OK ;

    }

    /* [SWS_NvM_00622] If development error detection is enabled for NvM module,
     * the function NvM_WriteBlock shall report the DET error NVM_E_PARAM_ADDRESS
     * when no permanent RAM block and no explicit synchronization are configured,
     * and a NULL pointer is passed via the parameter NvM_SrcPtr
     */
    else if((NvMBlockDescriptor[BlockId].NvMRamBlockDataAddress == NULL) && (NvMBlockDescriptor[BlockId].NvMBlockUseSyncMechanism == FALSE) && (NvM_SrcPtr == NULL)){

        #if(NVM_DEV_ERROR_DETECT == STD_ON)
            Det_ReportError(NVRAM_MANAGER_ID, NVRAM_MANAGER_INSTANCE, NVM_WRITEBLOCK_API_ID, NVM_E_PARAM_ADDRESS) ;
        #endif

    }
    /* [SWS_NvM_00624]
     * If development error detection is enabled for NvM module,
     * the function NvM_WriteBlock shall report the DET error NVM_E_PARAM_BLOCK_ID
     * when the passed BlockID is out of range
     */
    else if((BlockId <= 1) || (BlockId >= NUMBER_OF_NVM_BLOCKS)){

        #if(NVM_DEV_ERROR_DETECT == STD_ON)
            Det_ReportError(NVRAM_MANAGER_ID, NVRAM_MANAGER_INSTANCE, NVM_WRITEBLOCK_API_ID, NVM_E_PARAM_BLOCK_ID) ;
        #endif

    }


    else{

        Job_Parameters WriteJob ;
        WriteJob.ServiceId = NVM_WRITEBLOCK_API_ID ;
        WriteJob.Block_Id = BlockId ;

        /* [SWS_NvM_00210]
         * If the function NvM_WriteBlock is provided with a valid RAM block address then it is used
         */
        if(NvM_SrcPtr != NULL){
            WriteJob.RAM_Ptr = (void*)NvM_SrcPtr ;
        }

        /*[SWS_NvM_00900]
         * If the function NvM_WriteBlock is provided with NULL_PTR as a RAM block address
         * and it has a permanent RAM block configured then the permanent RAM block is used.
         */
        else if(NvMBlockDescriptor[BlockId].NvMRamBlockDataAddress != NULL){
            WriteJob.RAM_Ptr = (uint32*)NvMBlockDescriptor[BlockId].NvMRamBlockDataAddress ;
        }

        /*[SWS_NvM_00901]
         * If the function NvM_WriteBlock is provided with NULL_PTR as a RAM block address
         * and it has the explicit synchronization configured then the explicit synchronization is used
         */
        else if (NvMBlockDescriptor[BlockId].NvMBlockUseSyncMechanism == TRUE){
            /*Use explicit sync*/
            /*Use RAM mirror pointer*/
        }

        Return_Val = Job_Enqueue(WriteJob) ;
    }

    return Return_Val;
}


/****************************************************************************************/
/*    Function Name           : NvM_MainFunction_WriteBlock                              */
/*    Function Description    : Local Service to to execute write jobs                  */
/*    Parameter in            : none                                                    */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : none                                                    */
/*    Requirement             : SWS_NvM_00455 & SWS_NvM_00464                           */
/*    Notes                   :                                                         */
/****************************************************************************************/

void NvM_MainFunction_WriteBlock(void)
{
    static uint8 Current_State = CALC_CRC;
    static uint32 CRC_Val = 0 ;
    static uint8 Retry_Counter = 0;
    static uint8 redundant_block_Num = 0;

    uint16 counter = 0 ;
    uint16 Fee_Ea_Block_Num ;

    switch (Current_State){

     /* Case CALC_CRC :
      * Initial state
      */
     case CALC_CRC :

         /* [SWS_NvM_00212]
          * The job of the function NvM_WriteBlock shall request a CRC recalculation
          * before the RAM block will be copied to NV memory if the NV block is configured with CRC
          */
         if(NvMBlockDescriptor[Current_Job.Block_Id].NvMBlockUseCrc == TRUE){

             /* if the RAM block is not permanent,
              * OR the RAM block is permanent and NvMCalcRamBlockCrc is true
              * So , Calculate CRC
              */
             /* ECUC_NvM_00119 */
             if((NvMBlockDescriptor[Current_Job.Block_Id].NvMRamBlockDataAddress != Current_Job.RAM_Ptr) ||
                (NvMBlockDescriptor[Current_Job.Block_Id].NvMCalcRamBlockCrc == TRUE))
             {

                /* CRC_Val = Calculate_CRC() ; */

                 /* [SWS_NvM_00852]
                  * The job of the function NvM_WriteBlock shall skip writing and consider the job as
                  * successfully finished if the NvMBlockUseCRCCompMechanism attribute of the NVRAM Block
                  * is set to true and the RAM block CRC calculated by the write job is equal to the CRC
                  * calculated during the last successful read or write job.
                  * This mechanism shall not be applied to blocks for which a loss of redundancy has been detected.
                  */
                if(NvMBlockDescriptor[Current_Job.Block_Id].NvMBlockUseCRCCompMechanism == TRUE){

                   if(CRC_Val == AdministrativeBlock[Current_Job.Block_Id].PrevCRCVal){

                        redundant_block_Num = 1 ;
                        Current_State = WRITE_OK ;
                        break ;

                    }
                }

             }

             if(CRC_Val != 0){

                for (counter = 0 ; counter < NvMBlockDescriptor[Current_Job.Block_Id].NvMNvBlockLength - 4 ; counter++){

                      TempBuffer[counter] = *((uint8 *)(Current_Job.RAM_Ptr)) ;
                      Current_Job.RAM_Ptr = (void *)((uint8 *)(Current_Job.RAM_Ptr) + 1) ;

                }

                /*Add CRC Value to the buffer*/
                TempBuffer[NvMBlockDescriptor[Current_Job.Block_Id].NvMNvBlockLength - 4] = *((uint8 *)&CRC_Val) ;
                TempBuffer[NvMBlockDescriptor[Current_Job.Block_Id].NvMNvBlockLength - 3] = *(((uint8 *)&CRC_Val) + 1) ;
                TempBuffer[NvMBlockDescriptor[Current_Job.Block_Id].NvMNvBlockLength - 2] = *(((uint8 *)&CRC_Val) + 2) ;
                TempBuffer[NvMBlockDescriptor[Current_Job.Block_Id].NvMNvBlockLength - 1] = *(((uint8 *)&CRC_Val) + 3) ;

             }
         }

     /* case WRITE_NV_BLOCK */
     case WRITE_NV_BLOCK :

         if(MemIf_GetStatus(NvMBlockDescriptor[Current_Job.Block_Id].NvMNvramDeviceId) == MEMIF_IDLE){


            /* [SWS_NvM_00303]
             * The job of the function NvM_WriteBlock shall assume a referenced permanent RAM block or the RAM mirror
             * in the NvM module in case of explicit synchronization to be valid when the request is passed to the NvM module.
             * If the permanent RAM block is still in an invalid state, the function NvM_WriteBlock shall validate it automatically
             * before copying the RAM block contents to NV memory or after calling explicit synchronization callback
             */
            if((NvMBlockDescriptor[Current_Job.Block_Id].NvMRamBlockDataAddress == Current_Job.RAM_Ptr) && (AdministrativeBlock[Current_Job.Block_Id].PRAMStatus != VALID_CHANGED)){

                AdministrativeBlock[Current_Job.Block_Id].PRAMStatus = VALID_CHANGED ;

            }

            /*Calculate FEE/EA Block Number to send to the MemIf Module*/
            /*Native Block*/
            if(NvMBlockDescriptor[Current_Job.Block_Id].NvMBlockManagement == NVM_BLOCK_NATIVE){

               Fee_Ea_Block_Num = NvMBlockDescriptor[Current_Job.Block_Id].NvMNvBlockBaseNumber << NVM_DATASET_SELECTION_BITS ;

            }

            /* [SWS_NvM_00338]
             * The job of the function NvM_WriteBlock shall copy the RAM block to the corresponding NV block
             * which is selected via the data index in the administrative block
             * if the NVRAM block management type of the given NVRAM block is NVM_BLOCK_DATASET.
             */
            /*DataSet Block*/
            else if(NvMBlockDescriptor[Current_Job.Block_Id].NvMBlockManagement == NVM_BLOCK_DATASET){

                Fee_Ea_Block_Num = (NvMBlockDescriptor[Current_Job.Block_Id].NvMNvBlockBaseNumber << NVM_DATASET_SELECTION_BITS) + AdministrativeBlock[Current_Job.Block_Id].DataSetIndex ;

            }
            /* [SWS_NvM_00760] The job of the function NvM_WriteBlock shall copy the data content of the RAM block
             * to both corresponding NV blocks if the NVRAM block management type of the processed NVRAM block
             * is NVM_BLOCK_REDUNDANT.
             */
            /*Redundant Block*/
            else if(NvMBlockDescriptor[Current_Job.Block_Id].NvMBlockManagement == NVM_BLOCK_REDUNDANT){
                Fee_Ea_Block_Num = (NvMBlockDescriptor[Current_Job.Block_Id].NvMNvBlockBaseNumber << NVM_DATASET_SELECTION_BITS) + redundant_block_Num ;
            }

            Std_ReturnType InitWrite ;

            /*Call MemIf_Write function*/
            if(CRC_Val != 0){

                InitWrite = MemIf_Write(NvMBlockDescriptor[Current_Job.Block_Id].NvMNvramDeviceId, Fee_Ea_Block_Num, TempBuffer) ;

            }
            else{

                InitWrite = MemIf_Write(NvMBlockDescriptor[Current_Job.Block_Id].NvMNvramDeviceId, Fee_Ea_Block_Num, (const uint8 *)Current_Job.RAM_Ptr) ;

            }


            AdministrativeBlock[Current_Job.Block_Id].BlockStatus = NVM_REQ_PENDING ;

            if(InitWrite == E_NOT_OK){
                /*Job Failed*/
                Current_State = WRITE_FAILED ;
                break ;
            }
         }

         /* If the job is done and result is OK */
         if(MemIf_GetJobResult(NvMBlockDescriptor[Current_Job.Block_Id].NvMNvramDeviceId) == MEMIF_JOB_OK){
             Current_State = WRITE_OK ;
         }
         /* If the job failed */
         else if (MemIf_GetJobResult(NvMBlockDescriptor[Current_Job.Block_Id].NvMNvramDeviceId) == MEMIF_JOB_FAILED){
             Current_State = WRITE_FAILED ;
         }

         break ;

     /*Case WRITE_OK :
      * If the job is done and result is OK
      */
     case WRITE_OK :

         /* [SWS_NvM_00284]
          * The job of the function NvM_WriteBlock shall set NVM_REQ_OK as request result
          * if the passed BlockId references a NVRAM block of type NVM_BLOCK_REDUNDANT
          * and at least one of the NV blocks has been written successfully.
          */
         if(NvMBlockDescriptor[Current_Job.Block_Id].NvMBlockManagement == NVM_BLOCK_REDUNDANT && redundant_block_Num == 0){

            redundant_block_Num = 1 ;
            AdministrativeBlock[Current_Job.Block_Id].BlockStatus = NVM_REQ_OK ;
            Retry_Counter = 0 ;
            /* return to the last state again to write the other NV block */
            Current_State = WRITE_NV_BLOCK ;

         }

         else{

             redundant_block_Num = 0;
             AdministrativeBlock[Current_Job.Block_Id].BlockStatus = NVM_REQ_OK ;
             AdministrativeBlock[Current_Job.Block_Id].PRAMStatus = VALID_UNCHANGED ;

             if(CRC_Val != 0){
                 AdministrativeBlock[Current_Job.Block_Id].PrevCRCVal = CRC_Val ;
                 CRC_Val = 0 ;
             }

             Current_State = WRITE_END ;

         }

         break ;

     /* Case WRITE_FAILED :
      * If the job failed
      */
     case WRITE_FAILED :

         /* [SWS_NvM_00213]
          * The job of the function NvM_WriteBlock shall check the number of write retries using a write retry counter
          * to avoid infinite loops. Each negative result reported by the memory interface shall be followed by
          * an increment of the retry counter. In case of a retry counter overrun,
          * the job of the function NvM_WriteBlock shall set the request result to NVM_REQ_NOT_OK.
          */
         Retry_Counter++ ;

         if(Retry_Counter >= NvMBlockDescriptor[Current_Job.Block_Id].NvMMaxNumOfWriteRetries){

             AdministrativeBlock[Current_Job.Block_Id].BlockStatus = NVM_REQ_NOT_OK ;
             AdministrativeBlock[Current_Job.Block_Id].PRAMStatus = INVALID_UNCHANGED ;

             /*report NVM_E_REQ_FAILED to the DEM.*/

             Retry_Counter = 0 ;
             redundant_block_Num = 0 ;
             Current_State = WRITE_END ;
         }
         else {
             Current_State = WRITE_NV_BLOCK ;
         }

         break ;

     /* Case WRITE_END :
      * the write job has been finished
      */
     case WRITE_END :

         CRC_Val = 0 ;
         Retry_Counter = 0;
         redundant_block_Num = 0;

         Job_Dequeue() ;
         Get_SingleJob( &Current_Job ) ;

         Current_State = CALC_CRC ;
         break ;
    }
}

/****************************************************************************************/
/*    Function Name           : NvM_InvalidateNvBlock                                   */
/*    Function Description    : Service to invalidate a NV block.                       */
/*    Parameter in            : BlockId                                                 */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : Std_ReturnType                                          */
/*    Requirement             : SWS_NvM_00459                                          */
/*    Notes                   :                                                         */
/****************************************************************************************/

Std_ReturnType NvM_InvalidateNvBlock( NvM_BlockIdType BlockId )
{
    /*Variable to save return value*/
    Std_ReturnType Return_Val = E_OK ;

    /* [SWS_NvM_00638]
     * If development error detection is enabled for NvM module,
     * the function NvM_InvalidateNvBlock shall report the DET error
     * NVM_E_NOT_INITIALIZED when NVM is not yet initialized.
     */
    if(ModuleState != INIT_DONE){

       #if(NVM_DEV_ERROR_DETECT == STD_ON)
           Det_ReportError(NVRAM_MANAGER_ID, NVRAM_MANAGER_INSTANCE, NVM_INVALIDATEBLOCK_API_ID, NVM_E_NOT_INITIALIZED) ;
       #endif
       Return_Val = E_NOT_OK ;

    }

    /* [SWS_NvM_00639]
     * If development error detection is enabled for NvM module,
     * the function NvM_InvalidateNvBlock shall report the DET error
     * NVM_E_BLOCK_PENDING when NVRAM block identifier is already
     * queued or currently in progress.
     */
    else if(Search_Queue(BlockId) == E_OK){

       #if(NVM_DEV_ERROR_DETECT == STD_ON)
           Det_ReportError(NVRAM_MANAGER_ID, NVRAM_MANAGER_INSTANCE, NVM_INVALIDATEBLOCK_API_ID, NVM_E_BLOCK_PENDING) ;
       #endif
       Return_Val = E_NOT_OK ;
    }

    /* [SWS_NvM_00642]
     * If development error detection is enabled for NvM module,
     * the function NvM_InvalidateNvBlock shall report the DET error
     * NVM_E_PARAM_BLOCK_ID when the passed BlockID is out of range.
     */
    else if((BlockId <= 1) || (BlockId >= NUMBER_OF_NVM_BLOCKS)){

       #if(NVM_DEV_ERROR_DETECT == STD_ON)
           Det_ReportError(NVRAM_MANAGER_ID, NVRAM_MANAGER_INSTANCE, NVM_INVALIDATEBLOCK_API_ID, NVM_E_PARAM_BLOCK_ID) ;
       #endif
       Return_Val = E_NOT_OK ;
    }
    /* [SWS_NvM_00664]
     * The function NvM_InvalidateNvBlock shall return with E_NOT_OK if a ROM block
     * of a dataset NVRAM block is referenced by the BlockId parameter.
     */
    else if((NvMBlockDescriptor[BlockId].NvMBlockManagement == NVM_BLOCK_DATASET) &&
            (AdministrativeBlock[BlockId].DataSetIndex >= NvMBlockDescriptor[BlockId].NvMNvBlockNum))
    {
        Return_Val = E_NOT_OK ;
    }

    /* [SWS_NvM_00239]
     * The function NvM_InvalidateNvBlock shall take over
     * the given parameters, queue the request and return.
     */
    else{

       Job_Parameters InvalidateJob = {NVM_INVALIDATEBLOCK_API_ID, BlockId} ;
       Return_Val = Job_Enqueue(InvalidateJob) ;

    }

    return Return_Val ;
}

/****************************************************************************************/
/*    Function Name           : NvM_MainFunction_InvalidateBlock                        */
/*    Function Description    : Asynchronous processing of InvalidateBlock job          */
/*    Parameter in            : BlockId                                                 */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : none                                                    */
/*    Requirement             : SWS_NvM_00459                                           */
/*    Notes                   :                                                         */
/****************************************************************************************/
void NvM_MainFunction_InvalidateBlock(void)
{
    static uint8 Current_State = INVALIDATE_NV_BLOCK;
    static uint8 redundant_block_Num = 0;
    uint16 Fee_Ea_Block_Num ;
    Std_ReturnType InitInvalidate ;

    switch(Current_State){

      /*case INVALIDATE_NV_BLOCK : Initial state*/
      case INVALIDATE_NV_BLOCK :

          if(MemIf_GetStatus(NvMBlockDescriptor[Current_Job.Block_Id].NvMNvramDeviceId) == MEMIF_IDLE){


              /*Calculate FEE/EA Block Number to send to the MemIf Module*/
              /*Native Block*/
              if(NvMBlockDescriptor[Current_Job.Block_Id].NvMBlockManagement == NVM_BLOCK_NATIVE){

                 Fee_Ea_Block_Num = NvMBlockDescriptor[Current_Job.Block_Id].NvMNvBlockBaseNumber << NVM_DATASET_SELECTION_BITS ;

              }

              /*DataSet Block*/
              else if(NvMBlockDescriptor[Current_Job.Block_Id].NvMBlockManagement == NVM_BLOCK_DATASET){

                  Fee_Ea_Block_Num = (NvMBlockDescriptor[Current_Job.Block_Id].NvMNvBlockBaseNumber << NVM_DATASET_SELECTION_BITS) + AdministrativeBlock[Current_Job.Block_Id].DataSetIndex ;

              }

              /*Redundant Block*/
              else if(NvMBlockDescriptor[Current_Job.Block_Id].NvMBlockManagement == NVM_BLOCK_REDUNDANT){
                  Fee_Ea_Block_Num = (NvMBlockDescriptor[Current_Job.Block_Id].NvMNvBlockBaseNumber << NVM_DATASET_SELECTION_BITS) + redundant_block_Num ;
              }

              InitInvalidate = MemIf_InvalidateBlock(NvMBlockDescriptor[Current_Job.Block_Id].NvMNvramDeviceId, Fee_Ea_Block_Num) ;

              if(InitInvalidate == E_NOT_OK){

                  Current_State = INVALIDATE_FAILED ;
                  break ;

              }
              else {

                  /* If the job is done and result is OK */
                  if(MemIf_GetJobResult(NvMBlockDescriptor[Current_Job.Block_Id].NvMNvramDeviceId) == MEMIF_JOB_OK){
                      Current_State = INVALIDATE_OK ;
                  }
                  /* If the job failed */
                  else if (MemIf_GetJobResult(NvMBlockDescriptor[Current_Job.Block_Id].NvMNvramDeviceId) == MEMIF_JOB_FAILED){
                           Current_State = INVALIDATE_FAILED ;
                  }

              }
          }

          break ;

      /* case INVALIDATE_OK : when the invalidation done successfully */
      case INVALIDATE_OK :

          /* [SWS_NvM_00274]
           * If the referenced NVRAM block is of type NVM_BLOCK_REDUNDANT,
           * the function NvM_InvalidateNvBlock shall only set
           * the request result NvM_RequestResultType to NVM_REQ_OK
           * when both NV blocks have been invalidated.
           */
          if(NvMBlockDescriptor[Current_Job.Block_Id].NvMBlockManagement == NVM_BLOCK_REDUNDANT && redundant_block_Num == 0){

              redundant_block_Num = 1 ;
              /* return to the last state again to invalidate the other NV block */
              Current_State = INVALIDATE_NV_BLOCK ;

          }

          else {

              AdministrativeBlock[Current_Job.Block_Id].BlockStatus = NVM_REQ_OK ;
              AdministrativeBlock[Current_Job.Block_Id].PRAMStatus = INVALID_UNCHANGED ;

              Current_State = INVALIDATE_END ;

          }

          break ;

      /* case INVALIDATE_FAILED : when the invalidation failed */
      case INVALIDATE_FAILED :

          /* [SWS_NvM_00275]
           * The function NvM_InvalidateNvBlock shall set the request result to
           * NVM_REQ_NOT_OK if the processing of this service fails
           */
          AdministrativeBlock[Current_Job.Block_Id].BlockStatus = NVM_REQ_NOT_OK ;

          /* [SWS_NvM_00666]
           * The function NvM_InvalidateNvBlock shall report NVM_E_REQ_FAILED
           * to the DEM if the processing of this service fails.
           */
          /*report NVM_E_REQ_FAILED to the DEM.*/

          Current_State = INVALIDATE_END ;

          break ;

      /* case INVALIDATE_END : when the invalidation ends, dequeue the job from the queue */
      case INVALIDATE_END :

           redundant_block_Num = 0 ;
           Job_Dequeue() ;
           Get_SingleJob( &Current_Job ) ;

           Current_State = INVALIDATE_NV_BLOCK ;
           break ;

    }

}


/****************************************************************************************/
/*    Function Name           : NvM_WriteAll                                            */
/*    Function Description    : Initiates a multi block write request.                  */
/*    Parameter in            : none                                                    */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : none                                                    */
/*    Requirement             : SWS_NvM_00461                                           */
/*    Notes                   :                                                         */
/****************************************************************************************/

void NvM_WriteAll( void )
{

    /*[SWS_NvM_00647]
     * If development error detection is enabled for NvM module,
     * the function NvM_WriteAll shall report the DET error
     * NVM_E_NOT_INITIALIZED when NVM is not yet initialized.
     */
    if(ModuleState != INIT_DONE){

       #if(NVM_DEV_ERROR_DETECT == STD_ON)
           Det_ReportError(NVRAM_MANAGER_ID, NVRAM_MANAGER_INSTANCE, NVM_WRITE_ALL_API_ID, NVM_E_NOT_INITIALIZED) ;
       #endif

    }

    /* [SWS_NvM_00254]
     * The function NvM_WriteAll shall signal the request to the NvM module and return.
     * The NVRAM Manager shall defer the processing of the requested WriteAll until
     * all single block job queues are empty.
     */
    if(MultiBlcokRequest.ResultStatus != NVM_REQ_PENDING){

        MultiBlcokRequest.request = NVM_WRITE_ALL_API_ID ;
        MultiBlcokRequest.ResultStatus = NVM_REQ_PENDING ;
        MultiBlcokRequest.Internal_state = CALC_CRC ;

    }

}

/****************************************************************************************/
/*    Function Name           : NvM_MainFunction_WriteAll                               */
/*    Function Description    : Asynchronous processing of Write All job                */
/*    Parameter in            : none                                                    */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : none                                                    */
/*    Requirement             : SWS_NvM_00461                                           */
/*    Notes                   :                                                         */
/****************************************************************************************/

void NvM_MainFunction_WriteAll( void )
{
    static NvM_BlockIdType IdCounter = 2 ;
    static uint8 redundant_block_Num = 0 ;
    static uint8 Retry_Counter ;
    static boolean FailedJob = FALSE ;
    uint32 CrcCounter = 0 ;
    uint32 Fee_Ea_Block_Num ;

    switch(MultiBlcokRequest.Internal_state){

      case WRITE_NV_BLOCK :

          /*[SWS_NvM_00252]
           * The job of the function NvM_WriteAll shall process only the permanent RAM blocks
           * or call explicit synchronization callback (NvM_WriteRamBlockToNvm) for all blocks
           * for which the corresponding NVRAM block parameter NvMSelectBlockForWriteAll is configured to true.
           */
          /*[SWS_NvM_00682]
           * The job of the function NvM_WriteAll shall check the
           * “valid/modified” state for each RAM block in advance.
           */
          if(NvMBlockDescriptor[IdCounter].NvMSelectBlockForWriteAll == TRUE && AdministrativeBlock[IdCounter].PRAMStatus == VALID_CHANGED){

              /* [SWS_NvM_00549]
               * The job of the function NvM_ WriteAll shall set each proceeding block specific
               * request result for NVRAM blocks and the multi block request
               * result to NVM_REQ_PENDING in advance.
               */
              AdministrativeBlock[IdCounter].BlockStatus = NVM_REQ_PENDING ;

              if(MemIf_GetStatus(NvMBlockDescriptor[IdCounter].NvMNvramDeviceId) == MEMIF_IDLE){

                  uint8* PRamPtr = (uint8 *)NvMBlockDescriptor[IdCounter].NvMRamBlockDataAddress ;

                  /*Calculate FEE/EA Block Number to send to the MemIf Module*/
                  /*if Native Block*/
                  if(NvMBlockDescriptor[IdCounter].NvMBlockManagement == NVM_BLOCK_NATIVE){

                     Fee_Ea_Block_Num = NvMBlockDescriptor[IdCounter].NvMNvBlockBaseNumber << NVM_DATASET_SELECTION_BITS ;

                  }

                  /*DataSet Block*/
                  /*[SWS_NvM_00339]
                   * In case of NVRAM block management type NVM_BLOCK_DATASET,
                   * the job of the function NvM_WriteAll shall copy only the RAM block to the corresponding
                   * NV block which is selected via the data index in the administrative block.
                   */
                  else if(NvMBlockDescriptor[IdCounter].NvMBlockManagement == NVM_BLOCK_DATASET){

                      Fee_Ea_Block_Num = (NvMBlockDescriptor[IdCounter].NvMNvBlockBaseNumber << NVM_DATASET_SELECTION_BITS) + AdministrativeBlock[IdCounter].DataSetIndex ;

                  }

                  /*Redundant Block*/
                  else if(NvMBlockDescriptor[IdCounter].NvMBlockManagement == NVM_BLOCK_REDUNDANT){
                      Fee_Ea_Block_Num = (NvMBlockDescriptor[IdCounter].NvMNvBlockBaseNumber << NVM_DATASET_SELECTION_BITS) + redundant_block_Num ;
                  }

                  Std_ReturnType InitWrite ;

                  if(NvMBlockDescriptor[IdCounter].NvMBlockUseCrc == TRUE && NvMBlockDescriptor[IdCounter].NvMCalcRamBlockCrc == TRUE){

                     for (CrcCounter = 0 ; CrcCounter < NvMBlockDescriptor[IdCounter].NvMNvBlockLength - 4 ; CrcCounter++){
                          TempBuffer[CrcCounter] = *(PRamPtr) ;
                          PRamPtr += 1 ;
                     }

                     /*Add CRC Value to the buffer*/
                     TempBuffer[NvMBlockDescriptor[IdCounter].NvMNvBlockLength - 4] = *((uint8 *)(&AdministrativeBlock[IdCounter].PrevCRCVal)) ;
                     TempBuffer[NvMBlockDescriptor[IdCounter].NvMNvBlockLength - 3] = *(((uint8 *)(&AdministrativeBlock[IdCounter].PrevCRCVal)) + 1) ;
                     TempBuffer[NvMBlockDescriptor[IdCounter].NvMNvBlockLength - 2] = *(((uint8 *)(&AdministrativeBlock[IdCounter].PrevCRCVal)) + 2) ;
                     TempBuffer[NvMBlockDescriptor[IdCounter].NvMNvBlockLength - 1] = *(((uint8 *)(&AdministrativeBlock[IdCounter].PrevCRCVal)) + 3) ;

                     InitWrite = MemIf_Write(NvMBlockDescriptor[IdCounter].NvMNvramDeviceId, Fee_Ea_Block_Num, TempBuffer) ;
                  }
                  else {

                      InitWrite = MemIf_Write(NvMBlockDescriptor[IdCounter].NvMNvramDeviceId, Fee_Ea_Block_Num, (const uint8 *)PRamPtr) ;
                  }

                  /* [SWS_NvM_00549]
                   * The job of the function NvM_ WriteAll shall set each proceeding block
                   * specific request result for NVRAM blocks and the multi block request
                   * result to NVM_REQ_PENDING in advance.
                   */
                  AdministrativeBlock[IdCounter].BlockStatus = NVM_REQ_PENDING ;

                  if(InitWrite == E_NOT_OK){
                     /*Job Failed*/
                     MultiBlcokRequest.Internal_state = WRITE_FAILED ;
                     break ;
                  }

                  /* If the job is done and result is OK */
                  if(MemIf_GetJobResult(NvMBlockDescriptor[IdCounter].NvMNvramDeviceId) == MEMIF_JOB_OK){
                      MultiBlcokRequest.Internal_state = WRITE_OK ;
                  }

                  /* If the job failed */
                  else if (MemIf_GetJobResult(NvMBlockDescriptor[IdCounter].NvMNvramDeviceId) == MEMIF_JOB_FAILED){
                      MultiBlcokRequest.Internal_state = WRITE_FAILED ;
                  }

                  break ;

             }

          }
          else {

              /* [SWS_NvM_00298]
               * The job of the function NvM_WriteAll shall set the request result to
               * NVM_REQ_BLOCK_SKIPPED for each NVRAM block configured to be processed
               * by the job of the function NvM_WriteAll (NvMSelectBlockForWriteAll is checked)
               * and which has not been written during processing of the NvM_WriteAll job.
               */
              if(NvMBlockDescriptor[IdCounter].NvMSelectBlockForWriteAll)
              {
                  AdministrativeBlock[IdCounter].BlockStatus = NVM_REQ_BLOCK_SKIPPED ;
              }
              MultiBlcokRequest.Internal_state = WRITE_END ;
          }

      /*Case WRITE_OK :
       * If a single block write job is done and result is OK
       */
      case WRITE_OK :

          /*[SWS_NvM_00337]
           * The job of the function NvM_WriteAll shall set the single block request result to
           * NVM_REQ_OK if the processed NVRAM block is of type NVM_BLOCK_REDUNDANT and at least
           * one of the NV blocks has been written successfully.
           */
          if(NvMBlockDescriptor[IdCounter].NvMBlockManagement == NVM_BLOCK_REDUNDANT && redundant_block_Num == 0){

             redundant_block_Num = 1 ;
             AdministrativeBlock[IdCounter].BlockStatus = NVM_REQ_OK ;
             Retry_Counter = 0 ;

             /* [SWS_NvM_00762]
              * The job of the function NvM_WriteAll shall copy the data content of the RAM block to
              * both corresponding NV blocks if the NVRAM block management type of the processed NVRAM
              * block is NVM_BLOCK_REDUNDANT.
              */
             /* return to WRITE_NV_BLOCK state again to write the other NV block */
             MultiBlcokRequest.Internal_state = WRITE_NV_BLOCK ;

          }
          else {

              redundant_block_Num = 0;
              AdministrativeBlock[IdCounter].BlockStatus = NVM_REQ_OK ;
              AdministrativeBlock[IdCounter].PRAMStatus = VALID_UNCHANGED ;

              MultiBlcokRequest.Internal_state = WRITE_END ;
          }

          break ;

      /* Case WRITE_FAILED :
       * If a single block write job failed
       */
      case WRITE_FAILED :

          /*[SWS_NvM_00296]
           * The job of the function NvM_WriteAll shall check the number of write retries
           * by a write retry counter to avoid infinite loops. Each unsuccessful result
           * reported by the MemIf module shall be followed by an increment of the retry counter.
           */
          Retry_Counter++ ;

          /*[SWS_NvM_00683]
           * The job of the function NvM_WriteAll shall set the block specific request
           * result to NVM_REQ_NOT_OK if the write retry counter becomes greater than the
           * configured NVM_MAX_NUM_OF_WRITE_RETRIES.
           */
          if(Retry_Counter >= NvMBlockDescriptor[IdCounter].NvMMaxNumOfWriteRetries){

             AdministrativeBlock[IdCounter].BlockStatus = NVM_REQ_NOT_OK ;
             AdministrativeBlock[IdCounter].PRAMStatus = INVALID_UNCHANGED ;

             /*report error to the DEM.*/

             FailedJob = TRUE ;
             Retry_Counter = 0 ;
             redundant_block_Num = 0 ;
             MultiBlcokRequest.Internal_state = WRITE_END ;
          }
          else {

             MultiBlcokRequest.Internal_state = WRITE_NV_BLOCK ;
          }

          break ;

      /* Case WRITE_END :
       * If a single block write job ended
       */
      case WRITE_END :

          redundant_block_Num = 0 ;
          Retry_Counter = 0 ;
          CrcCounter = 0 ;
          Fee_Ea_Block_Num = 0 ;
          IdCounter++ ;

          /*if the blocks IDs finishes*/
          if(IdCounter >= NUMBER_OF_NVM_BLOCKS){

              /* [SWS_NvM_00318]
               * The job of the function NvM_WriteAll shall set the multi block request result
               * to NVM_REQ_NOT_OK if processing of one or even more NVRAM blocks fails.
               */
              if(FailedJob == TRUE){

                  MultiBlcokRequest.ResultStatus = NVM_REQ_NOT_OK ;
                  FailedJob = FALSE ;
              }

              /*[SWS_NvM_00896]
               * The job of the function NvM_WriteAll shall set the multi block request result
               * to NVM_REQ_OK if no job fails with the processing of the NVRAM blocks.
               */
              else {

                  MultiBlcokRequest.ResultStatus = NVM_REQ_OK ;
              }
          }
          else {

              MultiBlcokRequest.Internal_state = WRITE_NV_BLOCK ;
          }

          break ;

    }

}

/****************************************************************************************/
/*    Function Name           : NvM_MainFunction                                        */
/*    Function Description    : Service for performing the processing of the NvM jobs.  */
/*    Parameter in            : none                                                    */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : none                                                    */
/*    Requirement             : SWS_NvM_00464                                           */
/*    Notes                   :                                                         */
/****************************************************************************************/
void NvM_MainFunction( void )
{
    static uint8 Current_State = GET_JOB ;
    NvM_BlockIdType CRCBlockId ;

    /* [SWS_NvM_00257]
     * The NvM module shall only do/start job processing, queue management and CRC
     * recalculation if the NvM_Init function has internally set an “INIT DONE” signal
     */
    if(ModuleState != INIT_DONE)
    {
        return ;
    }

    switch(Current_State)
    {
      case GET_JOB :

          /* First get a single block job request if existed */
          Get_SingleJob( &Current_Job ) ;

          if(Current_Job.ServiceId != NVM_INIT_API_ID)
          {
              Current_State = EXECUTE_SINGLE_JOB ;
          }
          /* Second get a CRC request if existed */
          else if(CRC_Queue_Empty == FALSE)
          {
              Current_State = EXECUTE_CRC_JOB ;
          }
          /* Third get a multi block request if existed */
          else if(MultiBlcokRequest.ResultStatus == NVM_REQ_PENDING)
          {
              Current_State = EXECUTE_MULTI_JOB ;
          }
          break ;

      case EXECUTE_SINGLE_JOB :

          switch(Current_Job.ServiceId)
          {
            case NVM_READBLOCK_API_ID :

                NvM_MainFunction_ReadBlock() ;
                break ;

            case NVM_WRITEBLOCK_API_ID :

                NvM_MainFunction_WriteBlock() ;
                break ;

            case NVM_INVALIDATEBLOCK_API_ID :

                NvM_MainFunction_InvalidateBlock() ;
                break ;
            case NVM_INIT_API_ID :

                Current_State = GET_JOB ;
                break ;
          }
          break ;

      case EXECUTE_CRC_JOB :

          CRCBlockId = CRC_Job_Queue[CRC_Queue_Indeces.Head] ;
          /* AdministrativeBlock[CRCBlockId].PrevCRCVal = Calculate_CRC() ; */
          CRCJob_Dequeue() ;
          Current_State = GET_JOB ;
          break ;

      case EXECUTE_MULTI_JOB :

          switch(MultiBlcokRequest.request)
          {
             case NVM_READ_ALL_API_ID :

                 /* NvM_MainFunction_ReadAll() ; */
                 break ;

             case NVM_WRITE_ALL_API_ID :

                 NvM_MainFunction_WriteAll() ;
                 break ;
          }
          if((MultiBlcokRequest.ResultStatus == NVM_REQ_OK) || (MultiBlcokRequest.ResultStatus == NVM_REQ_NOT_OK))
          {
              Current_State = GET_JOB ;
          }
          break ;
    }
}








