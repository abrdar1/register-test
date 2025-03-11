#include <stdio.h>
#include <stdint.h>

#define CSR_BASE_ADDR 0x20000000

// RX FIFO Data Registers
#define RX_FIFO_TDATA_31_0  (*(volatile uint32_t *)(CSR_BASE_ADDR + 0x00))
#define RX_FIFO_TDATA_63_32 (*(volatile uint32_t *)(CSR_BASE_ADDR + 0x04))
#define RX_FIFO_TDATA_95_64 (*(volatile uint32_t *)(CSR_BASE_ADDR + 0x08))
#define RX_FIFO_TDATA_127_96 (*(volatile uint32_t *)(CSR_BASE_ADDR + 0x0C))

// TX FIFO Data Registers
#define TX_FIFO_TDATA_31_0  (*(volatile uint32_t *)(CSR_BASE_ADDR + 0x20))
#define TX_FIFO_TDATA_63_32 (*(volatile uint32_t *)(CSR_BASE_ADDR + 0x24))
#define TX_FIFO_TDATA_95_64 (*(volatile uint32_t *)(CSR_BASE_ADDR + 0x28))
#define TX_FIFO_TDATA_127_96 (*(volatile uint32_t *)(CSR_BASE_ADDR + 0x2C))

// Control registers
#define RX_FIFO_CONTROL (*(volatile uint32_t *)(CSR_BASE_ADDR + 0x10))
#define TX_FIFO_CONTROL (*(volatile uint32_t *)(CSR_BASE_ADDR + 0x30))

// Additional control registers
#define RX_TRIGGER_TVALID (*(volatile uint32_t *)(CSR_BASE_ADDR + 0x40))
#define RX_STATUS_TREADY  (*(volatile uint32_t *)(CSR_BASE_ADDR + 0x44))

#define TX_TRIGGER_TVALID (*(volatile uint32_t *)(CSR_BASE_ADDR + 0x48))
#define TX_STATUS_TREADY  (*(volatile uint32_t *)(CSR_BASE_ADDR + 0x52))

// Control reg fields
#define TKEEP_MASK         0xFFFF0000
#define TLAST_MASK         (1 << 8)
#define TUSER_BYPASS_ALL   (1 << 7)
#define TUSER_BYPASS_STAGE (1 << 6)
#define TUSER_SRC_MASK     (0b111 << 3)
#define TUSER_DST_MASK     (0b111 << 0)

// Testing whole registers (data and control)
void test_register(volatile uint32_t *reg, const char *name) {
    uint32_t read_val;

    // 1. Pos value
    *reg = 0x12345678;
    read_val = *reg;
    printf("%s = 0x%X (expected: 0x12345678)\n", name, read_val);

    // 2. Neg value
    *reg = 0x80000000;
    read_val = *reg;
    printf("%s = 0x%X (expected: 0x80000000)\n", name, read_val);

    // 3. Max value
    *reg = 0xFFFFFFFF;
    read_val = *reg;
    printf("%s = 0x%X (expected: 0xFFFFFFFF)\n", name, read_val);

    // 4. Min value
    *reg = 0x00000000;
    read_val = *reg;
    printf("%s = 0x%X (expected: 0x00000000)\n", name, read_val);
}

// Testing register fields
void test_control_register_fields(volatile uint32_t *reg, const char *name) {
    uint32_t read_val, original_value;

    original_value = *reg;

    // TKEEP field
    *reg |= TKEEP_MASK;
    read_val = *reg & TKEEP_MASK;
    printf("%s TKEEP = 0x%X (expected: 0xFFFF)\n", name, read_val);

    // TLAST field
    *reg |= TLAST_MASK;
    read_val = (*reg & TLAST_MASK) ? 1 : 0;
    printf("%s TLAST = %d (expected: 1)\n", name, read_val);

    // TUSER_BYPASS_ALL field
    *reg |= TUSER_BYPASS_ALL;
    read_val = (*reg & TUSER_BYPASS_ALL) ? 1 : 0;
    printf("%s TUSER_BYPASS_ALL = %d (expected: 1)\n", name, read_val);

    // TUSER_BYPASS_STAGE field
    *reg |= TUSER_BYPASS_STAGE;
    read_val = (*reg & TUSER_BYPASS_STAGE) ? 1 : 0;
    printf("%s TUSER_BYPASS_STAGE = %d (expected: 1)\n", name, read_val);

    // TUSER_SRC field
    *reg |= TUSER_SRC_MASK;
    read_val = (*reg & TUSER_SRC_MASK) >> 19;
    printf("%s TUSER_SRC = 0x%X (expected: 0xAA)\n", name, read_val);

    // TUSER_DST field
    *reg |= TUSER_DST_MASK;
    read_val = (*reg & TUSER_DST_MASK) >> 27;
    printf("%s TUSER_DST = 0x%X (expected: 0xBB)\n", name, read_val);

    // Check for other fields if they remain unchanged
    read_val = *reg;
    if ((read_val & ~(TKEEP_MASK | TLAST_MASK | TUSER_BYPASS_ALL | TUSER_BYPASS_STAGE | TUSER_SRC_MASK | TUSER_DST_MASK)) == (original_value & ~(TKEEP_MASK | TLAST_MASK | TUSER_BYPASS_ALL | TUSER_BYPASS_STAGE | TUSER_SRC_MASK | TUSER_DST_MASK))) {
        printf("%s - Other fields remain unchanged (PASS)\n", name);
    } else {
        printf("%s - Other fields changed (FAIL)\n", name);
    }

    *reg = original_value;
}

// Function to test TVALID (single pulse trigger) and TREADY (status) for RX and TX
void test_trigger_status_registers() {
    uint32_t read_val;

    // RX TVALID Test
    printf("\nTesting RX TRIGGER.TVALID...\n");
    RX_TRIGGER_TVALID = 1;
    read_val = RX_TRIGGER_TVALID;
    printf("RX TVALID after write: %d (expected: 1)\n", read_val);

    // Simulate hardware clearing TVALID
    RX_TRIGGER_TVALID = 0;
    read_val = RX_TRIGGER_TVALID;
    printf("RX TVALID after auto-clear: %d (expected: 0)\n", read_val);

    // RX TREADY Test
    printf("\nTesting RX STATUS.TREADY...\n");
    read_val = RX_STATUS_TREADY;
    printf("RX TREADY = %d (expected: depends on FIFO state)\n", read_val);

    // TX TVALID Test
    printf("\nTesting TX TRIGGER.TVALID...\n");
    TX_TRIGGER_TVALID = 1;
    read_val = TX_TRIGGER_TVALID;
    printf("TX TVALID after write: %d (expected: 1)\n", read_val);

    // Simulate hardware clearing TVALID
    TX_TRIGGER_TVALID = 0;
    read_val = TX_TRIGGER_TVALID;
    printf("TX TVALID after auto-clear: %d (expected: 0)\n", read_val);

    // TX TREADY Test
    printf("\nTesting TX STATUS.TREADY...\n");
    read_val = TX_STATUS_TREADY;
    printf("TX TREADY = %d (expected: depends on FIFO state)\n", read_val);
}

void test_data_registers() {
    printf("\nTesting RX FIFO TDATA registers:\n");
    test_register(&RX_FIFO_TDATA_31_0, "RX_FIFO_TDATA_31_0");
    test_register(&RX_FIFO_TDATA_63_32, "RX_FIFO_TDATA_63_32");
    test_register(&RX_FIFO_TDATA_95_64, "RX_FIFO_TDATA_95_64");
    test_register(&RX_FIFO_TDATA_127_96, "RX_FIFO_TDATA_127_96");

    printf("\nTesting TX FIFO TDATA registers:\n");
    test_register(&TX_FIFO_TDATA_31_0, "TX_FIFO_TDATA_31_0");
    test_register(&TX_FIFO_TDATA_63_32, "TX_FIFO_TDATA_63_32");
    test_register(&TX_FIFO_TDATA_95_64, "TX_FIFO_TDATA_95_64");
    test_register(&TX_FIFO_TDATA_127_96, "TX_FIFO_TDATA_127_96");
}

int main() {
    printf("Starting register test...\n");

    test_data_registers();  

    test_register(&RX_FIFO_CONTROL, "RX_FIFO_CONTROL");
    test_control_register_fields(&RX_FIFO_CONTROL, "RX_FIFO_CONTROL");

    test_register(&TX_FIFO_CONTROL, "TX_FIFO_CONTROL");
    test_control_register_fields(&TX_FIFO_CONTROL, "TX_FIFO_CONTROL");

    test_trigger_status_registers();

    printf("Test completed.\n");
    return 0;
}
