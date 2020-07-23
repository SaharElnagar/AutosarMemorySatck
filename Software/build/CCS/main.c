

/**
 * main.c
 */

#include "Det.h"
#include "NvM.h"
#include "Fee.h"
#include "Ea.h"
#include "sysctl.h"

extern Fls_ConfigType Fls_Config;
extern Eep_ConfigType Eep_Config ;
#define WRITE_SIZE  1024
uint8 Wdata[WRITE_SIZE]={1};
uint8 Rdata[WRITE_SIZE];
void Set_SystemClock(void);
typedef enum{
    Test_IDLE,
    Test_OK,
    MultiBlock_Test_Ok,
    Test_NOT_OK,
    MultiBlock_Test_NOT_OK,
    Read_default,
    Block_Invalid,
    Test_Pending,
}test_case_state_t;


test_case_state_t test_case_state = Test_IDLE;

Std_ReturnType Block_2_NvMSingleBlockCallback(uint8 ServiceId,NvM_RequestResultType JobResult )
{
    if(JobResult == NVM_REQ_OK)
    {
        test_case_state = Test_OK;
    }
    else if(JobResult == NVM_REQ_RESTORED_FROM_ROM)
    {
        test_case_state = Read_default;
    }
    else if(JobResult == NVM_REQ_NOT_OK )
    {
        test_case_state = Test_NOT_OK;
    }
    else if(JobResult == NVM_REQ_NV_INVALIDATED)
    {
        test_case_state = Block_Invalid ;
    }
    else
    {}

    return E_OK ;
}

uint8_t MBRequest = 55 ;
void NvM_MultiBlockCallbackFunction(uint8 ServiceId,NvM_RequestResultType JobResult)
{
    MBRequest = JobResult;
}
void Write_buffer(uint8 val)
{
    uint16 count =0 ;
    for(count = 0 ;count <WRITE_SIZE ;count++)
    {
        Wdata[count] = val ;
    }
}
uint8 flag=0;
void main(void)
{
    Set_SystemClock();
    Fls_Init(&Fls_Config);
    Eep_Init(&Eep_Config);

    Fls_Read(0x30000, Rdata, 1024);
    while(Fls_GetStatus()== MEMIF_BUSY)
    {
        Fls_MainFunction();
     }
    if(flag==2)
    {
    Fls_Erase(0x30000, 0x10000);
    while(Fls_GetStatus()== MEMIF_BUSY)
    {
        Fls_MainFunction();
    }
    }
    Ea_Init(NULL) ;
    Fee_Init(NULL);
    NvM_Init(NULL);
    while(Fee_GetStatus()== MEMIF_BUSY)
    {
        Fls_MainFunction();
        Fee_MainFunction();

    }
    while(Ea_GetStatus()==MEMIF_BUSY)
    {
        Eep_MainFunction();
        Ea_MainFunction();
    }
   test_case_state = Test_Pending ;
    Write_buffer(4) ;
    NvM_WriteBlock(NVM_NVRAM_BLOCK_2_ID, Wdata);
    while(test_case_state == Test_Pending )
  {
        Eep_MainFunction();
           Ea_MainFunction();
      NvM_MainFunction();
  }
    test_case_state = Test_Pending ;
   NvM_ReadBlock(NVM_NVRAM_BLOCK_2_ID, Rdata);
   while(test_case_state == Test_Pending )
 {

     Eep_MainFunction();
     Ea_MainFunction();
     NvM_MainFunction();
 }
   test_case_state = Test_Pending ;
  NvM_ReadAll();
  while(MBRequest!=NVM_REQ_OK)
{
    Fls_MainFunction();
    Fee_MainFunction();
    NvM_MainFunction();
}
   test_case_state = Test_Pending ;
   if(flag==5)
   {
   Write_buffer(8) ;
   NvM_WriteBlock(NVM_NVRAM_BLOCK_3_ID, Wdata);
   while(test_case_state == Test_Pending )
 {
     Fls_MainFunction();
     Fee_MainFunction();
     NvM_MainFunction();
 }
   }
   test_case_state = Test_Pending ;
  NvM_ReadBlock(NVM_NVRAM_BLOCK_3_ID, Rdata);
  while(test_case_state == Test_Pending )
{
    Fls_MainFunction();
    Fee_MainFunction();
    NvM_MainFunction();
}
     while(1)
     {}

}


void Set_SystemClock(void)
{
    /*Use 80 MHz clock*/
        SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);
        SysCtlDelay(20000000);
        SysCtlPeripheralEnable(SYSCTL_PERIPH_EEPROM0);
}
