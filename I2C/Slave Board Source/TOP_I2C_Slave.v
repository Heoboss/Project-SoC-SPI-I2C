`timescale 1ns / 1ps

module top_i2c_slave (
    input         clk,
    input         reset,
    input         SCL,
    inout         SDA,
    output [ 7:0] fnd_data,
    output [ 3:0] fnd_com
);

    wire [7:0] slv_reg0;  // FND 표시용 I2C가 제어
    wire [7:0] slv_reg1;  // 랭킹 1위
    wire [7:0] slv_reg2;  // 랭킹 2위
    wire [7:0] slv_reg3;  // 랭킹 3위
    // 참고 : 랭킹 4위는 slv_reg3 이후에 저장되도록 I2C_Slave 모듈 내부에서 처리

    I2C_Slave U_I2C_Slave (
        .clk(clk),
        .reset(reset),
        .SCL(SCL),
        .SDA(SDA),
        .slv_reg0(slv_reg0),
        .slv_reg1(slv_reg1),
        .slv_reg2(slv_reg2),
        .slv_reg3(slv_reg3)
    );

    FND_C U_FND_C (
        .clk(clk),
        .reset(reset),
        .slv_reg0(slv_reg0),
        // .slv_reg1(slv_reg1),
        // .slv_reg2(slv_reg2),
        // .slv_reg3(slv_reg3),
        .fnd_data(fnd_data),
        .fnd_com(fnd_com)
    );

endmodule
