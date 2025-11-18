/*
 * jump_game_ap.c
 *
 *  Created on: 2025. 11. 16.
 *      Author: heoboss
 */

#include "jump_game_ap.h"
#include "xil_printf.h"
#include "sleep.h"
#include <stdint.h>
#include <stdbool.h>
#include "../driver/btn/btn.h"
#include "xparameters.h"
#include "xil_io.h"
#include "../driver/ranking/ranking.h"
#include "../device/i2c/i2c.h"

// ===================================================
// ANSI 색상 코드 정의
// ===================================================
#define COLOR_RESET     "\033[0m"
#define COLOR_RED       "\033[1;31m"
#define COLOR_GREEN     "\033[1;32m"
#define COLOR_YELLOW    "\033[1;33m"
#define COLOR_BLUE      "\033[1;34m"
#define COLOR_MAGENTA   "\033[1;35m"
#define COLOR_CYAN      "\033[1;36m"
#define COLOR_WHITE     "\033[1;37m"

// 배경색이 있는 밝은 색상들
#define COLOR_BRIGHT_GREEN   "\033[1;92m"
#define COLOR_BRIGHT_YELLOW  "\033[1;93m"
#define COLOR_BRIGHT_BLUE    "\033[1;94m"
#define COLOR_BRIGHT_MAGENTA "\033[1;95m"
#define COLOR_BRIGHT_CYAN    "\033[1;96m"

// 배경색
#define BG_RED      "\033[41m"
#define BG_GREEN    "\033[42m"
#define BG_YELLOW   "\033[43m"
#define BG_BLUE     "\033[44m"
#define BG_MAGENTA  "\033[45m"
#define BG_CYAN     "\033[46m"

// ===================================================
// 게임 상수 정의
// ===================================================
#define PLAYER_CHAR 'O'  // 플레이어 캐릭터
#define OBSTACLE_CHAR '#'  // 블록 캐릭터
#define SCREEN_WIDTH 40
#define PLAYER_X_POS 10
#define GROUND_Y_POS 20
#define GRAVITY      1
#define JUMP_FORCE   3
#define FRAME_RATE   30000

#define PAUSE_MSG_Y 6
#define PAUSE_MSG_X 13
#define SCORE_MSG_Y 1
#define SCORE_MSG_X 1

static hButton jumpButton;
static hButton pauseButton;
static hButton startButton;

// ===================================================
// 내부 헬퍼 함수
// ===================================================

static void set_cursor_pos(int y, int x) {
    xil_printf("\033[%d;%dH", y, x);
}

static void clear_screen() {
    xil_printf("\033[2J\033[H");
}

static void hide_cursor() {
    xil_printf("\033[?25l");
}

static void show_cursor() {
    xil_printf("\033[?25h");
}

static void get_initials(char* buffer) {
    set_cursor_pos(PAUSE_MSG_Y + 4, PLAYER_X_POS - 7);
    xil_printf(COLOR_BRIGHT_YELLOW "Enter 3 Initials: " COLOR_RESET);

    int i;
    for (i = 0; i < 3; i++) {
        char c;
        while (1) {
            c = inbyte();
            if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9')) {
                buffer[i] = c;
                xil_printf(COLOR_BRIGHT_CYAN "%c" COLOR_RESET, c);
                break;
            }
        }
    }
}

static uint8_t get_random_lfsr() {
    static uint8_t lfsr = 0xACE1u % 255 + 1;
    uint8_t bit = ((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 4)) & 1;
    lfsr = (lfsr >> 1) | (bit << 7);
    return lfsr;
}

// ===================================================
// 메인 어플리케이션 함수
// ===================================================

void jump_game_main(void) {

    Button_Init(&jumpButton, BUTTON_GPIO, BUTTON_U);
    Button_Init(&pauseButton, BUTTON_GPIO, BUTTON_R);
    Button_Init(&startButton, BUTTON_GPIO, BUTTON_L);

    int player_y = GROUND_Y_POS;
    int player_y_old = GROUND_Y_POS;
    int player_vel_y = 0;
    int obstacle_x = SCREEN_WIDTH - 5;
    int obstacle_x_old = SCREEN_WIDTH - 5;
    int obstacle_y = GROUND_Y_POS;
    int obstacle_y_old = GROUND_Y_POS;
    int obstacle_width = 1;
    int obstacle_width_old = 1;

    int score = 0;
    int frame_counter = 0;
    int jump_frame = 0;

    bool game_over = false;
    bool on_ground = true;
    bool is_paused = false;

    // --- 게임 시작 화면 그리기 ---
    clear_screen();
    hide_cursor();

    // 컬러풀한 땅 그리기
    set_cursor_pos(GROUND_Y_POS + 1, 1);
    xil_printf(COLOR_GREEN);
    for (int i = 0; i < SCREEN_WIDTH; i++) xil_printf("=");
    xil_printf(COLOR_RESET);

    // 게임 안내 메시지
    set_cursor_pos(GROUND_Y_POS + 3, 1);
    xil_printf(COLOR_CYAN "Jump Game: " COLOR_YELLOW "BTN_U" COLOR_WHITE " to Jump, "
               COLOR_YELLOW "BTN_R" COLOR_WHITE " to PAUSE" COLOR_RESET);

    // 점수 표시
    set_cursor_pos(SCORE_MSG_Y, SCORE_MSG_X);
    xil_printf(COLOR_BRIGHT_MAGENTA "SCORE: " COLOR_BRIGHT_YELLOW "%04d" COLOR_RESET, score);

    // --- 랭킹 로드 및 표시 ---
    set_cursor_pos(2, SCORE_MSG_X);
    xil_printf(COLOR_BRIGHT_CYAN "Loading Rankings..." COLOR_RESET);

    load_all_rankings();

    // 컬러풀한 랭킹 표시
    set_cursor_pos(2, SCORE_MSG_X);
    xil_printf("\033[K");
    xil_printf(COLOR_YELLOW "1. " COLOR_BRIGHT_GREEN "%04d " COLOR_WHITE "%c%c%c" COLOR_RESET,
               rankings[0].score, rankings[0].initials[0], rankings[0].initials[1], rankings[0].initials[2]);

    set_cursor_pos(3, SCORE_MSG_X);
    xil_printf("\033[K");
    xil_printf(COLOR_YELLOW "2. " COLOR_BRIGHT_GREEN "%04d " COLOR_WHITE "%c%c%c" COLOR_RESET,
               rankings[1].score, rankings[1].initials[0], rankings[1].initials[1], rankings[1].initials[2]);

    set_cursor_pos(4, SCORE_MSG_X);
    xil_printf("\033[K");
    xil_printf(COLOR_YELLOW "3. " COLOR_BRIGHT_GREEN "%04d " COLOR_WHITE "%c%c%c" COLOR_RESET,
               rankings[2].score, rankings[2].initials[0], rankings[2].initials[1], rankings[2].initials[2]);

    set_cursor_pos(5, SCORE_MSG_X);
    xil_printf("\033[K");
    xil_printf(COLOR_YELLOW "4. " COLOR_BRIGHT_GREEN "%04d " COLOR_WHITE "%c%c%c" COLOR_RESET,
               rankings[3].score, rankings[3].initials[0], rankings[3].initials[1], rankings[3].initials[2]);

    // --- 게임 시작 대기 ---
    set_cursor_pos(PAUSE_MSG_Y, PAUSE_MSG_X - 1);
    xil_printf(COLOR_BRIGHT_CYAN "Press " COLOR_YELLOW "BTN_L" COLOR_BRIGHT_CYAN " to Start" COLOR_RESET);

    while (Button_getState(&startButton) != ACT_PUSHED) {
        usleep(10000);
    }

    set_cursor_pos(PAUSE_MSG_Y, PAUSE_MSG_X - 1);
    xil_printf("                          ");

    // --- 메인 게임 루프 ---
    while(1) {

        int pause_action = Button_getState(&pauseButton);
        int jump_action = Button_getState(&jumpButton);

        // --- Pause 처리 ---
        if (pause_action == ACT_PUSHED) {
            if (game_over) continue;
            is_paused = !is_paused;
            if (is_paused) {
                set_cursor_pos(PAUSE_MSG_Y, PAUSE_MSG_X);
                xil_printf(COLOR_BRIGHT_YELLOW "== PAUSED ==" COLOR_RESET);
            } else {
                set_cursor_pos(PAUSE_MSG_Y, PAUSE_MSG_X);
                xil_printf("            ");
            }
        }

        // --- 게임 오버 처리 ---
        if (game_over) {
            if (jump_action == ACT_PUSHED) {
                player_y = GROUND_Y_POS; player_y_old = GROUND_Y_POS;
                player_vel_y = 0; on_ground = true;
                obstacle_x = SCREEN_WIDTH - 5; obstacle_x_old = SCREEN_WIDTH - 5;
                obstacle_y = GROUND_Y_POS; obstacle_y_old = GROUND_Y_POS;
                obstacle_width = 1; obstacle_width_old = 1;
                score = 0; game_over = false; frame_counter = 0;

                clear_screen(); hide_cursor();

                set_cursor_pos(GROUND_Y_POS + 1, 1);
                xil_printf(COLOR_GREEN);
                for (int i = 0; i < SCREEN_WIDTH; i++) xil_printf("=");
                xil_printf(COLOR_RESET);

                set_cursor_pos(GROUND_Y_POS + 3, 1);
                xil_printf(COLOR_CYAN "Jump Game: " COLOR_YELLOW "BTN_U" COLOR_WHITE " to Jump, "
                           COLOR_YELLOW "BTN_R" COLOR_WHITE " to PAUSE" COLOR_RESET);

                set_cursor_pos(SCORE_MSG_Y, SCORE_MSG_X);
                xil_printf(COLOR_BRIGHT_MAGENTA "SCORE: " COLOR_BRIGHT_YELLOW "%04d" COLOR_RESET, score);

                // 랭킹 재표시
                set_cursor_pos(2, SCORE_MSG_X);
                xil_printf("\033[K");
                xil_printf(COLOR_YELLOW "1. " COLOR_BRIGHT_GREEN "%04d " COLOR_WHITE "%c%c%c" COLOR_RESET,
                           rankings[0].score, rankings[0].initials[0], rankings[0].initials[1], rankings[0].initials[2]);

                set_cursor_pos(3, SCORE_MSG_X);
                xil_printf("\033[K");
                xil_printf(COLOR_YELLOW "2. " COLOR_BRIGHT_GREEN "%04d " COLOR_WHITE "%c%c%c" COLOR_RESET,
                           rankings[1].score, rankings[1].initials[0], rankings[1].initials[1], rankings[1].initials[2]);

                set_cursor_pos(4, SCORE_MSG_X);
                xil_printf("\033[K");
                xil_printf(COLOR_YELLOW "3. " COLOR_BRIGHT_GREEN "%04d " COLOR_WHITE "%c%c%c" COLOR_RESET,
                           rankings[2].score, rankings[2].initials[0], rankings[2].initials[1], rankings[2].initials[2]);

                set_cursor_pos(5, SCORE_MSG_X);
                xil_printf("\033[K");
                xil_printf(COLOR_YELLOW "4. " COLOR_BRIGHT_GREEN "%04d " COLOR_WHITE "%c%c%c" COLOR_RESET,
                           rankings[3].score, rankings[3].initials[0], rankings[3].initials[1], rankings[3].initials[2]);
            }
            continue;
        }

        // --- 게임 로직 ---
        if (!is_paused) {

            // 점프 처리
            if (jump_action == ACT_PUSHED && on_ground) {
                player_vel_y = JUMP_FORCE;
                on_ground = false;
                jump_frame = 0;
            }

            // 플레이어 물리
            player_y_old = player_y;
            if (!on_ground) {
                player_y = player_y - player_vel_y;
                jump_frame++;
                if (jump_frame % 2 == 0) {
                    player_vel_y = player_vel_y - GRAVITY;
                }
                if (player_y > GROUND_Y_POS) {
                    player_y = GROUND_Y_POS;
                    player_vel_y = 0;
                    on_ground = true;
                }
            }

            // 장애물 업데이트
            obstacle_x_old = obstacle_x;
            obstacle_y_old = obstacle_y;
            obstacle_width_old = obstacle_width;
            obstacle_x--;

            if (obstacle_x <= 0) {
                obstacle_x = SCREEN_WIDTH - 5;
                score++;

                i2c_write_packet(CMD_FND_WRITE, score & 0xFF , 0, 0);

                int new_height = (get_random_lfsr() % 5) + 1;
                obstacle_y = (GROUND_Y_POS + 1) - new_height;
                obstacle_width = (get_random_lfsr() % 2) + 1;

                set_cursor_pos(SCORE_MSG_Y, SCORE_MSG_X + 7);
                xil_printf(COLOR_BRIGHT_YELLOW "%04d" COLOR_RESET, score);
            }

            // 렌더링
            // 플레이어 지우기
            set_cursor_pos(player_y_old, PLAYER_X_POS);
            xil_printf(" ");

            // 플레이어 그리기 (점프 중이면 파란색, 땅에 있으면 노란색)
            set_cursor_pos(player_y, PLAYER_X_POS);
            if (on_ground) {
                xil_printf(COLOR_BRIGHT_YELLOW "%c" COLOR_RESET, PLAYER_CHAR);
            } else {
                xil_printf(COLOR_BRIGHT_CYAN "%c" COLOR_RESET, PLAYER_CHAR);
            }

            // 이전 장애물 지우기
            for (int w = 0; w < obstacle_width_old; w++) {
                for (int y = obstacle_y_old; y <= GROUND_Y_POS; y++) {
                    set_cursor_pos(y, obstacle_x_old + w);
                    xil_printf(" ");
                }
            }

            // 새 장애물 그리기 (빨간색)
            for (int w = 0; w < obstacle_width; w++) {
                for (int y = obstacle_y; y <= GROUND_Y_POS; y++) {
                    set_cursor_pos(y, obstacle_x + w);
                    xil_printf(COLOR_RED "%c" COLOR_RESET, OBSTACLE_CHAR);
                }
            }

            // 충돌 감지
            bool x_collides = (PLAYER_X_POS >= obstacle_x) && (PLAYER_X_POS < (obstacle_x + obstacle_width));
            bool y_collides = (player_y >= obstacle_y);

            if (x_collides && y_collides) {
                if (!game_over) {
                    // 게임 오버 메시지
                    set_cursor_pos(PAUSE_MSG_Y, PLAYER_X_POS - 2);
                    xil_printf(COLOR_RED "== GAME OVER ==" COLOR_RESET);

                    set_cursor_pos(PAUSE_MSG_Y + 2, PLAYER_X_POS - 7);
                    xil_printf(COLOR_CYAN "(Press Jump to Restart)" COLOR_RESET);

                    i2c_write_packet(CMD_FND_WRITE, score & 0xFF, 0, 0);

                    char initials[3] = {' ', ' ', ' '};
                    get_initials(initials);

                    set_cursor_pos(PAUSE_MSG_Y + 5, PLAYER_X_POS - 7);
                    xil_printf(COLOR_BRIGHT_MAGENTA "Saving Ranking..." COLOR_RESET);

                    update_ranking_logic(score, initials);
                    save_all_rankings();

                    set_cursor_pos(PAUSE_MSG_Y + 5, PLAYER_X_POS - 7);
                    xil_printf(COLOR_BRIGHT_GREEN "Ranking Saved!   " COLOR_RESET);
                }
                game_over = true;
            }

            frame_counter++;
        }

        usleep(FRAME_RATE);
    }
}
