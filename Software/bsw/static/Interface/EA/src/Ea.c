
/*******************************************************************************
**                                                                            **
**  FILENAME     : Ea.c                                                       **
**                                                                            **
**  VERSION      : 4.3.1                                                      **
**                                                                            **
**  DATE         : 2020-2-17                                                  **
**                                                                            **
**  PLATFORM     : TIVA C                                                     **
**                                                                            **
**  AUTHOR       : Sahar Elnagar                                              **
**                                                                            **
**  Description  :   EEPROM Abstraction Interface                             **
*******************************************************************************/

/*  Each device should work on range of blocks so we can get
 *  the start address and the device from the logical block number
 *  [SRS_MemHwAb_14009]
 *  The physical device and the start address of a logical block shall
 *  be derived from the logical block identifier.
 * */


/*****************************************************************************************/
/*                                   Include Common headres                              */
/*****************************************************************************************/

/*****************************************************************************************/
/*                                   Include Other  headres                              */
/*****************************************************************************************/
#include "Det.h"

/*****************************************************************************************/
/*                                   Include Component headres                           */
/*****************************************************************************************/
#include "Ea.h"
/*****************************************************************************************/
/*                                   Local Macro Definition                              */
/*****************************************************************************************/
/*The first logical Block number of this module */
#define MODULE_LOGICAL_START_ADDRESS    EA_BLOCK_0_NUMBER

/*The Last logical Block number of this module */
#define MODULE_LOGICAL_END_ADDRESS      EA_BLOCK_2_NUMBER

/*Macro to calculate physical Writing address*/
#define CALC_PHSICAL_W_ADD(LogialBlockNum)          ((LogialBlockNum - MODULE_LOGICAL_START_ADDRESS)*\
                                                        EA_VIRTUAL_PAGE_SIZE)
/*Macro to calculate physical Reading address*/
#define CALC_PHSICAL_R_ADD(LogialBlockNum,Offset)   (((LogialBlockNum - MODULE_LOGICAL_START_ADDRESS)*\
                                                        EA_VIRTUAL_PAGE_SIZE) + Offset )

/*States of current job processing*/
#define IDLE_JOB        0x00
#define READ_JOB        0x01
#define WRITE_JOB       0x02
#define ERASE_JOB       0x03
#define COPMARE_JOB     0x04

/*****************************************************************************************/
/*                                   Local types Definition                              */
/*****************************************************************************************/

//*****************************************************************************
//  Strut to save the required data for the reading process
//  Address: Start address of data to be read must be divisible by 4
//  DataPtr: pointer  that points to the place where data will be saved
//  Length : the number of bytes to read must be divisible by 4
//*****************************************************************************
typedef struct
{
    Eep_AddressType PhysicalStartAddress ;
    uint16 Len;
    uint8* DataBufPtr;
}str_ParametersCopy;

/*****************************************************************************************/
/*                                Local Variables Definition                             */
/*****************************************************************************************/

/*Variable to save the module state*/
static MemIf_StatusType EA_ModuleState = MEMIF_UNINIT ;

/*Array to save each block size in bytes*/
 uint16 EA_BlocksSize[] = {EA_BLOCK_0_SIZE ,EA_BLOCK_1_SIZE ,EA_BLOCK_2_SIZE} ;

/*Struct to save the Read and Write functions parameters*/
static str_ParametersCopy ParametersCopy ;

/*Variable to save current job result  */
static MemIf_JobResultType JobResult = MEMIF_JOB_OK;

/*Variable to current processing job*/
static uint8 JobProcessing_State     = IDLE_JOB;
/*****************************************************************************************/
/*                                   Global Function Definition                          */
/*****************************************************************************************/

/****************************************************************************************/
/*    Function Name           : Ea_Init                                                 */
/*    Function Description    :  Initializes the module                                 */
/*    Parameter in            : const Ea_ConfigType* ConfigPtr                          */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : none                                                    */
/*    Requirment              : SWS_Ea_00084                                            */
/*    Notes                   :                                                         */
/****************************************************************************************/

void Ea_Init(const Ea_ConfigType* ConfigPtr )
{

#if(EA_DEV_ERROR_DETECT == STD_ON)
    /*Report development error if module is already initialized  */
    if(EA_ModuleState != MEMIF_UNINIT)
    {
        Det_ReportError(EA_MODULE_ID, EA_0_INSTANCE_ID, EA_INIT_API_ID, EA_E_INIT_FAILED);
    }
#endif



    /*[SWS_Ea_00017] The function Ea_Init shall shall set the module state from MEMIF_UNINIT
     * to MEMIF_BUSY_INTERNAL once it starts the module’s initialization. (SRS_BSW_00101)
     */
    EA_ModuleState = MEMIF_BUSY_INTERNAL ;

    /*Implement internal initializations*/

    /*[SWS_Ea_00128]  If initialization is finished within Ea_Init, the function Ea_Init
     *  shall set the module state from MEMIF_BUSY_INTERNAL to MEMIF_IDLE once initialization
     *   has been successfully finished. (SRS_BSW_00406
     */
    EA_ModuleState = MEMIF_IDLE ;

}

/****************************************************************************************/
/*    Function Name           : Ea_Read                                                 */
/*    Function Description    : Reads Length bytes of block Blocknumber at offset       */
/*                              BlockOffset into the buffer DataBufferPtr               */
/*    Parameter in            : BlockNumber , BlockOffset , Length                      */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : DataBufferPtr                                           */
/*    Return value            : Std_ReturnType                                          */
/*    Requirment              : SWS_Ea_00086                                            */
/*    Notes                   :                                                         */
/****************************************************************************************/

Std_ReturnType Ea_Read(uint16 BlockNumber,uint16 BlockOffset,uint8* DataBufferPtr, uint16 Length )
{
    uint16 BlockSize  = 0;
    uint16 Sum        = 0;
    uint16 BlockCount = 0;

    /*
     * [SWS_Ea_00130] If development error detection for the module EA is enabled:
     *  the function Ea_Read shall check if the module state is MEMIF_UNINIT.
     *  If this is the case, the function Ea_Read shall reject the read request,
     *  raise the development error EA_E_UNINIT and return with E_NOT_OK.  (SRS_BSW_00406)
     */
    if(EA_ModuleState == MEMIF_UNINIT)
    {
        #if(EA_DEV_ERROR_DETECT == STD_ON)
            Det_ReportError(EA_MODULE_ID, EA_0_INSTANCE_ID, EA_READ_API_ID, EA_E_UNINIT);
        #endif

        return E_NOT_OK ;
    }
    /*Check parameter pointer is NULL*/
    else if(DataBufferPtr == NULL_PTR)
    {
        #if(EA_DEV_ERROR_DETECT == STD_ON)
            Det_ReportError(EA_MODULE_ID, EA_0_INSTANCE_ID, EA_READ_API_ID, EA_E_PARAM_POINTER);
        #endif

        return E_NOT_OK ;
    }

    /*
     * [SWS_Ea_00147]If development error detection is enabled for the module:
     * the function Ea_Read shall check whether the given block number is valid
     *  (i.e. inside the configured range)
     */
    else if((BlockNumber < MODULE_LOGICAL_START_ADDRESS) || (BlockNumber > MODULE_LOGICAL_END_ADDRESS))
    {
        #if(EA_DEV_ERROR_DETECT == STD_ON)
            Det_ReportError(EA_MODULE_ID, EA_0_INSTANCE_ID, EA_READ_API_ID, EA_E_INVALID_BLOCK_NO);
        #endif

        return E_NOT_OK ;
    }
    /*[SWS_Ea_00167] The function Ea_Read shall check if the module state is MEMIF_BUSY.
     * If this is the case, the function Ea_Read shall reject the read request,
     * raise the runtime error EA_E_BUSY and return with E_NOT_OK.
     */
    else if(EA_ModuleState == MEMIF_BUSY)
    {
        /*report error to DEM Module to report runtime error*/

        return E_NOT_OK ;
    }

    /* Get Block configured size
     * Each block number depends on the previous preserved virtual pages for the previous blocks
     * [SWS_Ea_00005]Each configured logical block shall take up an integer multiple of the configured
     * virtual page size (see also Chapter 10.2.3, configuration parameter EA_VIRTUAL_PAGE_SIZE .
     * [SWS_Ea_00068]  Logical blocks must not overlap each other and must not be contained
     *  within one another.  (SRS_MemHwAb_14001)
     * */
    for(BlockCount = 0 ;BlockCount <BLOCKS_NUM ; BlockCount++)
    {
        if((BlockNumber-MODULE_LOGICAL_START_ADDRESS) == Sum)
        {
           /*Get Block size in bytes*/
            BlockSize = EA_BlocksSize[BlockCount] ;
            break;
        }
        else
        {
           /*Add the number of previous occupied pages */
            Sum+= EA_BlocksSize[BlockCount] / EA_VIRTUAL_PAGE_SIZE ;
        }
    }

    /*
     * [SWS_Ea_00168] If development error detection is enabled for the module:
     *  the function Ea_Read shall check that the given block offset is valid
     *  (i.e. that it is less than the block length configured for this block)
     */
    if(BlockOffset > BlockSize)
    {
        #if(EA_DEV_ERROR_DETECT == STD_ON)
            Det_ReportError(EA_MODULE_ID, EA_0_INSTANCE_ID, EA_READ_API_ID, EA_E_INVALID_BLOCK_OFS);
        #endif

        return E_NOT_OK ;
    }

    /*
     * [SWS_Ea_00169]  If development error detection is enabled for the module:
     *  the function Ea_Read shall check that the given length information is valid,
     *  i.e. that the requested length information plus the block offset do not exceed
     *  the block end address (block start address plus configured block length).
     *  If this is not the case, the function Ea_Read shall reject the read request,
     *  raise the development error EA_E_INVALID_BLOCK_LEN and return with E_NOT_OK.
     */
    else if((Length + BlockOffset)  > (CALC_PHSICAL_R_ADD(BlockNumber,BlockOffset)+ BlockSize ))
    {
    #if(EA_DEV_ERROR_DETECT == STD_ON)
        Det_ReportError(EA_MODULE_ID, EA_0_INSTANCE_ID, EA_READ_API_ID, EA_E_INVALID_BLOCK_LEN);
    #endif

    return E_NOT_OK ;
    }
    else
    {
        /*Save Parameters to be used in the main function*/
        ParametersCopy.PhysicalStartAddress = CALC_PHSICAL_R_ADD(BlockNumber,BlockOffset) ;
        ParametersCopy.Len        = Length ;
        ParametersCopy.DataBufPtr = DataBufferPtr ;

        /*Set current job to reading*/
        JobProcessing_State = READ_JOB ;

        /*Set module state to busy*/
        EA_ModuleState = MEMIF_BUSY ;

        /*Set current job result to pending*/
        JobResult = MEMIF_JOB_PENDING ;
    }
    return E_OK ;
}


/****************************************************************************************/
/*    Function Name           : Ea_Write                                                */
/*    Function Description    : Writes the contents of the DataBufferPtr to             */
/*                              the block BlockNumber.                                  */
/*    Parameter in            : BlockNumber ,DataBufferPtr                              */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : Std_ReturnType                                          */
/*    Requirment              : SWS_Ea_00087                                            */
/*    Notes                   :                                                         */
/****************************************************************************************/
Std_ReturnType Ea_Write( uint16 BlockNumber,const uint8* DataBufferPtr )
{
    uint16 BlockSize  = 0;
    uint16 Sum        = 0;
    uint16 BlockCount = 0;

    /*[SWS_Ea_00181] If the current module status is MEMIF_UNINIT or MEMIF_BUSY,
     * the function Ea_Write shall reject the job request and return with E_NOT_OK.
     *(SRS_MemHwAb_14010)
     */
    if(EA_ModuleState == MEMIF_UNINIT)
    {
        #if(EA_DEV_ERROR_DETECT == STD_ON)
            Det_ReportError(EA_MODULE_ID, EA_0_INSTANCE_ID, EA_WRITE_API_ID, EA_E_UNINIT);
        #endif

        return E_NOT_OK ;
    }

    /*[SWS_Ea_00172] If development error detection is enabled for the module:
     * the function Ea_Write shall check that the given data pointer is valid
     * (i.e. that it is not NULL). If this is not the case, the function
     * Ea_Write shall reject the write request, raise the development error
     * EA_E_PARAM_POINTER and return with E_NOT_OK.  (SRS_BSW_00323)
     */
    else if(DataBufferPtr == NULL_PTR)
    {
        #if(EA_DEV_ERROR_DETECT == STD_ON)
            Det_ReportError(EA_MODULE_ID, EA_0_INSTANCE_ID, EA_WRITE_API_ID, EA_E_PARAM_POINTER);
        #endif

        return E_NOT_OK ;
    }
    /*[SWS_Ea_00181] If the current module status is MEMIF_UNINIT or MEMIF_BUSY,
     * the function Ea_Write shall reject the job request and return with E_NOT_OK.
     *(SRS_MemHwAb_14010)
     */
    else if(EA_ModuleState == MEMIF_BUSY)
    {
        /*Call DEM module to report runtime error*/

        return E_NOT_OK ;
    }


    /*
    * [SWS_Ea_00147]If development error detection is enabled for the module:
    * the function Ea_Read shall check whether the given block number is valid
    *  (i.e. inside the configured range)
    */
    else if((BlockNumber < MODULE_LOGICAL_START_ADDRESS) || (BlockNumber > MODULE_LOGICAL_END_ADDRESS))
    {
        #if(EA_DEV_ERROR_DETECT == STD_ON)
            Det_ReportError(EA_MODULE_ID, EA_0_INSTANCE_ID, EA_WRITE_API_ID, EA_E_INVALID_BLOCK_NO);
        #endif

        return E_NOT_OK ;
    }
    /*
     * NO Errors Then:
     * Calculate physical Address
     * Put Length equal to configured size for this block
     * Copy Parameters to be used in the main function
     */
    else
    {
       /* Get Block configured size
        * Each block number depends on the previous preserved virtual pages for the previous blocks
        * [SWS_Ea_00005]Each configured logical block shall take up an integer multiple of the configured
        * virtual page size (see also Chapter 10.2.3, configuration parameter EA_VIRTUAL_PAGE_SIZE .
        * [SWS_Ea_00068]  Logical blocks must not overlap each other and must not be contained
        *  within one another.  (SRS_MemHwAb_14001)
        * */
        for(BlockCount = 0 ;BlockCount <BLOCKS_NUM ; BlockCount++)
        {
            if((BlockNumber-MODULE_LOGICAL_START_ADDRESS) == Sum)
            {
               /*Get Block size in bytes*/
                BlockSize = EA_BlocksSize[BlockCount] ;
                break;
            }
            else
            {
              /*Add the number of previous occupied pages */
               Sum+= EA_BlocksSize[BlockCount] / EA_VIRTUAL_PAGE_SIZE ;
            }
        }

        /*Save Parameters to be used in the main function*/
        ParametersCopy.PhysicalStartAddress = CALC_PHSICAL_W_ADD(BlockNumber) ;
        ParametersCopy.Len        = BlockSize ;
        ParametersCopy.DataBufPtr =(uint8*) DataBufferPtr ;

        /*Set current job to writing*/
        JobProcessing_State = WRITE_JOB ;

        /*Set module state to busy*/
        EA_ModuleState = MEMIF_BUSY ;

        /*Set current job result to pending*/
        JobResult = MEMIF_JOB_PENDING ;
    }
    return E_OK ;
}

/****************************************************************************************/
/*    Function Name           : Ea_Cancel                                               */
/*    Function Description    : Cancels the ongoing asynchronous operation              */
/*    Parameter in            : none                                                    */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : none                                                    */
/*    Requirment              : SWS_Ea_00088                                            */
/*    Notes                   : The cancel functions shall only reset their modules     */
/*                              internal variables so that a new job can be accepted    */
/*                              by the modules.                                         */
/****************************************************************************************/
void Ea_Cancel(void)
{
    /*[SWS_Ea_00132] If development error detection for the module EA is enabled:
     * the function Ea_Cancel shall check if the module state is MEMIF_UNINIT.
     * If this is the case, the function Ea_Cancel shall raise the development
     * error EA_E_UNINIT and return to the caller without changing any internal variables.
     *  (SRS_BSW_00406)
     *  */
    if(EA_ModuleState == MEMIF_UNINIT)
     {
         #if(EA_DEV_ERROR_DETECT == STD_ON)
             Det_ReportError(EA_MODULE_ID, EA_0_INSTANCE_ID, EA_CANCEL_API_ID, EA_E_UNINIT);
         #endif
     }

    /*[SWS_Ea_00173] If the current module status is not MEMIF_BUSY
     * (i.e. there is no job to cancel and therefore the request to cancel
     *  a pending job is rejected by the function Ea_Cancel), the function
     *  Ea_Cancel shall raise the runtime error EA_E_INVALID_CANCEL.
     *(SRS_BSW_00323)
     */
    else if(EA_ModuleState != MEMIF_BUSY)
    {
         /*Report Runtime error to DEM Module EA_E_INVALID_CANCEL*/
    }

    /*
     * [SWS_Ea_00077] If the current module status is MEMIF_BUSY
     *  (i.e. the request to cancel a pending job is accepted by the function Ea_Cancel),
     *  the function Ea_Cancel shall call the cancel function of the underlying EEPROM driver.
     * (SRS_MemHwAb_14031)
     */
    else
    {
        /*[SWS_Ea_00078] If the current module status is MEMIF_BUSY
         * (i.e. the request to cancel a pending job is accepted by the function Ea_Cancel),
         * the function Ea_Cancel shall reset the EA module’s internal variables to make
         * the module ready for a new job request. I.e. the function Ea_Cancel shall set
         * the job result to MEMIF_JOB_CANCELED and the module status to MEMIF_IDLE.
         * (SRS_MemHwAb_14031)
         */

        /*Set current job to writing*/
        JobProcessing_State = IDLE_JOB ;

        /*Set module state to busy*/
        EA_ModuleState = MEMIF_IDLE ;

        /*Set current job result to pending*/
        JobResult = MEMIF_JOB_CANCELED ;

        /*call the cancel function of the underlying EEPROM driver*/
        Eep_Cancel();
    }
}

/****************************************************************************************/
/*    Function Name           : Ea_GetStatus                                            */
/*    Function Description    : Service to return the Status.                           */
/*    Parameter in            : none                                                    */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : MemIf_StatusType                                        */
/*    Requirment              : SWS_Ea_00089                                            */
/*    Notes                   : none                                                    */
/****************************************************************************************/
MemIf_StatusType Ea_GetStatus(void)
{
    return EA_ModuleState ;
}

/****************************************************************************************/
/*    Function Name           : Ea_GetJobResult                                         */
/*    Function Description    : Service to return the JobResult.                        */
/*    Parameter in            : none                                                    */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : MemIf_StatusType                                        */
/*    Requirment              : SWS_Ea_00090                                            */
/*    Notes                   : none                                                    */
/****************************************************************************************/
MemIf_JobResultType Ea_GetJobResult(void)
{
    return JobResult ;
}
