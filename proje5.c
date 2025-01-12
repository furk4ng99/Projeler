#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_hibernate.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/hibernate.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"

#define HIB_WAKE_TIME  10

int main(void)
{
    SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | 
                   SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_HIBERNATE);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_HIBERNATE));

    HibernateEnableExpClk(SysCtlClockGet());
    HibernateGPIORetentionEnable();

    HibernateRTCSet(0);
    HibernateRTCEnable();

    HibernateRTCMatchSet(0, HIB_WAKE_TIME);  
    HibernateWakeSet(HIBERNATE_WAKE_RTC);  
    HibernateIntEnable(HIBERNATE_INT_RTC_MATCH_0);

    HibernateRequest();

    while(1)
    {
    }

    return 0;
}
