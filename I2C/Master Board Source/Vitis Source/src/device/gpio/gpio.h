/*
 * gpio.h
 *
 *  Created on: 2025. 11. 4.
 *      Author: kccistc
 */

#ifndef SRC_DEVICE_GPIO_GPIO_H_
#define SRC_DEVICE_GPIO_GPIO_H_
#include "xparameters.h"
#include <stdint.h>
#define __IO volatile

typedef struct {
	__IO uint32_t CR;
	__IO uint32_t ODR;
	__IO uint32_t IDR;
}GPIO_TypeDef;

#define GPIO_PIN_0    0
#define GPIO_PIN_1    1
#define GPIO_PIN_2    2
#define GPIO_PIN_3    3
#define GPIO_PIN_4    4
#define GPIO_PIN_5    5
#define GPIO_PIN_6    6
#define GPIO_PIN_7    7

#define GPIO_0_BASEADDR XPAR_GPIO2_0_S00_AXI_BASEADDR
//#define GPIO_1_BASEADDR XPAR_GPIO2_1_S00_AXI_BASEADDR
//#define GPIO_2_BASEADDR XPAR_GPIO2_2_S00_AXI_BASEADDR

#define GPIOA	((GPIO_TypeDef *)GPIO_0_BASEADDR)
//#define GPIOB	((GPIO_TypeDef *)GPIO_1_BASEADDR)
//#define GPIOC	((GPIO_TypeDef *)GPIO_2_BASEADDR)

void GPIO_Init(GPIO_TypeDef *gpio, uint8_t dir);
void GPIO_Write(GPIO_TypeDef *gpio, uint8_t data);
uint8_t GPIO_Read(GPIO_TypeDef *gpio);
void GPIO_Set(GPIO_TypeDef *gpio, uint8_t pinNum);
void GPIO_Reset(GPIO_TypeDef *gpio, uint8_t pinNum);
void GPIO_Toggle(GPIO_TypeDef *gpio, uint8_t pinNum);
uint8_t GPIO_ReadPin(GPIO_TypeDef *gpio, int pinNum);
#endif /* SRC_DEVICE_GPIO_GPIO_H_ */
