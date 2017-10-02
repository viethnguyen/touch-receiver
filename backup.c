#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_memmap.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.c"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.c"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"
#include "driverlib/debug.h"
#include "driverlib/adc.h"
#include "inc/tm4c123gh6pm.h"
#include "driverlib/udma.h"

#define SAMPLING_RATE 80000.0 //
#define TARGET_FREQUENCY 10000.0 //
#define BLOCK_SIZE 80
#define BUFFER_SIZE 255

//result cannot be declared in interrupt handler
uint8_t flag = 0;
uint32_t buffer[BUFFER_SIZE];
uint32_t index;
uint32_t Q1 = 0;
uint32_t Q2 = 0;

void resetGoertzel()
{
    Q1 = 0;
    Q2 = 0;
}

float initGoertzel() {
    int k;
    const float floatN = (float)BLOCK_SIZE;
    float omega;
    k = (int) (0.5 + ((floatN * TARGET_FREQUENCY ) / SAMPLING_RATE));
    omega = (2.0 * 3.14 * k)/floatN;
    float sine = sin(omega);
    float cosine = cos(omega);
    float coeff = 2.0 * cosine;

    return coeff;
}

float processSamples(uint32_t buffer[], float coeff)
{
    int i;

    for (i = 0; i < 255; i++)
    {
          float Q0;
          Q0 = coeff * Q1 - Q2 + buffer[i];
          Q2 = Q1;
          Q1 = Q0;
    }

    float result;
    result = Q1 * Q1 + Q2 * Q2 - Q1 * Q2 * coeff;
    return result;

}

void ADC3IntHandler(void)
{
   ADCIntClear(ADC0_BASE, 3);

    if (flag == 0)
    {
        GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_0, GPIO_PIN_0);
        flag = 1;
    }
    else
    {
        GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_0, 0);
        flag = 0;
    }

    if (index >= 255)
    {
        index = 0;
        float coeff = initGoertzel();
        processSamples(buffer, coeff);
        resetGoertzel();
    }

    ADCSequenceDataGet(ADC0_BASE, 3, (buffer + index));
    index++;
}

int main(void)
{
    //i still don't understand how the clocking works (28)
    SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_25MHZ);

    //Enable ADC0
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    SysCtlPeripheralReset(SYSCTL_PERIPH_ADC0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);

    //pin for testing
    GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_0);

    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
    TimerClockSourceSet(TIMER0_BASE, TIMER_CLOCK_SYSTEM);
    //SysClock running at 16 MHz / 400 = 40 kHz sampling rate
    TimerLoadSet(TIMER0_BASE, TIMER_A, 625);
    TimerControlTrigger(TIMER0_BASE, TIMER_A, true);

    //Disable before configuring
    ADCSequenceDisable(ADC0_BASE, 3);

    //Configure Sequence 3 - use TRIGGER TIMER
    ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_TIMER, 0);

    //Configure first step interrupt - note this only works if ADC_CTL are ordered correctly, e.g. interrupt | end | ch0
    ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_CH0 | ADC_CTL_IE | ADC_CTL_END);

    //sequencer 3 on PE3
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);

    //PE3 ADC pin
    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_3);

    IntMasterEnable();
    ADCIntClear(ADC0_BASE, 3);
    //interrupt does not work without this, not sure why (needs tm4c123 header at top)
    IntEnable(INT_ADC0SS3);
    ADCIntEnable(ADC0_BASE, 3);
    ADCSequenceEnable(ADC0_BASE, 3);
    SysCtlDelay(10);
    TimerEnable(TIMER0_BASE, TIMER_A);

    ADCIntClear(ADC0_BASE, 3);

    while (1) {}

	return 0;
}
