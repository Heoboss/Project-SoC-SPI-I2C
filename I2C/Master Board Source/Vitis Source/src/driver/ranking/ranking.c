/*
 * ranking.c
 *
 *  Created on: 2025. 11. 16.
 *      Author: heoboss
 */
#include "ranking.h"
#include "../../device/i2c/i2c.h" // 저수준 I2C 드라이버 포함

// ===================================================
// 전역 변수 정의
// ===================================================
struct RankEntry rankings[4]; // 랭킹 데이터의 실체

// ===================================================
// 공개 함수 구현
// ===================================================

void load_all_rankings() {
    uint32_t buf[4]; // 4바이트 임시 버퍼

    i2c_load_and_read_4_bytes(CMD_LOAD_P1, buf);
    rankings[0].score = (buf[0] << 8) | buf[1];
    rankings[0].initials[0] = buf[2];
    rankings[0].initials[1] = buf[3];

    i2c_load_and_read_4_bytes(CMD_LOAD_P2, buf);
    rankings[0].initials[2] = buf[0];
    rankings[1].score = (buf[1] << 8) | buf[2];
    rankings[1].initials[0] = buf[3];

    i2c_load_and_read_4_bytes(CMD_LOAD_P3, buf);
    rankings[1].initials[1] = buf[0];
    rankings[1].initials[2] = buf[1];
    rankings[2].score = (buf[2] << 8) | buf[3];

    i2c_load_and_read_4_bytes(CMD_LOAD_P4, buf);
    rankings[2].initials[0] = buf[0];
    rankings[2].initials[1] = buf[1];
    rankings[2].initials[2] = buf[2];
    rankings[3].score = (buf[3] << 8) | 0x00;

    i2c_load_and_read_4_bytes(CMD_LOAD_P5, buf);
    rankings[3].score |= buf[0];
    rankings[3].initials[0] = buf[1];
    rankings[3].initials[1] = buf[2];
    rankings[3].initials[2] = buf[3];
}

void save_all_rankings() {
    uint8_t s_h, s_l;

    s_h = (rankings[0].score >> 8) & 0xFF; s_l = rankings[0].score & 0xFF;
    i2c_write_packet(CMD_SAVE_R1_A, s_h, s_l, rankings[0].initials[0]);
    i2c_write_packet(CMD_SAVE_R1_B, rankings[0].initials[1], rankings[0].initials[2], 0);

    s_h = (rankings[1].score >> 8) & 0xFF; s_l = rankings[1].score & 0xFF;
    i2c_write_packet(CMD_SAVE_R2_A, s_h, s_l, rankings[1].initials[0]);
    i2c_write_packet(CMD_SAVE_R2_B, rankings[1].initials[1], rankings[1].initials[2], 0);

    s_h = (rankings[2].score >> 8) & 0xFF; s_l = rankings[2].score & 0xFF;
    i2c_write_packet(CMD_SAVE_R3_A, s_h, s_l, rankings[2].initials[0]);
    i2c_write_packet(CMD_SAVE_R3_B, rankings[2].initials[1], rankings[2].initials[2], 0);

    s_h = (rankings[3].score >> 8) & 0xFF; s_l = rankings[3].score & 0xFF;
    i2c_write_packet(CMD_SAVE_R4_A, s_h, s_l, rankings[3].initials[0]);
    i2c_write_packet(CMD_SAVE_R4_B, rankings[3].initials[1], rankings[3].initials[2], 0);
}

void update_ranking_logic(uint16_t new_score, char* new_initials) {
    int i, j;
    int insert_pos = -1;

    for (i = 0; i < 4; i++) {
        if (new_score > rankings[i].score) {
            insert_pos = i;
            break;
        }
    }
    if (insert_pos == -1) { return; }

    for (j = 3; j > insert_pos; j--) {
        rankings[j] = rankings[j - 1];
    }

    rankings[insert_pos].score = new_score;
    rankings[insert_pos].initials[0] = new_initials[0];
    rankings[insert_pos].initials[1] = new_initials[1];
    rankings[insert_pos].initials[2] = new_initials[2];
}

