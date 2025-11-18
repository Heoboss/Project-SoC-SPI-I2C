module I2C_Slave (
    input clk,
    input reset,
    input SCL,
    inout SDA,
    //output [7:0] LED
    output [15:0] LED,
    output [7:0] slv_reg0,
    output [7:0] slv_reg1,
    output [7:0] slv_reg2,
    output [7:0] slv_reg3
);

    // 20 byte 랭킹 메모리
    reg [7:0] rank_memory[0:19];

    // FND 전용 1byte 레지스터
    reg [7:0] fnd_display_reg;

    // [추가] I2C Write 버퍼 (명령 1B + 데이터 3B)
    reg [7:0] i2c_buffer[0:3];

    // [추가] I2C Read 버퍼 (4바이트 전송용)
    reg [7:0] read_buffer[0:3];
    reg [7:0] last_command_reg;  // READ FSM이 참조할 마지막 명령어

    // [수정] 디버깅 및 FND 출력을 내부 레지스터에 연결
    assign slv_reg0 = fnd_display_reg;  // FND는 fnd_display_reg 값을 표시
    assign slv_reg1 = rank_memory[0];  // 디버깅용 (랭킹 1위 점수 H)
    assign slv_reg2 = rank_memory[1];  // 디버깅용 (랭킹 1위 점수 L)
    assign slv_reg3 = rank_memory[2];  // 디버깅용 (랭킹 1위 이니셜 1)

    parameter IDLE=0, ADDR=1, ACK=2, READ=3, DATA=4, READ_ACK=5, READ_CNT=6, DATA_ACK = 7, DATA_NACK=8, STOP=9;

    reg [3:0] state, state_next;
    reg [7:0] temp_rx_data_reg, temp_rx_data_next;
    reg [7:0] temp_tx_data_reg, temp_tx_data_next;
    reg [7:0] temp_addr_reg, temp_addr_next;
    reg [3:0] bit_counter_reg, bit_counter_next;
    reg [1:0] slv_count_reg, slv_count_next;
    reg en;
    reg o_data;
    reg read_ack_reg, read_ack_next;



    reg sclk_sync0, sclk_sync1;
    wire sclk_rising, sclk_falling;

    reg sda_sync0, sda_sync1;
    wire sda_rising, sda_falling;

    reg [15:0] led_reg, led_next;
    assign SDA = en ? o_data : 1'bz;
    assign LED = led_reg;



    always @(posedge clk or negedge reset) begin
        if (!reset) begin
            state <= IDLE;
            sclk_sync0 <= 1;
            sclk_sync1 <= 1;
            sda_sync0 <= 1;
            sda_sync1 <= 1;
            temp_rx_data_reg <= 0;
            temp_tx_data_reg <= 0;
            bit_counter_reg <= 0;
            temp_addr_reg <= 0;
            led_reg <= 0;
            read_ack_reg <= 1'bz;
        end else begin
            state <= state_next;
            sclk_sync0 <= SCL;
            sclk_sync1 <= sclk_sync0;
            sda_sync0 <= SDA;
            sda_sync1 <= sda_sync0;
            temp_rx_data_reg <= temp_rx_data_next;
            temp_tx_data_reg <= temp_tx_data_next;
            bit_counter_reg <= bit_counter_next;
            temp_addr_reg <= temp_addr_next;
            led_reg <= led_next;
            read_ack_reg <= read_ack_next;
        end
    end

    always @(posedge clk or negedge reset) begin
        if (!reset) begin
            slv_count_reg <= 0;
            fnd_display_reg <= 0;
            i2c_buffer[0] <= 0;
            i2c_buffer[1] <= 0;
            i2c_buffer[2] <= 0;
            i2c_buffer[3] <= 0;
            last_command_reg <= 0;
            // [추가] 랭킹 메모리 리셋 로직
            rank_memory[0] <= 8'h00;
            rank_memory[1] <= 8'h00;
            rank_memory[2] <= 8'h00;
            rank_memory[3] <= 8'h00;
            rank_memory[4] <= 8'h00;
            rank_memory[5] <= 8'h00;
            rank_memory[6] <= 8'h00;
            rank_memory[7] <= 8'h00;
            rank_memory[8] <= 8'h00;
            rank_memory[9] <= 8'h00;
            rank_memory[10] <= 8'h00;
            rank_memory[11] <= 8'h00;
            rank_memory[12] <= 8'h00;
            rank_memory[13] <= 8'h00;
            rank_memory[14] <= 8'h00;
            rank_memory[15] <= 8'h00;
            rank_memory[16] <= 8'h00;
            rank_memory[17] <= 8'h00;
            rank_memory[18] <= 8'h00;
            rank_memory[19] <= 8'h00;
        end else begin
            slv_count_reg   <= slv_count_next;
            fnd_display_reg <= fnd_display_reg;  // 기본값 유지

            // I2C Write 버퍼 업데이트 (DATA_ACK 상태에서)
            if (state_next == DATA_ACK) begin
                case (slv_count_reg)
                    2'd0: i2c_buffer[0] <= temp_rx_data_reg;  // Command
                    2'd1: i2c_buffer[1] <= temp_rx_data_reg;  // Data A
                    2'd2: i2c_buffer[2] <= temp_rx_data_reg;  // Data B
                    2'd3: i2c_buffer[3] <= temp_rx_data_reg;  // Data C
                endcase
            end

            // 명령어 실행 (DATA 상태에서)
            if (state == DATA && state_next == DATA_ACK && slv_count_reg == 3) begin
                last_command_reg <= i2c_buffer[0]; // READ용 마지막 명령어 저장

                case (i2c_buffer[0])  // 1번째 바이트 (Command)
                    // --- FND 실시간 쓰기 ---
                    8'h01:  // [명령] FND에 점수 표시
                    fnd_display_reg <= i2c_buffer[1];  // Data A (점수)

                    // --- 랭킹 1위 저장 ---
                    8'h11:  // [명령] R1 (Score, Init1)
                    begin
                        {rank_memory[0], rank_memory[1], rank_memory[2]} <= {
                            i2c_buffer[1],
                            i2c_buffer[2],
                            temp_rx_data_reg  // [수정]
                        };
                        // [추가] 랭킹 1위 저장 시 FND에도 강제로 점수(L)를 쓴다
                        fnd_display_reg <= i2c_buffer[2];
                    end
                    8'h12:  // [명령] R1 (Init2, Init3)
                    {rank_memory[3], rank_memory[4]} <= {
                        i2c_buffer[1], i2c_buffer[2]
                    };

                    // --- 랭킹 2위 저장 ---
                    8'h21:  // [명령] R2 (Score, Init1)
                    {rank_memory[5], rank_memory[6], rank_memory[7]} <= {
                        i2c_buffer[1],
                        i2c_buffer[2],
                        temp_rx_data_reg  // [수정]
                    };
                    8'h22:  // [명령] R2 (Init2, Init3)
                    {rank_memory[8], rank_memory[9]} <= {
                        i2c_buffer[1], i2c_buffer[2]
                    };

                    // --- 랭킹 3위 저장 ---
                    8'h31:  // [명령] R3 (Score, Init1)
                    {rank_memory[10], rank_memory[11], rank_memory[12]} <= {
                        i2c_buffer[1],
                        i2c_buffer[2],
                        temp_rx_data_reg  // [수정]
                    };
                    8'h32:  // [명령] R3 (Init2, Init3)
                    {rank_memory[13], rank_memory[14]} <= {
                        i2c_buffer[1], i2c_buffer[2]
                    };

                    // --- 랭킹 4위 저장 ---
                    8'h41:  // [명령] R4 (Score, Init1)
                    {rank_memory[15], rank_memory[16], rank_memory[17]} <= {
                        i2c_buffer[1],
                        i2c_buffer[2],
                        temp_rx_data_reg  // [수정]
                    };
                    8'h42:  // [명령] R4 (Init2, Init3)
                    {rank_memory[18], rank_memory[19]} <= {
                        i2c_buffer[1], i2c_buffer[2]
                    };

                    // --- [명령] 랭킹 읽기 (Read 버퍼 로드) ---
                    8'hA1:
                    {read_buffer[0], read_buffer[1], read_buffer[2], read_buffer[3]} <= {
                        rank_memory[0],
                        rank_memory[1],
                        rank_memory[2],
                        rank_memory[3]
                    };  // R1 (S_H, S_L, I1, I2)
                    8'hA2:
                    {read_buffer[0], read_buffer[1], read_buffer[2], read_buffer[3]} <= {
                        rank_memory[4],
                        rank_memory[5],
                        rank_memory[6],
                        rank_memory[7]
                    };  // R1(I3), R2(S_H, S_L, I1)
                    8'hA3:
                    {read_buffer[0], read_buffer[1], read_buffer[2], read_buffer[3]} <= {
                        rank_memory[8],
                        rank_memory[9],
                        rank_memory[10],
                        rank_memory[11]
                    };  // R2(I2, I3), R3(S_H, S_L)
                    8'hA4:
                    {read_buffer[0], read_buffer[1], read_buffer[2], read_buffer[3]} <= {
                        rank_memory[12],
                        rank_memory[13],
                        rank_memory[14],
                        rank_memory[15]
                    };  // R3(I1, I2, I3), R4(S_H)
                    8'hA5:
                    {read_buffer[0], read_buffer[1], read_buffer[2], read_buffer[3]} <= {
                        rank_memory[16],
                        rank_memory[17],
                        rank_memory[18],
                        rank_memory[19]
                    };  // R4(S_L, I1, I2, I3)
                endcase
            end
        end
    end

    assign sclk_rising  = sclk_sync0 & ~sclk_sync1;
    assign sclk_falling = ~sclk_sync0 & sclk_sync1;

    assign sda_rising   = sda_sync0 & ~sda_sync1;
    assign sda_falling  = ~sda_sync0 & sda_sync1;

    always @(*) begin
        state_next = state;
        en = 1'b0;
        o_data = 1'b0;
        temp_rx_data_next = temp_rx_data_reg;
        temp_tx_data_next = temp_tx_data_reg;
        bit_counter_next = bit_counter_reg;
        temp_addr_next = temp_addr_reg;
        read_ack_next = read_ack_reg;
        led_next = led_reg;
        slv_count_next = slv_count_reg;
        case (state)
            IDLE: begin
                led_next[15:8] = 8'b1000_0000;
                if (sclk_falling && ~SDA) begin
                    state_next = ADDR;
                    bit_counter_next = 0;
                    slv_count_next = 0;
                end
            end
            ADDR: begin
                led_next[15:8] = 8'b0100_0000;
                if (sclk_rising) begin
                    temp_addr_next = {temp_addr_reg[6:0], SDA};
                end
                if (sclk_falling) begin
                    if (bit_counter_reg == 8 - 1) begin
                        bit_counter_next = 0;
                        state_next = ACK;
                    end else begin
                        bit_counter_next = bit_counter_reg + 1;
                    end
                end
            end
            ACK: begin
                led_next[15:8] = 8'b0010_0000;
                if (temp_addr_reg[7:1] == 7'b1010101) begin
                    en = 1'b1;
                    o_data = 1'b0;
                    if (sclk_falling) begin
                        if (temp_addr_reg[0]) begin
                            state_next = READ;
                            temp_tx_data_next = read_buffer[0];
                        end else begin
                            state_next = DATA;
                        end
                    end
                end else begin
                    state_next = IDLE;
                end
            end
            READ: begin
                led_next[15:8] = 8'b001_0000;
                en = 1'b1;
                o_data = temp_tx_data_reg[7];
                if (sclk_falling) begin
                    if (bit_counter_reg == 8 - 1) begin
                        bit_counter_next = 0;
                        state_next = READ_ACK;
                    end else begin
                        temp_tx_data_next = {temp_tx_data_reg[6:0], 1'b0};
                        bit_counter_next  = bit_counter_reg + 1;
                    end
                end
            end
            READ_ACK: begin
                led_next[15:8] = 8'b000_1000;
                en = 1'b0;
                if (sclk_rising) begin
                    read_ack_next = SDA;
                end
                if (sclk_falling) begin
                    if (read_ack_reg == 1'b1) begin
                        state_next = STOP;
                        read_ack_next = 1'bz;
                    end else if (read_ack_reg == 1'b0) begin
                        state_next = READ_CNT;
                        slv_count_next = slv_count_reg + 1;
                        read_ack_next = 1'bz;
                    end
                end
                if (slv_count_reg == 3) begin
                    state_next = STOP;
                end
            end
            READ_CNT: begin
                state_next = READ;
                case (slv_count_reg)
                    2'd0:
                    temp_tx_data_next = read_buffer[0]; // (이미 READ에서 로드됨)
                    2'd1: temp_tx_data_next = read_buffer[1];
                    2'd2: temp_tx_data_next = read_buffer[2];
                    2'd3: temp_tx_data_next = read_buffer[3];
                endcase
            end
            DATA: begin
                led_next[15:8] = 8'b000_0010;
                
                if (sclk_rising) begin
                    temp_rx_data_next = {temp_rx_data_reg[6:0], SDA};
                end
                
                if (sclk_falling) begin
                    if (bit_counter_reg == 8 - 1) begin
                        bit_counter_next = 0;
                        state_next = DATA_ACK;
                        slv_count_next = slv_count_reg + 1;
                        
                        // [추가] ACK 준비를 미리 시작
                        en = 1'b1;
                        o_data = 1'b0;
                    end else begin
                        bit_counter_next = bit_counter_reg + 1;
                    end
                end
                
                if (SCL && sda_rising) begin
                    state_next = STOP;
                end
            end
            DATA_ACK: begin
                led_next[15:8] = 8'b000_0001;
                en = 1'b1;
                o_data = 1'b0;
                if (sclk_falling) begin
                    state_next = DATA;
                end
            end
            STOP: begin
                led_next[15:8] = 8'b000_1111;
                if (SDA && SCL) begin
                    state_next = IDLE;
                end
            end
        endcase
    end

endmodule
