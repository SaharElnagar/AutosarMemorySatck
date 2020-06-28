

/**
 * main.c
 */
#include "Det.h"
#include "sysctl.h"
#include "Eep.h"

void Set_SystemClock(void);

extern Eep_ConfigType Eep_Config ;
int main(void)
{
    uint32 pui32Data[4];
    uint32 pui32Read[4];

    pui32Data[0] = 0x00000001;
    pui32Data[1] = 0x00000002;
    pui32Data[2] = 3 ;
    pui32Data[3] = 4 ;
    Set_SystemClock();
    Eep_Init(&Eep_Config) ;
    /*Eep_Erase(0, EepEraseUnitSize);
    Eep_MainFunction();
       while(Eep_GetStatus()== MEMIF_BUSY)
       {

       }*/
    Eep_Read(8,(uint8*) pui32Read, sizeof(pui32Read));

    while(Eep_GetStatus()== MEMIF_BUSY)
    {
        Eep_MainFunction();
    }
    Eep_Write(0, (uint8*)pui32Data, sizeof(pui32Data));

    while(Eep_GetStatus()== MEMIF_BUSY)
   {
       Eep_MainFunction();
   }
    Eep_Read(0, (uint8*)pui32Read, sizeof(pui32Read));
    while(Eep_GetStatus()== MEMIF_BUSY)
       {
           Eep_MainFunction();
       }
    while(1)
    {

    }
	return 0;
}


void Set_SystemClock(void)
{
    /*Use 80 MHz clock*/
        SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);
        SysCtlDelay(20000000);
        SysCtlPeripheralEnable(SYSCTL_PERIPH_EEPROM0);
}


