#ifndef DFS_H
#define DFS_H

#include <cstdint>
#include <tuple>
#include <vector>
#include <set>
#include <map>
#include <unordered_map>
#include "defines.h"
#include "types.h"

// ------------------------------------------------------------
// DFS state (grouped to reduce global variable sprawl)
// ------------------------------------------------------------
struct DFSContext {
    int  visited[MAX_PUZZLE_SIZE][MAX_PUZZLE_SIZE] = {};
    int  visited_index = 0;
    int  empty_count = 0;
    std::set<std::pair<Node, Node>> empty_block_line_node_pairs;
    int  empty_block_line_count = 0;
    int  symbol_count = 0;
    int  slash_count[10] = {};
    std::vector<Node> compass_nodes;
    std::vector<CompassStates> compass_node_states;
    std::vector<int> area_shape_sizes;

    int  group_mark_index = 0;

    // Adjacency for block-line bipartite matching (built from empty_block_line_node_pairs)
    std::unordered_map<uint64_t, std::vector<Node>> block_adj;

    // place_visited: encode Node{x,y} as uint64_t for O(1) hash lookup
    std::unordered_map<uint64_t, int> place_visited;
};
extern DFSContext dfs_ctx;

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
