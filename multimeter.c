#include "multimeter.h"
#include "Board_LED.h"
#include "M:\Design, Construction & Test\WORKING ADC\PB_LCD_Drivers.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

// Begin interrupt stuff
static volatile uint32_t ticks = 0;
void SysTick_Handler(void)
{
	ticks++;
}

void waitForATime(uint32_t howLongToWait)
{
	uint32_t whenStarted = ticks;
	while (ticks - whenStarted < howLongToWait);
}
// End interrupt stuff

int convertADCValue(uint16_t value)
{
	// Added typecasts, check if this solves the displaying 0 issue
	value = (int) ((float) value / 4096) * 3;
	return value;
}

double convertADCValue2(uint32_t value) 
{
	// Added typecasts, check if this solves the displaying 0 issue
	double con = 0;
	con = ((float) value / 4096) * 3.33;
	return con;
}

void AdcConfig(void)
{
	// Enable GPIO-C clock
	RCC->AHB1ENR = (RCC->AHB1ENR & ~RCC_AHB1ENR_GPIOCEN_Msk) | (0x1 << RCC_AHB1ENR_GPIOCEN_Pos); 
	// Enable ADC1 clock
	RCC->APB2ENR = (RCC->APB2ENR & ~RCC_APB2ENR_ADC1EN_Msk) | (0x1 << RCC_APB2ENR_ADC1EN_Pos);
	// Sets mode register 14 to analog mode
	GPIOC->MODER = (GPIOC->MODER & ~GPIO_MODER_MODER4_Msk) | (0x3 << GPIO_MODER_MODER4_Pos);
	// Set ADC to discontinuous mode
	ADC1->CR1 = (ADC1->CR1 & ~ADC_CR1_DISCEN_Msk) | (0x1 << ADC_CR1_DISCEN_Pos);
	// Sets end of conversion flag mode
	ADC1->CR2 = (ADC1->CR2 & ~ADC_CR2_EOCS_Msk) | (0x1 << ADC_CR2_EOCS_Pos);
	// Set ADC to perform one conversion before raising EOC flag
	ADC1->SQR1 = (ADC1->SQR1 & ~ADC_SQR1_L_Msk) | (0x0 << ADC_SQR1_L_Pos);
	// Set ADC to read input 14
	ADC1->SQR3 = (ADC1->SQR3 & ~ADC_SQR3_SQ1_Msk) | (0xE << ADC_SQR3_SQ1_Pos);
	// Enable ADC :D
	ADC1->CR2 = (ADC1->CR2 & ~ADC_CR2_ADON_Msk) | (0x1 << ADC_CR2_ADON_Pos);
}

void DacConfig(void)
{
	// Enable GPIO-A clock
	RCC->AHB1ENR = (RCC->AHB1ENR & ~RCC_AHB1ENR_GPIOAEN_Msk) | (0x1 << RCC_AHB1ENR_GPIOAEN_Pos);
	// Enable DAC1 clock
	RCC->APB1ENR = (RCC->APB1ENR & ~RCC_APB1ENR_DACEN_Msk) | (0x1 << RCC_APB1ENR_DACEN_Pos);
	// Sets mode register 14 to analog mode
	GPIOA->MODER = (GPIOA->MODER & ~GPIO_MODER_MODER4_Msk) | (0x3 << GPIO_MODER_MODER4_Pos);
	// Enable channel one of the DAC
	DAC1->CR = (DAC->CR & ~DAC_CR_EN1_Msk) | (0x1 << DAC_CR_EN1_Pos);
}

int main(void)
{
	// Initialise board
	SystemCoreClockUpdate();
	SysTick_Config(SystemCoreClock / 2);

	// Initialise LCD
	PB_LCD_Init();
	PB_LCD_Clear();	

	// Initialise ADC
	AdcConfig();
	DacConfig();

	// Declare internal variables to store readings
	uint32_t AdcValue;
	double AdcFinal;

	// Write the value of the ADC to the DAC
	DAC1->DHR12R1 = (DAC1->DHR12R1 & ~DAC_DHR12R1_DACC1DHR_Msk) | (/* 1 volt */ 0x3E8 << DAC_DHR12R1_DACC1DHR_Pos);

	while (true)
	{
		bool finished = false;
		// starts conversion of 'standard' channels
		ADC1->CR2 = (ADC1->CR2 & ~ADC_CR2_SWSTART_Msk) | (0x1 << ADC_CR2_SWSTART_Pos);
		while (!finished)
		{
			// Checks if end of conversion flag has been set
			if ((ADC1->SR & ADC_SR_EOC_Msk) == ADC_SR_EOC_Msk) 
			{
				// Retrieve converted value from data register
				AdcValue = ADC1->DR;
				
				// Converts from internal numerical value to actual value
				AdcFinal = convertADCValue2(AdcValue);

				// Wait 1 second between readings
				waitForATime(1);

				// Create a readable string to display on LCD
				PB_LCD_Clear();
				char VoltageValue[8];
				strcpy(VoltageValue, "00000000");
				snprintf(VoltageValue, 8, "%0.3f", AdcFinal);

				// Try out adding "Voltage: " to the start
				char FinalText[16] = "Voltage: ";
				strcat(FinalText, VoltageValue);

				// Write to LCD
				PB_LCD_WriteString(FinalText, (int) strlen(FinalText));

				// Mark conversion as finished
				finished = true;
			}	
		}
	}

}
