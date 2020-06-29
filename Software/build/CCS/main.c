

/**
 * main.c
 */

#include "Det.h"
#include "Fee.h"

extern Fls_ConfigType Fls_Config;

uint8 Wdata[100]={1,2};
uint8 Rdata[40];

void Write_buffer(uint8 val)
{
    uint8 count =0 ;
    for(count = 0 ;count <100 ;count++)
    {
        Wdata[count] = val ;
    }
}
int main(void)
{
    Fls_Init(&Fls_Config);
    Fls_Erase(0x30000, 1024);
    while(Fls_GetStatus()== MEMIF_BUSY)
        {
            Fls_MainFunction();
        }
    Fee_Init(NULL);

    while(Fee_GetStatus()== MEMIF_BUSY)
    {
        Fls_MainFunction();
        Fee_MainFunction();
    }
    
    Write_buffer(1) ;
    Fee_Write(BLOCK_1_NUMBER, Wdata);
    while(Fee_GetStatus()== MEMIF_BUSY)
  {
      Fls_MainFunction();
      Fee_MainFunction();
  }
   Fee_Read(BLOCK_1_NUMBER, 0, Rdata, BLOCK_1_SIZE);
   while(Fee_GetStatus()== MEMIF_BUSY)
     {
         Fls_MainFunction();
         Fee_MainFunction();
     }
   Write_buffer(2) ;
      Fee_Write(BLOCK_1_NUMBER, Wdata);
      while(Fee_GetStatus()== MEMIF_BUSY)
    {
        Fls_MainFunction();
        Fee_MainFunction();
    }
     Fee_Read(BLOCK_1_NUMBER, 0, Rdata, BLOCK_1_SIZE);
     while(Fee_GetStatus()== MEMIF_BUSY)
       {
           Fls_MainFunction();
           Fee_MainFunction();
       }
     while(1)
     {}
	return 0;
}
