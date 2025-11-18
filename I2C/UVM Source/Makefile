# ====================================================================
# UVM Testbench Makefile for VCS/Verdi
# ====================================================================

# --- Directories ---
RTL_DIR   = ./rtl/
TB_DIR    = ./tb/
BUILD_DIR = ./build/

# --- File Lists ---
# List all RTL files (DUT)
RTL_FILES = \
	$(RTL_DIR)/i2c_master.v \
	$(RTL_DIR)/i2c_slave.v \
	$(RTL_DIR)/i2c_top.sv

# [수정] 단일 UVM 테스트벤치 파일
TEST_BENCH = $(TB_DIR)/tb_i2c_top.sv

# Combine all files for VCS (RTL -> TB)
VCS_FILES = \
	$(RTL_FILES) \
	$(TEST_BENCH)

# --- VCS Compile Options ---
COMP_OPTS = \
	-full64 \
	-sverilog \
	-timescale=1ns/100ps \
	-ntb_opts uvm-1.2 \
	-debug_access+all -kdb \
	-o $(BUILD_DIR)simv \
	-l $(BUILD_DIR)comp.log

# --- VCS Simulation Options ---
TEST_NAME1 = i2c_address_test
TEST_NAME= i2c_write_read_test
FSDB_FILE = $(BUILD_DIR)i2c_uvm_test.fsdb
SIMV 	  = $(BUILD_DIR)simv
SIM_OPTS  = \
	+UVM_TESTNAME=$(TEST_NAME) \
	-l $(BUILD_DIR)simv.log \
	+fsdbfile+$(FSDB_FILE)

# --- Targets ---
.PHONY: all vcs simv verdi clean

all: vcs simv

vcs:
	@mkdir -p $(BUILD_DIR)
	@echo "--- Compiling UVM Testbench (VCS) ---"
	vcs $(COMP_OPTS) $(VCS_FILES)

simv:
	@echo "--- Running Simulation (SIMV) ---"
	$(SIMV) $(SIM_OPTS)

verdi:
	@echo "--- Launching Verdi ---"
	verdi -dbdir $(BUILD_DIR)simv.daidir -ssf $(FSDB_FILE) &

clean:
	@echo "--- Cleaning up build files ---"
	rm -rf $(BUILD_DIR) csrc *.key *.h uvm_dump.py DVEfiles/ verdiLog/ *simv* *daidir* *fsdb*