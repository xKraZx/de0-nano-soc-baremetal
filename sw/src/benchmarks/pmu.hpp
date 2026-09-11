#pragma once

#include <cstdint>

static inline uint32_t pmu_read_cycle_counter()
{
    uint32_t v;

    __asm__ volatile (
        "mrc p15, 0, %0, c9, c13, 0"
        : "=r"(v)
    );

    return v;
}

static inline void pmu_start_cycle_counter()
{
    uint32_t v;

    /*
     * PMCR:
     * E = 1 -> enable counters
     * C = 1 -> reset PMCCNTR
     * D = 0 -> count every cycle
     */
    v = (1 << 0) | (1 << 2);

    __asm__ volatile (
        "mcr p15, 0, %0, c9, c12, 0"
        :
        : "r"(v)
    );

    /*
     * PMCNTENSET[31] = 1
     * Enable cycle counter
     */
    v = (1u << 31);

    __asm__ volatile (
        "mcr p15, 0, %0, c9, c12, 1"
        :
        : "r"(v)
    );

    __asm__ volatile ("isb");
}

