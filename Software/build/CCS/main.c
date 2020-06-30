

/**
 * main.c
 */

#include "Det.h"
#include "NvM.h"
#include "Fee.h"

extern Fls_ConfigType Fls_Config;

uint8 Wdata[100]={1,2};
uint8 Rdata[40];

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

void Write_buffer(uint8 val)
{
    uint8 count =0 ;
    for(count = 0 ;count <100 ;count++)
    {
        Wdata[count] = val ;
    }
}
void main(void)
{
    Fls_Init(&Fls_Config);
    Fls_Erase(0x30000, 1024);
    while(Fls_GetStatus()== MEMIF_BUSY)
    {
        Fls_MainFunction();
    }
    Fee_Init(NULL);
    NvM_Init(NULL);
    while(Fee_GetStatus()== MEMIF_BUSY)
    {
        Fls_MainFunction();
        Fee_MainFunction();
    }
    test_case_state = Test_Pending ;
    Write_buffer(3) ;
    NvM_WriteBlock(NVM_NVRAM_BLOCK_2_ID, Wdata);
    while(test_case_state == Test_Pending )
  {
      Fls_MainFunction();
      Fee_MainFunction();
      NvM_MainFunction();
  }
    test_case_state = Test_Pending ;
   NvM_ReadBlock(NVM_NVRAM_BLOCK_2_ID, Rdata);
   while(test_case_state == Test_Pending )
 {
     Fls_MainFunction();
     Fee_MainFunction();
     NvM_MainFunction();
 }

     while(1)
     {}

}
