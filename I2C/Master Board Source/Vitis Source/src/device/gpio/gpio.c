#include "gpio.h"

void GPIO_Init(GPIO_TypeDef *gpio, uint8_t dir){
	gpio->CR = dir;
}
void GPIO_Write(GPIO_TypeDef *gpio, uint8_t data){
	gpio->ODR = data;
}
uint8_t GPIO_Read(GPIO_TypeDef *gpio){
	return gpio->IDR;
}
void GPIO_Set(GPIO_TypeDef *gpio, uint8_t pinNum){
	gpio->ODR |= 1 << pinNum; // pinNum 위치만 1로
}
void GPIO_Reset(GPIO_TypeDef *gpio, uint8_t pinNum){
	gpio->ODR &= ~(1 << pinNum); // pinNum 위치만 0으로
}
void GPIO_Toggle(GPIO_TypeDef *gpio, uint8_t pinNum){
	gpio->ODR ^= (1<<pinNum);
}
uint8_t GPIO_ReadPin(GPIO_TypeDef *gpio, int pinNum) {
    if ((gpio->IDR & (1 << pinNum)) != 0) {
        return 1;
    } else {
        return 0;
    }
}
