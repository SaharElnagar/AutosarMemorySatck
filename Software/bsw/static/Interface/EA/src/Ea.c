
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
#define MODULE_LOGICAL_START_ADDRESS    EA_BLOCK_0_NUMBER
#define MODULE_UNINIT                   (0U)
#define MODULE_READY                    (1U)

/*Macro to calculate physical Writing address*/
#define CALC_PHSICAL_W_ADD(LogialBlockNum)          ((LogialBlockNum-MODULE_LOGICAL_START_ADDRESS)*\
                                                        EA_VIRTUAL_PAGE_SIZE)
/*Macro to calculate physical Reading address*/
#define CALC_PHSICAL_W_ADD(LogialBlockNum,Offset)   (((LogialBlockNum-MODULE_LOGICAL_START_ADDRESS)*\
                                                        EA_VIRTUAL_PAGE_SIZE) + Offset ) ;

/*****************************************************************************************/
/*                                   Local types Definition                              */
/*****************************************************************************************/


/*****************************************************************************************/
/*                                Local Variables Definition                             */
/*****************************************************************************************/

/*Variable to save the module state*/
static uint8 ModuleState = MODULE_UNINIT ;



/*****************************************************************************************/
/*                                   Global Function Definition                          */
/*****************************************************************************************/

/****************************************************************************************/
/*    Function Description    :                                                         */
/*    Parameter in            : const Ea_ConfigType* ConfigPtr                          */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : none                                                    */
/*    Requirment              : SWS_Ea_00084                                            */
/*    Notes                   :                                                         */
/*****************************************************************************************/

void Ea_Init(const Ea_ConfigType* ConfigPtr )
{

#if(EA_DEV_ERROR_DETECT == STD_ON)
    /*Report development error if module is already initialized  */
    if(ModuleState != MODULE_UNINIT)
    {
        Det_ReportError(EA_MODULE_ID, EA_0_INSTANCE_ID, EA_INIT_API_ID, EA_E_INIT_FAILED);
    }
#endif
}
