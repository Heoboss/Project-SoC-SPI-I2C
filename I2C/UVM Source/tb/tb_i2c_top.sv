`timescale 1ns / 100ps
`include "uvm_macros.svh"
import uvm_pkg::*;

// ========================================================================================
// 1. INTERFACE
// ========================================================================================
interface i2c_top_intf (
    input logic clk
);
    logic reset;
    logic i2c_start;
    logic i2c_stop;
    logic i2c_en;
    logic [7:0] tx_data;
    logic ready;
    logic tx_done;
    logic rx_done;
    logic [7:0] rx_data;
    logic [7:0] slave_fnd_register_out;
    logic [15:0] master_led;
    logic [15:0] slave_led;
endinterface

// ========================================================================================
// 2. SEQUENCE ITEM
// ========================================================================================
class i2c_item extends uvm_sequence_item;
    typedef enum {
        WRITE_PKT,
        READ_PKT,
        ADDR_TEST_PKT  // [추가] 주소 테스트 타입
    } transaction_type_e;

    rand transaction_type_e trans_type;
    rand bit [7:0] cmd;
    rand bit [7:0] data1;
    rand bit [7:0] data2;
    rand bit [7:0] data3;
    bit [7:0] read_buffer[4];

    // [추가] 주소 테스트용 필드
    rand bit [6:0] test_address;
    bit nack_received;

    constraint cmd_c {
        if (trans_type != ADDR_TEST_PKT) {
            cmd inside {8'h01, 8'h11, 8'h12, 8'h21, 8'h22, 8'h31, 8'h32, 8'h41,
                        8'h42, 8'hA1, 8'hA2, 8'hA3, 8'hA4, 8'hA5};
        }
    }

    constraint type_c {
        if (cmd inside {8'hA1, 8'hA2, 8'hA3, 8'hA4, 8'hA5})
        trans_type == READ_PKT;
        else
        if (trans_type == WRITE_PKT)
        cmd inside {8'h01, 8'h11, 8'h12, 8'h21, 8'h22, 8'h31, 8'h32, 8'h41, 8'h42};
    }

    constraint addr_c {
        if (trans_type == ADDR_TEST_PKT) {
            test_address != 7'b1010101;  // 0x55 제외
        }
    }

    function new(string name = "i2c_item");
        super.new(name);
    endfunction

    `uvm_object_utils_begin(i2c_item)
        `uvm_field_enum(transaction_type_e, trans_type, UVM_DEFAULT)
        `uvm_field_int(cmd, UVM_DEFAULT)
        `uvm_field_int(data1, UVM_DEFAULT)
        `uvm_field_int(data2, UVM_DEFAULT)
        `uvm_field_int(data3, UVM_DEFAULT)
        `uvm_field_int(test_address, UVM_DEFAULT)
        `uvm_field_int(nack_received, UVM_DEFAULT)
    `uvm_object_utils_end
endclass

// ========================================================================================
// 3. SEQUENCE
// ========================================================================================
class i2c_base_sequence extends uvm_sequence #(i2c_item);
    `uvm_object_utils(i2c_base_sequence)

    function new(string name = "i2c_base_sequence");
        super.new(name);
    endfunction

    virtual task body();
    endtask
endclass

// Write/Read 쌍 테스트 시퀀스
class i2c_write_read_pair_sequence extends i2c_base_sequence;
    `uvm_object_utils(i2c_write_read_pair_sequence)

    function new(string name = "i2c_write_read_pair_sequence");
        super.new(name);
    endfunction

    task body();
        i2c_item item;

        `uvm_info("SEQ", "========================================", UVM_LOW)
        `uvm_info("SEQ", "Write/Read Pair Test (256 iterations)", UVM_LOW)
        `uvm_info("SEQ", "========================================", UVM_LOW)

        for (int data_val = 0; data_val < 256; data_val++) begin
            // Write 1
            item = i2c_item::type_id::create("item");
            start_item(item);
            assert (item.randomize() with {
                cmd == 8'h11;
                data1 == 8'h00;
                data2 == data_val;
                data3 == 8'h41;
                trans_type == WRITE_PKT;
            });
            finish_item(item);

            // Write 2
            item = i2c_item::type_id::create("item");
            start_item(item);
            assert (item.randomize() with {
                cmd == 8'h12;
                data1 == 8'h42;
                data2 == 8'h43;
                data3 == 8'h00;
                trans_type == WRITE_PKT;
            });
            finish_item(item);

            // Read 1
            item = i2c_item::type_id::create("item");
            start_item(item);
            assert (item.randomize() with {
                cmd == 8'hA1;
                trans_type == READ_PKT;
            });
            finish_item(item);

            // Read 2
            item = i2c_item::type_id::create("item");
            start_item(item);
            assert (item.randomize() with {
                cmd == 8'hA2;
                trans_type == READ_PKT;
            });
            finish_item(item);
        end

        `uvm_info("SEQ", "Write/Read Pair Test Complete!", UVM_LOW)
    endtask
endclass

// [새로 추가] 주소 테스트 시퀀스
class i2c_address_test_sequence extends i2c_base_sequence;
    `uvm_object_utils(i2c_address_test_sequence)

    function new(string name = "i2c_address_test_sequence");
        super.new(name);
    endfunction

    task body();
        i2c_item item;

        `uvm_info("SEQ", "========================================", UVM_LOW)
        `uvm_info("SEQ", "I2C Address Test - Testing 127 Wrong Addresses",
                  UVM_LOW)
        `uvm_info("SEQ", "Valid address: 0x55 (excluded from test)", UVM_LOW)
        `uvm_info("SEQ", "========================================", UVM_LOW)

        // 0x00 ~ 0x7F 중 0x55를 제외한 127개 주소 테스트
        for (int addr = 0; addr < 128; addr++) begin
            if (addr == 7'b1010101) continue;  // 0x55 건너뛰기

            item = i2c_item::type_id::create("item");
            start_item(item);
            item.trans_type = i2c_item::ADDR_TEST_PKT;
            item.test_address = addr[6:0];
            item.nack_received = 0;
            finish_item(item);
        end

        `uvm_info("SEQ", "========================================", UVM_LOW)
        `uvm_info("SEQ", "Address Test Sequence Complete!", UVM_LOW)
        `uvm_info("SEQ", "========================================", UVM_LOW)
    endtask
endclass

// ========================================================================================
// 4. DRIVER
// ========================================================================================
class i2c_driver extends uvm_driver #(i2c_item);
    `uvm_component_utils(i2c_driver)

    virtual i2c_top_intf vif;
    uvm_analysis_port #(i2c_item) ap;
    int transaction_count = 0;

    function new(string name = "i2c_driver", uvm_component parent);
        super.new(name, parent);
        ap = new("ap", this);
    endfunction

    function void build_phase(uvm_phase phase);
        super.build_phase(phase);
        if (!uvm_config_db#(virtual i2c_top_intf)::get(this, "", "vif", vif))
            `uvm_fatal("DRV", "Failed to get virtual interface")
    endfunction

    task wait_ready();
        int timeout = 0;
        while (vif.ready !== 1'b1) begin
            @(posedge vif.clk);
            timeout++;
            if (timeout > 100000) begin
                `uvm_error("DRV", "Timeout waiting for ready")
                return;
            end
        end
    endtask

    task send_start();
        wait_ready();
        @(posedge vif.clk);
        vif.i2c_start <= 1'b1;
        vif.i2c_en    <= 1'b1;
        vif.i2c_stop  <= 1'b0;
        repeat (10) @(posedge vif.clk);
        vif.i2c_start <= 1'b0;
        vif.i2c_en    <= 1'b0;
        wait_ready();
    endtask

    task send_byte(bit [7:0] data);
        wait_ready();
        @(posedge vif.clk);
        vif.tx_data <= data;
        vif.i2c_en <= 1'b1;
        vif.i2c_start <= 1'b0;
        vif.i2c_stop <= 1'b0;
        repeat (10) @(posedge vif.clk);
        vif.i2c_en <= 1'b0;
        wait_ready();
    endtask

    task send_stop();
        wait_ready();
        @(posedge vif.clk);
        vif.i2c_stop <= 1'b1;
        vif.i2c_en <= 1'b1;
        vif.i2c_start <= 1'b0;
        repeat (10) @(posedge vif.clk);
        vif.i2c_stop <= 1'b0;
        vif.i2c_en   <= 1'b0;
        repeat (100) @(posedge vif.clk);
    endtask

    task trigger_read_4bytes();
        wait_ready();
        @(posedge vif.clk);
        vif.i2c_start <= 1'b1;
        vif.i2c_stop  <= 1'b1;
        vif.i2c_en    <= 1'b1;
        repeat (10) @(posedge vif.clk);
        vif.i2c_start <= 1'b0;
        vif.i2c_stop  <= 1'b0;
        vif.i2c_en    <= 1'b0;
        repeat (5000) @(posedge vif.clk);
        wait_ready();
    endtask

    task i2c_write_packet(bit [7:0] cmd, bit [7:0] d1, bit [7:0] d2,
                          bit [7:0] d3);
        send_start();
        send_byte(8'hAA);
        send_byte(cmd);
        send_byte(d1);
        send_byte(d2);
        send_byte(d3);
        send_stop();
    endtask

    task i2c_read_4_bytes(bit [7:0] load_cmd, output bit [7:0] buffer[4]);
        i2c_write_packet(load_cmd, 8'h00, 8'h00, 8'h00);
        repeat (200) @(posedge vif.clk);
        send_start();
        send_byte(8'hAB);
        trigger_read_4bytes();
        buffer[0] = 8'h00;
        buffer[1] = 8'h00;
        buffer[2] = 8'h00;
        buffer[3] = vif.rx_data;
        send_stop();
    endtask

    // [수정] 주소 테스트용 트랜잭션
    // [수정] 주소 테스트용 트랜잭션
    task send_address_only(bit [6:0] addr, bit rw, output bit nack);
        bit [7:0] addr_byte = {addr, rw};
        int timeout_count = 0;

        // START
        send_start();

        // ADDRESS
        @(posedge vif.clk);
        vif.tx_data <= addr_byte;
        vif.i2c_en <= 1'b1;
        vif.i2c_start <= 1'b0;
        vif.i2c_stop <= 1'b0;
        repeat (10) @(posedge vif.clk);
        vif.i2c_en <= 1'b0;

        // ACK/NACK 확인 (timeout 포함)
        while (vif.ready !== 1'b1 && timeout_count < 1000) begin
            @(posedge vif.clk);
            timeout_count++;
        end

        if (timeout_count >= 1000) begin
            // Timeout = NACK
            nack = 1'b1;

            // 강제로 상태 복구 (STOP 신호)
            vif.i2c_stop <= 1'b1;
            vif.i2c_en   <= 1'b1;
            repeat (10) @(posedge vif.clk);
            vif.i2c_stop <= 1'b0;
            vif.i2c_en   <= 1'b0;

            // Master를 IDLE로 리셋
            repeat (200) @(posedge vif.clk);
        end else begin
            // ready가 1이면 ACK
            nack = 1'b0;
            send_stop();
        end
    endtask

    task run_phase(uvm_phase phase);
        i2c_item item;
        bit nack;

        vif.i2c_start <= 1'b0;
        vif.i2c_stop  <= 1'b0;
        vif.i2c_en    <= 1'b0;
        vif.tx_data   <= 8'h00;

        wait (vif.reset == 1'b1);
        `uvm_info("DRV", "Reset released, starting transactions", UVM_LOW)
        repeat (50) @(posedge vif.clk);

        forever begin
            seq_item_port.get_next_item(item);
            transaction_count++;

            case (item.trans_type)
                i2c_item::ADDR_TEST_PKT: begin
                    send_address_only(item.test_address, 1'b0, nack);
                    item.nack_received = nack;
                    `uvm_info("DRV", $sformatf(
                              "[TX #%0d ADDR] Address=0x%02X Result=%s",
                              transaction_count,
                              item.test_address,
                              nack ? "NACK" : "ACK"
                              ), UVM_HIGH)
                end

                i2c_item::WRITE_PKT: begin
                    i2c_write_packet(item.cmd, item.data1, item.data2,
                                     item.data3);
                    `uvm_info("DRV", $sformatf(
                              "[TX #%0d WRITE] CMD=0x%02X Data=[0x%02X, 0x%02X, 0x%02X]",
                              transaction_count,
                              item.cmd,
                              item.data1,
                              item.data2,
                              item.data3
                              ), UVM_MEDIUM)
                end

                i2c_item::READ_PKT: begin
                    i2c_read_4_bytes(item.cmd, item.read_buffer);
                    `uvm_info("DRV", $sformatf(
                              "[TX #%0d READ] CMD=0x%02X Buffer=[0x%02X, 0x%02X, 0x%02X, 0x%02X]",
                              transaction_count,
                              item.cmd,
                              item.read_buffer[0],
                              item.read_buffer[1],
                              item.read_buffer[2],
                              item.read_buffer[3]
                              ), UVM_MEDIUM)
                end
            endcase

            ap.write(item);
            seq_item_port.item_done();
        end
    endtask
endclass

// ========================================================================================
// 5. MONITOR
// ========================================================================================
class i2c_monitor extends uvm_monitor;
    `uvm_component_utils(i2c_monitor)

    virtual i2c_top_intf vif;
    uvm_analysis_port #(i2c_item) ap;

    function new(string name = "i2c_monitor", uvm_component parent);
        super.new(name, parent);
        ap = new("ap", this);
    endfunction

    function void build_phase(uvm_phase phase);
        super.build_phase(phase);
        if (!uvm_config_db#(virtual i2c_top_intf)::get(this, "", "vif", vif))
            `uvm_fatal("MON", "Failed to get virtual interface")
    endfunction

    task run_phase(uvm_phase phase);
        forever begin
            @(posedge vif.clk);
        end
    endtask
endclass

// ========================================================================================
// 6. SCOREBOARD
// ========================================================================================
class i2c_scoreboard extends uvm_scoreboard;
    `uvm_component_utils(i2c_scoreboard)

    uvm_analysis_imp #(i2c_item, i2c_scoreboard) recv;

    int write_count = 0;
    int read_count = 0;
    int pass_count = 0;
    int fail_count = 0;
    int addr_test_count = 0;
    int addr_nack_count = 0;

    bit [7:0] expected_rank_mem[20];
    int write_cmd_coverage[bit [7:0]];
    int read_cmd_coverage[bit [7:0]];
    int data_coverage[256];

    function new(string name = "i2c_scoreboard", uvm_component parent);
        super.new(name, parent);
        recv = new("recv", this);
        for (int i = 0; i < 256; i++) data_coverage[i] = 0;
    endfunction

    function void write(i2c_item item);
        case (item.trans_type)
            i2c_item::ADDR_TEST_PKT: check_address_test(item);
            i2c_item::WRITE_PKT: check_write_transaction(item);
            i2c_item::READ_PKT: check_read_transaction(item);
        endcase
    endfunction

    function void check_address_test(i2c_item item);
        addr_test_count++;
        if (item.nack_received) begin
            addr_nack_count++;
            `uvm_info("SCB", $sformatf("[ADDR %0d/127] 0x%02X -> NACK ✓",
                                       addr_test_count, item.test_address),
                      UVM_HIGH)
        end else begin
            `uvm_error("SCB", $sformatf(
                       "[ADDR %0d/127] 0x%02X -> ACK (Expected NACK!) ✗",
                       addr_test_count,
                       item.test_address
                       ))
        end
    endfunction

    function void check_write_transaction(i2c_item item);
        write_count++;
        write_cmd_coverage[item.cmd]++;
        if (item.cmd == 8'h11) data_coverage[item.data2]++;
        update_expected_memory(item);
    endfunction

    function void check_read_transaction(i2c_item item);
        bit [7:0] expected[4];

        read_count++;
        read_cmd_coverage[item.cmd]++;

        case (item.cmd)
            8'hA1:
            {expected[0], expected[1], expected[2], expected[3]} = {
                expected_rank_mem[0],
                expected_rank_mem[1],
                expected_rank_mem[2],
                expected_rank_mem[3]
            };
            8'hA2:
            {expected[0], expected[1], expected[2], expected[3]} = {
                expected_rank_mem[4],
                expected_rank_mem[5],
                expected_rank_mem[6],
                expected_rank_mem[7]
            };
            8'hA3:
            {expected[0], expected[1], expected[2], expected[3]} = {
                expected_rank_mem[8],
                expected_rank_mem[9],
                expected_rank_mem[10],
                expected_rank_mem[11]
            };
            8'hA4:
            {expected[0], expected[1], expected[2], expected[3]} = {
                expected_rank_mem[12],
                expected_rank_mem[13],
                expected_rank_mem[14],
                expected_rank_mem[15]
            };
            8'hA5:
            {expected[0], expected[1], expected[2], expected[3]} = {
                expected_rank_mem[16],
                expected_rank_mem[17],
                expected_rank_mem[18],
                expected_rank_mem[19]
            };
        endcase

        if (item.read_buffer[3] == expected[3]) begin
            pass_count++;
        end else begin
            fail_count++;
            `uvm_error("SCB", $sformatf(
                       "[READ #%0d FAIL] CMD=0x%02X Expected=0x%02X, Got=0x%02X",
                       read_count,
                       item.cmd,
                       expected[3],
                       item.read_buffer[3]
                       ))
        end
    endfunction

    function void update_expected_memory(i2c_item item);
        case (item.cmd)
            8'h11:
            {expected_rank_mem[0], expected_rank_mem[1], expected_rank_mem[2]} = {
                item.data1, item.data2, item.data3
            };
            8'h12:
            {expected_rank_mem[3], expected_rank_mem[4]} = {
                item.data1, item.data2
            };
            8'h21:
            {expected_rank_mem[5], expected_rank_mem[6], expected_rank_mem[7]} = {
                item.data1, item.data2, item.data3
            };
            8'h22:
            {expected_rank_mem[8], expected_rank_mem[9]} = {
                item.data1, item.data2
            };
            8'h31:
            {expected_rank_mem[10], expected_rank_mem[11], expected_rank_mem[12]} = {
                item.data1, item.data2, item.data3
            };
            8'h32:
            {expected_rank_mem[13], expected_rank_mem[14]} = {
                item.data1, item.data2
            };
            8'h41:
            {expected_rank_mem[15], expected_rank_mem[16], expected_rank_mem[17]} = {
                item.data1, item.data2, item.data3
            };
            8'h42:
            {expected_rank_mem[18], expected_rank_mem[19]} = {
                item.data1, item.data2
            };
        endcase
    endfunction

    function void report_phase(uvm_phase phase);
        int unique_data = 0;
        for (int i = 0; i < 256; i++) if (data_coverage[i] > 0) unique_data++;

        `uvm_info("SCB", "========================================", UVM_NONE)
        `uvm_info("SCB", "         FINAL TEST REPORT              ", UVM_NONE)
        `uvm_info("SCB", "========================================", UVM_NONE)

        if (addr_test_count > 0) begin
            `uvm_info("SCB", " ADDRESS TEST SUMMARY:", UVM_NONE)
            `uvm_info("SCB", $sformatf("   Wrong Address (NACK): %0d",
                                       addr_nack_count), UVM_NONE)
            `uvm_info("SCB", "----------------------------------------",
                      UVM_NONE)
            `uvm_info("SCB", $sformatf(" Total Transactions: %0d",
                                       addr_test_count), UVM_NONE)
            `uvm_info("SCB", $sformatf("   Write: 0"), UVM_NONE)
            `uvm_info("SCB", $sformatf("   Read:  0"), UVM_NONE)
            `uvm_info("SCB", "----------------------------------------",
                      UVM_NONE)
            `uvm_info("SCB", " ADDRESS COVERAGE:", UVM_NONE)
            `uvm_info("SCB",
                      $sformatf(
                          "   Address Test: %0d/%0d wrong addresses NACKed",
                          addr_nack_count, addr_test_count), UVM_NONE)
            `uvm_info("SCB", "========================================",
                      UVM_NONE)

            if (addr_nack_count == 127 && addr_test_count == 127) begin
                `uvm_info("SCB",
                          "✓ Address Test: 127/127 wrong addresses NACKed!",
                          UVM_NONE)
            end else begin
                `uvm_error("SCB", $sformatf(
                           "✗ ADDRESS TEST FAILED - Expected 127 NACKs, got %0d!",
                           addr_nack_count
                           ))
            end
        end else begin
            `uvm_info("SCB", $sformatf(
                      " Total Transactions: %0d", write_count + read_count),
                      UVM_NONE)
            `uvm_info("SCB", $sformatf("   Write: %0d", write_count), UVM_NONE)
            `uvm_info("SCB", $sformatf("   Read:  %0d", read_count), UVM_NONE)
            `uvm_info("SCB", "----------------------------------------",
                      UVM_NONE)

            if (fail_count == 0 && read_count > 0) begin
                `uvm_info("SCB", "✓ TEST PASSED - All Reads Matched!",
                          UVM_NONE)
            end else if (fail_count > 0) begin
                `uvm_error("SCB", $sformatf("✗ TEST FAILED - %0d Mismatches!",
                                            fail_count))
            end
        end

        `uvm_info("SCB", "========================================", UVM_NONE)
    endfunction
endclass

// ========================================================================================
// 7. AGENT, ENV, TEST
// ========================================================================================
class i2c_agent extends uvm_agent;
    `uvm_component_utils(i2c_agent)

    i2c_driver drv;
    i2c_monitor mon;
    uvm_sequencer #(i2c_item) sqr;
    uvm_analysis_port #(i2c_item) drv_ap;

    function new(string name = "i2c_agent", uvm_component parent);
        super.new(name, parent);
        drv_ap = new("drv_ap", this);
    endfunction

    function void build_phase(uvm_phase phase);
        super.build_phase(phase);
        drv = i2c_driver::type_id::create("drv", this);
        mon = i2c_monitor::type_id::create("mon", this);
        sqr = uvm_sequencer#(i2c_item)::type_id::create("sqr", this);
    endfunction

    function void connect_phase(uvm_phase phase);
        drv.seq_item_port.connect(sqr.seq_item_export);
        drv.ap.connect(drv_ap);
    endfunction
endclass

class i2c_env extends uvm_env;
    `uvm_component_utils(i2c_env)

    i2c_agent agt;
    i2c_scoreboard scb;

    function new(string name = "i2c_env", uvm_component parent);
        super.new(name, parent);
    endfunction

    function void build_phase(uvm_phase phase);
        super.build_phase(phase);
        agt = i2c_agent::type_id::create("agt", this);
        scb = i2c_scoreboard::type_id::create("scb", this);
    endfunction

    function void connect_phase(uvm_phase phase);
        agt.drv_ap.connect(scb.recv);
    endfunction
endclass

class i2c_write_read_test extends uvm_test;
    `uvm_component_utils(i2c_write_read_test)

    i2c_env env;
    i2c_write_read_pair_sequence seq;

    function new(string name = "i2c_write_read_test", uvm_component parent);
        super.new(name, parent);
    endfunction

    function void build_phase(uvm_phase phase);
        super.build_phase(phase);
        env = i2c_env::type_id::create("env", this);
        seq = i2c_write_read_pair_sequence::type_id::create("seq");
    endfunction

    task run_phase(uvm_phase phase);
        phase.raise_objection(this);
        seq.start(env.agt.sqr);
        #500us;
        phase.drop_objection(this);
    endtask
endclass

class i2c_address_test extends uvm_test;
    `uvm_component_utils(i2c_address_test)

    i2c_env env;
    i2c_address_test_sequence seq;

    function new(string name = "i2c_address_test", uvm_component parent);
        super.new(name, parent);
    endfunction

    function void build_phase(uvm_phase phase);
        super.build_phase(phase);
        env = i2c_env::type_id::create("env", this);
        seq = i2c_address_test_sequence::type_id::create("seq");
    endfunction

    task run_phase(uvm_phase phase);
        phase.raise_objection(this);
        seq.start(env.agt.sqr);
        #200us;
        phase.drop_objection(this);
    endtask
endclass

// ========================================================================================
// 8. TOP MODULE
// ========================================================================================
module tb_i2c_top;
    logic clk;
    i2c_top_intf intf (clk);

    logic [7:0] rx_data_1, rx_data_2, rx_data_3, rx_data_4;

    i2c_top dut (
        .clk(intf.clk),
        .reset(intf.reset),
        .i2c_start(intf.i2c_start),
        .i2c_stop(intf.i2c_stop),
        .i2c_en(intf.i2c_en),
        .tx_data(intf.tx_data),
        .ready(intf.ready),
        .tx_done(intf.tx_done),
        .rx_done(intf.rx_done),
        .rx_data(intf.rx_data),
        .rx_data_1(rx_data_1),  // [추가]
        .rx_data_2(rx_data_2),  // [추가]
        .rx_data_3(rx_data_3),  // [추가]
        .rx_data_4(rx_data_4),  // [추가]
        .slave_fnd_register_out(intf.slave_fnd_register_out),
        .master_led(intf.master_led),
        .slave_led(intf.slave_led)
    );

    pullup (dut.SDA);
    pullup (dut.SCL);

    defparam dut.u_i2c_master.FCOUNT = 50; defparam dut.u_i2c_master.CLK0 = 25;
        defparam dut.u_i2c_master.CLK1 = 50;
        defparam dut.u_i2c_master.CLK2 = 75;
        defparam dut.u_i2c_master.CLK3 = 100;

    always #5 clk = ~clk;

    initial begin
        $fsdbDumpfile("./build/i2c_uvm_test.fsdb");
        $fsdbDumpvars(0, tb_i2c_top);
        clk = 0;
        intf.reset = 0;
        repeat (50) @(posedge clk);
        intf.reset = 1;
    end

    initial begin
        uvm_config_db#(virtual i2c_top_intf)::set(null, "*", "vif", intf);
        run_test();
    end

    initial begin
        #10s;
        `uvm_fatal("TB_TOP", "Simulation timeout!")
    end
endmodule
