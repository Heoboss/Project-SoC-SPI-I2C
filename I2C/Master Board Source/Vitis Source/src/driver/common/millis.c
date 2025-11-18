/*
 * millis.c
 *
 *  Created on: 2025. 11. 5.
 *      Author: kccistc
 */
#include "millis.h"

uint32_t m_counter = 0;

void incMillis(){
	m_counter++;
}

void clearMillis(){
	m_counter = 0;
}

void setMillis(uint32_t t){
	m_counter = t;
}

uint32_t Millis(){
	return m_counter;
}

