/*
 * i2c.c
 *
 *  Created on: 2025. 11. 16.
 *      Author: heoboss
 */
#include "i2c.h"
#include "xparameters.h" // I2C_BASE_ADDR
#include "sleep.h"       // usleep

// ===================================================
// I2C 하드웨어 정의 (이 파일 안에서만 사용됨)
// ===================================================

#define I2C_BASE_ADDR XPAR_I2C_MASTER_0_S00_AXI_BASEADDR

I2C_Typedef* I2C = (I2C_Typedef*) I2C_BASE_ADDR;

// CR/SR 비트
#define CR_I2C_EN (1 << 0)
#define CR_STOP   (1 << 1)
#define CR_START  (1 << 2)
#define SR_READY  (1 << 0)

// Slave 주소
#define SLAVE_ADDR_WRITE 0xAA
#define SLAVE_ADDR_READ  0xAB

// ===================================================
// 내부 헬퍼 함수 (static)
// ===================================================

static void i2c_wait_ready() {
    while (!(I2C->SR & SR_READY));
}

static void i2c_start() {
    i2c_wait_ready();
    usleep(1000);
    I2C->CR = (CR_START | CR_I2C_EN); // 0x05
    I2C->CR = 0x00;
    i2c_wait_ready();
}

static void i2c_stop() {
    i2c_wait_ready();
    usleep(1000);
    I2C->CR = (CR_STOP | CR_I2C_EN); // 0x03
    I2C->CR = 0x00;
    i2c_wait_ready();
}

static void i2c_write_byte(uint8_t data) {
    i2c_wait_ready();
    usleep(1000);
    I2C->WDATA = data;
    I2C->CR = CR_I2C_EN; // 0x01
    I2C->CR = 0x00;
    i2c_wait_ready();
}

// ===================================================
// 공개 함수 구현
// ===================================================

void i2c_write_packet(uint8_t cmd, uint8_t d1, uint8_t d2, uint8_t d3) {
    i2c_start();
    i2c_write_byte(SLAVE_ADDR_WRITE);
    i2c_write_byte(cmd); // Command
    i2c_write_byte(d1);  // Data 1
    i2c_write_byte(d2);  // Data 2
    i2c_write_byte(d3);  // Data 3
    i2c_stop();
}

void i2c_load_and_read_4_bytes(uint8_t load_cmd, uint32_t* buffer) {
    // 1. "Write" 명령 전송
    i2c_write_packet(load_cmd, 0, 0, 0);

    // 2. Write/Read 사이 딜레이
    usleep(1000);

    // 3. "Read" 수행
    i2c_start();
    i2c_write_byte(SLAVE_ADDR_READ); // Read 주소 전송

    // 4. 읽기 시작
    i2c_wait_ready();
    usleep(1000);
    I2C->CR = (CR_START | CR_STOP | CR_I2C_EN); // 0x07
    I2C->CR = 0x00;

    // 5. 4바이트 수신 대기
    i2c_wait_ready();

    // 6. 데이터 읽기
    buffer[0] = I2C->DATA1;
    buffer[1] = I2C->DATA2;
    buffer[2] = I2C->DATA3;
    buffer[3] = I2C->DATA4;

    // 7. 정지
    i2c_stop();
}

