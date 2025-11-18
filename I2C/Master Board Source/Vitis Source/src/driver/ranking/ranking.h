/*
 * ranking.h
 *
 *  Created on: 2025. 11. 16.
 *      Author: heoboss
 */
#ifndef DRIVER_RANKING_RANKING_H_
#define DRIVER_RANKING_RANKING_H_

#include <stdint.h>

// ===================================================
// 랭킹 데이터 구조체
// ===================================================
struct RankEntry {
    uint16_t score;
    char initials[3];
};

// C 코드가 관리할 랭킹 배열 (ranking.c에 실제 데이터가 있음)
extern struct RankEntry rankings[4];


// ===================================================
// 랭킹 관련 명령어 정의
// ===================================================
#define CMD_SAVE_R1_A 0x11 // R1 (Score, Init1)
#define CMD_SAVE_R1_B 0x12 // R1 (Init2, Init3)
#define CMD_SAVE_R2_A 0x21
#define CMD_SAVE_R2_B 0x22
#define CMD_SAVE_R3_A 0x31
#define CMD_SAVE_R3_B 0x32
#define CMD_SAVE_R4_A 0x41
#define CMD_SAVE_R4_B 0x42
#define CMD_LOAD_P1 0xA1 // R1(S_H, S_L, I1, I2)
#define CMD_LOAD_P2 0xA2 // R1(I3), R2(S_H, S_L, I1)
#define CMD_LOAD_P3 0xA3 // R2(I2, I3), R3(S_H, S_L)
#define CMD_LOAD_P4 0xA4 // R3(I1, I2, I3), R4(S_H)
#define CMD_LOAD_P5 0xA5 // R4(S_L, I1, I2, I3)


// ===================================================
// 공개 함수 프로토타입
// ===================================================

/**
 * @brief Slave로부터 20바이트 랭킹을 모두 읽어 C 전역 배열(rankings)에 저장
 */
void load_all_rankings(void);

/**
 * @brief C 전역 배열(rankings)의 내용을 Slave 메모리에 덮어쓰기
 */
void save_all_rankings(void);

/**
 * @brief 새 점수를 랭킹 배열에 삽입하고 정렬합니다. (I2C 통신 없음)
 */
void update_ranking_logic(uint16_t new_score, char* new_initials);


#endif /* DRIVER_RANKING_RANKING_H_ */
