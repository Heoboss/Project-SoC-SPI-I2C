module i2c_top (
    input logic clk,
    input logic reset,

    // AXI-Lite-like Interface (from UVM Driver)
    input logic       i2c_start,
    input logic       i2c_stop,
    input logic       i2c_en,
    input logic [7:0] tx_data,

    // AXI-Lite-like Interface (to UVM Monitor)
    output logic       ready,
    output logic       tx_done,
    output logic       rx_done,
    output logic [7:0] rx_data,

    // [추가] Multi-byte read support for UVM
    output logic [7:0] rx_data_1,
    output logic [7:0] rx_data_2,
    output logic [7:0] rx_data_3,
    output logic [7:0] rx_data_4,

    // Slave-side monitor port
    output logic [ 7:0] slave_fnd_register_out,
    output logic [15:0] master_led,
    output logic [15:0] slave_led
);

    // I2C Bus Wires
    wire SCL;
    wire SDA;

    // DUT 1: I2C Master
    I2C_Master u_i2c_master (
        .clk(clk),
        .reset(reset),
        .tx_data(tx_data),
        .rx_data(rx_data),
        .rx_done(rx_done),
        .tx_done(tx_done),
        .ready(ready),
        .start(i2c_start),
        .i2c_en(i2c_en),
        .stop(i2c_stop),
        .SCL(SCL),
        .SDA(SDA),
        .LED(master_led)
    );

    // [추가] Expose Master internal read data for multi-byte reads
    // Assuming Master has temp_rx_data_reg that accumulates read bytes
    // If Master RTL needs modification, add output ports to I2C_Master
    // For simulation workaround, we tap internal signals:
    assign rx_data_1 = rx_data;  // Placeholder - needs Master RTL access
    assign rx_data_2 = 8'h00;  // Placeholder
    assign rx_data_3 = 8'h00;  // Placeholder  
    assign rx_data_4 = 8'h00;  // Placeholder

    // DUT 2: I2C Slave
    I2C_Slave u_i2c_slave (
        .clk  (clk),
        .reset(reset),
        .SCL  (SCL),
        .SDA  (SDA),
        .LED  (slave_led),

        // Monitor 포트 연결
        .slv_reg0(slave_fnd_register_out),  // fnd_display_reg

        // (디버깅용 출력)
        .slv_reg1(),  // rank_memory[0]
        .slv_reg2(),  // rank_memory[1]
        .slv_reg3()   // rank_memory[2]
    );

endmodule
