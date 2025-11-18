//#include "xil_printf.h"
//#include "sleep.h"
//#include <stdint.h>
//#include <stdbool.h>
//#include "driver/btn/btn.h"
//#include "xparameters.h"
//#include "xil_io.h" // inbyte
//
//// ===================================================
//// I2C 드라이버 및 정의
//// ===================================================
//
//#define I2C_BASE_ADDR XPAR_I2C_MASTER_0_S00_AXI_BASEADDR
//
//// C 코드에서 본 I2C 레지스터 구조
//typedef struct {
//    volatile uint32_t CR;        // 0x00: Control
//    volatile uint32_t WDATA;     // 0x04: Write Data
//    volatile uint32_t SR;        // 0x08: Status
//    volatile uint32_t DATA1;     // 0x0C: Read Data 1
//    volatile uint32_t DATA2;     // 0x10: Read Data 2
//    volatile uint32_t DATA3;     // 0x14: Read Data 3
//    volatile uint32_t DATA4;     // 0x18: Read Data 4
//} I2C_Typedef;
//
//I2C_Typedef* I2C = (I2C_Typedef*) I2C_BASE_ADDR;
//
//// CR 레지스터 비트
//#define CR_I2C_EN (1 << 0)
//#define CR_STOP   (1 << 1)
//#define CR_START  (1 << 2)
//// SR 레지스터 비트
//#define SR_READY  (1 << 0)
//
//// I2C 통신 프로토콜 (Slave와 약속)
//#define SLAVE_ADDR_WRITE 0xAA
//#define SLAVE_ADDR_READ  0xAB
//
//// --- Slave 명령어 정의 ---
//#define CMD_FND_WRITE 0x01
//#define CMD_SAVE_R1_A 0x11 // R1 (Score, Init1)
//#define CMD_SAVE_R1_B 0x12 // R1 (Init2, Init3)
//#define CMD_SAVE_R2_A 0x21
//#define CMD_SAVE_R2_B 0x22
//#define CMD_SAVE_R3_A 0x31
//#define CMD_SAVE_R3_B 0x32
//#define CMD_SAVE_R4_A 0x41
//#define CMD_SAVE_R4_B 0x42
//#define CMD_LOAD_P1 0xA1 // R1(S_H, S_L, I1, I2)
//#define CMD_LOAD_P2 0xA2 // R1(I3), R2(S_H, S_L, I1)
//#define CMD_LOAD_P3 0xA3 // R2(I2, I3), R3(S_H, S_L)
//#define CMD_LOAD_P4 0xA4 // R3(I1, I2, I3), R4(S_H)
//#define CMD_LOAD_P5 0xA5 // R4(S_L, I1, I2, I3)
//
//// ===================================================
//// 랭킹 데이터 구조체
//// ===================================================
//struct RankEntry {
//    uint16_t score;
//    char initials[3];
//};
//
//// C 코드가 관리할 랭킹 배열 (전역 변수)
//struct RankEntry rankings[4];
//
//
//// ===================================================
//// 게임 상수 정의
//// ===================================================
//#define PLAYER_CHAR '0'
//#define OBSTACLE_CHAR '#'
//#define SCREEN_WIDTH 40
//#define PLAYER_X_POS 10
//#define GROUND_Y_POS 20
//#define GRAVITY      1
//#define JUMP_FORCE   3
//#define FRAME_RATE   30000 // 40ms
//
//// PAUSE 메시지 위치
//#define PAUSE_MSG_Y 6
//#define PAUSE_MSG_X 13
//// 점수 표시 위치
//#define SCORE_MSG_Y 1
//#define SCORE_MSG_X 1
//
//// --- 전역 버튼 변수 ---
//hButton jumpButton;
//hButton pauseButton;
//hButton startButton;
//
//// ===================================================
//// I2C 드라이버 함수
//// ===================================================
//
//void i2c_wait_ready() {
//    // ready 비트가 1이 될 때까지 대기
//    while (!(I2C->SR & SR_READY));
//}
//
//void i2c_start() {
//    i2c_wait_ready();
//    usleep(1000);
//    I2C->CR = (CR_START | CR_I2C_EN); // 0x05
//    I2C->CR = 0x00;
//    i2c_wait_ready();
//}
//
//void i2c_stop() {
//    i2c_wait_ready();
//    usleep(1000);
//    I2C->CR = (CR_STOP | CR_I2C_EN); // 0x03
//    I2C->CR = 0x00;
//    i2c_wait_ready();
//}
//
//void i2c_write_byte(uint8_t data) {
//    i2c_wait_ready();
//    usleep(1000);
//    I2C->WDATA = data;
//    I2C->CR = CR_I2C_EN; // 0x01
//    I2C->CR = 0x00;
//    i2c_wait_ready();
//}
//
//void i2c_write_packet(uint8_t cmd, uint8_t d1, uint8_t d2, uint8_t d3) {
//    i2c_start();
//    i2c_write_byte(SLAVE_ADDR_WRITE);
//    i2c_write_byte(cmd); // Command
//    i2c_write_byte(d1);  // Data 1
//    i2c_write_byte(d2);  // Data 2
//    i2c_write_byte(d3);  // Data 3
//    i2c_stop();
//}
//
//void i2c_load_and_read_4_bytes(uint8_t load_cmd, uint32_t* buffer) {
//    // 1. "Write" 명령 전송
//    i2c_write_packet(load_cmd, 0, 0, 0);
//
//    // Write와 Read 동작 사이에 딜레이 (Slave가 Read 버퍼 준비할 시간)
//    usleep(1000);
//
//    // 2. "Read" 수행
//    i2c_start();
//    i2c_write_byte(SLAVE_ADDR_READ); // Read 주소 전송
//
//    // 3. 읽기 시작 (CR 0x07)
//    i2c_wait_ready();
//    usleep(1000);
//    I2C->CR = (CR_START | CR_STOP | CR_I2C_EN); // 0x07
//    I2C->CR = 0x00;
//
//    // 4. FSM이 4바이트를 다 읽고 HOLD 상태로 돌아올 때까지 대기
//    i2c_wait_ready();
//
//    // 5. DATA 레지스터 읽기
//    buffer[0] = I2C->DATA1;
//    buffer[1] = I2C->DATA2;
//    buffer[2] = I2C->DATA3;
//    buffer[3] = I2C->DATA4;
//
//    // 6. 정식으로 STOP
//    i2c_stop();
//}
//
//// ===================================================
//// 랭킹 및 터미널 함수
//// ===================================================
//
//void set_cursor_pos(int y, int x) {
//    xil_printf("\033[%d;%dH", y, x);
//}
//
//void clear_screen() {
//    xil_printf("\033[2J\033[H");
//}
//
//void hide_cursor() {
//    xil_printf("\033[?25l");
//}
//
//void show_cursor() {
//    xil_printf("\033[?25h");
//}
//
//void get_initials(char* buffer) {
//    set_cursor_pos(PAUSE_MSG_Y + 4, PLAYER_X_POS - 7);
//    xil_printf("Enter 3 Initials: ");
//
//	int i;
//	for (i = 0; i < 3; i++) {
//		char c;
//		while (1) { // 유효한 문자가 입력될 때까지 무한 반복
//			c = inbyte(); // 1. 키보드에서 한 글자 입력받기 (Blocking)
//
//			// 2. 영문 대/소문자 (A-Z, a-z) 또는 숫자 (0-9)인지 확인
//			if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9')) {
//				buffer[i] = c;        // 3. 유효하면 버퍼에 저장
//				xil_printf("%c", c);    // 4. 터미널에 해당 문자 표시 (Echo)
//				break;                // 5. 다음 글자 입력을 위해 while 루프 탈출
//			}
//			// 3-b. 유효하지 않은 문자(백스페이스, 엔터, 한글 등)는 무시하고
//			//      while 루프가 반복되어 다음 입력을 기다림.
//		}
//	}
//}
//
//void load_all_rankings() {
//    uint32_t buf[4]; // 4바이트 임시 버퍼
//
//    i2c_load_and_read_4_bytes(CMD_LOAD_P1, buf);
//    rankings[0].score = (buf[0] << 8) | buf[1];
//    rankings[0].initials[0] = buf[2];
//    rankings[0].initials[1] = buf[3];
//
//    i2c_load_and_read_4_bytes(CMD_LOAD_P2, buf);
//    rankings[0].initials[2] = buf[0];
//    rankings[1].score = (buf[1] << 8) | buf[2];
//    rankings[1].initials[0] = buf[3];
//
//    i2c_load_and_read_4_bytes(CMD_LOAD_P3, buf);
//    rankings[1].initials[1] = buf[0];
//    rankings[1].initials[2] = buf[1];
//    rankings[2].score = (buf[2] << 8) | buf[3];
//
//    i2c_load_and_read_4_bytes(CMD_LOAD_P4, buf);
//    rankings[2].initials[0] = buf[0];
//    rankings[2].initials[1] = buf[1];
//    rankings[2].initials[2] = buf[2];
//    rankings[3].score = (buf[3] << 8) | 0x00; // P4는 4바이트만 가져옴
//
//    i2c_load_and_read_4_bytes(CMD_LOAD_P5, buf);
//    rankings[3].score |= buf[0]; // P5에서 Score_L 마저 채움
//    rankings[3].initials[0] = buf[1];
//    rankings[3].initials[1] = buf[2];
//    rankings[3].initials[2] = buf[3];
//}
//
//void save_all_rankings() {
//    uint8_t s_h, s_l;
//
//    s_h = (rankings[0].score >> 8) & 0xFF; s_l = rankings[0].score & 0xFF;
//    i2c_write_packet(CMD_SAVE_R1_A, s_h, s_l, rankings[0].initials[0]);
//    i2c_write_packet(CMD_SAVE_R1_B, rankings[0].initials[1], rankings[0].initials[2], 0);
//
//    s_h = (rankings[1].score >> 8) & 0xFF; s_l = rankings[1].score & 0xFF;
//    i2c_write_packet(CMD_SAVE_R2_A, s_h, s_l, rankings[1].initials[0]);
//    i2c_write_packet(CMD_SAVE_R2_B, rankings[1].initials[1], rankings[1].initials[2], 0);
//
//    s_h = (rankings[2].score >> 8) & 0xFF; s_l = rankings[2].score & 0xFF;
//    i2c_write_packet(CMD_SAVE_R3_A, s_h, s_l, rankings[2].initials[0]);
//    i2c_write_packet(CMD_SAVE_R3_B, rankings[2].initials[1], rankings[2].initials[2], 0);
//
//    s_h = (rankings[3].score >> 8) & 0xFF; s_l = rankings[3].score & 0xFF;
//    i2c_write_packet(CMD_SAVE_R4_A, s_h, s_l, rankings[3].initials[0]);
//    i2c_write_packet(CMD_SAVE_R4_B, rankings[3].initials[1], rankings[3].initials[2], 0);
//}
//
//void update_ranking_logic(uint16_t new_score, char* new_initials) {
//    int i, j;
//    int insert_pos = -1;
//
//    for (i = 0; i < 4; i++) {
//        if (new_score > rankings[i].score) {
//            insert_pos = i;
//            break;
//        }
//    }
//    if (insert_pos == -1) { return; }
//
//    for (j = 3; j > insert_pos; j--) {
//        rankings[j] = rankings[j - 1];
//    }
//
//    rankings[insert_pos].score = new_score;
//    rankings[insert_pos].initials[0] = new_initials[0];
//    rankings[insert_pos].initials[1] = new_initials[1];
//    rankings[insert_pos].initials[2] = new_initials[2];
//}
//
//uint8_t get_random_lfsr() {
//    static uint8_t lfsr = 0xACE1u % 255 + 1;
//    uint8_t bit = ((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 4)) & 1;
//    lfsr = (lfsr >> 1) | (bit << 7);
//    return lfsr;
//}
//
//// ===================================================
//// MAIN 함수
//// ===================================================
//
//int main() {
//
//    Button_Init(&jumpButton, BUTTON_GPIO, BUTTON_U);
//    Button_Init(&pauseButton, BUTTON_GPIO, BUTTON_R);
//    Button_Init(&startButton, BUTTON_GPIO, BUTTON_L);
//
//    // --- 게임 변수 초기화 ---
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
//    // --- 게임 시작 (화면 그리기) ---
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
//    // --- [수정] I2C 랭킹 읽어오기 ---
//    set_cursor_pos(2, SCORE_MSG_X);
//    xil_printf("Loading Rankings...");
//
//    load_all_rankings(); // Slave -> C 배열로 랭킹 로드
//
//    // C 배열의 랭킹을 UART에 표시
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
//    // --- [추가] 게임 시작 대기 ---
//    set_cursor_pos(PAUSE_MSG_Y, PAUSE_MSG_X - 1); // "== PAUSED ==" 위치 근처
//    xil_printf("Press BTN_L to Start");
//
//    // L 버튼이 눌릴 때까지(ACT_PUSHED) 대기
//    while (Button_getState(&startButton) != ACT_PUSHED) {
//        usleep(10000); // 10ms
//    }
//
//    // "Press BTN_L to Start" 메시지 지우기
//    set_cursor_pos(PAUSE_MSG_Y, PAUSE_MSG_X - 1);
//    xil_printf("                     ");
//    // --- 시작 대기 끝 ---
//
//    // --- 메인 게임 루프 ---
//    while(1) {
//
//        int pause_action = Button_getState(&pauseButton);
//        int jump_action = Button_getState(&jumpButton);
//
//        // --- 1. 입력 (Pause) ---
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
//                xil_printf("\033[K"); // [추가]
//                xil_printf("1. %04d %c%c%c", rankings[0].score, rankings[0].initials[0], rankings[0].initials[1], rankings[0].initials[2]);
//                set_cursor_pos(3, SCORE_MSG_X);
//                xil_printf("\033[K"); // [추가]
//                xil_printf("2. %04d %c%c%c", rankings[1].score, rankings[1].initials[0], rankings[1].initials[1], rankings[1].initials[2]);
//                set_cursor_pos(4, SCORE_MSG_X);
//                xil_printf("\033[K"); // [추가]
//                xil_printf("3. %04d %c%c%c", rankings[2].score, rankings[2].initials[0], rankings[2].initials[1], rankings[2].initials[2]);
//                set_cursor_pos(5, SCORE_MSG_X);
//                xil_printf("\033[K"); // [추가]
//                xil_printf("4. %04d %c%c%c", rankings[3].score, rankings[3].initials[0], rankings[3].initials[1], rankings[3].initials[2]);
//            }
//            continue;
//        }
//
//        // --- 3. 게임 로직 & 렌더링 (Pause가 아닐 때만 실행) ---
//        if (!is_paused) {
//
//            // (A) 입력 (Jump)
//            if (jump_action == ACT_PUSHED && on_ground) {
//                player_vel_y = JUMP_FORCE;
//                on_ground = false;
//                jump_frame = 0;
//            }
//
//            // (B) 플레이어 물리 업데이트
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
//                // 실시간 FND
//                i2c_write_packet(CMD_FND_WRITE, score & 0xFF , 0, 0);
//
//                int new_height = (get_random_lfsr() % 5) + 1; // 1~5
//                obstacle_y = (GROUND_Y_POS + 1) - new_height;
//                obstacle_width = (get_random_lfsr() % 2) + 1; // 1 or 2
//
//                set_cursor_pos(SCORE_MSG_Y, SCORE_MSG_X + 7);
//                xil_printf("%04d", score);
//            }
//
//            // (E) 그리기 (Render)
//            set_cursor_pos(player_y_old, PLAYER_X_POS); xil_printf(" ");
//            set_cursor_pos(player_y, PLAYER_X_POS); xil_printf("%c", PLAYER_CHAR);
//
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
//            // (D) 충돌 감지 (Collision Check)
//            bool x_collides = (PLAYER_X_POS >= obstacle_x) && (PLAYER_X_POS < (obstacle_x + obstacle_width));
//            bool y_collides = (player_y >= obstacle_y);
//
//            if (x_collides && y_collides) {
//
//                if (!game_over) { // 게임오버 '첫 순간'
//
//                    // 1. GAME OVER 메시지 표시
//                    set_cursor_pos(PAUSE_MSG_Y, PLAYER_X_POS);
//                    xil_printf("== GAME OVER ==");
//                    set_cursor_pos(PAUSE_MSG_Y + 2, PLAYER_X_POS - 5);
//                    xil_printf("(Press Jump to Restart)");
//
//                    // 2. FND에 최종 점수 1회 전송
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
//
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
//
//    return 0;
//}
