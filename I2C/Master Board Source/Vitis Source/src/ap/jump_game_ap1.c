///*
// * jump_game_ap.c
// *
// *  Created on: 2025. 11. 16.
// *      Author: heoboss
// */
//
//#include "jump_game_ap.h"
//#include "xil_printf.h"
//#include "sleep.h"
//#include <stdint.h>
//#include <stdbool.h>
//#include "../driver/btn/btn.h"
//#include "xparameters.h"
//#include "xil_io.h"       // inbyte
//#include "../driver/ranking/ranking.h" // 랭킹 기능
//#include "../device/i2c/i2c.h"         // FND 쓰기 기능
//
//
//// ===================================================
//// 게임 상수 정의
//// ===================================================
//// ... (PLAYER_CHAR, SCREEN_WIDTH 등 정의는 동일) ...
//#define PLAYER_CHAR '0'
//#define OBSTACLE_CHAR '#'
//#define SCREEN_WIDTH 40
//#define PLAYER_X_POS 10
//#define GROUND_Y_POS 20
//#define GRAVITY      1
//#define JUMP_FORCE   3
//#define FRAME_RATE   40000 // 40ms (usleep)
//
//// PAUSE 메시지 위치
//#define PAUSE_MSG_Y 6
//#define PAUSE_MSG_X 13
//// 점수 표시 위치
//#define SCORE_MSG_Y 1
//#define SCORE_MSG_X 1
//
//
//// --- 전역 버튼 변수 ---
//static hButton jumpButton;
//static hButton pauseButton;
//static hButton startButton;
//
//// ===================================================
//// 내부 헬퍼 함수 (static)
//// ===================================================
//
//static void set_cursor_pos(int y, int x) {
//// ... (이하 모든 static 함수들:
//// clear_screen, hide_cursor, show_cursor,
//// get_initials, get_random_lfsr 는 동일) ...
//// ... (생략) ...
//    xil_printf("\033[%d;%dH", y, x);
//}
//
//static void clear_screen() {
//    xil_printf("\033[2J\033[H");
//}
//
//static void hide_cursor() {
//    xil_printf("\033[?25l");
//}
//
//static void show_cursor() {
//    xil_printf("\033[?25h");
//}
//
//static void get_initials(char* buffer) {
//    set_cursor_pos(PAUSE_MSG_Y + 4, PLAYER_X_POS - 7);
//    xil_printf("Enter 3 Initials: ");
//
//    int i;
//    for (i = 0; i < 3; i++) {
//        char c;
//        while (1) {
//            c = inbyte();
//            if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9')) {
//                buffer[i] = c;
//                xil_printf("%c", c);
//                break;
//            }
//        }
//    }
//}
//
//static uint8_t get_random_lfsr() {
//    static uint8_t lfsr = 0xACE1u % 255 + 1;
//    uint8_t bit = ((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 4)) & 1;
//    lfsr = (lfsr >> 1) | (bit << 7);
//    return lfsr;
//}
//
//
//// ===================================================
//// 메인 어플리케이션 함수
//// ===================================================
//
//void jump_game_main(void) {
//
//    Button_Init(&jumpButton, BUTTON_GPIO, BUTTON_U);
//    Button_Init(&pauseButton, BUTTON_GPIO, BUTTON_R);
//    Button_Init(&startButton, BUTTON_GPIO, BUTTON_L);
//
//    // --- 게임 변수 초기화 ---
//    // ... (player_y, score 등 모든 게임 변수 초기화 동일) ...
//    int player_y = GROUND_Y_POS;
//    int player_y_old = GROUND_Y_POS;
//    int player_vel_y = 0;
//    int obstacle_x = SCREEN_WIDTH - 5;
//    int obstacle_x_old = SCREEN_WIDTH - 5;
//    int obstacle_y = GROUND_Y_POS;
//    int obstacle_y_old = GROUND_Y_POS;
//    int obstacle_width = 1;
//    int obstacle_width_old = 1;
//
//    int score = 0;
//    int frame_counter = 0;
//    int jump_frame = 0;
//
//    bool game_over = false;
//    bool on_ground = true;
//    bool is_paused = false;
//
//
//    // --- 게임 시작 (화면 그리기) ---
//    // ... (clear_screen, 땅 그리기, 안내 메시지 등 동일) ...
//    clear_screen();
//    hide_cursor();
//
//    set_cursor_pos(GROUND_Y_POS + 1, 1); // 땅 그리기
//    for (int i = 0; i < SCREEN_WIDTH; i++) xil_printf("=");
//
//    set_cursor_pos(GROUND_Y_POS + 3, 1);
//    xil_printf("Jump Game: Press BTN_U to Jump, BTN_R to PAUSE");
//
//    set_cursor_pos(SCORE_MSG_Y, SCORE_MSG_X);
//    xil_printf("SCORE: %04d", score);
//
//
//    // --- I2C 랭킹 읽어오기 ---
//    set_cursor_pos(2, SCORE_MSG_X);
//    xil_printf("Loading Rankings...");
//
//    load_all_rankings(); // [호출] 랭킹 드라이버 함수
//
//    // C 배열의 랭킹을 UART에 표시
//    // ... (랭킹 표시 xil_printf 4줄 동일) ...
//    set_cursor_pos(2, SCORE_MSG_X);
//    xil_printf("\033[K");
//    xil_printf("1. %04d %c%c%c", rankings[0].score, rankings[0].initials[0], rankings[0].initials[1], rankings[0].initials[2]);
//    set_cursor_pos(3, SCORE_MSG_X);
//    xil_printf("\033[K");
//    xil_printf("2. %04d %c%c%c", rankings[1].score, rankings[1].initials[0], rankings[1].initials[1], rankings[1].initials[2]);
//    set_cursor_pos(4, SCORE_MSG_X);
//    xil_printf("\033[K");
//    xil_printf("3. %04d %c%c%c", rankings[2].score, rankings[2].initials[0], rankings[2].initials[1], rankings[2].initials[2]);
//    set_cursor_pos(5, SCORE_MSG_X);
//    xil_printf("\033[K");
//    xil_printf("4. %04d %c%c%c", rankings[3].score, rankings[3].initials[0], rankings[3].initials[1], rankings[3].initials[2]);
//
//
//    // --- 게임 시작 대기 ---
//    // ... (BTN_L 대기 로직 동일) ...
//    set_cursor_pos(PAUSE_MSG_Y, PAUSE_MSG_X - 1); // "== PAUSED ==" 위치 근처
//    xil_printf("Press BTN_L to Start");
//
//    while (Button_getState(&startButton) != ACT_PUSHED) {
//        usleep(10000); // 10ms
//    }
//
//    set_cursor_pos(PAUSE_MSG_Y, PAUSE_MSG_X - 1);
//    xil_printf("                     ");
//
//
//    // --- 메인 게임 루프 ---
//    while(1) {
//
//        // ... (pause_action, jump_action 동일) ...
//        int pause_action = Button_getState(&pauseButton);
//        int jump_action = Button_getState(&jumpButton);
//
//
//        // --- 1. 입력 (Pause) ---
//        // ... (Pause 로직 동일) ...
//        if (pause_action == ACT_PUSHED) {
//            if (game_over) continue;
//            is_paused = !is_paused;
//            if (is_paused) {
//                set_cursor_pos(PAUSE_MSG_Y, PAUSE_MSG_X);
//                xil_printf("== PAUSED ==");
//            } else {
//                set_cursor_pos(PAUSE_MSG_Y, PAUSE_MSG_X);
//                xil_printf("            ");
//            }
//        }
//
//        // --- 2. 게임 오버 상태 처리 ---
//        // ... (게임오버 리셋 로직 동일) ...
//        if (game_over) {
//            if (jump_action == ACT_PUSHED) {
//                // (모든 변수 초기화)
//                player_y = GROUND_Y_POS; player_y_old = GROUND_Y_POS;
//                player_vel_y = 0; on_ground = true;
//                obstacle_x = SCREEN_WIDTH - 5; obstacle_x_old = SCREEN_WIDTH - 5;
//                obstacle_y = GROUND_Y_POS; obstacle_y_old = GROUND_Y_POS;
//                obstacle_width = 1; obstacle_width_old = 1;
//                score = 0; game_over = false; frame_counter = 0;
//
//                // (화면 다시 그리기)
//                clear_screen(); hide_cursor();
//                set_cursor_pos(GROUND_Y_POS + 1, 1);
//                for (int i = 0; i < SCREEN_WIDTH; i++) xil_printf("=");
//                set_cursor_pos(GROUND_Y_POS + 3, 1);
//                xil_printf("Jump Game: Press BTN_U to Jump, BTN_R to PAUSE");
//                set_cursor_pos(SCORE_MSG_Y, SCORE_MSG_X);
//                xil_printf("SCORE: %04d", score);
//
//                // (랭킹 다시 표시)
//                set_cursor_pos(2, SCORE_MSG_X);
//                xil_printf("\033[K");
//                xil_printf("1. %04d %c%c%c", rankings[0].score, rankings[0].initials[0], rankings[0].initials[1], rankings[0].initials[2]);
//                set_cursor_pos(3, SCORE_MSG_X);
//                xil_printf("\033[K");
//                xil_printf("2. %04d %c%c%c", rankings[1].score, rankings[1].initials[0], rankings[1].initials[1], rankings[1].initials[2]);
//                set_cursor_pos(4, SCORE_MSG_X);
//                xil_printf("\033[K");
//                xil_printf("3. %04d %c%c%c", rankings[2].score, rankings[2].initials[0], rankings[2].initials[1], rankings[2].initials[2]);
//                set_cursor_pos(5, SCORE_MSG_X);
//                xil_printf("\033[K");
//                xil_printf("4. %04d %c%c%c", rankings[3].score, rankings[3].initials[0], rankings[3].initials[1], rankings[3].initials[2]);
//            }
//            continue;
//        }
//
//        // --- 3. 게임 로직 & 렌더링 ---
//        if (!is_paused) {
//
//            // ... (A, B: 점프 및 물리 로직 동일) ...
//            if (jump_action == ACT_PUSHED && on_ground) {
//                player_vel_y = JUMP_FORCE;
//                on_ground = false;
//                jump_frame = 0;
//            }
//
//            player_y_old = player_y;
//            if (!on_ground) {
//                player_y = player_y - player_vel_y;
//                jump_frame++;
//                if (jump_frame % 2 == 0) {
//                    player_vel_y = player_vel_y - GRAVITY;
//                }
//                if (player_y > GROUND_Y_POS) {
//                    player_y = GROUND_Y_POS;
//                    player_vel_y = 0;
//                    on_ground = true;
//                }
//            }
//
//
//            // (C) 장애물 업데이트
//            obstacle_x_old = obstacle_x;
//            obstacle_y_old = obstacle_y;
//            obstacle_width_old = obstacle_width;
//            obstacle_x--;
//
//            if (obstacle_x <= 0) {
//                obstacle_x = SCREEN_WIDTH - 5;
//                score++;
//
//                // [수정] 실시간 FND (저수준 I2C 함수 직접 호출)
//                i2c_write_packet(CMD_FND_WRITE, score & 0xFF , 0, 0);
//
//                int new_height = (get_random_lfsr() % 5) + 1;
//                obstacle_y = (GROUND_Y_POS + 1) - new_height;
//                obstacle_width = (get_random_lfsr() % 2) + 1;
//
//                set_cursor_pos(SCORE_MSG_Y, SCORE_MSG_X + 7);
//                xil_printf("%04d", score);
//            }
//
//            // (E) 그리기 (Render)
//            // ... (그리기 로직 동일) ...
//            set_cursor_pos(player_y_old, PLAYER_X_POS); xil_printf(" ");
//            set_cursor_pos(player_y, PLAYER_X_POS); xil_printf("%c", PLAYER_CHAR);
//            for (int w = 0; w < obstacle_width_old; w++) {
//                for (int y = obstacle_y_old; y <= GROUND_Y_POS; y++) {
//                    set_cursor_pos(y, obstacle_x_old + w);
//                    xil_printf(" ");
//                }
//            }
//            for (int w = 0; w < obstacle_width; w++) {
//                for (int y = obstacle_y; y <= GROUND_Y_POS; y++) {
//                    set_cursor_pos(y, obstacle_x + w);
//                    xil_printf("%c", OBSTACLE_CHAR);
//                }
//            }
//
//
//            // (D) 충돌 감지 (Collision Check)
//            bool x_collides = (PLAYER_X_POS >= obstacle_x) && (PLAYER_X_POS < (obstacle_x + obstacle_width));
//            bool y_collides = (player_y >= obstacle_y);
//
//            if (x_collides && y_collides) {
//                if (!game_over) { // 게임오버 '첫 순간'
//
//                    // 1. 메시지 표시
//                    set_cursor_pos(PAUSE_MSG_Y, PLAYER_X_POS);
//                    xil_printf("== GAME OVER ==");
//                    set_cursor_pos(PAUSE_MSG_Y + 2, PLAYER_X_POS - 5);
//                    xil_printf("(Press Jump to Restart)");
//
//                    // 2. FND에 최종 점수 전송
//                    i2c_write_packet(CMD_FND_WRITE, score & 0xFF, 0, 0);
//
//                    // 3. 랭킹 처리
//                    char initials[3] = {' ', ' ', ' '};
//                    get_initials(initials); // (Blocking) 이니셜 입력
//
//                    set_cursor_pos(PAUSE_MSG_Y + 5, PLAYER_X_POS - 7);
//                    xil_printf("Saving Ranking...");
//
//                    update_ranking_logic(score, initials); // C 배열 정렬
//                    save_all_rankings(); // C 배열 -> Slave 메모리 전송
//
//                    set_cursor_pos(PAUSE_MSG_Y + 5, PLAYER_X_POS - 7);
//                    xil_printf("Ranking Saved!   ");
//                }
//                game_over = true;
//            }
//
//            frame_counter++;
//        }
//
//        // --- 4. 딜레이 (Frame Rate) ---
//        usleep(FRAME_RATE);
//    }
//}
//
