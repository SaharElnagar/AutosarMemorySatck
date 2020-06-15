#ifndef NVM_SHARED_H_
#define NVM_SHARED_H_

/*****************************************************************************************/
/*                                   Include headers                                     */
/*****************************************************************************************/

#include "NvM.h"
#include "Det.h"
#include "MemIf_Types.h"
#include "MemIf.h"

/*****************************************************************************************/
/*                                   Local types Definition                              */
/*****************************************************************************************/

/*Module state type*/
typedef uint8 ModuleStateType ;

/*Permanent RAM Status type*/
typedef uint8 PRamStatusType ;

/* [SWS_NvM_00134]
 * Administrative block Type
 * The Administrative block shall be located in RAM
 * shall contain a block index which is used in association with Data set NV blocks.
 * error/status information of the corresponding NVRAM block shall be contained
 * use state information of the permanent RAM block (valid / Invalid)
 * The Administrative block shall be invisible for the application
 * shall use an error/status field to manage the error/status value of the last request
 * shall use an attribute field to manage the NV block write protection in order to
 * protect/unprotect a NV block data field
 */
typedef struct
{
    uint8 DataSetIndex ;
    NvM_RequestResultType BlockStatus ;
    PRamStatusType PRAMStatus ;
    uint32 PrevCRCVal ;

}AdministrativeBlockType ;

/* struct to hold the parameters for the job request*/
typedef struct
{
   uint8 ServiceId ;
   NvM_BlockIdType Block_Id ;
   void* RAM_Ptr ;

}Job_Parameters ;

/*struct to hold the indices which point to the queue
 *Used for queue implementation
 */
typedef struct{
  uint16 Head ;
  uint16 Tail ;
}Queue_Indices_Struct ;

/*Multi  block request Information */
typedef struct
{
    NvM_RequestResultType ResultStatus ;
    uint8 request ;
    uint8 Internal_state ;
}MultiBlockRequestType;

#if(NVM_POLLING_MODE == STD_OF)
/*Struct to save end job status from underlying layer Successful or failed*/
typedef struct
{
    uint8 EndJobSuccess : 1;
    uint8 EndJobFailed  : 1;
}EndJobStatusType;
#endif

/*****************************************************************************************/
/*                               Local Macros Definition                                 */
/*****************************************************************************************/

/*Size of CRC job queue*/
#define NVM_SIZE_CRC_JOB_QUEUE      (10U)

/*Permenant Ram block status*/
#define INVALID_UNCHANGED           ((PRamStatusType)0U)
#define VALID_UNCHANGED             ((PRamStatusType)1U)
#define VALID_CHANGED               ((PRamStatusType)2U)


/*Size of the largest block in our module*/
#define LARGEST_BLOCK_SIZE          (100U)

/*Permenant Ram block status*/
#define INVALID_UNCHANGED           ((PRamStatusType)0U)
#define VALID_UNCHANGED             ((PRamStatusType)1U)
#define VALID_CHANGED               ((PRamStatusType)2U)


/*Empty Queue size*/
#define EMPTY_QUEUE                 (0U)

/*Module States*/
#define MODULE_UNINITIALIZED        ((ModuleStateType)0U)
#define INIT_DONE                   ((ModuleStateType)1U)


/*Block Type of the non volatile memory*/
#define NV_BLOCK                    (0U)
#define ROM_BLOCK                   (1U)

/*Read Block states */
#define GET_MEMIF_BLOCK_ID          (0U)
#define READ_NV_BLOCK               (1U)
#define READ_ROM_BLOCK              (2U)
#define CHECK_CRC                   (3U)
#define END_JOB                     (4U)

/*Write Block states*/
#define CALC_CRC                    (0U)
#define WRITE_NV_BLOCK              (1U)
#define WRITE_OK                    (2U)
#define WRITE_FAILED                (3U)
#define WRITE_END                   (4U)

/*Invalidate Block states*/
#define INVALIDATE_NV_BLOCK         (0U)
#define INVALIDATE_OK               (1U)
#define INVALIDATE_FAILED           (2U)
#define INVALIDATE_END              (3U)

/*Main function states*/
#define GET_JOB                     (0U)
#define EXECUTE_SINGLE_JOB          (1U)
#define EXECUTE_CRC_JOB             (2U)
#define EXECUTE_MULTI_JOB           (3U)

/*****************************************************************************************/
/*                                   Local variables Definition                          */
/*****************************************************************************************/

/*Administrative block for each NVRAM block
 *The “Administrative Block” is a “Basic Storage Object”.
 *It resides in RAM. The  “Administrative Block” is a mandatory part of a “NVRAM Block”.
 */
static AdministrativeBlockType AdministrativeBlock[NUMBER_OF_NVM_BLOCKS];

static MultiBlockRequestType MultiBlcokRequest ;

/*****************************************************************************************/
/*                                   Local Functions Prototypes                          */
/*****************************************************************************************/

void Init_Queues(void) ;
Std_ReturnType Search_Queue(NvM_BlockIdType BlockId) ;
Std_ReturnType Job_Enqueue(Job_Parameters Job) ;
Std_ReturnType Job_Dequeue(void) ;
void Get_SingleJob(Job_Parameters* Job) ;
Std_ReturnType CRCJob_Enqueue(NvM_BlockIdType BlockId) ;
Std_ReturnType CRCJob_Dequeue(void) ;

void NvM_MainFunction_WriteBlock(void) ;
void NvM_MainFunction_InvalidateBlock(void) ;
void NvM_MainFunction_WriteAll( void ) ;

/*****************************************************************************************/
/*                                   external variables                                  */
/*****************************************************************************************/

/*Blocks configurations */
extern NvMBlockDescriptorType NvMBlockDescriptor[NUMBER_OF_NVM_BLOCKS];

/*****************************************************************************************/
/*                                   Local variables Definition                          */
/*****************************************************************************************/

/*Administrative block for each NVRAM block
 *The “Administrative Block” is a “Basic Storage Object”.
 *It resides in RAM. The  “Administrative Block” is a mandatory part of a “NVRAM Block”.
 */
static AdministrativeBlockType AdministrativeBlock[NUMBER_OF_NVM_BLOCKS];

/*[SWS_NvM_00195] The function NvM_ReadBlock shall take over the given parameters,
 *queue the read request in the job queue and return
 *Hint: this requirement is the same for all asynchronous single block requests.
 */
/* standard job queue */
static Job_Parameters Standard_Job_Queue[NVM_SIZE_STANDARD_JOB_QUEUE] ;
static Queue_Indices_Struct Stand_Queue_Indeces = {0, 0} ;

/*[SWS_NvM_00378]
 * In case of priority based job processing order,
 * the NvM module shall use two queues, one for immediate write jobs (crash data) another for all other jobs.
 */
/* immediate job queue */
#if (NVM_JOB_PRIORITIZATION == STD_ON)
  static Job_Parameters Immediate_Job_Queue[NVM_SIZE_IMMEDIATE_JOB_QUEUE] ;
  static Queue_Indices_Struct Immed_Queue_Indeces = {0, 0} ;
#endif

/*[SWS_NvM_00121] For blocks with a permanently configured RAM,
 * the function NvM_SetRamBlockStatus shall request the recalculation of
 * CRC in the background, i.e. the CRC recalculation shall be processed by
 * the NvM_MainFunction, if the given “BlockChanged” parameter is TRUE and
 * CRC calculation in RAM is configured.
 */
/* CRC job queue */
static NvM_BlockIdType CRC_Job_Queue[NVM_SIZE_CRC_JOB_QUEUE] ;
static Queue_Indices_Struct CRC_Queue_Indeces = {0, 0} ;

/*Variable to save module state*/
static ModuleStateType ModuleState = MODULE_UNINITIALIZED ;

/*Variable to hold the current job being processed by the main function*/
static Job_Parameters Current_Job ;

/*Temporary variable to store the NV data combined with CRC value */
static uint8 TempBuffer[LARGEST_BLOCK_SIZE] ;

/*****************************************************************************************/
/*                                   Queue Flags                                         */
/*****************************************************************************************/
static boolean Standard_Queue_Empty = TRUE ;
static boolean Standard_Queue_FULL = FALSE ;

#if (NVM_JOB_PRIORITIZATION == STD_ON)
  static boolean Immediate_Queue_Empty = TRUE ;
  static boolean Immediate_Queue_FULL = FALSE ;
#endif

static boolean CRC_Queue_Empty = TRUE ;
static boolean CRC_Queue_Full = FALSE ;


#endif

