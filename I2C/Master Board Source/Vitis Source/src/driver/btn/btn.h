/*
 * button.h
 *
 *  Created on: 2025. 11. 5.
 *      Author: kccistc
 */

#ifndef SRC_DRIVER_BTN_BUTTON_H_
#define SRC_DRIVER_BTN_BUTTON_H_

#include <stdint.h>
#include "../../device/gpio/gpio.h"
#include "sleep.h"

#define BUTTON_GPIO		GPIOA
#define BUTTON_U		GPIO_PIN_0
#define BUTTON_L		GPIO_PIN_1
#define BUTTON_R		GPIO_PIN_2
#define BUTTON_D		GPIO_PIN_3

enum {RELEASED = 0, PUSHED};
enum {ACT_PUSHED = 0, ACT_RELEASED, NO_ACT};

typedef struct {
	GPIO_TypeDef *gpio;
	int pinNum;
	int prevState;
}hButton;

int Button_getState();
void Button_Init(hButton *btn, GPIO_TypeDef *gpio, uint8_t pinNum);

#endif /* SRC_DRIVER_BTN_BUTTON_H_ */
