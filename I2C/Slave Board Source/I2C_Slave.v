`timescale 1ns/1ps

module I2C_Slave(
    input clk,
    input reset,
    input SCL,
    inout SDA,
    output [7:0] slv_reg0,
    output [7:0] slv_reg1,
    output [7:0] slv_reg2,
    output [7:0] slv_reg3
);

    reg [7:0] rank_memory [0:19];
    reg [7:0] fnd_display_reg;
    reg [7:0] i2c_buffer [0:3];
    reg [7:0] read_buffer [0:3];
    reg [7:0] last_command_reg;

    assign slv_reg0 = fnd_display_reg;
    assign slv_reg1 = rank_memory[0];
    assign slv_reg2 = rank_memory[1];
    assign slv_reg3 = rank_memory[2];

    parameter IDLE=0, ADDR=1, ACK=2, READ=3, DATA=4, READ_ACK=5, READ_CNT=6, DATA_ACK=7, DATA_NACK=8, STOP=9;

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

    assign SDA = en ? o_data : 1'bz;

    always @(posedge clk or posedge reset) begin
        if (reset) begin
            state <= IDLE;
            sclk_sync0 <= 1;
            sclk_sync1 <= 1;
            sda_sync0 <= 1;
            sda_sync1 <= 1;
            temp_rx_data_reg <= 0;
            temp_tx_data_reg <= 0;
            bit_counter_reg <= 0;
            temp_addr_reg <= 0;
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
            read_ack_reg <= read_ack_next;
        end
    end

    always @(posedge clk or posedge reset) begin
        if (reset) begin
            slv_count_reg <= 0;
            fnd_display_reg <= 0;
            i2c_buffer[0] <= 0;
            i2c_buffer[1] <= 0;
            i2c_buffer[2] <= 0;
            i2c_buffer[3] <= 0;
            last_command_reg <= 0;
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
            slv_count_reg <= slv_count_next;
            fnd_display_reg <= fnd_display_reg;

            if (state_next == DATA_ACK) begin
                case (slv_count_reg)
                    2'd0: i2c_buffer[0] <= temp_rx_data_reg;
                    2'd1: i2c_buffer[1] <= temp_rx_data_reg;
                    2'd2: i2c_buffer[2] <= temp_rx_data_reg;
                    2'd3: i2c_buffer[3] <= temp_rx_data_reg;
                endcase
            end

            if (state == DATA && state_next == DATA_ACK && slv_count_reg == 3) begin
                last_command_reg <= i2c_buffer[0];
                case (i2c_buffer[0])
                    8'h01: fnd_display_reg <= i2c_buffer[1];
                    8'h11: begin 
                        {rank_memory[0], rank_memory[1], rank_memory[2]} <= {i2c_buffer[1], i2c_buffer[2], temp_rx_data_reg};
                        fnd_display_reg <= i2c_buffer[2];
                    end
                    8'h12: {rank_memory[3], rank_memory[4]} <= {i2c_buffer[1], i2c_buffer[2]};
                    8'h21: {rank_memory[5], rank_memory[6], rank_memory[7]} <= {i2c_buffer[1], i2c_buffer[2], temp_rx_data_reg};
                    8'h22: {rank_memory[8], rank_memory[9]} <= {i2c_buffer[1], i2c_buffer[2]};
                    8'h31: {rank_memory[10], rank_memory[11], rank_memory[12]} <= {i2c_buffer[1], i2c_buffer[2], temp_rx_data_reg};
                    8'h32: {rank_memory[13], rank_memory[14]} <= {i2c_buffer[1], i2c_buffer[2]};
                    8'h41: {rank_memory[15], rank_memory[16], rank_memory[17]} <= {i2c_buffer[1], i2c_buffer[2], temp_rx_data_reg};
                    8'h42: {rank_memory[18], rank_memory[19]} <= {i2c_buffer[1], i2c_buffer[2]};
                    8'hA1: {read_buffer[0], read_buffer[1], read_buffer[2], read_buffer[3]} <= {rank_memory[0], rank_memory[1], rank_memory[2], rank_memory[3]};
                    8'hA2: {read_buffer[0], read_buffer[1], read_buffer[2], read_buffer[3]} <= {rank_memory[4], rank_memory[5], rank_memory[6], rank_memory[7]};
                    8'hA3: {read_buffer[0], read_buffer[1], read_buffer[2], read_buffer[3]} <= {rank_memory[8], rank_memory[9], rank_memory[10], rank_memory[11]};
                    8'hA4: {read_buffer[0], read_buffer[1], read_buffer[2], read_buffer[3]} <= {rank_memory[12], rank_memory[13], rank_memory[14], rank_memory[15]};
                    8'hA5: {read_buffer[0], read_buffer[1], read_buffer[2], read_buffer[3]} <= {rank_memory[16], rank_memory[17], rank_memory[18], rank_memory[19]};
                endcase
            end
        end
    end

    assign sclk_rising = sclk_sync0 & ~sclk_sync1;
    assign sclk_falling = ~sclk_sync0 & sclk_sync1;
    assign sda_rising = sda_sync0 & ~sda_sync1;
    assign sda_falling = ~sda_sync0 & sda_sync1;

    always @(*) begin
        state_next = state;
        en = 1'b0;
        o_data = 1'b0;
        temp_rx_data_next = temp_rx_data_reg;
        temp_tx_data_next = temp_tx_data_reg;
        bit_counter_next = bit_counter_reg;
        temp_addr_next = temp_addr_reg;
        read_ack_next = read_ack_reg;
        slv_count_next = slv_count_reg;
        
        case (state)
            IDLE: begin
                if(sclk_falling && ~SDA) begin
                    state_next = ADDR;
                    bit_counter_next = 0;
                    slv_count_next = 0;
                end
            end
            ADDR: begin
                if(sclk_rising) temp_addr_next = {temp_addr_reg[6:0], SDA};
                if(sclk_falling) begin
                    if (bit_counter_reg == 7) begin
                        bit_counter_next = 0;
                        state_next = ACK;
                    end else bit_counter_next = bit_counter_reg + 1;
                end
            end
            ACK: begin
                if (temp_addr_reg[7:1] == 7'b1010101) begin
                    en = 1'b1;
                    o_data = 1'b0;
                    if(sclk_falling) begin
                        if(temp_addr_reg[0]) begin
                            state_next = READ;
                            temp_tx_data_next = read_buffer[0];
                        end else state_next = DATA;
                    end
                end else state_next = IDLE;
            end
            READ: begin
                en = 1'b1;
                o_data = temp_tx_data_reg[7];
                if(sclk_falling) begin
                    if (bit_counter_reg == 7) begin
                        bit_counter_next = 0;
                        state_next = READ_ACK;
                    end else begin
                        temp_tx_data_next = {temp_tx_data_reg[6:0], 1'b0};
                        bit_counter_next = bit_counter_reg + 1;
                    end
                end
            end
            READ_ACK: begin
                en = 1'b0;
                if(sclk_rising) read_ack_next = SDA;
                if(sclk_falling) begin
                    if(read_ack_reg == 1'b1) begin
                        state_next = STOP;
                        read_ack_next = 1'bz;
                    end else if (read_ack_reg == 1'b0) begin
                        state_next = READ_CNT;
                        slv_count_next = slv_count_reg + 1;
                        read_ack_next = 1'bz;
                    end
                end
                if(slv_count_reg == 3) state_next = STOP;
            end
            READ_CNT: begin
                state_next = READ;
                case(slv_count_reg)
                    2'd0: temp_tx_data_next = read_buffer[0];
                    2'd1: temp_tx_data_next = read_buffer[1];
                    2'd2: temp_tx_data_next = read_buffer[2];
                    2'd3: temp_tx_data_next = read_buffer[3];
                endcase
            end
            DATA: begin
                if(sclk_rising) temp_rx_data_next = {temp_rx_data_reg[6:0], SDA};
                if (sclk_falling) begin
                    if (bit_counter_reg == 7) begin
                        bit_counter_next = 0;
                        state_next = DATA_ACK;
                        slv_count_next = slv_count_reg + 1;
                    end else bit_counter_next = bit_counter_reg + 1;
                end
                if(SCL && sda_rising) state_next = STOP;
            end
            DATA_ACK: begin
                en = 1'b1;
                o_data = 1'b0;
                if(sclk_falling) state_next = DATA;
            end
            STOP: begin
                if(SDA && SCL) state_next = IDLE;
            end
        endcase
    end

endmodule