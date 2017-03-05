#pragma once
#include <stdint.h>

#define PG_PRESENT (1 << 0)
#define PG_RW (1 << 1)
#define PG_HUGE (1 << 7)


struct pml4te {
    uint64_t p : 1;
    uint64_t rw : 1;
    uint64_t us : 1;
    uint64_t pwt : 1;
    uint64_t pcd : 1;
    uint64_t a : 1;
    uint64_t ign1 : 1;
    uint64_t zero1 : 1;
    uint64_t ign2 : 4;
    uint64_t addr : 40;
    uint64_t ign3 : 11;
    uint64_t xd : 1;
} __attribute__((packed));


struct pdpte {
    uint64_t p : 1;
    uint64_t rw : 1;
    uint64_t us : 1;
    uint64_t pwt : 1;
    uint64_t pcd : 1;
    uint64_t a : 1;
    uint64_t ign1 : 1;
    uint64_t ps : 1;
    uint64_t ign2 : 4;
    uint64_t addr : 40;
    uint64_t ign3 : 11;
    uint64_t xd : 1;
} __attribute__((packed));


struct pde {
    uint64_t p : 1;
    uint64_t rw : 1;
    uint64_t us : 1;
    uint64_t pwt : 1;
    uint64_t pcd : 1;
    uint64_t a : 1;
    uint64_t d : 1;
    uint64_t ps : 1;
    uint64_t g : 1;
    uint64_t ign1 : 3;
    uint64_t pat : 1;
    uint64_t zero1 : 8;
    uint64_t addr : 31;
    uint64_t ign2 : 7;
    uint64_t pk : 4;
    uint64_t xd : 1;
} __attribute__((packed));

void reload_paging();
