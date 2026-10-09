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

// ------------------------------------------------------------
// Loopy (no 3-way intersection) forced/forbidden derivation
// ------------------------------------------------------------
// One vertex of a 2x2 block where exactly cell E is empty; P/Q are E's
// neighbors in the cell cycle around the vertex, O is the opposite cell.
// id_X is the solve_puzzle value (0 = empty). cur = region id being grown.
// The vertex degree = number of region transitions around it; loopy forbids 3.
// E can only join the current shape (if adjacent to it) or a brand-new region
// (completed regions never grow), so both outcomes' degrees are computable.
enum LoopyVertexResult {
    LOOPY_NONE = 0, // no constraint
    LOOPY_FORCE,    // E must join the current shape
    LOOPY_FORBID,   // E must NOT join the current shape
    LOOPY_FAIL      // no valid outcome for E: contradiction
};
LoopyVertexResult loopy_derive_vertex(uint32_t id_P, uint32_t id_O, uint32_t id_Q, uint32_t cur);

// ------------------------------------------------------------
// Tatami (no 4-way intersection) forced derivation
// ------------------------------------------------------------
// Same vertex setup as loopy, but the forbidden degree is 4. With one empty
// cell E and three resolved cells, deg(E fresh) = 2 + t hits 4 exactly when
// t = (P != O) + (O != Q) == 2; then E must join the current shape (FORCE),
// or the branch is dead if E cannot join (FAIL). Tatami never forbids.
enum TatamiVertexResult {
    TATAMI_NONE = 0,  // no constraint
    TATAMI_FORCE,     // E must join the current shape
    TATAMI_FAIL       // E cannot join and fresh is invalid: contradiction
};
TatamiVertexResult tatami_derive_vertex(uint32_t id_P, uint32_t id_O, uint32_t id_Q, uint32_t cur);

#endif // CHECKS_H
