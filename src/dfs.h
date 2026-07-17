#ifndef DFS_H
#define DFS_H

#include <cstdint>
#include <tuple>
#include "defines.h"

// ------------------------------------------------------------
// DFS entry points
// ------------------------------------------------------------
int DFS(uint32_t index, uint32_t** solve_puzzle);

// ------------------------------------------------------------
// Empty area DFS (exposed for checks module)
// ------------------------------------------------------------
void DFS_empty(int x, int y, uint32_t** solve_puzzle);
bool DFS_empty_compass_check(int x, int y, uint32_t** solve_puzzle);

// ------------------------------------------------------------
// Empty area check
// ------------------------------------------------------------
bool empty_area_check(uint32_t** solve_puzzle);

// ------------------------------------------------------------
// Empty area size range
// ------------------------------------------------------------
std::tuple<int, int, int> empty_area_size_range(int x, int y, uint32_t** solve_puzzle);

// ------------------------------------------------------------
// Find special start area
// ------------------------------------------------------------
std::tuple<uint32_t, int, int> find_special_start_area(uint32_t** solve_puzzle);

// ------------------------------------------------------------
// Place non-predefined shape (DFS expansion)
// ------------------------------------------------------------
int place_non_predifined_shape(int index, int x, int y, uint32_t size,
                               bool up_left_seq, int known_shape_index, uint32_t** solve_puzzle);

// ------------------------------------------------------------
// Group mark helpers
// ------------------------------------------------------------
void DFS_group_mark();
bool DFS_in_group_mark(int x, int y, uint32_t** solve_puzzle);

#endif // DFS_H
