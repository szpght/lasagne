#pragma once

#define PIT_CH0 0x40
#define PIT_CH1 0x41
#define PIT_CH2 0x42
#define PIT_COMMAND 0x43

#define PIT_CH0_SQUARE 0x34

void pit_initialize();
void pit_handler();
