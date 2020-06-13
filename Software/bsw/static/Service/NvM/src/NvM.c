/*
 * NvM.c
 *
 *
 *      Author: Sahar
 */


/*****************************************************************************************/
/*                                   Include headers                                     */
/*****************************************************************************************/

#include "NvM.h"
#include "Det.h"
#include "NvM_Shared.h"


/*****************************************************************************************/
/*                                   extern variables                                    */
/*****************************************************************************************/

/*Blocks configurations */
extern NvMBlockDescriptorType NvMBlockDescriptor[NUMBER_OF_NVM_BLOCKS] ;
static uint16 BlockNumber ;
static MemIf_JobResultType MemIf_JobResult ;

#if(NVM_POLLING_MODE == STD_OFF)
/*save end job status from underlying layer Successful or failed*/
static EndJobStatusType EndJobStatus ;
#endif

/*****************************************************************************************/
/*                                   Local Functions Prototypes                          */
/*****************************************************************************************/

static void NvM_MainFunction_ReadBlock(void) ;

/*****************************************************************************************/
/*                                   Global Function Definition                          */
/*****************************************************************************************/


/****************************************************************************************/
/*    Function Name           : NvM_Init                                                */
/*    Function Description    : Service for resetting all internal variables.           */
/*    Parameter in            : none                                                    */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : none                                                    */
/*    Requirement             : SWS_NvM_00447                                           */
/*    Notes                   :[SWS_NvM_00881]The Configuration pointer ConfigPtr shall */
/*                             always have a NULL_PTR value.                            */
/****************************************************************************************/

void NvM_Init(const NvM_ConfigType* ConfigPtr )
{
    /*counter to loop blocks*/
    uint32 counter = 0 ;

    /*[SWS_NvM_00399] The function NvM_Init shall reset all internal variables,
     * e.g. the queues, request flags, state machines, to their initial values.
     * It shall signal “INIT DONE” internally, e.g. to enable job processing
     * and queue management. (SRS_BSW_00101, SRS_BSW_00406)
     */

    /*Initialize Queue*/
    Init_Queue() ;

    /*[SWS_NvM_00192]The function NvM_Init shall set the data set index
     * of all NVRAM blocks of type NVM_BLOCK_DATASET to zero.
     */
    for(counter = 0 ; counter < NUMBER_OF_NVM_BLOCKS ; counter++)
    {
        /*check if block type is data set*/
        if(NvMBlockDescriptor[counter].NvMBlockManagement == NVM_BLOCK_DATASET )
        {
            AdministrativeBlock[counter].DataSetIndex = 0 ;
        }

        /*Initialize block status in Administrative block*/
        AdministrativeBlock[counter].BlockStatus = NVM_REQ_OK ;

        /*After the Initialization the RAM Block is in state INVALID/UNCHANGED until it is
          updated via NvM_ReadAll,
         */
        AdministrativeBlock[counter].PRAMStatus        = INVALID_UNCHANGED ;
    }

    /*Initialize End job status*/
    #if(NVM_POLLING_MODE == STD_OFF)
     EndJobStatus.EndJobFailed  = 0 ;
     EndJobStatus.EndJobSuccess = 0;
    #endif

    /*signal “INIT DONE” internally*/
    ModuleState = INIT_DONE ;
}


/****************************************************************************************/
/*    Function Name           : NvM_SetDataIndex                                        */
/*    Function Description    : Service for resetting all internal variables.           */
/*    Parameter in            : BlockId , DataIndex                                     */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : none                                                    */
/*    Requirement             : SWS_NvM_00448                                           */
/*    Notes                   : none                                                    */
/****************************************************************************************/
Std_ReturnType NvM_SetDataIndex(NvM_BlockIdType BlockId, uint8 DataIndex )
{
    /*Variable to save return value*/
    Std_ReturnType rtn_val = E_OK ;

    /*Check if Module not internally initialized
     *[SWS_NvM_00707] The NvM module’s environment shall have initialized
     *the NvM module before it calls the function NvM_SetDataIndex.
     */
    if(ModuleState != INIT_DONE)
    {
        #if(NVM_DEV_ERROR_DETECT == STD_ON)
            Det_ReportError(NVRAM_MANAGER_ID, NVRAM_MANAGER_INSTANCE, NVM_SET_DATAINDEX_API_ID ,NVM_E_NOT_INITIALIZED) ;
        #endif
        rtn_val = E_NOT_OK ;
    }
    /*check if Block ID is not in range*/
    else if( (BlockId <=1 ) || (BlockId >= NUMBER_OF_NVM_BLOCKS) )
    {
        #if(NVM_DEV_ERROR_DETECT == STD_ON)
            Det_ReportError(NVRAM_MANAGER_ID, NVRAM_MANAGER_INSTANCE, NVM_SET_DATAINDEX_API_ID , NVM_E_PARAM_BLOCK_ID ) ;
        #endif
        rtn_val = E_NOT_OK ;
    }
    /*[SWS_NvM_00599] If development error detection is enabled for NvM module,
     * the function NvM_SetDataIndex shall report the DET error NVM_E_PARAM_BLOCK_DATA_IDX
     * when DataIndex parameter exceeds the total number of configured data sets
     */
    else if(DataIndex >= (1<<NVM_DATASET_SELECTION_BITS) )
    {
        #if(NVM_DEV_ERROR_DETECT == STD_ON)
            Det_ReportError(NVRAM_MANAGER_ID, NVRAM_MANAGER_INSTANCE, NVM_SET_DATAINDEX_API_ID , NVM_E_PARAM_BLOCK_DATA_IDX ) ;
        #endif
        rtn_val = E_NOT_OK ;
    }
    /*Check if block management type not data set
     *[SWS_NvM_00264] For blocks with block management different from NVM_BLOCK_DATASET,
     *NvM_SetDataIndex shall return without any effect in production mode.
     *Further, E_NOT_OK shall be returned
     */
    else if(NvMBlockDescriptor[BlockId].NvMBlockManagement != NVM_BLOCK_DATASET )
    {
        rtn_val = E_NOT_OK ;
    }
    /*
     [SWS_NvM_00598] If development error detection is enabled for NvM module,
     the function NvM_SetDataIndex shall report the DET error NVM_E_BLOCK_PENDING
     when NVRAM block identifier is already queued or currently in progress
     */
    else if(Search_Queue(BlockId) == E_OK)
    {
        #if(NVM_DEV_ERROR_DETECT == STD_ON)
            Det_ReportError(NVRAM_MANAGER_ID, NVRAM_MANAGER_INSTANCE, NVM_SET_DATAINDEX_API_ID ,NVM_E_BLOCK_PENDING) ;
        #endif
        rtn_val = E_NOT_OK ;
    }
    else
    {
        /*Save data index in the administrative block*/
        AdministrativeBlock[BlockId].DataSetIndex = DataIndex ;
    }

    return rtn_val ;
}


/****************************************************************************************/
/*    Function Name           : NvM_GetDataIndex                                        */
/*    Function Description    : Service for getting the currently set DataIndex of      */
/*                              a data set NVRAM block                                  */
/*    Parameter in            : BlockId                                                 */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : DataIndexPtr                                            */
/*    Return value            : none                                                    */
/*    Requirement             : SWS_NvM_00449                                           */
/*    Notes                   : none                                                    */
/****************************************************************************************/
Std_ReturnType NvM_GetDataIndex(NvM_BlockIdType BlockId, uint8* DataIndexPtr )
{
    /*Variable to save return value*/
    Std_ReturnType rtn_val = E_OK ;

    /*Check if Module not internally initialized */
    if(ModuleState != INIT_DONE)
    {
        #if(NVM_DEV_ERROR_DETECT == STD_ON)
            Det_ReportError(NVRAM_MANAGER_ID, NVRAM_MANAGER_INSTANCE, NVM_GET_DATAINDEX_API_ID ,NVM_E_NOT_INITIALIZED) ;
        #endif
        rtn_val = E_NOT_OK ;
    }
    /*[SWS_NvM_00605] If development error detection is enabled for NvM module,
     * the function NvM_GetDataIndex shall report the DET error NVM_E_PARAM_DATA
     * when a NULL pointer is passed via the parameter DataIndexPtr.
     */
    else if(DataIndexPtr == NULL_PTR)
    {
        #if(NVM_DEV_ERROR_DETECT == STD_ON)
            Det_ReportError(NVRAM_MANAGER_ID, NVRAM_MANAGER_INSTANCE, NVM_GET_DATAINDEX_API_ID ,NVM_E_PARAM_DATA) ;
        #endif
        rtn_val = E_NOT_OK ;
    }
    /*check if Block ID is not in range*/
    else if( (BlockId <=1 ) || (BlockId >= NUMBER_OF_NVM_BLOCKS) )
    {
        #if(NVM_DEV_ERROR_DETECT == STD_ON)
            Det_ReportError(NVRAM_MANAGER_ID, NVRAM_MANAGER_INSTANCE, NVM_GET_DATAINDEX_API_ID , NVM_E_PARAM_BLOCK_ID ) ;
        #endif
        rtn_val = E_NOT_OK ;
    }
    /* [SWS_NvM_00265] For blocks with block management different from NVM_BLOCK_DATASET,
     * NvM_GetDataIndex shall set the index pointed by DataIndexPtr to zero. Further,
     * E_NOT_OK shall be returned.
     */
    else if(NvMBlockDescriptor[BlockId].NvMBlockManagement != NVM_BLOCK_DATASET )
    {
        rtn_val = E_NOT_OK ;
    }
    else
    {
        /*Save data index in the administrative block*/
         *DataIndexPtr = AdministrativeBlock[BlockId].DataSetIndex ;
    }
    return rtn_val ;
}

/****************************************************************************************/
/*    Function Name           : NvM_GetErrorStatus                                      */
/*    Function Description    : read the block dependent error/status information.      */
/*    Parameter in            : BlockId                                                 */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : RequestResultPtr                                        */
/*    Return value            : none                                                    */
/*    Requirement             : SWS_NvM_00451                                           */
/*    Notes                   : none                                                    */
/****************************************************************************************/

Std_ReturnType NvM_GetErrorStatus( NvM_BlockIdType BlockId, NvM_RequestResultType* RequestResultPtr)
{
    /*Variable to save return value*/
    Std_ReturnType rtn_val = E_OK ;

    /*
     * [SWS_NvM_00710] The NvM module’s environment shall have initialized the NvM module before
     * it calls the function NvM_GetErrorStatus.
     */
    if(ModuleState != INIT_DONE)
    {
        #if(NVM_DEV_ERROR_DETECT == STD_ON)
            Det_ReportError(NVRAM_MANAGER_ID, NVRAM_MANAGER_INSTANCE, NVM_GET_ERROR_STATUS_API_ID ,NVM_E_NOT_INITIALIZED) ;
        #endif
        rtn_val = E_NOT_OK ;
    }
    /*check if the input parameter pointer is  NULL*/
    else if(RequestResultPtr == NULL_PTR)
    {
        #if(NVM_DEV_ERROR_DETECT == STD_ON)
            Det_ReportError(NVRAM_MANAGER_ID, NVRAM_MANAGER_INSTANCE, NVM_GET_ERROR_STATUS_API_ID ,NVM_E_PARAM_POINTER) ;
        #endif
        rtn_val = E_NOT_OK ;
    }
    else
    {
        /*[SWS_NvM_00015] The function NvM_GetErrorStatus shall read the block dependent error/status information
          in the administrative part of a NVRAM block.
          The status/error information of a NVRAM block shall be set by a former or current asynchronous
          request.  (SRS_Mem_00020)
          */
        *RequestResultPtr = AdministrativeBlock[BlockId].BlockStatus ;
    }
    return rtn_val ;
}


/****************************************************************************************/
/*    Function Name           : NvM_SetRamBlockStatus                                   */
/*    Function Description    : Service for setting the RAM block status of a permanent */
/*                              RAM block                                               */
/*    Parameter in            : BlockId , BlockChanged                                  */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : Std_ReturnType                                          */
/*    Requirement             : SWS_NvM_00453                                           */
/*    Notes                   : none                                                    */
/****************************************************************************************/
Std_ReturnType NvM_SetRamBlockStatus( NvM_BlockIdType BlockId, boolean BlockChanged )
{
    Std_ReturnType rtn_val =E_OK ;

    /*[SWS_NvM_00643] If development error detection is enabled for NvM module,
     * the function NvM_SetRamBlockStatus shall report the DET error NVM_E_NOT_INITIALIZED
     * when NVM not yet initialized.
     */
    if(ModuleState != INIT_DONE)
    {
        #if(NVM_DEV_ERROR_DETECT == STD_ON)
            Det_ReportError(NVRAM_MANAGER_ID, NVRAM_MANAGER_INSTANCE, NVM_SET_RAMBLOCK_STATUS_API_ID ,NVM_E_NOT_INITIALIZED) ;
        #endif
        rtn_val = E_NOT_OK ;
    }
    /*[SWS_NvM_00644] If development error detection is enabled for NvM module,
     * the function NvM_SetRamBlockStatus shall report the DET error NVM_E_BLOCK_PENDING
     * when NVRAM block identifier is already queued or currently in progress.
     */
    else if(AdministrativeBlock[BlockId].BlockStatus == NVM_REQ_PENDING)
    {
        #if(NVM_DEV_ERROR_DETECT == STD_ON)
            Det_ReportError(NVRAM_MANAGER_ID, NVRAM_MANAGER_INSTANCE, NVM_SET_RAMBLOCK_STATUS_API_ID ,NVM_E_BLOCK_PENDING) ;
        #endif
        rtn_val = E_NOT_OK ;
    }
    /*[SWS_NvM_00645] If development error detection is enabled for NvM module,
     * the function NvM_SetRamBlockStatus shall report the DET error NVM_E_PARAM_BLOCK_ID
     * when the passed BlockID is out of range.
     */
    else if( (BlockId <=1 ) || (BlockId >= NUMBER_OF_NVM_BLOCKS) )
    {
        #if(NVM_DEV_ERROR_DETECT == STD_ON)
            Det_ReportError(NVRAM_MANAGER_ID, NVRAM_MANAGER_INSTANCE, NVM_SET_RAMBLOCK_STATUS_API_ID , NVM_E_PARAM_BLOCK_ID ) ;
        #endif
        rtn_val = E_NOT_OK ;
    }
    /*Check if it's not a permanent block */
    else if(NvMBlockDescriptor[BlockId].NvMRamBlockDataAddress != NULL_PTR)
    {
        rtn_val = E_NOT_OK ;
    }
    else
    {
        /*check requested state */
        if(BlockChanged == TRUE)
        {
            /*[SWS_NvM_00406]  When the “BlockChanged” parameter passed to the function
             * NvM_SetRamBlockStatus is TRUE, the corresponding permanent RAM block or
             * the content of the RAM mirror in the NvM module ( in case of explicit synchronization)
             * is valid and changed.
             */
            AdministrativeBlock[BlockId].PRAMStatus = VALID_CHANGED ;

            /*[SWS_NvM_00121] For blocks with a permanently configured RAM,
             * the function NvM_SetRamBlockStatus shall request the recalculation of
             * CRC in the background, i.e. the CRC recalculation shall be processed by
             * the NvM_MainFunction, if the given “BlockChanged” parameter is TRUE and
             * CRC calculation in RAM is configured
             */
            if(NvMBlockDescriptor[BlockId].NvMCalcRamBlockCrc)
            {
                Enqueue_CRCJobs(BlockId);
            }
        }
        else
        {
            /*[SWS_NvM_00405] When the “BlockChanged” parameter passed to the function
             * NvM_SetRamBlockStatus is FALSE the corresponding RAM block is either
             * invalid or unchanged (or both).
             */
            AdministrativeBlock[BlockId].PRAMStatus = INVALID_UNCHANGED ;
        }
    }

    return rtn_val ;
}


/****************************************************************************************/
/*    Function Name           : NvM_ReadBlock                                           */
/*    Function Description    : Service to copy the data of the NV block to its         */
/*                              corresponding RAM block                                 */
/*    Parameter in            : BlockId                                                 */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : NvM_DstPtr                                              */
/*    Return value            : none                                                    */
/*    Requirement             : SWS_NvM_00454                                           */
/*    Notes                   : none                                                    */
/****************************************************************************************/
Std_ReturnType NvM_ReadBlock( NvM_BlockIdType BlockId, void* NvM_DstPtr )
{
    /*Variable to save return value*/
    Std_ReturnType rtn_val = E_OK ;

    /*Variables used in calculations*/
    uint16 dataIndex , NvNumberOfBlocks ;

    /*Variable to save the job request information*/
    JobInfoType JobInfo ;

    dataIndex = AdministrativeBlock[BlockId].DataSetIndex ;
    NvNumberOfBlocks = NvMBlockDescriptor[BlockId].NvMNvBlockNum ;

    /* Check if Module not internally initialized
     * [SWS_NvM_00614] If development error detection is enabled for NvM module,
     * the function NvM_ReadBlock shall report the DET error NVM_E_NOT_INITIALIZED
     * when NVM is not yet initialized.
     */
    if(ModuleState != INIT_DONE)
    {
        #if(NVM_DEV_ERROR_DETECT == STD_ON)
            Det_ReportError(NVRAM_MANAGER_ID, NVRAM_MANAGER_INSTANCE, NVM_READBLOCK_API_ID ,NVM_E_NOT_INITIALIZED) ;
        #endif
        rtn_val = E_NOT_OK ;
    }

    /* [SWS_NvM_00615] If development error detection is enabled for NvM module,
     * the function NvM_ReadBlock shall report the DET error NVM_E_BLOCK_PENDING
     * when NVRAM block identifier is already queued or currently in progress.
     */
    else if(AdministrativeBlock[BlockId].BlockStatus == NVM_REQ_PENDING)
    {
        #if(NVM_DEV_ERROR_DETECT == STD_ON)
            Det_ReportError(NVRAM_MANAGER_ID, NVRAM_MANAGER_INSTANCE, NVM_READBLOCK_API_ID ,NVM_E_BLOCK_PENDING) ;
        #endif
        rtn_val = E_NOT_OK ;
    }
    /* [SWS_NvM_00616] If development error detection is enabled for NvM module,
     * the function NvM_ReadBlock shall report the DET error NVM_E_PARAM_ADDRESS
     * when no permanent RAM block and no explicit synchronization are configured
     * and a NULL pointer is passed via the parameter NvM_DstPtr.
     */
    else if(NvM_DstPtr == NULL_PTR && NvMBlockDescriptor[BlockId].NvMRamBlockDataAddress == NULL_PTR \
            && NvMBlockDescriptor[BlockId].NvMBlockUseSyncMechanism == STD_OFF)
    {
        #if(NVM_DEV_ERROR_DETECT == STD_ON)
            Det_ReportError(NVRAM_MANAGER_ID, NVRAM_MANAGER_INSTANCE, NVM_READBLOCK_API_ID ,NVM_E_PARAM_ADDRESS) ;
        #endif
        rtn_val = E_NOT_OK ;
    }
    /* [SWS_NvM_00618] If development error detection is enabled for NvM module,
     * the function NvM_ReadBlock shall report the DET error NVM_E_PARAM_BLOCK_ID
     * when the passed BlockID is out of range.
     */
    else if( (BlockId <=1 ) || (BlockId >= NUMBER_OF_NVM_BLOCKS) )
    {
        #if(NVM_DEV_ERROR_DETECT == STD_ON)
            Det_ReportError(NVRAM_MANAGER_ID, NVRAM_MANAGER_INSTANCE, NVM_READBLOCK_API_ID , NVM_E_PARAM_BLOCK_ID ) ;
        #endif
        rtn_val = E_NOT_OK ;
    }
    /* wrong data set value , return E_NOT_OK
     */
    else if(NvMBlockDescriptor[BlockId].NvMBlockManagement == NVM_BLOCK_DATASET &&\
            AdministrativeBlock[BlockId].DataSetIndex >= (1<<NVM_DATASET_SELECTION_BITS))
    {
        rtn_val = E_NOT_OK;
    }
    /*Return E_NOT_OK if the required index is in ROM and ROM address not configured*/
    else if(dataIndex >= NvNumberOfBlocks && NvMBlockDescriptor[BlockId].NvMRomBlockDataAddress == NULL_PTR )
    {
        rtn_val = E_NOT_OK ;
    }
    /*[SWS_NvM_00355] The job of the function NvM_ReadBlock shall not copy the NV
      block to the corresponding RAM block if the NVRAM block management type is
      NVM_BLOCK_DATASET and the NV block selected by the dataset index is
      invalidate.
     */
    else if(AdministrativeBlock[BlockId].BlockStatus == NVM_REQ_NV_INVALIDATED)
    {
        rtn_val = E_NOT_OK ;
    }
    /*[SWS_NvM_00651] The job of the function NvM_ReadBlock shall not copy the NV
      block to the corresponding RAM block if the NVRAM block management type is
      NVM_BLOCK_DATASET and the NV block selected by the dataset index is
      inconsistent.
     */
    else if(AdministrativeBlock[BlockId].BlockStatus == NVM_REQ_INTEGRITY_FAILED)
    {
        rtn_val = E_NOT_OK ;
    }
    /*No detected errors*/
    else
    {
        /*[SWS_NvM_00195] The function NvM_ReadBlock shall take over the given
          parameters, queue the read request in the job queue and return.
         (SRS_Mem_00016)
         */
        JobInfo.BlockId    = BlockId ;
        JobInfo.JobRequest = NVM_READBLOCK_API_ID ;

        /*Check if the required RAM block is temporary or permanent*/
        if(NvM_DstPtr != NULL_PTR )
        {
            JobInfo.RamAddress = (uint8*) NvM_DstPtr ;
        }
        else
        {
            JobInfo.RamAddress  = NvMBlockDescriptor[BlockId].NvMRamBlockDataAddress ;
        }

        JobInfo.Job_InternalState = GET_MEMIF_BLOCK_ID;

        /*Save job request as pending in the Administrative block*/
        AdministrativeBlock[BlockId].BlockStatus = NVM_REQ_PENDING ;

        /*Initialize End job status*/
        #if(NVM_POLLING_MODE == STD_OF)
         EndJobStatus.EndJobFailed  = 0 ;
         EndJobStatus.EndJobSuccess = 0;
        #endif

        /*Save job request in the queue , if queue Full it will return E_NOT_OK */
        rtn_val = Enqueue_Jobs(JobInfo) ;

        /*[SWS_NvM_00198] The function NvM_ReadBlock shall invalidate a permanent
         * RAM block immediately when the block is successfully enqueued or the
         * job processing starts, i.e. copying data from NV memory or ROM to RAM.
         * If the block has a synchronization callback (NvM_ReadRamBlockFromNvm)
         * configured the invalidation will be done just before NvMReadRamBlockFromNvM is called.
         */
        if(NvMBlockDescriptor[BlockId].NvMRamBlockDataAddress != NULL_PTR)
        {
            AdministrativeBlock[BlockId].PRAMStatus = INVALID_UNCHANGED ;
        }
    }
    return rtn_val ;
}

/****************************************************************************************/
/*    Function Name           : NvM_ReadAll                                             */
/*    Function Description    : Initiates a multi block read request.                   */
/*    Parameter in            : none                                                    */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : none                                                    */
/*    Requirement             : SWS_NvM_00460                                           */
/*    Notes                   : none                                                    */
/****************************************************************************************/
void NvM_ReadAll(void)
{
    /*[SWS_NvM_00646] If development error detection is enabled for NvM module,
     * the function NvM_ReadAll shall report the DET error NVM_E_NOT_INITIALIZED
     * when NVM is not yet initialized.
     */
    if(ModuleState != INIT_DONE)
    {
        #if(NVM_DEV_ERROR_DETECT == STD_ON)
            Det_ReportError(NVRAM_MANAGER_ID, NVRAM_MANAGER_INSTANCE, NVM_READ_ALL_API_ID ,NVM_E_NOT_INITIALIZED) ;
        #endif
    }

    /*[SWS_NvM_00243] The function NvM_ReadAll shall signal the request to the NvM
      module and return. The NVRAM Manager shall defer the processing of the requested
      ReadAll until all single block job queues are empty.
     */
    MultiBlcokRequest.request = NVM_READ_ALL_API_ID ;
    MultiBlcokRequest.ResultStatus = NVM_REQ_PENDING ;
    MultiBlcokRequest.Internal_state = GET_MEMIF_BLOCK_ID ;
}


/****************************************************************************************/
/*    Function Name           : NvM_MainFunction_ReadBlock                              */
/*    Function Description    : Asynchronous processing of ReadBlock job                */
/*    Parameter in            : none                                                    */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : none                                                    */
/****************************************************************************************/
static void NvM_MainFunction_ReadBlock(void)
{
    MemIf_StatusType    MemIf_Status    ;
    uint16 dataIndex , NvNumberOfBlocks  ,Length ;
    uint8  DeviceId ;
    Std_ReturnType rtn_val ;

    switch(Queue.Buffer[Queue.front].Job_InternalState)
    {

/********************case : GET_MEMIF_BLOCK_ID*******************/
    case GET_MEMIF_BLOCK_ID :
        dataIndex = AdministrativeBlock[Queue.Buffer[Queue.front].BlockId].DataSetIndex  ;
        NvNumberOfBlocks  = NvMBlockDescriptor[Queue.Buffer[Queue.front].BlockId].NvMNvBlockNum ;

        /*Check if the required index is a ROM block
         * [SWS_NvM_00146] If the basic storage object ROM block is selected as optional part,
         * the index range which normally selects a dataset is extended to the ROM to make it
         * possible to select a ROM block instead of a NV block. The index covers all NV/ROM
         * blocks which may build up the NVRAM Dataset block.
         */
        if(dataIndex >= NvNumberOfBlocks )
        {

            /*Go to next state to read data from block*/
            Queue.Buffer[Queue.front].Job_InternalState = READ_ROM_BLOCK ;
        }
        else
        {
            BlockNumber = (NvMBlockDescriptor[Queue.Buffer[Queue.front].BlockId].NvMNvBlockBaseNumber <<\
                    NVM_DATASET_SELECTION_BITS ) + dataIndex ;
            /*Go to next state to read data from block*/
                Queue.Buffer[Queue.front].Job_InternalState = READ_NV_BLOCK ;
        }

    break ;

/********************case : READ_NV_BLOCK*******************/
    case READ_NV_BLOCK :

        DeviceId = NvMBlockDescriptor[Queue.Buffer[Queue.front].BlockId].NvMNvramDeviceId ;
        Length   = NvMBlockDescriptor[Queue.Buffer[Queue.front].BlockId].NvMNvBlockLength ;

        /*Read underlying device status*/
        MemIf_JobResult = MemIf_GetJobResult(DeviceId)  ;
        MemIf_Status    = MemIf_GetStatus(DeviceId);

        /*Initiate a request if underlying device not busy*/
         if(MemIf_Status != MEMIF_BUSY)
         {
             MemIf_Read(DeviceId , BlockNumber , Queue.Buffer[Queue.front].RamAddress ,Length) ;
         }

         /*Check if Job result is no longer pending*/
         if(MemIf_JobResult != MEMIF_JOB_PENDING)
         {
             if(MemIf_JobResult == MEMIF_JOB_OK)
             {
                 /*Check if CRC is required for this block*/
                if(NvMBlockDescriptor[Queue.Buffer[Queue.front].BlockId].NvMBlockUseCrc == TRUE)
                {
                    /*Go to check CRC state*/
                    Queue.Buffer[Queue.front].Job_InternalState = CHECK_CRC ;
                }
                else
                {
                    Queue.Buffer[Queue.front].Job_InternalState  = END_JOB ;
                    AdministrativeBlock[Queue.Buffer[Queue.front].BlockId].BlockStatus = NVM_REQ_OK ;

                    /*[SWS_NvM_00200] The job of the function NvM_ReadBlock shall set the RAM block
                     * to valid and assume it to be unchanged after a successful copy process of the
                     * NV block to RAM.
                     */
                    AdministrativeBlock[Queue.Buffer[Queue.front].BlockId].PRAMStatus = VALID_UNCHANGED ;
                }
             }
             /*[SWS_NvM_00657] The job of the function NvM_ReadBlock shall load the default
              * values according to processing of NvM_RestoreBlockDefaults (also set the
              * request result to NVM_REQ_RESTORED_FROM_ROM) if the read request passed to
              * the underlying layer fails (MemIf reports MEMIF_JOB_FAILED or MEMIF_BLOCK_INCONSISTENT)
              * and if the default values are available.*/
             else if((MemIf_JobResult == MEMIF_JOB_FAILED))
             {
                 /*Job failed , the get default data*/
                 Queue.Buffer[Queue.front].Job_InternalState = READ_ROM_BLOCK ;
                 AdministrativeBlock[Queue.Buffer[Queue.front].BlockId].BlockStatus = NVM_REQ_NOT_OK ;
             }
             else if(MemIf_JobResult == MEMIF_BLOCK_INCONSISTENT)
             {
                 /*Job failed , the get default data*/
                 Queue.Buffer[Queue.front].Job_InternalState = READ_ROM_BLOCK ;
                 AdministrativeBlock[Queue.Buffer[Queue.front].BlockId].BlockStatus = NVM_REQ_INTEGRITY_FAILED ;
             }
             else
             {
                 AdministrativeBlock[Queue.Buffer[Queue.front].BlockId].BlockStatus = NVM_REQ_NV_INVALIDATED ;
                 Queue.Buffer[Queue.front].Job_InternalState = END_JOB ;

             }
         }

    break ;
/********************case : READ_ROM_BLOCK*******************/
    case READ_ROM_BLOCK :
        /*Check if ROM block address is  configured*/
        if(NvMBlockDescriptor[Queue.Buffer[Queue.front].BlockId].NvMRomBlockDataAddress != NULL_PTR )
        {
            ReadRomBlock(NvMBlockDescriptor[Queue.Buffer[Queue.front].BlockId].NvMRomBlockDataAddress ,\
                         Queue.Buffer[Queue.front].RamAddress , \
                         NvMBlockDescriptor[Queue.Buffer[Queue.front].BlockId].NvMNvBlockLength);

             AdministrativeBlock[Queue.Buffer[Queue.front].BlockId].BlockStatus = NVM_REQ_RESTORED_FROM_ROM ;

             /*[SWS_NvM_00366] The job of the function NvM_ReadBlock shall set the RAM
              * block to valid and assume it to be changed if the default values are copied
              * to the RAM successfully.
              */
             AdministrativeBlock[Queue.Buffer[Queue.front].BlockId].PRAMStatus = VALID_CHANGED ;
        }

        /*Check if NvMInitBlockCallback is  configured */
        else if(NvMBlockDescriptor[Queue.Buffer[Queue.front].BlockId].NvMInitBlockCallback != NULL_PTR)
        {
            rtn_val=NvMBlockDescriptor[Queue.Buffer[Queue.front].BlockId].NvMInitBlockCallback();

            /*Check return value*/
            if(rtn_val == E_OK )
            {
                 Queue.Buffer[Queue.front].Job_InternalState = END_JOB ;

                 /*[SWS_NvM_00202] The job of the function NvM_ReadBlock shall load the default
                    values according to processing of NvM_RestoreBlockDefaults (also set the request
                    result to NVM_REQ_RESTORED_FROM_ROM) if the recalculated CRC is not equal
                    to the CRC stored in NV memory.
                  */
                 AdministrativeBlock[Queue.Buffer[Queue.front].BlockId].BlockStatus = NVM_REQ_RESTORED_FROM_ROM ;

                 /*[SWS_NvM_00366] The job of the function NvM_ReadBlock shall set the RAM
                  * block to valid and assume it to be changed if the default values are copied
                  * to the RAM successfully.
                  */
                 AdministrativeBlock[Queue.Buffer[Queue.front].BlockId].PRAMStatus = VALID_CHANGED ;
            }
            else
            {
                Queue.Buffer[Queue.front].Job_InternalState = END_JOB;
            }
        }
        else
        {
            Queue.Buffer[Queue.front].Job_InternalState = END_JOB ;
        }
        break;
/********************case : CHECK_CRC*******************/
    case CHECK_CRC :
        /* TODO later*/
        break;

/********************case : END_JOB*******************/
    case END_JOB:

        /*Check if single block call back is configured */
        if(NvMBlockDescriptor[Queue.Buffer[Queue.front].BlockId].NvMSingleBlockCallback != NULL_PTR)
        {
            NvMBlockDescriptor[Queue.Buffer[Queue.front].BlockId].\
            NvMSingleBlockCallback(Queue.Buffer[Queue.front].JobRequest,\
            AdministrativeBlock[Queue.Buffer[Queue.front].BlockId].BlockStatus);
        }
        /*remove job request from queue*/
        Dequeue_Jobs();
        break ;
    }
}


/*****************************************************************************************/
/*                                   Local Function Definition                           */
/*****************************************************************************************/



/****************************************************************************************/
/*    Function Name           : Init_Queue                                              */
/*    Function Description    : put queue in it's initialized state                     */
/*    Parameter in            : none                                                    */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : none                                                    */
/****************************************************************************************/


