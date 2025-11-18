`timescale 1ns / 1ps

module FND_C(
    input clk, 
    input reset,
    input  [7:0] slv_reg0,
    output [7:0] fnd_data,
    output [3:0] fnd_com
    );

    fnd_controller U_FND(
    .clk(clk), 
    .reset(reset),
    .Digit(slv_reg0),
    .seg(fnd_data),
    .seg_comm(fnd_com)
);
endmodule