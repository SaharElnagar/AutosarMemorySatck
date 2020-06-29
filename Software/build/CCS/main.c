

/**
 * main.c
 */
#include "Det.h"
#include "sysctl.h"
#include "Ea.h"

void Set_SystemClock(void);

extern Eep_ConfigType Eep_Config ;

uint8 pData[50];
uint8 pRead[50];

void WriteBlock(uint8 val)
{
    uint8 count = 0;
    for(count =0 ; count <50;count++)
    {
        pData[count] = val;
    }
}

int main(void)
{

    Set_SystemClock();
    Eep_Init(&Eep_Config) ;
    Ea_Init(NULL);
    WriteBlock(5);
    Ea_Write(EA_BLOCK_1_NUMBER, pData);
    while(Ea_GetStatus()== MEMIF_BUSY)
    {
       Eep_MainFunction();
       Ea_MainFunction() ;
    }

    Ea_Read(EA_BLOCK_1_NUMBER, 0, pRead, EA_BLOCK_1_SIZE);
    while(Ea_GetStatus()== MEMIF_BUSY)
       {
           Eep_MainFunction();
           Ea_MainFunction();
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


