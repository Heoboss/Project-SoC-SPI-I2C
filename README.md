# SoC AXI4-Lite 기반 I2C Master/Slave 설계 및 UVM 검증 프로젝트

---

## 📜 목차

1. [**프로젝트 개요**](#-프로젝트-개요)
2. [**사용 기술 및 환경**](#-사용-기술-및-환경)
3. [**System Architecture**](#-system-architecture)
4. [**UVM Verification**](#-uvm-verification)
5. [**C Application (Jump Game)**](#-c-application-jump-game)
6. [**트러블슈팅 및 고찰**](#-트러블슈팅-및-고찰)

---

## 🚀 프로젝트 개요
본 프로젝트는 **MicroBlaze CPU**와 연동되는 **AXI4-Lite 인터페이스 기반의 I2C Master 및 Slave IP**를 직접 설계하고, UVM을 통해 검증한 뒤 실제 FPGA 보드에서 동작하는 게임 어플리케이션을 구현하는 것을 목표로 합니다.

- **설계 목표**: AXI4-Lite Bus 프로토콜을 준수하는 I2C Master/Slave Peripheral 설계
- **시스템 통합**: MicroBlaze, UART, GPIO, I2C IP를 연동하여 C코드로 제어하는 HW/SW 통합 시스템 구축
- **검증**: Synopsys VCS, Verdi를 활용한 UVM(Universal Verification Methodology) 기반 IP 검증
- **최종 구현**: 두 대의 FPGA(Basys3)를 연결하여 마스터-슬레이브 간 통신을 활용한 점프 게임 구현

---

## 🔨 사용 기술 및 환경

- **하드웨어**: Digilent Basys3 (Xilinx Artix-7 FPGA) 2대
- **설계 언어**: SystemVerilog
- **개발 도구**: Xilinx Vivado, Vitis
- **검증 도구**: Synopsys VCS, Verdi
- **통신 프로토콜**: AXI4-Lite, I2C, UART

---

## 🔧 System Architecture

### 1. 전체 시스템 블록 다이어그램
MicroBlaze CPU가 AXI4-Lite Interconnect를 통해 GPIO, UART, I2C Master Peripheral을 제어합니다. I2C Master는 외부 핀(SCL, SDA)을 통해 I2C Slave Board와 연결되며, Slave는 수신한 데이터를 바탕으로 FND Controller를 제어하여 점수를 표시합니다.

<img width="1648" height="879" alt="블록다이어그램" src="https://github.com/user-attachments/assets/f9cfe508-d45a-4a6e-b83c-e3183a33c662" />
> *MicroBlaze(Master Board)와 FND Controller(Slave Board)가 I2C로 연결된 구조*

### 2. HW-SW Interface (Register Map)
[cite_start]C 코드에서 하드웨어를 제어하기 위해 `0x44A00000` 주소에 매핑된 레지스터를 사용합니다. [cite: 180]

| Offset | Register | Description |
|:---:|:---:|:---|
| 0x00 | **CR** | [cite_start]Control Register (i2c_en, start, stop 제어) [cite: 182] |
| 0x04 | **WDATA** | [cite_start]Slave로 전송할 1 Byte 데이터 [cite: 183] |
| 0x08 | **SR** | [cite_start]Master의 상태를 나타내는 Status Register [cite: 184] |
| 0x0C~ | **DATA** | [cite_start]Slave로부터 Read한 데이터 저장 (DATA1 ~ DATA4) [cite: 185] |

### 3. FSM 설계
- [cite_start]**I2C Master**: AXI Bus로부터 명령을 받아 실제 I2C 신호(Start, Stop, Write, Read)를 생성하며, `ready`, `tx_done` 신호로 핸드셰이크합니다. [cite: 205, 227]
- [cite_start]**I2C Slave**: SCL, SDA 라인을 모니터링하며 자신의 주소(Address)와 일치할 경우 ACK를 보내고 데이터를 송수신합니다. [cite: 224, 256]

---

## 🧪 UVM Verification

설계한 I2C IP의 신뢰성을 확보하기 위해 UVM 검증 환경을 구축하였습니다.

### 1. 검증 환경 (Testbench Architecture)
- [cite_start]**Component**: Sequencer, Driver, Monitor, Scoreboard로 구성된 Agent 구조 [cite: 293-322]
- [cite_start]**Sequence**: `i2c_write_read_pair_sequence`를 통해 Write 후 Read 동작을 반복 수행 [cite: 304]

### 2. 검증 범위 및 목표
- [cite_start]**Target**: I2C Master (AXI4-Lite) 및 Slave (Address: `0x55` / `1010101`) [cite: 283, 284]
- **Coverage 목표**:
  - [cite_start]Write Transaction: 256가지 데이터 (0x00~0xFF) 전송 [cite: 286]
  - [cite_start]Read Transaction: Write 직후 Read 수행 (총 512회 Transaction) [cite: 287]
  - [cite_start]Pass Rate: 100% 달성 목표 [cite: 292]

### 3. 검증 결과
- [cite_start]**Data Coverage**: 256/256 패턴 (100%) 달성 [cite: 290]
- [cite_start]**Address Test**: 잘못된 주소로 접근 시 127회의 NACK 발생 확인 (정상 동작) [cite: 357]
- [cite_start]**최종 결과**: **TEST PASSED (Pass Rate 100%)** [cite: 350]

---

## 🎮 C Application (Jump Game)

검증된 하드웨어 위에 C언어로 'Chrome Dino Run'과 유사한 점프 게임을 구현하였습니다.

### 1. 동작 시나리오
1. [cite_start]**Master Board**: 게임 로직이 수행되며, 버튼으로 점프(Jump)/일시정지(Pause)를 제어합니다. [cite: 368]
2. [cite_start]**통신**: 획득한 점수는 실시간으로 I2C 통신을 통해 Slave Board로 전송됩니다. [cite: 369]
3. [cite_start]**Slave Board**: 전송받은 점수 데이터를 FND(7-Segment)에 디스플레이합니다. [cite: 369]
4. [cite_start]**랭킹 시스템**: 게임 오버 시 이니셜(3글자)을 입력받아 저장하고, 재시작 시 랭킹을 로드합니다. [cite: 370, 371]

### 2. 실행 화면
> **[게임 실행 화면 이미지 삽입 위치]**
> [cite_start]*PC 터미널의 게임 화면과 FPGA FND에 표시된 점수(0002) 연동 화면* [cite: 376, 501]

---

## 🛠 트러블슈팅 및 고찰

### 1. 트러블슈팅
- **UVM Simulation Timeout 문제**
  - [cite_start]**현상**: UVM Testbench 작성 후 시뮬레이션이 종료되지 않고 Timeout 발생하는 문제 [cite: 505]
  - [cite_start]**해결**: Synopsys Verdi를 통해 파형을 디버깅하여, Ready/Valid 핸드셰이크 타이밍 오차를 수정하고 해결 [cite: 530]

### 2. 고찰 및 개선
- **코드 모듈화 (Modularization)**
  - [cite_start]초기 `main.c`에 모든 기능이 집중되어 가독성이 떨어지는 문제가 발생했습니다. [cite: 535]
  - [cite_start]이를 해결하기 위해 `jump_game_ap.c` (애플리케이션 로직), `device` (I2C, GPIO 드라이버) 등으로 파일을 분리하여 결합도(Coupling)를 낮추고 유지보수성을 높였습니다. [cite: 567]

---

## 📂 참고 자료
- [cite_start]**Source**: 251117_SoC_SPI_I2C_프로젝트_허현강.pdf [cite: 3]
- [cite_start]**Author**: 허현강 (Harman 2기) [cite: 4]
