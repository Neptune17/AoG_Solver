#ifndef CHECKS_H
#define CHECKS_H

#include <cstdint>
#include "defines.h"

// ------------------------------------------------------------
// Constraint check functions
// ------------------------------------------------------------
bool check_nearby_shape(uint32_t x, uint32_t y, uint32_t n_row, uint32_t n_col, uint32_t** solve_puzzle);
bool check_nearby_size(uint32_t x, uint32_t y, uint32_t n_row, uint32_t n_col, uint32_t** solve_puzzle);
bool check_edge_shape(uint32_t x, uint32_t y, uint32_t n_row, uint32_t n_col, uint32_t** solve_puzzle);
bool check_edge(uint32_t x, uint32_t y, uint32_t n_row, uint32_t n_col, uint32_t** solve_puzzle);
bool check_palisade_type2(uint32_t x, uint32_t y, uint32_t n_row, uint32_t n_col, uint32_t** solve_puzzle);
bool check_palisade_type1(uint32_t x, uint32_t y, uint32_t n_row, uint32_t n_col, uint32_t** solve_puzzle);
bool check_tatami(uint32_t x, uint32_t y, uint32_t n_row, uint32_t n_col, uint32_t** solve_puzzle);
bool check_loopy(uint32_t x, uint32_t y, uint32_t n_row, uint32_t n_col, uint32_t** solve_puzzle);
bool check_radar(uint32_t x, uint32_t y, uint32_t n_row, uint32_t n_col, uint32_t** solve_puzzle);

// ------------------------------------------------------------
// Palisade forced/forbidden derivation (constraint propagation)
// ------------------------------------------------------------
// Neighbor slots: 0=up(x-1), 1=down(x+1), 2=left(y-1), 3=right(y+1)
// nb_state[i]: 0=open (may still join the shape), 1=in_shape, 2=dead
// (blocked area / block line / filled by another region / forbidden)
struct PalisadeDeriveResult {
    bool fail;
    uint8_t forced_mask;    // bit i set => neighbor i must join the shape
    uint8_t forbidden_mask; // bit i set => neighbor i must NOT join the shape
};
PalisadeDeriveResult palisade_derive(int palisade_type, const uint8_t nb_state[4]);

#endif // CHECKS_H
