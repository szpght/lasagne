#pragma once
#include <stdint.h>

void outb(uint16_t port, uint8_t data);
uint8_t inb(uint16_t port);
uint64_t rdmsr(uint32_t msr);
void wrmsr(uint32_t msr, uint64_t value);
