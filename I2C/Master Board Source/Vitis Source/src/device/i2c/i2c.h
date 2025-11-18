/*
 * i2c.h
 *
 *  Created on: 2025. 11. 16.
 *      Author: heoboss
 */

#ifndef DEVICE_I2C_I2C_H_
#define DEVICE_I2C_I2C_H_

#include <stdint.h>

// ===================================================
// I2C 하드웨어 레지스터 정의
// ===================================================

typedef struct {
    volatile uint32_t CR;        // 0x00: Control
    volatile uint32_t WDATA;     // 0x04: Write Data
    volatile uint32_t SR;        // 0x08: Status
    volatile uint32_t DATA1;     // 0x0C: Read Data 1
    volatile uint32_t DATA2;     // 0x10: Read Data 2
    volatile uint32_t DATA3;     // 0x14: Read Data 3
    volatile uint32_t DATA4;     // 0x18: Read Data 4
} I2C_Typedef;

extern I2C_Typedef* I2C; // I2C 포인터 (i2c.c에 정의됨)

// ===================================================
// 기본 I2C 패킷 명령어 (저수준)
// ===================================================
#define CMD_FND_WRITE 0x01 // FND에 값을 쓰는 기본 명령어

// ===================================================
// 공개 함수 프로토타입 (저수준)
// ===================================================

/**
 * @brief I2C로 4바이트 커맨드 패킷을 전송합니다. (Blocking)
 */
void i2c_write_packet(uint8_t cmd, uint8_t d1, uint8_t d2, uint8_t d3);

/**
 * @brief Slave의 Read 버퍼를 로드하고, 그 4바이트를 읽어옵니다. (Blocking)
 */
void i2c_load_and_read_4_bytes(uint8_t load_cmd, uint32_t* buffer);


#endif /* DEVICE_I2C_I2C_H_ */
