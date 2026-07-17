#include "dfs.h"
#include "puzzle.h"
#include "shapes.h"
#include "checks.h"

#include <cstring>
#include <optional>
#include <set>
#include <map>
#include <vector>
#include <algorithm>
#include <iostream>

// ------------------------------------------------------------
// DFS state (single struct instance)
// ------------------------------------------------------------
DFSContext dfs_ctx;

// Reusable shape buffer for place_non_predifined_shape (global avoids 40KB stack per call)
static uint32_t _shape_buf[MAX_SHAPE_SIZE][MAX_SHAPE_SIZE];
static uint32_t* temp_shape[MAX_SHAPE_SIZE];
static bool _temp_shape_init = []() {
    for (int k = 0; k < MAX_SHAPE_SIZE; ++k) temp_shape[k] = _shape_buf[k];
    return true;
}();

// Per-recursion-level pools for mark_skip_shape / mark_size (DFS depth <= 128)
static constexpr int MAX_DFS_DEPTH = 128;
static constexpr int MARK_SKIP_CAP = 262144;
static constexpr int MARK_SIZE_CAP = 128;
static bool mark_skip_pool[MAX_DFS_DEPTH][MARK_SKIP_CAP];
static bool mark_size_pool[MAX_DFS_DEPTH][MARK_SIZE_CAP];

// Encode Node{x, y} into a single uint64_t for fast hashing
inline uint64_t encode_node(int x, int y) {
    return (static_cast<uint64_t>(static_cast<uint32_t>(x)) << 32) | static_cast<uint32_t>(y);
}

// ------------------------------------------------------------
// dfs_empty - DFS over empty areas
// ------------------------------------------------------------
void dfs_empty(int x, int y, uint32_t** solve_puzzle) {

    int puzzle_x = to_puzzle_x(x);
    int puzzle_y = to_puzzle_y(y);

    if (puzzle[puzzle_x][puzzle_y] == AREA_BLOCK) {
        return;
    }
    if (solve_puzzle[puzzle_x][puzzle_y] != AREA_NORMAL) {
        return;
    }
    if (dfs_ctx.visited[puzzle_x][puzzle_y] == dfs_ctx.visited_index) {
        return;
    }

    dfs_ctx.visited[puzzle_x][puzzle_y] = dfs_ctx.visited_index;

    dfs_ctx.empty_count += 1;

    // if there is a block line between connected empty areas.
    bool block_line_flag = false;
    if ((puzzle[puzzle_x - 1][puzzle_y] & LINE_BLOCK) && (dfs_ctx.visited[puzzle_x - 2][puzzle_y] == dfs_ctx.visited_index)) {
        dfs_ctx.empty_block_line_node_pairs.insert(std::make_pair(Node(x - 1, y), Node(x, y)));
        block_line_flag = true;
    }
    if ((puzzle[puzzle_x + 1][puzzle_y] & LINE_BLOCK) && (dfs_ctx.visited[puzzle_x + 2][puzzle_y] == dfs_ctx.visited_index)) {
        dfs_ctx.empty_block_line_node_pairs.insert(std::make_pair(Node(x + 1, y), Node(x, y)));
        block_line_flag = true;
    }
    if ((puzzle[puzzle_x][puzzle_y - 1] & LINE_BLOCK) && (dfs_ctx.visited[puzzle_x][puzzle_y - 2] == dfs_ctx.visited_index)) {
        dfs_ctx.empty_block_line_node_pairs.insert(std::make_pair(Node(x, y), Node(x, y - 1)));
        block_line_flag = true;
    }
    if ((puzzle[puzzle_x][puzzle_y + 1] & LINE_BLOCK) && (dfs_ctx.visited[puzzle_x][puzzle_y + 2] == dfs_ctx.visited_index)) {
        dfs_ctx.empty_block_line_node_pairs.insert(std::make_pair(Node(x, y), Node(x, y + 1)));
        block_line_flag = true;
    }
    if (block_line_flag) {
        dfs_ctx.empty_block_line_count += 1;
    }

    if (area_contain_symbol(x, y)) {
        dfs_ctx.symbol_count += 1;
    }

    if (puzzle[puzzle_x][puzzle_y] & AREA_SLASH_INDEX_BIT) {
        dfs_ctx.slash_count[puzzle[puzzle_x][puzzle_y] >> AREA_SLASH_INDEX_BIT_SHIFT] += 1;
    }

    if (puzzle[puzzle_x][puzzle_y] & AREA_SHAPE_SIZE_BIT) {
        dfs_ctx.area_shape_sizes.push_back((puzzle[puzzle_x][puzzle_y] & AREA_SHAPE_SIZE_BIT) >> AREA_SHAPE_SIZE_BIT_SHIFT);
    }

    if (puzzle[puzzle_x][puzzle_y] & AREA_COMPASS_ENABLE) {
        dfs_ctx.compass_nodes.push_back(Node(x, y));
        dfs_ctx.compass_node_states.push_back(CompassStates(0, 0, 0, 0));
    }

    if ((puzzle[puzzle_x - 1][puzzle_y] & LINE_BLOCK) == 0) {
        dfs_empty(x - 1, y, solve_puzzle);
    }
    if ((puzzle[puzzle_x + 1][puzzle_y] & LINE_BLOCK) == 0) {
        dfs_empty(x + 1, y, solve_puzzle);
    }
    if ((puzzle[puzzle_x][puzzle_y - 1] & LINE_BLOCK) == 0) {
        dfs_empty(x, y - 1, solve_puzzle);
    }
    if ((puzzle[puzzle_x][puzzle_y + 1] & LINE_BLOCK) == 0) {
        dfs_empty(x, y + 1, solve_puzzle);
    }
}

// ------------------------------------------------------------
// dfs_empty_compass - DFS to count compass distances
// ------------------------------------------------------------
void dfs_empty_compass(int x, int y, uint32_t** solve_puzzle) {

    int puzzle_x = to_puzzle_x(x);
    int puzzle_y = to_puzzle_y(y);

    if (puzzle[puzzle_x][puzzle_y] == AREA_BLOCK) {
        return;
    }
    if (solve_puzzle[puzzle_x][puzzle_y] != AREA_NORMAL) {
        return;
    }
    if (dfs_ctx.visited[puzzle_x][puzzle_y] == dfs_ctx.visited_index) {
        return;
    }

    dfs_ctx.visited[puzzle_x][puzzle_y] = dfs_ctx.visited_index;

    for (int i = 0; i < (int)dfs_ctx.compass_nodes.size(); ++i) {
        if (x < dfs_ctx.compass_nodes[i].x) dfs_ctx.compass_node_states[i].up++;
        if (x > dfs_ctx.compass_nodes[i].x) dfs_ctx.compass_node_states[i].down++;
        if (y < dfs_ctx.compass_nodes[i].y) dfs_ctx.compass_node_states[i].left++;
        if (y > dfs_ctx.compass_nodes[i].y) dfs_ctx.compass_node_states[i].right++;
    }

    if ((puzzle[puzzle_x - 1][puzzle_y] & LINE_BLOCK) == 0) {
        dfs_empty_compass(x - 1, y, solve_puzzle);
    }
    if ((puzzle[puzzle_x + 1][puzzle_y] & LINE_BLOCK) == 0) {
        dfs_empty_compass(x + 1, y, solve_puzzle);
    }
    if ((puzzle[puzzle_x][puzzle_y - 1] & LINE_BLOCK) == 0) {
        dfs_empty_compass(x, y - 1, solve_puzzle);
    }
    if ((puzzle[puzzle_x][puzzle_y + 1] & LINE_BLOCK) == 0) {
        dfs_empty_compass(x, y + 1, solve_puzzle);
    }
}

// ------------------------------------------------------------
// DFS_empty_compass_check
// ------------------------------------------------------------
bool DFS_empty_compass_check(int x, int y, uint32_t** solve_puzzle) {

    dfs_ctx.visited_index += 1;
    dfs_empty_compass(x, y, solve_puzzle);

    for (int i = 0; i < (int)dfs_ctx.compass_nodes.size(); ++i) {
        int puzzle_x = to_puzzle_x(dfs_ctx.compass_nodes[i].x);
        int puzzle_y = to_puzzle_y(dfs_ctx.compass_nodes[i].y);
        if ((puzzle_compass_up[puzzle_x][puzzle_y] != -1) && (puzzle_compass_up[puzzle_x][puzzle_y] > dfs_ctx.compass_node_states[i].up)) {
            return false;
        }
        if ((puzzle_compass_down[puzzle_x][puzzle_y] != -1) && (puzzle_compass_down[puzzle_x][puzzle_y] > dfs_ctx.compass_node_states[i].down)) {
            return false;
        }
        if ((puzzle_compass_left[puzzle_x][puzzle_y] != -1) && (puzzle_compass_left[puzzle_x][puzzle_y] > dfs_ctx.compass_node_states[i].left)) {
            return false;
        }
        if ((puzzle_compass_right[puzzle_x][puzzle_y] != -1) && (puzzle_compass_right[puzzle_x][puzzle_y] > dfs_ctx.compass_node_states[i].right)) {
            return false;
        }
    }

    return true;
}

// ------------------------------------------------------------
// try_place_id - bipartite partition helper
// ------------------------------------------------------------
int try_place_id(int x, int y, bool value, int visited_value) {
    if (dfs_ctx.place_visited.find(encode_node(x, y)) == dfs_ctx.place_visited.end()) {
        dfs_ctx.place_visited[encode_node(x, y)] = 0;
    }

    if (dfs_ctx.place_visited[encode_node(x, y)] == visited_value) {
        return 0;
    }

    dfs_ctx.place_visited[encode_node(x, y)] = visited_value;
    int count = value;

    for (auto pair : dfs_ctx.empty_block_line_node_pairs) {
        if (pair.first == Node(x, y)) {
            count += try_place_id(pair.second.x, pair.second.y, !value, visited_value);
        }
        if (pair.second == Node(x, y)) {
            count += try_place_id(pair.first.x, pair.first.y, !value, visited_value);
        }
    }

    return count;
}

// ------------------------------------------------------------
// DFS_empty - high-level empty area analysis
// ------------------------------------------------------------
void DFS_empty(int x, int y, uint32_t** solve_puzzle) {
    dfs_ctx.empty_count = 0;
    dfs_ctx.empty_block_line_count = 0;
    dfs_ctx.empty_block_line_node_pairs.clear();
    dfs_ctx.symbol_count = 0;
    memset(dfs_ctx.slash_count, 0, sizeof(dfs_ctx.slash_count));
    dfs_ctx.compass_nodes.clear();
    dfs_ctx.compass_node_states.clear();
    dfs_ctx.area_shape_sizes.clear();

    dfs_ctx.visited_index += 1;
    dfs_empty(x, y, solve_puzzle);

    dfs_ctx.place_visited.clear();
    dfs_ctx.empty_block_line_count = 0;
    for (auto pair : dfs_ctx.empty_block_line_node_pairs) {
        if (dfs_ctx.place_visited.find(encode_node(pair.first.x, pair.first.y)) == dfs_ctx.place_visited.end()) {
            dfs_ctx.empty_block_line_count += std::min(try_place_id(pair.first.x, pair.first.y, 0, 1),
                                                   try_place_id(pair.first.x, pair.first.y, 1, 2));
        }
        if (dfs_ctx.place_visited.find(encode_node(pair.second.x, pair.second.y)) == dfs_ctx.place_visited.end()) {
            dfs_ctx.empty_block_line_count += std::min(try_place_id(pair.second.x, pair.second.y, 0, 1),
                                                   try_place_id(pair.second.x, pair.second.y, 1, 2));
        }
    }
}

// ------------------------------------------------------------
// Group mark helpers
// ------------------------------------------------------------
void DFS_group_mark() {
    dfs_ctx.group_mark_index = dfs_ctx.visited_index;
}

bool DFS_in_group_mark(int x, int y, uint32_t** /*solve_puzzle*/) {
    int puzzle_x = to_puzzle_x(x);
    int puzzle_y = to_puzzle_y(y);
    return dfs_ctx.visited[puzzle_x][puzzle_y] > dfs_ctx.group_mark_index;
}

// ------------------------------------------------------------
// empty_area_check - main constraint check on remaining empty areas
// ------------------------------------------------------------
bool empty_area_check(uint32_t** solve_puzzle) {
    DFS_group_mark();
    for (int x = 1; x <= (int)puzzle_n_row; ++x) {
        for (int y = 1; y <= (int)puzzle_n_col; ++y) {
            if ((puzzle[to_puzzle_x(x)][to_puzzle_y(y)] != AREA_BLOCK) &&
                (solve_puzzle[to_puzzle_x(x)][to_puzzle_y(y)] == AREA_NORMAL) &&
                !DFS_in_group_mark(x, y, solve_puzzle)) {
                DFS_empty(x, y, solve_puzzle);

                int max_area_size = dfs_ctx.empty_count - dfs_ctx.empty_block_line_count;
                if (max_area_size < config.shape_size_lower_bound) {
                    return false;
                }

                std::set<int> unique_val_set;
                int area_shape_sizes_required_size = 0;
                for (int val : dfs_ctx.area_shape_sizes) {
                    if (unique_val_set.insert(val).second) {
                        area_shape_sizes_required_size += val;
                    }
                }
                if (area_shape_sizes_required_size > dfs_ctx.empty_count) {
                    return false;
                }

                if (config.one_symbol_per_region) {
                    if (dfs_ctx.symbol_count == 0) {
                        return false;
                    }
                    if ((dfs_ctx.symbol_count == 1) && (dfs_ctx.empty_block_line_count != 0)) {
                        return false;
                    }
                }

                if (slash_check_enable) {
                    for (int i = 1; i <= slash_check_slash_cnt; ++i) {
                        if (dfs_ctx.slash_count[i] != dfs_ctx.slash_count[1]) {
                            return false;
                        }
                    }
                    if (dfs_ctx.slash_count[1] == 0) {
                        return false;
                    }
                    if ((dfs_ctx.slash_count[1] == 1) && (dfs_ctx.empty_block_line_count != 0)) {
                        return false;
                    }
                }

                if (config.shape_size_lower_bound == config.shape_size_upper_bound) {
                    if (dfs_ctx.empty_count % config.shape_size_lower_bound != 0) {
                        return false;
                    }
                    if ((dfs_ctx.empty_count == config.shape_size_lower_bound) && (dfs_ctx.empty_block_line_count != 0)) {
                        return false;
                    }
                }

                if (config.all_shapes_same && all_shapes_same_check_shape_index != -1) {
                    int shape_size = shape_index_to_shape_size_map[all_shapes_same_check_shape_index];
                    if (dfs_ctx.empty_count % shape_size != 0) {
                        return false;
                    }
                }

                if (dfs_ctx.compass_nodes.size() != 0) {
                    if (!DFS_empty_compass_check(x, y, solve_puzzle)) {
                        return false;
                    }
                }
            }
            if ((puzzle[to_puzzle_x(x)][to_puzzle_y(y)] != AREA_BLOCK) &&
                (solve_puzzle[to_puzzle_x(x)][to_puzzle_y(y)] == AREA_NORMAL) &&
                (puzzle[to_puzzle_x(x)][to_puzzle_y(y)] & AREA_PALISADE_INDEX_BIT)) {
                int palisade_type = (puzzle[to_puzzle_x(x)][to_puzzle_y(y)] & AREA_PALISADE_INDEX_BIT) >> AREA_PALISADE_INDEX_BIT_SHIFT;

                bool up = false, down = false, left = false, right = false;
                if ((puzzle[to_puzzle_x(x) - 2][to_puzzle_y(y)] != AREA_BLOCK) &&
                    (solve_puzzle[to_puzzle_x(x) - 2][to_puzzle_y(y)] == AREA_NORMAL)) left = true;
                if ((puzzle[to_puzzle_x(x) + 2][to_puzzle_y(y)] != AREA_BLOCK) &&
                    (solve_puzzle[to_puzzle_x(x) + 2][to_puzzle_y(y)] == AREA_NORMAL)) right = true;
                if ((puzzle[to_puzzle_x(x)][to_puzzle_y(y) - 2] != AREA_BLOCK) &&
                    (solve_puzzle[to_puzzle_x(x)][to_puzzle_y(y) - 2] == AREA_NORMAL)) up = true;
                if ((puzzle[to_puzzle_x(x)][to_puzzle_y(y) + 2] != AREA_BLOCK) &&
                    (solve_puzzle[to_puzzle_x(x)][to_puzzle_y(y) + 2] == AREA_NORMAL)) down = true;

                if (palisade_type == 1) {
                    if (!up || !down || !left || !right) return false;
                }
                else if (palisade_type == 2) {
                    int sum = up + down + left + right;
                    if (sum < 3) return false;
                }
                else if (palisade_type == 3) {
                    if ((!up && !right) || (!right && !down) || (!down && !left) || (!left && !up)) return false;
                }
                else if (palisade_type == 4) {
                    if (!up && !down && !left && !right) return false;
                }
                else if (palisade_type == 5) {
                    if ((!up && !down) || (!right && !left)) return false;
                }
            }
        }
    }
    return true;
}

// ------------------------------------------------------------
// _empty_area_shape_count
// ------------------------------------------------------------
int _empty_area_shape_count(int /*x*/, int /*y*/, uint32_t** /*solve_puzzle*/) {

    if (dfs_ctx.empty_block_line_count != 0) {
        return 0;
    }

    if (dfs_ctx.empty_count <= config.shape_size_lower_bound) {
        return 1;
    }

    if ((dfs_ctx.area_shape_sizes.size() == 1) && (dfs_ctx.area_shape_sizes[0] == dfs_ctx.empty_count)) {
        return 1;
    }

    if (config.one_symbol_per_region && dfs_ctx.symbol_count == 1) {
        return 1;
    }

    if (slash_check_enable) {
        return dfs_ctx.slash_count[1];
    }

    return 0;
}

// ------------------------------------------------------------
// empty_area_size_range
// ------------------------------------------------------------
std::tuple<int, int, int> empty_area_size_range(int x, int y, uint32_t** solve_puzzle) {

    DFS_empty(x, y, solve_puzzle);

    int shape_size_lower_bound = config.shape_size_lower_bound;
    int shape_size_upper_bound = config.shape_size_upper_bound;

    int max_area_size = dfs_ctx.empty_count - dfs_ctx.empty_block_line_count;
    if (max_area_size < config.shape_size_lower_bound) {
        return std::make_tuple(-1, -1, -1);
    }
    shape_size_upper_bound = std::min(shape_size_upper_bound, max_area_size);

    int shape_count = _empty_area_shape_count(x, y, solve_puzzle);
    if (shape_count == 1) {
        if (max_area_size < shape_size_lower_bound || max_area_size > shape_size_upper_bound) {
            return std::make_tuple(-1, -1, -1);
        }
        shape_size_lower_bound = max_area_size;
    }

    return std::make_tuple(0, shape_size_lower_bound, shape_size_upper_bound);
}

// ------------------------------------------------------------
// Find functions - locate the best empty starting area
// ------------------------------------------------------------
std::tuple<int, int, int> find_empty_compass_area(uint32_t** solve_puzzle) {
    for (int i = 1; i <= (int)puzzle_n_row; ++i) {
        for (int j = 1; j <= (int)puzzle_n_col; ++j) {
            if (((puzzle[to_puzzle_x(i)][to_puzzle_y(j)] & AREA_COMPASS_ENABLE) != 0) &&
                (solve_puzzle[to_puzzle_x(i)][to_puzzle_y(j)] == AREA_NORMAL)) {
                return std::make_tuple(0, i, j);
            }
        }
    }
    return std::make_tuple(-1, -1, -1);
}

std::tuple<int, int, int> find_empty_alone_area(uint32_t** solve_puzzle) {
    for (int i = 1; i <= (int)puzzle_n_row; ++i) {
        for (int j = 1; j <= (int)puzzle_n_col; ++j) {
            if ((solve_puzzle[to_puzzle_x(i)][to_puzzle_y(j)] == AREA_NORMAL)) {
                int block_status = 0;
                if ((puzzle[to_puzzle_x(i - 1)][to_puzzle_y(j)] & AREA_BLOCK) ||
                    (solve_puzzle[to_puzzle_x(i - 1)][to_puzzle_y(j)] != AREA_NORMAL) ||
                    (puzzle[to_puzzle_x(i) - 1][to_puzzle_y(j)] & LINE_BLOCK)) {
                    block_status |= (1 << 3);
                }
                if ((puzzle[to_puzzle_x(i + 1)][to_puzzle_y(j)] & AREA_BLOCK) ||
                    (solve_puzzle[to_puzzle_x(i + 1)][to_puzzle_y(j)] != AREA_NORMAL) ||
                    (puzzle[to_puzzle_x(i) + 1][to_puzzle_y(j)] & LINE_BLOCK)) {
                    block_status |= (1 << 2);
                }
                if ((puzzle[to_puzzle_x(i)][to_puzzle_y(j - 1)] & AREA_BLOCK) ||
                    (solve_puzzle[to_puzzle_x(i)][to_puzzle_y(j - 1)] != AREA_NORMAL) ||
                    (puzzle[to_puzzle_x(i)][to_puzzle_y(j) - 1] & LINE_BLOCK)) {
                    block_status |= (1 << 1);
                }
                if ((puzzle[to_puzzle_x(i)][to_puzzle_y(j + 1)] & AREA_BLOCK) ||
                    (solve_puzzle[to_puzzle_x(i)][to_puzzle_y(j + 1)] != AREA_NORMAL) ||
                    (puzzle[to_puzzle_x(i)][to_puzzle_y(j) + 1] & LINE_BLOCK)) {
                    block_status |= (1 << 0);
                }
                if (block_status == 15) {
                    return std::make_tuple(0, i, j);
                }
                if (puzzle[to_puzzle_x(i)][to_puzzle_y(j)] & AREA_PALISADE_INDEX_BIT) {
                    int palisade_type = (puzzle[to_puzzle_x(i)][to_puzzle_y(j)] & AREA_PALISADE_INDEX_BIT) >> AREA_PALISADE_INDEX_BIT_SHIFT;
                    if (palisade_type == 6) {
                        return std::make_tuple(0, i, j);
                    }
                }
            }
        }
    }
    return std::make_tuple(-1, -1, -1);
}

std::tuple<int, int, int> find_size_limit_small_area(uint32_t** solve_puzzle) {
    DFS_group_mark();
    for (int x = 1; x <= (int)puzzle_n_row; ++x) {
        for (int y = 1; y <= (int)puzzle_n_col; ++y) {
            if ((puzzle[to_puzzle_x(x)][to_puzzle_y(y)] != AREA_BLOCK) &&
                (solve_puzzle[to_puzzle_x(x)][to_puzzle_y(y)] == AREA_NORMAL) &&
                !DFS_in_group_mark(x, y, solve_puzzle)) {
                DFS_empty(x, y, solve_puzzle);

                if (config.shape_size_lower_bound == config.shape_size_upper_bound &&
                    (dfs_ctx.empty_count == config.shape_size_lower_bound)) {
                    return std::make_tuple(0, x, y);
                }
            }
        }
    }
    return std::make_tuple(-1, -1, -1);
}

std::tuple<int, int, int> find_empty_shape_index_area(uint32_t** solve_puzzle) {
    for (int i = 1; i <= (int)puzzle_n_row; ++i) {
        for (int j = 1; j <= (int)puzzle_n_col; ++j) {
            if (((puzzle[to_puzzle_x(i)][to_puzzle_y(j)] & AREA_SHAPE_INDEX_BIT) != 0) &&
                (solve_puzzle[to_puzzle_x(i)][to_puzzle_y(j)] == AREA_NORMAL)) {
                return std::make_tuple(0, i, j);
            }
        }
    }
    return std::make_tuple(-1, -1, -1);
}

std::tuple<int, int, int> find_empty_shape_size_area(uint32_t** solve_puzzle) {
    for (auto _node : shape_size_nodes) {
        if (((puzzle[to_puzzle_x(_node.x)][to_puzzle_y(_node.y)] & AREA_SHAPE_SIZE_BIT) != 0) &&
            (solve_puzzle[to_puzzle_x(_node.x)][to_puzzle_y(_node.y)] == AREA_NORMAL)) {
            return std::make_tuple(0, _node.x, _node.y);
        }
    }
    return std::make_tuple(-1, -1, -1);
}

std::tuple<int, int, int> find_empty_corner_area(uint32_t** solve_puzzle) {
    for (int i = 1; i <= (int)puzzle_n_row; ++i) {
        for (int j = 1; j <= (int)puzzle_n_col; ++j) {
            if (puzzle[to_puzzle_x(i)][to_puzzle_y(j)] != AREA_BLOCK &&
                solve_puzzle[to_puzzle_x(i)][to_puzzle_y(j)] == AREA_NORMAL) {
                int block_line_count =
                    ((puzzle[to_puzzle_x(i) - 1][to_puzzle_y(j)] & LINE_BLOCK) != 0 || puzzle[to_puzzle_x(i) - 2][to_puzzle_y(j)] == AREA_BLOCK) +
                    ((puzzle[to_puzzle_x(i) + 1][to_puzzle_y(j)] & LINE_BLOCK) != 0 || puzzle[to_puzzle_x(i) + 2][to_puzzle_y(j)] == AREA_BLOCK) +
                    ((puzzle[to_puzzle_x(i)][to_puzzle_y(j) - 1] & LINE_BLOCK) != 0 || puzzle[to_puzzle_x(i)][to_puzzle_y(j) - 2] == AREA_BLOCK) +
                    ((puzzle[to_puzzle_x(i)][to_puzzle_y(j) + 1] & LINE_BLOCK) != 0 || puzzle[to_puzzle_x(i)][to_puzzle_y(j) + 2] == AREA_BLOCK);
                if (slash_check_enable && (puzzle[to_puzzle_x(i)][to_puzzle_y(j)] & AREA_SLASH_INDEX_BIT)) {
                    block_line_count +=
                        (((puzzle[to_puzzle_x(i) - 1][to_puzzle_y(j)] & LINE_BLOCK) == 0) &&
                         ((puzzle[to_puzzle_x(i) - 2][to_puzzle_y(j)] & AREA_SLASH_INDEX_BIT) == (puzzle[to_puzzle_x(i)][to_puzzle_y(j)] & AREA_SLASH_INDEX_BIT)))
                        + (((puzzle[to_puzzle_x(i) + 1][to_puzzle_y(j)] & LINE_BLOCK) == 0) &&
                           ((puzzle[to_puzzle_x(i) + 2][to_puzzle_y(j)] & AREA_SLASH_INDEX_BIT) == (puzzle[to_puzzle_x(i)][to_puzzle_y(j)] & AREA_SLASH_INDEX_BIT)))
                        + (((puzzle[to_puzzle_x(i)][to_puzzle_y(j) - 1] & LINE_BLOCK) == 0) &&
                           ((puzzle[to_puzzle_x(i)][to_puzzle_y(j) - 2] & AREA_SLASH_INDEX_BIT) == (puzzle[to_puzzle_x(i)][to_puzzle_y(j)] & AREA_SLASH_INDEX_BIT)))
                        + (((puzzle[to_puzzle_x(i)][to_puzzle_y(j) + 1] & LINE_BLOCK) == 0) &&
                           ((puzzle[to_puzzle_x(i)][to_puzzle_y(j) + 2] & AREA_SLASH_INDEX_BIT) == (puzzle[to_puzzle_x(i)][to_puzzle_y(j)] & AREA_SLASH_INDEX_BIT)));
                }
                if (block_line_count >= 3) {
                    return std::make_tuple(0, i, j);
                }
                if (puzzle[to_puzzle_x(i)][to_puzzle_y(j)] & AREA_PALISADE_INDEX_BIT) {
                    int palisade_type = (puzzle[to_puzzle_x(i)][to_puzzle_y(j)] & AREA_PALISADE_INDEX_BIT) >> AREA_PALISADE_INDEX_BIT_SHIFT;
                    if (palisade_type == 4) {
                        return std::make_tuple(0, i, j);
                    }
                }
            }
        }
    }
    return std::make_tuple(-1, -1, -1);
}

std::tuple<int, int, int> find_empty_line_equal_area(uint32_t** solve_puzzle) {
    for (int i = 1; i <= (int)puzzle_n_row; ++i) {
        for (int j = 1; j <= (int)puzzle_n_col; ++j) {
            if (puzzle[to_puzzle_x(i)][to_puzzle_y(j)] != AREA_BLOCK &&
                solve_puzzle[to_puzzle_x(i)][to_puzzle_y(j)] == AREA_NORMAL) {
                if ((puzzle[to_puzzle_x(i) - 1][to_puzzle_y(j)] & LINE_EQUAL) &&
                    (solve_puzzle[to_puzzle_x(i) - 2][to_puzzle_y(j)] != AREA_NORMAL) &&
                    (puzzle[to_puzzle_x(i) - 2][to_puzzle_y(j)] != AREA_BLOCK)) {
                    return std::make_tuple(0, i, j);
                }
                if ((puzzle[to_puzzle_x(i) + 1][to_puzzle_y(j)] & LINE_EQUAL) &&
                    (solve_puzzle[to_puzzle_x(i) + 2][to_puzzle_y(j)] != AREA_NORMAL) &&
                    (puzzle[to_puzzle_x(i) + 2][to_puzzle_y(j)] != AREA_BLOCK)) {
                    return std::make_tuple(0, i, j);
                }
                if ((puzzle[to_puzzle_x(i)][to_puzzle_y(j) - 1] & LINE_EQUAL) &&
                    (solve_puzzle[to_puzzle_x(i)][to_puzzle_y(j) - 2] != AREA_NORMAL) &&
                    (puzzle[to_puzzle_x(i)][to_puzzle_y(j) - 2] != AREA_BLOCK)) {
                    return std::make_tuple(0, i, j);
                }
                if ((puzzle[to_puzzle_x(i)][to_puzzle_y(j) + 1] & LINE_EQUAL) &&
                    (solve_puzzle[to_puzzle_x(i)][to_puzzle_y(j) + 2] != AREA_NORMAL) &&
                    (puzzle[to_puzzle_x(i)][to_puzzle_y(j) + 2] != AREA_BLOCK)) {
                    return std::make_tuple(0, i, j);
                }
            }
        }
    }
    return std::make_tuple(-1, -1, -1);
}

std::tuple<int, int, int> find_empty_line_size_diff_area(uint32_t** solve_puzzle) {
    for (int i = 1; i <= (int)puzzle_n_row; ++i) {
        for (int j = 1; j <= (int)puzzle_n_col; ++j) {
            if (puzzle[to_puzzle_x(i)][to_puzzle_y(j)] != AREA_BLOCK &&
                solve_puzzle[to_puzzle_x(i)][to_puzzle_y(j)] == AREA_NORMAL) {
                if ((puzzle[to_puzzle_x(i) - 1][to_puzzle_y(j)] & LINE_SIZE_DIFF_BIT) &&
                    (solve_puzzle[to_puzzle_x(i) - 2][to_puzzle_y(j)] != AREA_NORMAL) &&
                    (puzzle[to_puzzle_x(i) - 2][to_puzzle_y(j)] != AREA_BLOCK)) {
                    return std::make_tuple(0, i, j);
                }
                if ((puzzle[to_puzzle_x(i) + 1][to_puzzle_y(j)] & LINE_SIZE_DIFF_BIT) &&
                    (solve_puzzle[to_puzzle_x(i) + 2][to_puzzle_y(j)] != AREA_NORMAL) &&
                    (puzzle[to_puzzle_x(i) + 2][to_puzzle_y(j)] != AREA_BLOCK)) {
                    return std::make_tuple(0, i, j);
                }
                if ((puzzle[to_puzzle_x(i)][to_puzzle_y(j) - 1] & LINE_SIZE_DIFF_BIT) &&
                    (solve_puzzle[to_puzzle_x(i)][to_puzzle_y(j) - 2] != AREA_NORMAL) &&
                    (puzzle[to_puzzle_x(i)][to_puzzle_y(j) - 2] != AREA_BLOCK)) {
                    return std::make_tuple(0, i, j);
                }
                if ((puzzle[to_puzzle_x(i)][to_puzzle_y(j) + 1] & LINE_SIZE_DIFF_BIT) &&
                    (solve_puzzle[to_puzzle_x(i)][to_puzzle_y(j) + 2] != AREA_NORMAL) &&
                    (puzzle[to_puzzle_x(i)][to_puzzle_y(j) + 2] != AREA_BLOCK)) {
                    return std::make_tuple(0, i, j);
                }
            }
        }
    }
    return std::make_tuple(-1, -1, -1);
}

std::tuple<int, int, int> find_empty_line_larger_or_smaller_area(uint32_t** solve_puzzle) {
    for (int i = 1; i <= (int)puzzle_n_row; ++i) {
        for (int j = 1; j <= (int)puzzle_n_col; ++j) {
            if (puzzle[to_puzzle_x(i)][to_puzzle_y(j)] != AREA_BLOCK &&
                solve_puzzle[to_puzzle_x(i)][to_puzzle_y(j)] == AREA_NORMAL) {
                if (((puzzle[to_puzzle_x(i) - 1][to_puzzle_y(j)] & LINE_LARGER) || (puzzle[to_puzzle_x(i) - 1][to_puzzle_y(j)] & LINE_SMALLER)) &&
                    (solve_puzzle[to_puzzle_x(i) - 2][to_puzzle_y(j)] != AREA_NORMAL) &&
                    (puzzle[to_puzzle_x(i) - 2][to_puzzle_y(j)] != AREA_BLOCK)) {
                    return std::make_tuple(0, i, j);
                }
                if (((puzzle[to_puzzle_x(i) + 1][to_puzzle_y(j)] & LINE_LARGER) || (puzzle[to_puzzle_x(i) + 1][to_puzzle_y(j)] & LINE_SMALLER)) &&
                    (solve_puzzle[to_puzzle_x(i) + 2][to_puzzle_y(j)] != AREA_NORMAL) &&
                    (puzzle[to_puzzle_x(i) + 2][to_puzzle_y(j)] != AREA_BLOCK)) {
                    return std::make_tuple(0, i, j);
                }
                if (((puzzle[to_puzzle_x(i)][to_puzzle_y(j) - 1] & LINE_LARGER) || (puzzle[to_puzzle_x(i)][to_puzzle_y(j) - 1] & LINE_SMALLER)) &&
                    (solve_puzzle[to_puzzle_x(i)][to_puzzle_y(j) - 2] != AREA_NORMAL) &&
                    (puzzle[to_puzzle_x(i)][to_puzzle_y(j) - 2] != AREA_BLOCK)) {
                    return std::make_tuple(0, i, j);
                }
                if (((puzzle[to_puzzle_x(i)][to_puzzle_y(j) + 1] & LINE_LARGER) || (puzzle[to_puzzle_x(i)][to_puzzle_y(j) + 1] & LINE_SMALLER)) &&
                    (solve_puzzle[to_puzzle_x(i)][to_puzzle_y(j) + 2] != AREA_NORMAL) &&
                    (puzzle[to_puzzle_x(i)][to_puzzle_y(j) + 2] != AREA_BLOCK)) {
                    return std::make_tuple(0, i, j);
                }
            }
        }
    }
    return std::make_tuple(-1, -1, -1);
}

std::tuple<int, int, int> find_empty_line_constraint_area(uint32_t** solve_puzzle) {
    for (int i = 1; i <= (int)puzzle_n_row; ++i) {
        for (int j = 1; j <= (int)puzzle_n_col; ++j) {
            if (puzzle[to_puzzle_x(i)][to_puzzle_y(j)] != AREA_BLOCK &&
                solve_puzzle[to_puzzle_x(i)][to_puzzle_y(j)] == AREA_NORMAL) {
                uint32_t line[4];
                line[0] = puzzle[to_puzzle_x(i) - 1][to_puzzle_y(j)];
                line[1] = puzzle[to_puzzle_x(i) + 1][to_puzzle_y(j)];
                line[2] = puzzle[to_puzzle_x(i)][to_puzzle_y(j) - 1];
                line[3] = puzzle[to_puzzle_x(i)][to_puzzle_y(j) + 1];
                for (int k = 0; k <= 3; ++k) {
                    if ((line[k] & LINE_EQUAL) || (line[k] & LINE_LARGER) || (line[k] & LINE_SMALLER) ||
                        (line[k] & LINE_DIFFERENT) || (line[k] & LINE_SIZE_DIFF_BIT)) {
                        return std::make_tuple(0, i, j);
                    }
                }
            }
        }
    }
    return std::make_tuple(-1, -1, -1);
}

std::tuple<int, int, int> find_empty_area(uint32_t** solve_puzzle) {
    for (int i = 1; i <= (int)puzzle_n_row; ++i) {
        for (int j = 1; j <= (int)puzzle_n_col; ++j) {
            if (puzzle[to_puzzle_x(i)][to_puzzle_y(j)] != AREA_BLOCK &&
                solve_puzzle[to_puzzle_x(i)][to_puzzle_y(j)] == AREA_NORMAL) {
                return std::make_tuple(0, i, j);
            }
        }
    }
    return std::make_tuple(-1, -1, -1);
}

std::tuple<uint32_t, int, int> find_special_start_area(uint32_t** solve_puzzle) {

    std::tuple<uint32_t, int, int> ret_data;
    uint32_t special_start_type = 0xffffffffu;
    int ret;

    ret_data = find_empty_alone_area(solve_puzzle);
    special_start_type = SPECIAL_START_SIZE_1_REGION;
    ret = std::get<0>(ret_data);
    if (ret == -1) {
        ret_data = find_size_limit_small_area(solve_puzzle);
        special_start_type = SPECIAL_START_SIZE_MATCH_REGION;
    }
    ret = std::get<0>(ret_data);
    if (ret == -1) {
        ret_data = find_empty_line_equal_area(solve_puzzle);
        special_start_type = SPECIAL_START_LINE_SAME;
    }
    ret = std::get<0>(ret_data);
    if (ret == -1) {
        ret_data = find_empty_line_size_diff_area(solve_puzzle);
        special_start_type = SPECIAL_START_LINE_SIZE_DIFF;
    }
    if (ret == -1) {
        ret_data = find_empty_line_larger_or_smaller_area(solve_puzzle);
        special_start_type = SPECIAL_START_LINE_SMALLER_OR_LARGER;
    }
    ret = std::get<0>(ret_data);
    if (ret == -1) {
        ret_data = find_empty_shape_index_area(solve_puzzle);
        special_start_type = SPECIAL_START_AREA_INDEX;
    }
    ret = std::get<0>(ret_data);
    if (ret == -1) {
        ret_data = find_empty_shape_size_area(solve_puzzle);
        special_start_type = SPECIAL_START_AREA_SIZE;
    }
    ret = std::get<0>(ret_data);
    if (ret == -1) {
        ret_data = find_empty_compass_area(solve_puzzle);
        special_start_type = SPECIAL_START_COMPASS;
    }
    ret = std::get<0>(ret_data);
    if (ret == -1) {
        ret_data = find_empty_line_constraint_area(solve_puzzle);
        special_start_type = SPECIAL_START_LINE_CONSTRAINT;
    }
    ret = std::get<0>(ret_data);
    if (ret == -1) {
        ret_data = find_empty_corner_area(solve_puzzle);
        special_start_type = SPECIAL_START_CORNER;
    }
    ret = std::get<0>(ret_data);
    if (ret == -1) {
        ret_data = find_empty_area(solve_puzzle);
        special_start_type = SPECIAL_START_DEFAULT;
    }

    int x = std::get<1>(ret_data);
    int y = std::get<2>(ret_data);

    return std::make_tuple(special_start_type, x, y);
}

// ------------------------------------------------------------
// place_non_predifined_shape
// ------------------------------------------------------------
int place_non_predifined_shape(int index, int x, int y, uint32_t size, bool up_left_seq,
                               int known_shape_index, uint32_t** solve_puzzle) {

    const int MAX_EXPAND_CANDIDATES = (MAX_SHAPE_SIZE + 2) * 3;

    bool mark_slash[slash_check_slash_cnt + 1];
    memset(mark_slash, 0, sizeof(mark_slash));
    int slash_distance[MAX_SHAPE_SIZE][slash_check_slash_cnt + 1][slash_nodes[1].size() + 1];

    Node dfs_current_shape[MAX_SHAPE_SIZE];
    int dfs_current_shape_cnt = 0;
    Node dfs_expand_candidates[MAX_EXPAND_CANDIDATES];
    int dfs_expand_candidates_distance[MAX_EXPAND_CANDIDATES];
    int dfs_expand_candidates_cnt = 0;

    int dfs_rectangle_up[MAX_SHAPE_SIZE];
    int dfs_rectangle_down[MAX_SHAPE_SIZE];
    int dfs_rectangle_left[MAX_SHAPE_SIZE];
    int dfs_rectangle_right[MAX_SHAPE_SIZE];

    Node palisade_visited[MAX_SHAPE_SIZE];
    int palisade_visited_cnt = 0;

    Node compass_visited[MAX_SHAPE_SIZE];
    int compass_visited_up_cnt[MAX_SHAPE_SIZE];
    int compass_visited_down_cnt[MAX_SHAPE_SIZE];
    int compass_visited_left_cnt[MAX_SHAPE_SIZE];
    int compass_visited_right_cnt[MAX_SHAPE_SIZE];
    int compass_visited_cnt = 0;

    std::optional<Node> symbol_loc;

    int stack_size[MAX_SHAPE_SIZE];
    int stack_expand_distance_lb[MAX_SHAPE_SIZE];
    int stack_expand_x_lb[MAX_SHAPE_SIZE];
    int stack_expand_y_lb[MAX_SHAPE_SIZE];
    int stack_candidates_i[MAX_SHAPE_SIZE];
    int stack_candidates_size[MAX_SHAPE_SIZE];
    int stack_top = 0;

    dfs_expand_candidates[dfs_expand_candidates_cnt] = {0, 0};
    dfs_expand_candidates_distance[dfs_expand_candidates_cnt] = 0;
    dfs_expand_candidates_cnt++;

    stack_size[stack_top] = 0;
    stack_expand_distance_lb[stack_top] = 0;
    stack_expand_x_lb[stack_top] = 0;
    stack_expand_y_lb[stack_top] = 0;
    stack_candidates_i[stack_top] = 0;
    stack_candidates_size[stack_top] = 1;
    stack_top++;

    int current_size;
    int expand_distance_lb;
    int expand_x_lb;
    int expand_y_lb;
    int candidates_i;
    int candidates_size;
    int temp_x;
    int temp_y;

    int expand_x;
    int expand_y;
    int expand_distance;

    while (stack_top) {
        current_size = stack_size[stack_top - 1];
        expand_distance_lb = stack_expand_distance_lb[stack_top - 1];
        expand_x_lb = stack_expand_x_lb[stack_top - 1];
        expand_y_lb = stack_expand_y_lb[stack_top - 1];
        candidates_i = stack_candidates_i[stack_top - 1];
        candidates_size = stack_candidates_size[stack_top - 1];

        stack_top--;

        while (dfs_current_shape_cnt > current_size) {
            temp_x = x + dfs_current_shape[dfs_current_shape_cnt - 1].x;
            temp_y = y + dfs_current_shape[dfs_current_shape_cnt - 1].y;
            solve_puzzle[to_puzzle_x(temp_x)][to_puzzle_y(temp_y)] = AREA_NORMAL;
            if (puzzle[to_puzzle_x(temp_x)][to_puzzle_y(temp_y)] & AREA_SLASH_INDEX_BIT) {
                int slash_index = puzzle[to_puzzle_x(temp_x)][to_puzzle_y(temp_y)] >> AREA_SLASH_INDEX_BIT_SHIFT;
                mark_slash[slash_index] = false;
            }
            if (puzzle[to_puzzle_x(temp_x)][to_puzzle_y(temp_y)] & AREA_PALISADE_INDEX_BIT) {
                palisade_visited_cnt--;
            }
            if (puzzle[to_puzzle_x(temp_x)][to_puzzle_y(temp_y)] & AREA_COMPASS_ENABLE) {
                compass_visited_cnt--;
            }

            if (config.one_symbol_per_region && symbol_loc.has_value() && symbol_loc->x == temp_x && symbol_loc->y == temp_y) {
                symbol_loc.reset();
            }

            for (int i = 0; i < compass_visited_cnt; ++i) {
                if (dfs_current_shape[dfs_current_shape_cnt - 1].x < compass_visited[i].x) compass_visited_up_cnt[i]--;
                if (dfs_current_shape[dfs_current_shape_cnt - 1].x > compass_visited[i].x) compass_visited_down_cnt[i]--;
                if (dfs_current_shape[dfs_current_shape_cnt - 1].y < compass_visited[i].y) compass_visited_left_cnt[i]--;
                if (dfs_current_shape[dfs_current_shape_cnt - 1].y > compass_visited[i].y) compass_visited_right_cnt[i]--;
            }

            dfs_current_shape_cnt -= 1;
        }
        dfs_expand_candidates_cnt = candidates_size;

        if (current_size == (int)size) {
            bool slash_check_fail_flag = false;
            for (int i = 1; i <= slash_check_slash_cnt; i++) {
                if (!mark_slash[i]) {
                    slash_check_fail_flag = true;
                    break;
                }
            }
            bool no_rectangle_check_fail_flag = false;
            if (config.no_rectangles) {
                int rectangle_width = dfs_rectangle_right[dfs_current_shape_cnt - 1] - dfs_rectangle_left[dfs_current_shape_cnt - 1] + 1;
                int rectangle_height = dfs_rectangle_down[dfs_current_shape_cnt - 1] - dfs_rectangle_up[dfs_current_shape_cnt - 1] + 1;
                if (rectangle_width * rectangle_height == (int)size) {
                    no_rectangle_check_fail_flag = true;
                }
            }
            bool one_symbol_per_region_check_fail_flag = false;
            if (config.one_symbol_per_region) {
                if (!symbol_loc.has_value()) {
                    one_symbol_per_region_check_fail_flag = true;
                }
            }

            if (slash_check_fail_flag || no_rectangle_check_fail_flag || one_symbol_per_region_check_fail_flag) {
                continue;
            }

            memset(_shape_buf, 0, sizeof(_shape_buf));
            int start_x = 1000, start_y = 1000;
            int shape_size = 0;
            for (int i = 0; i < current_size; ++i) {
                start_x = std::min(start_x, dfs_current_shape[i].x);
                start_y = std::min(start_y, dfs_current_shape[i].y);
            }
            for (int i = 0; i < current_size; ++i) {
                int sx = dfs_current_shape[i].x - start_x;
                int sy = dfs_current_shape[i].y - start_y;
                temp_shape[sx][sy] = 1;
                shape_size = std::max(shape_size, std::max(sx, sy) + 1);
            }

            uint32_t shape_index = shapes_search(temp_shape, shape_size);
            if (shape_index != 0xffffffffu) {
                if (up_left_seq && (int)shape_index <= known_shape_index) {
                    continue;
                }
                if (config.all_shapes_different &&
                    all_shapes_different_check_shape_index_pool.find(shape_index) != all_shapes_different_check_shape_index_pool.end()) {
                    continue;
                }
            }
            else {
                shapes_insert(temp_shape, shape_size);
                shape_index = shapes_search(temp_shape, shape_size);
            }


            for (int i = 0; i < current_size; ++i) {
                solve_puzzle[((x + dfs_current_shape[i].x) << 1) + 1][((y + dfs_current_shape[i].y) << 1) + 1] |=
                    (shape_index << SOLVE_AREA_SHAPE_INDEX_BIT_SHIFT);
            }

            bool shape_check_fail_flag = false;
            bool nearby_shape_check_fail_flag = false;
            bool nearby_size_check_fail_flag = false;
            bool shape_in_puzzle_fail_flag = false;
            bool tatami_check_fail_flag = false;
            bool loopy_check_fail_flag = false;
            bool radar_check_fail_flag = false;
            for (int i = 0; i < current_size; ++i) {
                int new_x = ((x + dfs_current_shape[i].x) << 1) + 1;
                int new_y = ((y + dfs_current_shape[i].y) << 1) + 1;
                if (!check_edge_shape(new_x, new_y, puzzle_n_row, puzzle_n_col, solve_puzzle)) {
                    shape_check_fail_flag = true;
                    break;
                }
                if (config.adjacent_shapes_different && !check_nearby_shape(new_x, new_y, puzzle_n_row, puzzle_n_col, solve_puzzle)) {
                    nearby_shape_check_fail_flag = true;
                    break;
                }
                if (config.adjacent_sizes_different && !check_nearby_size(new_x, new_y, puzzle_n_row, puzzle_n_col, solve_puzzle)) {
                    nearby_size_check_fail_flag = true;
                    break;
                }
                if (puzzle[new_x][new_y] & AREA_SHAPE_INDEX_BIT) {
                    if ((solve_puzzle[new_x][new_y] >> SOLVE_AREA_SHAPE_INDEX_BIT_SHIFT) !=
                        (puzzle[new_x][new_y] >> AREA_SHAPE_INDEX_BIT_SHIFT)) {
                        shape_in_puzzle_fail_flag = true;
                        break;
                    }
                }
                if (config.no_4_way_intersections && !check_tatami(new_x, new_y, puzzle_n_row, puzzle_n_col, solve_puzzle)) {
                    tatami_check_fail_flag = true;
                    break;
                }
                if (config.no_3_way_intersections && !check_loopy(new_x, new_y, puzzle_n_row, puzzle_n_col, solve_puzzle)) {
                    loopy_check_fail_flag = true;
                    break;
                }
                if (!check_radar(new_x, new_y, puzzle_n_row, puzzle_n_col, solve_puzzle)) {
                    radar_check_fail_flag = true;
                    break;
                }
            }
            bool palisade_fail_flag = false;
            for (int i = 0; i < palisade_visited_cnt; ++i) {
                int new_x = ((x + palisade_visited[i].x) << 1) + 1;
                int new_y = ((y + palisade_visited[i].y) << 1) + 1;
                if (!check_palisade_type2(new_x, new_y, puzzle_n_row, puzzle_n_col, solve_puzzle)) {
                    palisade_fail_flag = true;
                    break;
                }
            }

            bool compass_fail_flag = false;
            for (int i = 0; i < compass_visited_cnt; ++i) {
                int new_x = ((x + compass_visited[i].x) << 1) + 1;
                int new_y = ((y + compass_visited[i].y) << 1) + 1;
                if (puzzle_compass_up[new_x][new_y] != -1 && puzzle_compass_up[new_x][new_y] != compass_visited_up_cnt[i]) {
                    compass_fail_flag = true; break;
                }
                if (puzzle_compass_down[new_x][new_y] != -1 && puzzle_compass_down[new_x][new_y] != compass_visited_down_cnt[i]) {
                    compass_fail_flag = true; break;
                }
                if (puzzle_compass_left[new_x][new_y] != -1 && puzzle_compass_left[new_x][new_y] != compass_visited_left_cnt[i]) {
                    compass_fail_flag = true; break;
                }
                if (puzzle_compass_right[new_x][new_y] != -1 && puzzle_compass_right[new_x][new_y] != compass_visited_right_cnt[i]) {
                    compass_fail_flag = true; break;
                }
            }

            if (shape_check_fail_flag || nearby_shape_check_fail_flag || shape_in_puzzle_fail_flag ||
                nearby_size_check_fail_flag || palisade_fail_flag || tatami_check_fail_flag ||
                loopy_check_fail_flag || radar_check_fail_flag || compass_fail_flag) {
                for (int i = 0; i < current_size; ++i) {
                    solve_puzzle[((x + dfs_current_shape[i].x) << 1) + 1][((y + dfs_current_shape[i].y) << 1) + 1] &=
                        (~SOLVE_AREA_SHAPE_INDEX_BIT);
                }
                continue;
            }

            int ret;

            if (!empty_area_check(solve_puzzle)) {
                ret = -1;
            }
            else {
                if (config.all_shapes_same) {
                    all_shapes_same_check_shape_index = shape_index;
                }
                all_shapes_different_check_shape_index_pool.insert(shape_index);
                ret = DFS(index + 1, solve_puzzle);
                if (config.all_shapes_same) {
                    all_shapes_same_check_shape_index = -1;
                }
            }

            if (ret != -1) {
                return ret;
            }
            else {
                all_shapes_different_check_shape_index_pool.erase(shape_index);
                for (int i = 0; i < current_size; ++i) {
                    solve_puzzle[((x + dfs_current_shape[i].x) << 1) + 1][((y + dfs_current_shape[i].y) << 1) + 1] &=
                        (~SOLVE_AREA_SHAPE_INDEX_BIT);
                }
            }
            continue;
        }

        while (candidates_i < candidates_size) {

            expand_x = x + dfs_expand_candidates[candidates_i].x;
            expand_y = y + dfs_expand_candidates[candidates_i].y;
            expand_distance = dfs_expand_candidates_distance[candidates_i];

            bool jump_for_ordered_search = false;
            if (expand_distance < expand_distance_lb) {
                candidates_i++;
                jump_for_ordered_search = true;
            }
            else if (expand_distance == expand_distance_lb) {
                if (expand_x < expand_x_lb) {
                    candidates_i++;
                    jump_for_ordered_search = true;
                }
                else if (expand_x == expand_x_lb) {
                    if (expand_y <= expand_y_lb) {
                        candidates_i++;
                        jump_for_ordered_search = true;
                    }
                }
            }
            if (jump_for_ordered_search) {
                bool in_current_shape = false;
                for (int j = 0; j < dfs_current_shape_cnt; ++j) {
                    if (dfs_current_shape[j].x == dfs_expand_candidates[candidates_i - 1].x &&
                        dfs_current_shape[j].y == dfs_expand_candidates[candidates_i - 1].y) {
                        in_current_shape = true;
                        break;
                    }
                }
                if (!in_current_shape) {
                    DFS_empty(expand_x, expand_y, solve_puzzle);
                    int max_area_size = dfs_ctx.empty_count - dfs_ctx.empty_block_line_count;
                    if (max_area_size < config.shape_size_lower_bound) {
                        break;
                    }
                    if (slash_check_enable) {
                        bool check_flag = false;
                        for (int i = 1; i <= slash_check_slash_cnt; i++) {
                            if (std::abs(dfs_ctx.slash_count[i] - dfs_ctx.slash_count[1]) > 1) {
                                check_flag = true;
                            }
                        }
                        if (check_flag) {
                            break;
                        }
                    }
                }
                continue;
            }

            int slash_index = 0;
            if (puzzle[to_puzzle_x(expand_x)][to_puzzle_y(expand_y)] & AREA_SLASH_INDEX_BIT) {
                slash_index = puzzle[to_puzzle_x(expand_x)][to_puzzle_y(expand_y)] >> AREA_SLASH_INDEX_BIT_SHIFT;
                if (mark_slash[slash_index]) {
                    candidates_i++;
                    continue;
                }
            }

            if (puzzle[to_puzzle_x(expand_x)][to_puzzle_y(expand_y)] & AREA_SHAPE_SIZE_BIT) {
                int target_size = puzzle[to_puzzle_x(expand_x)][to_puzzle_y(expand_y)] >> AREA_SHAPE_SIZE_BIT_SHIFT;
                if ((uint32_t)target_size != size) {
                    candidates_i++;
                    continue;
                }
            }

            solve_puzzle[to_puzzle_x(expand_x)][to_puzzle_y(expand_y)] = index;
            if (!check_edge(to_puzzle_x(expand_x), to_puzzle_y(expand_y), puzzle_n_row, puzzle_n_col, solve_puzzle)) {
                solve_puzzle[to_puzzle_x(expand_x)][to_puzzle_y(expand_y)] = AREA_NORMAL;
                candidates_i++;
                continue;
            }

            mark_slash[slash_index] = true;

            dfs_current_shape[dfs_current_shape_cnt].x = dfs_expand_candidates[candidates_i].x;
            dfs_current_shape[dfs_current_shape_cnt].y = dfs_expand_candidates[candidates_i].y;
            if (config.only_rectangles || config.no_rectangles) {
                if (dfs_current_shape_cnt == 0) {
                    dfs_rectangle_up[dfs_current_shape_cnt] = dfs_expand_candidates[candidates_i].x;
                    dfs_rectangle_down[dfs_current_shape_cnt] = dfs_expand_candidates[candidates_i].x;
                    dfs_rectangle_left[dfs_current_shape_cnt] = dfs_expand_candidates[candidates_i].y;
                    dfs_rectangle_right[dfs_current_shape_cnt] = dfs_expand_candidates[candidates_i].y;
                }
                else {
                    dfs_rectangle_up[dfs_current_shape_cnt] = std::min(dfs_rectangle_up[dfs_current_shape_cnt - 1], dfs_expand_candidates[candidates_i].x);
                    dfs_rectangle_down[dfs_current_shape_cnt] = std::max(dfs_rectangle_down[dfs_current_shape_cnt - 1], dfs_expand_candidates[candidates_i].x);
                    dfs_rectangle_left[dfs_current_shape_cnt] = std::min(dfs_rectangle_left[dfs_current_shape_cnt - 1], dfs_expand_candidates[candidates_i].y);
                    dfs_rectangle_right[dfs_current_shape_cnt] = std::max(dfs_rectangle_right[dfs_current_shape_cnt - 1], dfs_expand_candidates[candidates_i].y);
                }
            }
            for (int i = 0; i < compass_visited_cnt; ++i) {
                if (dfs_current_shape[dfs_current_shape_cnt].x < compass_visited[i].x) compass_visited_up_cnt[i]++;
                if (dfs_current_shape[dfs_current_shape_cnt].x > compass_visited[i].x) compass_visited_down_cnt[i]++;
                if (dfs_current_shape[dfs_current_shape_cnt].y < compass_visited[i].y) compass_visited_left_cnt[i]++;
                if (dfs_current_shape[dfs_current_shape_cnt].y > compass_visited[i].y) compass_visited_right_cnt[i]++;
            }
            for (int i = 1; i <= slash_check_slash_cnt; ++i) {
                for (int j = 0; j < (int)slash_nodes[1].size(); ++j) {

                    if (dfs_current_shape_cnt == 0) {
                        slash_distance[dfs_current_shape_cnt][i][j] = dfs_current_shape_cnt;
                        continue;
                    }
                    else {
                        slash_distance[dfs_current_shape_cnt][i][j] = slash_distance[dfs_current_shape_cnt - 1][i][j];
                    }

                    int new_distance = std::abs(x + dfs_current_shape[dfs_current_shape_cnt].x - slash_nodes[i][j].x) +
                                       std::abs(y + dfs_current_shape[dfs_current_shape_cnt].y - slash_nodes[i][j].y);
                    int old_distance = std::abs(x + dfs_current_shape[slash_distance[dfs_current_shape_cnt][i][j]].x - slash_nodes[i][j].x) +
                                       std::abs(y + dfs_current_shape[slash_distance[dfs_current_shape_cnt][i][j]].y - slash_nodes[i][j].y);

                    if (new_distance < old_distance) {
                        slash_distance[dfs_current_shape_cnt][i][j] = dfs_current_shape_cnt;
                    }
                }
            }

            dfs_current_shape_cnt++;

            if (puzzle[to_puzzle_x(expand_x)][to_puzzle_y(expand_y)] & AREA_PALISADE_INDEX_BIT) {
                palisade_visited[palisade_visited_cnt] = {dfs_expand_candidates[candidates_i].x, dfs_expand_candidates[candidates_i].y};
                palisade_visited_cnt++;
            }
            if (puzzle[to_puzzle_x(expand_x)][to_puzzle_y(expand_y)] & AREA_COMPASS_ENABLE) {
                compass_visited[compass_visited_cnt] = {dfs_expand_candidates[candidates_i].x, dfs_expand_candidates[candidates_i].y};
                compass_visited_up_cnt[compass_visited_cnt] = 0;
                compass_visited_down_cnt[compass_visited_cnt] = 0;
                compass_visited_left_cnt[compass_visited_cnt] = 0;
                compass_visited_right_cnt[compass_visited_cnt] = 0;
                for (int w = 0; w < dfs_current_shape_cnt; ++w) {
                    if (dfs_current_shape[w].x < compass_visited[compass_visited_cnt].x) compass_visited_up_cnt[compass_visited_cnt]++;
                    if (dfs_current_shape[w].x > compass_visited[compass_visited_cnt].x) compass_visited_down_cnt[compass_visited_cnt]++;
                    if (dfs_current_shape[w].y < compass_visited[compass_visited_cnt].y) compass_visited_left_cnt[compass_visited_cnt]++;
                    if (dfs_current_shape[w].y > compass_visited[compass_visited_cnt].y) compass_visited_right_cnt[compass_visited_cnt]++;
                }
                compass_visited_cnt++;
            }

            bool rectangle_fail_flag = false;
            if (config.only_rectangles) {
                int rectangle_width = dfs_rectangle_right[dfs_current_shape_cnt - 1] - dfs_rectangle_left[dfs_current_shape_cnt - 1] + 1;
                int rectangle_height = dfs_rectangle_down[dfs_current_shape_cnt - 1] - dfs_rectangle_up[dfs_current_shape_cnt - 1] + 1;
                if (rectangle_width * rectangle_height > (int)size) {
                    rectangle_fail_flag = true;
                }
            }

            bool palisade_fail_flag = false;
            for (int i = 0; i < palisade_visited_cnt; ++i) {
                int new_x = ((x + palisade_visited[i].x) << 1) + 1;
                int new_y = ((y + palisade_visited[i].y) << 1) + 1;
                if (!check_palisade_type1(new_x, new_y, puzzle_n_row, puzzle_n_col, solve_puzzle)) {
                    palisade_fail_flag = true;
                    break;
                }
            }

            bool compass_fail_flag = false;
            for (int i = 0; i < compass_visited_cnt; ++i) {
                int new_x = ((x + compass_visited[i].x) << 1) + 1;
                int new_y = ((y + compass_visited[i].y) << 1) + 1;
                if (puzzle_compass_up[new_x][new_y] != -1 && puzzle_compass_up[new_x][new_y] < compass_visited_up_cnt[i]) {
                    compass_fail_flag = true; break;
                }
                if (puzzle_compass_down[new_x][new_y] != -1 && puzzle_compass_down[new_x][new_y] < compass_visited_down_cnt[i]) {
                    compass_fail_flag = true; break;
                }
                if (puzzle_compass_left[new_x][new_y] != -1 && puzzle_compass_left[new_x][new_y] < compass_visited_left_cnt[i]) {
                    compass_fail_flag = true; break;
                }
                if (puzzle_compass_right[new_x][new_y] != -1 && puzzle_compass_right[new_x][new_y] < compass_visited_right_cnt[i]) {
                    compass_fail_flag = true; break;
                }
            }

            bool slash_distance_fail_flag = false;
            if (slash_check_slash_cnt != 0) {
                int distance_predict = 0x0fffffff;
                int slash_node_indexs[slash_check_slash_cnt + 1];
                for (int i = 1; i <= slash_check_slash_cnt; ++i) {
                    slash_node_indexs[i] = 0;
                }
                while (true) {
                    int minx = 0, maxx = 0, miny = 0, maxy = 0;
                    for (int i = 1; i <= slash_check_slash_cnt; ++i) {
                        if (mark_slash[i]) continue;
                        int _x = x + dfs_current_shape[slash_distance[dfs_current_shape_cnt - 1][i][slash_node_indexs[i]]].x -
                                 slash_nodes[i][slash_node_indexs[i]].x;
                        int _y = y + dfs_current_shape[slash_distance[dfs_current_shape_cnt - 1][i][slash_node_indexs[i]]].y -
                                 slash_nodes[i][slash_node_indexs[i]].y;
                        minx = std::min(_x, minx);
                        maxx = std::max(_x, maxx);
                        miny = std::min(_y, miny);
                        maxy = std::max(_y, maxy);
                    }
                    distance_predict = std::min(distance_predict, maxx + maxy - minx - miny);

                    slash_node_indexs[1] += 1;
                    int temp_loc = 1;
                    bool final_flag = false;
                    while (true) {
                        if (slash_node_indexs[temp_loc] == (int)slash_nodes[1].size()) {
                            slash_node_indexs[temp_loc] = 0;
                            if (temp_loc < slash_check_slash_cnt) {
                                slash_node_indexs[temp_loc + 1] += 1;
                                temp_loc += 1;
                            }
                            else {
                                final_flag = true;
                                break;
                            }
                        }
                        else {
                            break;
                        }
                    }
                    if (final_flag) break;
                }

                int remain_size = size - current_size - 1;

                if (distance_predict > remain_size) {
                    slash_distance_fail_flag = true;
                }
            }

            if (palisade_fail_flag || rectangle_fail_flag || compass_fail_flag || slash_distance_fail_flag) {
                mark_slash[slash_index] = false;
                solve_puzzle[to_puzzle_x(expand_x)][to_puzzle_y(expand_y)] = AREA_NORMAL;
                dfs_current_shape_cnt--;
                candidates_i++;
                if (puzzle[to_puzzle_x(expand_x)][to_puzzle_y(expand_y)] & AREA_PALISADE_INDEX_BIT) {
                    palisade_visited_cnt--;
                }
                if (puzzle[to_puzzle_x(expand_x)][to_puzzle_y(expand_y)] & AREA_COMPASS_ENABLE) {
                    compass_visited_cnt--;
                }
                for (int i = 0; i < compass_visited_cnt; ++i) {
                    if (dfs_current_shape[dfs_current_shape_cnt].x < compass_visited[i].x) compass_visited_up_cnt[i]--;
                    if (dfs_current_shape[dfs_current_shape_cnt].x > compass_visited[i].x) compass_visited_down_cnt[i]--;
                    if (dfs_current_shape[dfs_current_shape_cnt].y < compass_visited[i].y) compass_visited_left_cnt[i]--;
                    if (dfs_current_shape[dfs_current_shape_cnt].y > compass_visited[i].y) compass_visited_right_cnt[i]--;
                }
                continue;
            }

            if (config.one_symbol_per_region) {
                bool symbol_found = false;
                if (puzzle[to_puzzle_x(expand_x)][to_puzzle_y(expand_y)] & AREA_PALISADE_INDEX_BIT) symbol_found = true;
                if (puzzle[to_puzzle_x(expand_x)][to_puzzle_y(expand_y)] & AREA_SLASH_INDEX_BIT) symbol_found = true;
                if (puzzle[to_puzzle_x(expand_x)][to_puzzle_y(expand_y)] & AREA_SHAPE_INDEX_BIT) symbol_found = true;
                if (puzzle[to_puzzle_x(expand_x)][to_puzzle_y(expand_y)] & AREA_SHAPE_SIZE_BIT) symbol_found = true;
                if (puzzle[to_puzzle_x(expand_x)][to_puzzle_y(expand_y)] & AREA_COMPASS_ENABLE) symbol_found = true;
                if (symbol_found) {
                    if (!symbol_loc.has_value()) {
                        symbol_loc.emplace(expand_x, expand_y);
                    }
                    else {
                        mark_slash[slash_index] = false;
                        solve_puzzle[to_puzzle_x(expand_x)][to_puzzle_y(expand_y)] = AREA_NORMAL;
                        dfs_current_shape_cnt--;
                        candidates_i++;
                        if (puzzle[to_puzzle_x(expand_x)][to_puzzle_y(expand_y)] & AREA_PALISADE_INDEX_BIT) palisade_visited_cnt--;
                        if (puzzle[to_puzzle_x(expand_x)][to_puzzle_y(expand_y)] & AREA_COMPASS_ENABLE) compass_visited_cnt--;
                        for (int i = 0; i < compass_visited_cnt; ++i) {
                            if (dfs_current_shape[dfs_current_shape_cnt].x < compass_visited[i].x) compass_visited_up_cnt[i]--;
                            if (dfs_current_shape[dfs_current_shape_cnt].x > compass_visited[i].x) compass_visited_down_cnt[i]--;
                            if (dfs_current_shape[dfs_current_shape_cnt].y < compass_visited[i].y) compass_visited_left_cnt[i]--;
                            if (dfs_current_shape[dfs_current_shape_cnt].y > compass_visited[i].y) compass_visited_right_cnt[i]--;
                        }
                        continue;
                    }
                }
            }

            int expand_dx[] = {0, 0, -1, 1};
            int expand_dy[] = {-1, 1, 0, 0};

            for (int d = 0; d < 4; ++d) {
                int new_x = dfs_expand_candidates[candidates_i].x + expand_dx[d];
                int new_y = dfs_expand_candidates[candidates_i].y + expand_dy[d];

                uint32_t puzzle_value = puzzle[((x + new_x) << 1) + 1][((y + new_y) << 1) + 1];
                if (puzzle_value == AREA_BLOCK) continue;

                int solve_puzzle_value = solve_puzzle[((x + new_x) << 1) + 1][((y + new_y) << 1) + 1];
                if (solve_puzzle_value != AREA_NORMAL) continue;

                bool alreay_in_candidates = false;
                for (int j = 0; j < dfs_current_shape_cnt; ++j) {
                    if (dfs_current_shape[j].x == new_x && dfs_current_shape[j].y == new_y) {
                        alreay_in_candidates = true;
                        break;
                    }
                }
                for (int j = 0; j < dfs_expand_candidates_cnt; ++j) {
                    if (!alreay_in_candidates && dfs_expand_candidates[j].x == new_x && dfs_expand_candidates[j].y == new_y) {
                        alreay_in_candidates = true;
                        break;
                    }
                }
                if (alreay_in_candidates) continue;

                dfs_expand_candidates[dfs_expand_candidates_cnt] = {new_x, new_y};
                dfs_expand_candidates_distance[dfs_expand_candidates_cnt] = expand_distance + 1;
                dfs_expand_candidates_cnt++;
            }

            stack_size[stack_top] = current_size;
            stack_expand_distance_lb[stack_top] = expand_distance_lb;
            stack_expand_x_lb[stack_top] = expand_x_lb;
            stack_expand_y_lb[stack_top] = expand_y_lb;
            stack_candidates_i[stack_top] = candidates_i + 1;
            stack_candidates_size[stack_top] = candidates_size;
            stack_top++;

            stack_size[stack_top] = current_size + 1;
            stack_expand_distance_lb[stack_top] = expand_distance;
            stack_expand_x_lb[stack_top] = expand_x;
            stack_expand_y_lb[stack_top] = expand_y;
            stack_candidates_i[stack_top] = 0;
            stack_candidates_size[stack_top] = dfs_expand_candidates_cnt;
            stack_top++;
            break;
        }
    }

    return -1;
}

// ------------------------------------------------------------
// DFS - main recursive search
// ------------------------------------------------------------
int DFS(uint32_t index, uint32_t** solve_puzzle) {

    auto [ret, x, y] = find_special_start_area(solve_puzzle);

    if (ret == SPECIAL_START_DEFAULT && x == -1 && y == -1) {
        return 0; // Solved
    }

    auto* mark_skip_shape = mark_skip_pool[index];
    memset(mark_skip_shape, 0, MARK_SKIP_CAP);

    bool mark_slash[slash_check_slash_cnt + 1];

    Node compass_visited[MAX_SHAPE_SIZE];
    int compass_visited_cnt = 0;

    uint32_t ret_code;

    bool one_symbol_per_region_check;

    auto* mark_size = mark_size_pool[index];
    memset(mark_size, 0, MARK_SIZE_CAP);

    auto [range_check, shape_size_lower_bound, shape_size_upper_bound] = empty_area_size_range(x, y, solve_puzzle);

    shape_size_lower_bound = std::max(shape_size_lower_bound, config.shape_size_lower_bound);
    shape_size_upper_bound = std::min(shape_size_upper_bound, config.shape_size_upper_bound);

    for (int i = shape_size_lower_bound; i <= shape_size_upper_bound; ++i) {
        mark_size[i] = true;
    }

    // Type -1 check
    if (range_check == -1) {
        return -1;
    }

    if (ret == SPECIAL_START_AREA_SIZE) {
        for (int i = shape_size_lower_bound; i <= shape_size_upper_bound; ++i) {
            if (i == (int)((puzzle[to_puzzle_x(x)][to_puzzle_y(y)] & AREA_SHAPE_SIZE_BIT) >> AREA_SHAPE_SIZE_BIT_SHIFT)) {
                continue;
            }
            mark_size[i] = false;
        }
    }

    if (ret == SPECIAL_START_LINE_SMALLER_OR_LARGER) {
        // Up
        if (((puzzle[to_puzzle_x(x) - 1][to_puzzle_y(y)] & LINE_LARGER) || (puzzle[to_puzzle_x(x) - 1][to_puzzle_y(y)] & LINE_SMALLER)) &&
            (solve_puzzle[to_puzzle_x(x) - 2][to_puzzle_y(y)] != AREA_NORMAL) &&
            (puzzle[to_puzzle_x(x) - 2][to_puzzle_y(y)] != AREA_BLOCK)) {
            if (puzzle[to_puzzle_x(x) - 1][to_puzzle_y(y)] & LINE_LARGER) {
                for (int i = shape_index_to_shape_size_map[solve_puzzle[to_puzzle_x(x) - 2][to_puzzle_y(y)] >> SOLVE_AREA_SHAPE_INDEX_BIT_SHIFT];
                     i <= shape_size_upper_bound; ++i) mark_size[i] = false;
            }
            else {
                for (int i = shape_size_lower_bound;
                     i <= shape_index_to_shape_size_map[solve_puzzle[to_puzzle_x(x) - 2][to_puzzle_y(y)] >> SOLVE_AREA_SHAPE_INDEX_BIT_SHIFT];
                     ++i) mark_size[i] = false;
            }
        }
        // Down
        if (((puzzle[to_puzzle_x(x) + 1][to_puzzle_y(y)] & LINE_LARGER) || (puzzle[to_puzzle_x(x) + 1][to_puzzle_y(y)] & LINE_SMALLER)) &&
            (solve_puzzle[to_puzzle_x(x) + 2][to_puzzle_y(y)] != AREA_NORMAL) &&
            (puzzle[to_puzzle_x(x) + 2][to_puzzle_y(y)] != AREA_BLOCK)) {
            if (puzzle[to_puzzle_x(x) + 1][to_puzzle_y(y)] & LINE_LARGER) {
                for (int i = shape_size_lower_bound;
                     i <= shape_index_to_shape_size_map[solve_puzzle[to_puzzle_x(x) + 2][to_puzzle_y(y)] >> SOLVE_AREA_SHAPE_INDEX_BIT_SHIFT];
                     ++i) mark_size[i] = false;
            }
            else {
                for (int i = shape_index_to_shape_size_map[solve_puzzle[to_puzzle_x(x) + 2][to_puzzle_y(y)] >> SOLVE_AREA_SHAPE_INDEX_BIT_SHIFT];
                     i <= shape_size_upper_bound; ++i) mark_size[i] = false;
            }
        }
        // Left
        if (((puzzle[to_puzzle_x(x)][to_puzzle_y(y) - 1] & LINE_LARGER) || (puzzle[to_puzzle_x(x)][to_puzzle_y(y) - 1] & LINE_SMALLER)) &&
            (solve_puzzle[to_puzzle_x(x)][to_puzzle_y(y) - 2] != AREA_NORMAL) &&
            (puzzle[to_puzzle_x(x)][to_puzzle_y(y) - 2] != AREA_BLOCK)) {
            if (puzzle[to_puzzle_x(x)][to_puzzle_y(y) - 1] & LINE_LARGER) {
                for (int i = shape_index_to_shape_size_map[solve_puzzle[to_puzzle_x(x)][to_puzzle_y(y) - 2] >> SOLVE_AREA_SHAPE_INDEX_BIT_SHIFT];
                     i <= shape_size_upper_bound; ++i) mark_size[i] = false;
            }
            else {
                for (int i = shape_size_lower_bound;
                     i <= shape_index_to_shape_size_map[solve_puzzle[to_puzzle_x(x)][to_puzzle_y(y) - 2] >> SOLVE_AREA_SHAPE_INDEX_BIT_SHIFT];
                     ++i) mark_size[i] = false;
            }
        }
        // Right
        if (((puzzle[to_puzzle_x(x)][to_puzzle_y(y) + 1] & LINE_LARGER) || (puzzle[to_puzzle_x(x)][to_puzzle_y(y) + 1] & LINE_SMALLER)) &&
            (solve_puzzle[to_puzzle_x(x)][to_puzzle_y(y) + 2] != AREA_NORMAL) &&
            (puzzle[to_puzzle_x(x)][to_puzzle_y(y) + 2] != AREA_BLOCK)) {
            if (puzzle[to_puzzle_x(x)][to_puzzle_y(y) + 1] & LINE_LARGER) {
                for (int i = shape_size_lower_bound;
                     i <= shape_index_to_shape_size_map[solve_puzzle[to_puzzle_x(x)][to_puzzle_y(y) + 2] >> SOLVE_AREA_SHAPE_INDEX_BIT_SHIFT];
                     ++i) mark_size[i] = false;
            }
            else {
                for (int i = shape_index_to_shape_size_map[solve_puzzle[to_puzzle_x(x)][to_puzzle_y(y) + 2] >> SOLVE_AREA_SHAPE_INDEX_BIT_SHIFT];
                     i <= shape_size_upper_bound; ++i) mark_size[i] = false;
            }
        }
    }

    if (ret == SPECIAL_START_LINE_SIZE_DIFF) {
        auto filter_size_diff = [&](int dir_x, int dir_y, int puz_x, int puz_y) {
            if ((puzzle[puz_x][puz_y] & LINE_SIZE_DIFF_BIT) && (solve_puzzle[puz_x - dir_x][puz_y - dir_y] != AREA_NORMAL) &&
                (puzzle[puz_x - dir_x][puz_y - dir_y] != AREA_BLOCK)) {
                int diff_size = (puzzle[puz_x][puz_y] & LINE_SIZE_DIFF_BIT) >> LINE_SIZE_DIFF_BIT_SHIFT;
                int neighbor_size = shape_index_to_shape_size_map[solve_puzzle[puz_x - dir_x][puz_y - dir_y] >> SOLVE_AREA_SHAPE_INDEX_BIT_SHIFT];
                for (int i = shape_size_lower_bound; i <= shape_size_upper_bound; ++i) {
                    if (i == neighbor_size + diff_size || i == neighbor_size - diff_size) continue;
                    mark_size[i] = false;
                }
            }
        };
        filter_size_diff(2, 0, to_puzzle_x(x) - 1, to_puzzle_y(y));
        filter_size_diff(-2, 0, to_puzzle_x(x) + 1, to_puzzle_y(y));
        filter_size_diff(0, 2, to_puzzle_x(x), to_puzzle_y(y) - 1);
        filter_size_diff(0, -2, to_puzzle_x(x), to_puzzle_y(y) + 1);
    }

    for (int i = 0; i < (int)shapes.size(); ++i) {

        ret_code = 0;

        // ----------------------------------------------------
        // Type 0 check

        if (ret == SPECIAL_START_DEFAULT && mark_skip_shape[i]) {
            ret_code |= RET_CODE_SHAPE_CONTAIN_BLOCK_AREA;
            continue;
        }

        // Type 0 check: all shapes different
        if (config.all_shapes_different) {
            if (all_shapes_different_check_shape_index_pool.find(shapes[i].shape_index) != all_shapes_different_check_shape_index_pool.end()) {
                ret_code |= RET_CODE_ALL_SHAPE_DIFFERENT;
                continue;
            }
        }

        // Type 0 check: all shapes same
        if (config.all_shapes_same) {
            if ((all_shapes_same_check_shape_index != -1) && (shapes[i].shape_index != (uint32_t)all_shapes_same_check_shape_index)) {
                ret_code |= RET_CODE_ALL_SHAPE_SAME;
                continue;
            }
        }

        // Type 0 check: shape size range
        if ((int)shapes[i].nodes.size() < config.shape_size_lower_bound || (int)shapes[i].nodes.size() > config.shape_size_upper_bound) {
            ret_code |= RET_CODE_SHAPE_SIZE_RANGE;
            continue;
        }

        if (!mark_size[shapes[i].nodes.size()]) {
            ret_code |= RET_CODE_SHAPE_SIZE_RANGE;
            continue;
        }

        // Type 0 check finished
        if (ret_code != 0) continue;
        // ----------------------------------------------------

        for (int p = 0; p < (int)shapes[i].nodes.size(); ++p) {

            ret_code = 0;

            if (ret == SPECIAL_START_DEFAULT && p != 0) break;

            int start_x = shapes[i].nodes[p].x;
            int start_y = shapes[i].nodes[p].y;

            // ----------------------------------------------------
            // Type 1 check

            one_symbol_per_region_check = false;
            memset(mark_slash, 0, sizeof(mark_slash));
            compass_visited_cnt = 0;

            for (int j = 0; j < (int)shapes[i].nodes.size(); ++j) {
                int new_x = x + shapes[i].nodes[j].x - start_x;
                int new_y = y + shapes[i].nodes[j].y - start_y;
                int puzzle_new_x = to_puzzle_x(new_x);
                int puzzle_new_y = to_puzzle_y(new_y);

                if (new_x < 1 || new_x > (int)puzzle_n_row || new_y < 1 || new_y > (int)puzzle_n_col) {
                    ret_code |= RET_CODE_BLOCK_AREA;
                    break;
                }

                // Type 1 check: BLOCK
                if (puzzle[puzzle_new_x][puzzle_new_y] == AREA_BLOCK) {
                    ret_code |= RET_CODE_BLOCK_AREA;
                    Node error_node = {shapes[i].nodes[j].x, shapes[i].nodes[j].y};
                    for (int skip_index : node_to_shape_index[error_node]) {
                        mark_skip_shape[skip_index] = 1;
                    }
                    break;
                }

                // Type 1 check: FILLED
                if (solve_puzzle[puzzle_new_x][puzzle_new_y] != AREA_NORMAL) {
                    ret_code |= RET_CODE_FILLED_AREA;
                    Node error_node = {shapes[i].nodes[j].x, shapes[i].nodes[j].y};
                    for (int skip_index : node_to_shape_index[error_node]) {
                        mark_skip_shape[skip_index] = 1;
                    }
                    break;
                }

                // Type 1 check: AREA_SLASH_CHECK
                if (puzzle[puzzle_new_x][puzzle_new_y] & AREA_SLASH_INDEX_BIT) {
                    int slash_index = puzzle[puzzle_new_x][puzzle_new_y] >> AREA_SLASH_INDEX_BIT_SHIFT;
                    if (mark_slash[slash_index]) {
                        ret_code |= RET_CODE_SLASH;
                        break;
                    }
                    mark_slash[slash_index] = true;
                }

                // Type 1 check: AREA_SHAPE_INDEX_CHECK
                if (puzzle[puzzle_new_x][puzzle_new_y] & AREA_SHAPE_INDEX_BIT) {
                    int target_index = puzzle[puzzle_new_x][puzzle_new_y] >> AREA_SHAPE_INDEX_BIT_SHIFT;
                    if ((uint32_t)target_index != shapes[i].shape_index) {
                        ret_code |= RET_CODE_AREA_SHAPE_INDEX;
                        break;
                    }
                }

                // Type 1 check: AREA_SHAPE_SIZE_CHECK
                if (puzzle[puzzle_new_x][puzzle_new_y] & AREA_SHAPE_SIZE_BIT) {
                    int target_size = puzzle[puzzle_new_x][puzzle_new_y] >> AREA_SHAPE_SIZE_BIT_SHIFT;
                    if ((size_t)target_size != shapes[i].nodes.size()) {
                        ret_code |= RET_CODE_AREA_SHAPE_SIZE;
                        break;
                    }
                }

                // Type 1 check: ONE_SYMBOL_PER_REGION
                if (config.one_symbol_per_region) {
                    if (area_contain_symbol(new_x, new_y)) {
                        if (one_symbol_per_region_check) {
                            ret_code |= RET_CODE_ONE_SYMBOL_PER_REGION;
                            break;
                        }
                        one_symbol_per_region_check = true;
                    }
                }

                // Type 1 check: AREA_COMPASS_CHECK
                if (puzzle[puzzle_new_x][puzzle_new_y] & AREA_COMPASS_ENABLE) {
                    compass_visited[compass_visited_cnt] = {new_x, new_y};
                    compass_visited_cnt++;
                }
            }

            // Type 1 check: AREA_SLASH_CHECK
            if (slash_check_enable) {
                for (int j = 1; j <= slash_check_slash_cnt; ++j) {
                    if (!mark_slash[j]) {
                        ret_code |= RET_CODE_SLASH;
                        break;
                    }
                }
            }

            // Type 1 check: ONE_SYMBOL_PER_REGION
            if (config.one_symbol_per_region) {
                if (!one_symbol_per_region_check) {
                    ret_code |= RET_CODE_ONE_SYMBOL_PER_REGION;
                }
            }

            // Type 1 check: AREA_COMPASS_CHECK
            for (int j = 0; j < compass_visited_cnt; ++j) {
                int compass_x = compass_visited[j].x;
                int compass_y = compass_visited[j].y;
                int compass_puzzle_x = to_puzzle_x(compass_x);
                int compass_puzzle_y = to_puzzle_y(compass_y);

                CompassStates compass_states;
                for (int k = 0; k < (int)shapes[i].nodes.size(); ++k) {
                    int new_x = x + shapes[i].nodes[k].x - start_x;
                    int new_y = y + shapes[i].nodes[k].y - start_y;
                    if (new_x < compass_x) compass_states.up++;
                    if (new_x > compass_x) compass_states.down++;
                    if (new_y < compass_y) compass_states.left++;
                    if (new_y > compass_y) compass_states.right++;
                }

                if (puzzle_compass_up[compass_puzzle_x][compass_puzzle_y] != -1 && puzzle_compass_up[compass_puzzle_x][compass_puzzle_y] != compass_states.up)
                { ret_code |= RET_CODE_COMPASS; break; }
                if (puzzle_compass_down[compass_puzzle_x][compass_puzzle_y] != -1 && puzzle_compass_down[compass_puzzle_x][compass_puzzle_y] != compass_states.down)
                { ret_code |= RET_CODE_COMPASS; break; }
                if (puzzle_compass_left[compass_puzzle_x][compass_puzzle_y] != -1 && puzzle_compass_left[compass_puzzle_x][compass_puzzle_y] != compass_states.left)
                { ret_code |= RET_CODE_COMPASS; break; }
                if (puzzle_compass_right[compass_puzzle_x][compass_puzzle_y] != -1 && puzzle_compass_right[compass_puzzle_x][compass_puzzle_y] != compass_states.right)
                { ret_code |= RET_CODE_COMPASS; break; }
            }

            // Type 1 check finished
            if (ret_code != 0) continue;
            // ----------------------------------------------------

            // ----------------------------------------------------
            // Type 2 check

            int l = 0;

            for (; l < (int)shapes[i].nodes.size(); ++l) {
                int new_x = x + shapes[i].nodes[l].x - start_x;
                int new_y = y + shapes[i].nodes[l].y - start_y;
                int puzzle_new_x = to_puzzle_x(new_x);
                int puzzle_new_y = to_puzzle_y(new_y);

                solve_puzzle[puzzle_new_x][puzzle_new_y] = index | (shapes[i].shape_index << SOLVE_AREA_SHAPE_INDEX_BIT_SHIFT);

                if (!check_edge(puzzle_new_x, puzzle_new_y, puzzle_n_row, puzzle_n_col, solve_puzzle)) {
                    ret_code |= RET_CODE_IN_SHAPE_EDGE;
                    break;
                }

                if (!check_edge_shape(puzzle_new_x, puzzle_new_y, puzzle_n_row, puzzle_n_col, solve_puzzle)) {
                    ret_code |= RET_CODE_EDGE_CONSTRAINT;
                    break;
                }

                if (config.adjacent_shapes_different && !check_nearby_shape(puzzle_new_x, puzzle_new_y, puzzle_n_row, puzzle_n_col, solve_puzzle)) {
                    ret_code |= RET_CODE_ADJACENT_SHAPE_DIFFERENT;
                    break;
                }

                if (config.adjacent_sizes_different && !check_nearby_size(puzzle_new_x, puzzle_new_y, puzzle_n_row, puzzle_n_col, solve_puzzle)) {
                    ret_code |= RET_CODE_ADJACENT_SIZE_DIFFERENT;
                    break;
                }
            }

            // Type 2 check finished
            if (ret_code != 0) {
                for (int j = 0; j <= l; ++j) {
                    int new_x = x + shapes[i].nodes[j].x - start_x;
                    int new_y = y + shapes[i].nodes[j].y - start_y;
                    solve_puzzle[to_puzzle_x(new_x)][to_puzzle_y(new_y)] = AREA_NORMAL;
                }
                continue;
            }
            // ----------------------------------------------------

            // ----------------------------------------------------
            // Type 3 check

            for (int j = 0; j < (int)shapes[i].nodes.size(); ++j) {
                int new_x = x + shapes[i].nodes[j].x - start_x;
                int new_y = y + shapes[i].nodes[j].y - start_y;
                int new_puzzle_x = to_puzzle_x(new_x);
                int new_puzzle_y = to_puzzle_y(new_y);

                if (puzzle[new_puzzle_x][new_puzzle_y] & AREA_PALISADE_INDEX_BIT) {
                    if (!check_palisade_type2(new_puzzle_x, new_puzzle_y, puzzle_n_row, puzzle_n_col, solve_puzzle)) {
                        ret_code |= RET_CODE_PALISADE;
                        break;
                    }
                }

                if (!check_radar(new_puzzle_x, new_puzzle_y, puzzle_n_row, puzzle_n_col, solve_puzzle)) {
                    ret_code |= RET_CODE_RADAR;
                    break;
                }

                if (config.no_4_way_intersections) {
                    if (!check_tatami(new_puzzle_x, new_puzzle_y, puzzle_n_row, puzzle_n_col, solve_puzzle)) {
                        ret_code |= RET_CODE_TATAMI;
                        break;
                    }
                }

                if (config.no_3_way_intersections) {
                    if (!check_loopy(new_puzzle_x, new_puzzle_y, puzzle_n_row, puzzle_n_col, solve_puzzle)) {
                        ret_code |= RET_CODE_LOOPY;
                        break;
                    }
                }
            }

            // Type 3 check finished
            if (ret_code != 0) {
                for (int j = 0; j < (int)shapes[i].nodes.size(); ++j) {
                    int new_x = x + shapes[i].nodes[j].x - start_x;
                    int new_y = y + shapes[i].nodes[j].y - start_y;
                    solve_puzzle[to_puzzle_x(new_x)][to_puzzle_y(new_y)] = AREA_NORMAL;
                }
                continue;
            }
            // ----------------------------------------------------

            // ----------------------------------------------------
            // Type 4 check

            if (!empty_area_check(solve_puzzle)) {
                ret_code |= RET_CODE_EMPTY_CHECK;
            }

            // Type 4 check finished
            if (ret_code != 0) {
                for (int j = 0; j < (int)shapes[i].nodes.size(); ++j) {
                    int new_x = x + shapes[i].nodes[j].x - start_x;
                    int new_y = y + shapes[i].nodes[j].y - start_y;
                    solve_puzzle[to_puzzle_x(new_x)][to_puzzle_y(new_y)] = AREA_NORMAL;
                }
                continue;
            }
            // ----------------------------------------------------

            // ----------------------------------------------------
            // DFS

            bool first_shape_flag = false;
            if (config.all_shapes_same && all_shapes_same_check_shape_index == -1) {
                all_shapes_same_check_shape_index = shapes[i].shape_index;
                first_shape_flag = true;
            }

            if (config.all_shapes_different) {
                all_shapes_different_check_shape_index_pool.insert(shapes[i].shape_index);
            }

            int dfs_ret = DFS(index + 1, solve_puzzle);
            if (dfs_ret != -1) {
                return dfs_ret;
            }

            if (first_shape_flag) {
                all_shapes_same_check_shape_index = -1;
            }

            if (config.all_shapes_different) {
                all_shapes_different_check_shape_index_pool.erase(shapes[i].shape_index);
            }

            // DFS finished
            for (int j = 0; j < (int)shapes[i].nodes.size(); ++j) {
                int new_x = x + shapes[i].nodes[j].x - start_x;
                int new_y = y + shapes[i].nodes[j].y - start_y;
                solve_puzzle[to_puzzle_x(new_x)][to_puzzle_y(new_y)] = AREA_NORMAL;
            }
            // ----------------------------------------------------
        }
    }

    if (ret == SPECIAL_START_AREA_INDEX) return -1;
    if (ret == SPECIAL_START_LINE_SAME) return -1;
    if (config.predefine_shapes_only || config.only_rectangles) return -1;
    if (config.all_shapes_same && all_shapes_same_check_shape_index != -1) return -1;

    int known_shape_index = -1;
    if (!shapes.empty()) {
        known_shape_index = shapes[shapes.size() - 1].shape_index;
    }

    for (int size = shape_size_lower_bound; size <= shape_size_upper_bound; ++size) {
        if (!mark_size[size]) continue;
        int place_ret = place_non_predifined_shape(index, x, y, size, true, known_shape_index, solve_puzzle);
        if (place_ret != -1) return place_ret;
    }

    return -1;
}
