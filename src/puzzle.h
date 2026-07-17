#ifndef PUZZLE_H
#define PUZZLE_H

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <cstring>
#include <algorithm>

#include "defines.h"
#include "types.h"

// ------------------------------------------------------------
// Global puzzle configuration flags
// ------------------------------------------------------------
extern bool puzzle_only_rectangles;
extern bool puzzle_no_rectangles;
extern bool puzzle_adjacent_shapes_different;
extern bool puzzle_adjacent_sizes_different;
extern bool puzzle_all_shapes_different;
extern bool puzzle_all_shapes_same;
extern bool puzzle_one_symbol_per_region;
extern bool puzzle_predefine_shapes_only;
extern bool puzzle_no_4_way_intersections;
extern bool puzzle_no_3_way_intersections;

extern int puzzle_shape_size_lower_bound;
extern int puzzle_shape_size_upper_bound;

// ------------------------------------------------------------
// Puzzle grid state
// ------------------------------------------------------------
extern uint32_t puzzle_n_row;
extern uint32_t puzzle_n_col;

extern uint32_t puzzle[MAX_PUZZLE_SIZE][MAX_PUZZLE_SIZE];
extern int puzzle_compass_up[MAX_PUZZLE_SIZE][MAX_PUZZLE_SIZE];
extern int puzzle_compass_down[MAX_PUZZLE_SIZE][MAX_PUZZLE_SIZE];
extern int puzzle_compass_left[MAX_PUZZLE_SIZE][MAX_PUZZLE_SIZE];
extern int puzzle_compass_right[MAX_PUZZLE_SIZE][MAX_PUZZLE_SIZE];

extern bool slash_check_enable;
extern int slash_check_slash_cnt;

extern bool shape_compare_enable;

extern int all_shapes_same_check_shape_index;

extern std::set<uint32_t> all_shapes_different_check_shape_index_pool;

extern std::map<uint32_t, uint32_t> shape_index_modify_map;

// ------------------------------------------------------------
// Coordinate conversion helpers
// ------------------------------------------------------------
int to_puzzle_x(int x);
int to_puzzle_y(int y);

// ------------------------------------------------------------
// Parse functions
// ------------------------------------------------------------
uint32_t parse_vertex(char c);
uint32_t parse_line(char c);
uint32_t parse_area(char c1, char c2, char c3 = 'X');

// ------------------------------------------------------------
// Puzzle I/O
// ------------------------------------------------------------
void modify_shape_index_in_puzzle(int original_index, int new_index);
void read_puzzle();
void build_solve_puzzle(uint32_t** solve_puzzle);
void print_puzzle(uint32_t** solve_puzzle);
void print_DFS_puzzle(uint32_t** solve_puzzle);

// ------------------------------------------------------------
// Area helpers
// ------------------------------------------------------------
bool area_in_puzzle_range(uint32_t x, uint32_t y, uint32_t n_row, uint32_t n_col);
bool area_contain_symbol(int x, int y);

#endif // PUZZLE_H
