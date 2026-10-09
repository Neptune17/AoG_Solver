#include "checks.h"
#include "puzzle.h"
#include "shapes.h"

#include <set>
#include <algorithm>

// ------------------------------------------------------------
// check_nearby_shape
// ------------------------------------------------------------
bool check_nearby_shape(uint32_t x, uint32_t y, uint32_t n_row, uint32_t n_col, uint32_t** solve_puzzle) {
    uint32_t index = solve_puzzle[x][y];
    if (area_in_puzzle_range(x - 2, y, n_row, n_col)) {
        if (solve_puzzle[x - 2][y] != AREA_NORMAL && solve_puzzle[x - 2][y] != solve_puzzle[x][y] &&
            ((solve_puzzle[x - 2][y] & SOLVE_AREA_SHAPE_INDEX_BIT) == (index & SOLVE_AREA_SHAPE_INDEX_BIT))) {
            return false;
        }
    }
    if (area_in_puzzle_range(x + 2, y, n_row, n_col)) {
        if (solve_puzzle[x + 2][y] != AREA_NORMAL && solve_puzzle[x + 2][y] != solve_puzzle[x][y] &&
            ((solve_puzzle[x + 2][y] & SOLVE_AREA_SHAPE_INDEX_BIT) == (index & SOLVE_AREA_SHAPE_INDEX_BIT))) {
            return false;
        }
    }
    if (area_in_puzzle_range(x, y - 2, n_row, n_col)) {
        if (solve_puzzle[x][y - 2] != AREA_NORMAL && solve_puzzle[x][y - 2] != solve_puzzle[x][y] &&
            ((solve_puzzle[x][y - 2] & SOLVE_AREA_SHAPE_INDEX_BIT) == (index & SOLVE_AREA_SHAPE_INDEX_BIT))) {
            return false;
        }
    }
    if (area_in_puzzle_range(x, y + 2, n_row, n_col)) {
        if (solve_puzzle[x][y + 2] != AREA_NORMAL && solve_puzzle[x][y + 2] != solve_puzzle[x][y] &&
            ((solve_puzzle[x][y + 2] & SOLVE_AREA_SHAPE_INDEX_BIT) == (index & SOLVE_AREA_SHAPE_INDEX_BIT))) {
            return false;
        }
    }
    return true;
}

// ------------------------------------------------------------
// check_nearby_size
// ------------------------------------------------------------
bool check_nearby_size(uint32_t x, uint32_t y, uint32_t n_row, uint32_t n_col, uint32_t** solve_puzzle) {
    uint32_t index = solve_puzzle[x][y];
    uint32_t my_key = (index & SOLVE_AREA_SHAPE_INDEX_BIT) >> SOLVE_AREA_SHAPE_INDEX_BIT_SHIFT;
    int my_size = shape_size_by_index[my_key];
    if (area_in_puzzle_range(x - 2, y, n_row, n_col)) {
        if (solve_puzzle[x - 2][y] != AREA_NORMAL && solve_puzzle[x - 2][y] != AREA_BLOCK && solve_puzzle[x - 2][y] != solve_puzzle[x][y]) {
            int nearby_size = shape_size_by_index[(solve_puzzle[x - 2][y] & SOLVE_AREA_SHAPE_INDEX_BIT) >> SOLVE_AREA_SHAPE_INDEX_BIT_SHIFT];
            if (my_size == nearby_size) return false;
        }
    }
    if (area_in_puzzle_range(x + 2, y, n_row, n_col)) {
        if (solve_puzzle[x + 2][y] != AREA_NORMAL && solve_puzzle[x + 2][y] != AREA_BLOCK && solve_puzzle[x + 2][y] != solve_puzzle[x][y]) {
            int nearby_size = shape_size_by_index[(solve_puzzle[x + 2][y] & SOLVE_AREA_SHAPE_INDEX_BIT) >> SOLVE_AREA_SHAPE_INDEX_BIT_SHIFT];
            if (my_size == nearby_size) return false;
        }
    }
    if (area_in_puzzle_range(x, y - 2, n_row, n_col)) {
        if (solve_puzzle[x][y - 2] != AREA_NORMAL && solve_puzzle[x][y - 2] != AREA_BLOCK && solve_puzzle[x][y - 2] != solve_puzzle[x][y]) {
            int nearby_size = shape_size_by_index[(solve_puzzle[x][y - 2] & SOLVE_AREA_SHAPE_INDEX_BIT) >> SOLVE_AREA_SHAPE_INDEX_BIT_SHIFT];
            if (my_size == nearby_size) return false;
        }
    }
    if (area_in_puzzle_range(x, y + 2, n_row, n_col)) {
        if (solve_puzzle[x][y + 2] != AREA_NORMAL && solve_puzzle[x][y + 2] != AREA_BLOCK && solve_puzzle[x][y + 2] != solve_puzzle[x][y]) {
            int nearby_size = shape_size_by_index[(solve_puzzle[x][y + 2] & SOLVE_AREA_SHAPE_INDEX_BIT) >> SOLVE_AREA_SHAPE_INDEX_BIT_SHIFT];
            if (my_size == nearby_size) return false;
        }
    }
    return true;
}

// ------------------------------------------------------------
// check_edge_shape
// ------------------------------------------------------------
bool check_edge_shape(uint32_t x, uint32_t y, uint32_t n_row, uint32_t n_col, uint32_t** solve_puzzle) {
    uint32_t index = solve_puzzle[x][y];
    uint32_t my_key = (index & SOLVE_AREA_SHAPE_INDEX_BIT) >> SOLVE_AREA_SHAPE_INDEX_BIT_SHIFT;
    // Up
    if (area_in_puzzle_range(x - 2, y, n_row, n_col)) {
        if (solve_puzzle[x - 2][y] != AREA_NORMAL && solve_puzzle[x - 2][y] != AREA_BLOCK && puzzle[x - 1][y] & LINE_EQUAL &&
            ((solve_puzzle[x - 2][y] & SOLVE_AREA_SHAPE_INDEX_BIT) != (index & SOLVE_AREA_SHAPE_INDEX_BIT))) {
            return false;
        }
        if (solve_puzzle[x - 2][y] != AREA_NORMAL && solve_puzzle[x - 2][y] != AREA_BLOCK && puzzle[x - 1][y] & LINE_DIFFERENT &&
            ((solve_puzzle[x - 2][y] & SOLVE_AREA_SHAPE_INDEX_BIT) == (index & SOLVE_AREA_SHAPE_INDEX_BIT))) {
            return false;
        }
        if (solve_puzzle[x - 2][y] != AREA_NORMAL && solve_puzzle[x - 2][y] != AREA_BLOCK &&
            (puzzle[x - 1][y] & LINE_LARGER || puzzle[x - 1][y] & LINE_SMALLER || (puzzle[x - 1][y] & LINE_SIZE_DIFF_BIT) != 0)) {
            int left_size = shape_size_by_index[(solve_puzzle[x - 2][y] & SOLVE_AREA_SHAPE_INDEX_BIT) >> SOLVE_AREA_SHAPE_INDEX_BIT_SHIFT];
            int right_size = shape_size_by_index[my_key];
            if (puzzle[x - 1][y] & LINE_LARGER && left_size <= right_size) return false;
            if (puzzle[x - 1][y] & LINE_SMALLER && left_size >= right_size) return false;
            if (puzzle[x - 1][y] & LINE_SIZE_DIFF_BIT) {
                int diff_val = ((puzzle[x - 1][y] & LINE_SIZE_DIFF_BIT) >> LINE_SIZE_DIFF_BIT_SHIFT) - 1;
                if ((std::max(left_size, right_size) - std::min(left_size, right_size)) != diff_val) return false;
            }
        }
    }
    // Down
    if (area_in_puzzle_range(x + 2, y, n_row, n_col)) {
        if (solve_puzzle[x + 2][y] != AREA_NORMAL && solve_puzzle[x + 2][y] != AREA_BLOCK && puzzle[x + 1][y] & LINE_EQUAL &&
            ((solve_puzzle[x + 2][y] & SOLVE_AREA_SHAPE_INDEX_BIT) != (index & SOLVE_AREA_SHAPE_INDEX_BIT))) {
            return false;
        }
        if (solve_puzzle[x + 2][y] != AREA_NORMAL && solve_puzzle[x + 2][y] != AREA_BLOCK && puzzle[x + 1][y] & LINE_DIFFERENT &&
            ((solve_puzzle[x + 2][y] & SOLVE_AREA_SHAPE_INDEX_BIT) == (index & SOLVE_AREA_SHAPE_INDEX_BIT))) {
            return false;
        }
        if (solve_puzzle[x + 2][y] != AREA_NORMAL && solve_puzzle[x + 2][y] != AREA_BLOCK &&
            (puzzle[x + 1][y] & LINE_LARGER || puzzle[x + 1][y] & LINE_SMALLER || (puzzle[x + 1][y] & LINE_SIZE_DIFF_BIT) != 0)) {
            int left_size = shape_size_by_index[my_key];
            int right_size = shape_size_by_index[(solve_puzzle[x + 2][y] & SOLVE_AREA_SHAPE_INDEX_BIT) >> SOLVE_AREA_SHAPE_INDEX_BIT_SHIFT];
            if (puzzle[x + 1][y] & LINE_LARGER && left_size <= right_size) return false;
            if (puzzle[x + 1][y] & LINE_SMALLER && left_size >= right_size) return false;
            if (puzzle[x + 1][y] & LINE_SIZE_DIFF_BIT) {
                int diff_val = ((puzzle[x + 1][y] & LINE_SIZE_DIFF_BIT) >> LINE_SIZE_DIFF_BIT_SHIFT) - 1;
                if ((std::max(left_size, right_size) - std::min(left_size, right_size)) != diff_val) return false;
            }
        }
    }
    // Left
    if (area_in_puzzle_range(x, y - 2, n_row, n_col)) {
        if (solve_puzzle[x][y - 2] != AREA_NORMAL && solve_puzzle[x][y - 2] != AREA_BLOCK && puzzle[x][y - 1] & LINE_EQUAL &&
            ((solve_puzzle[x][y - 2] & SOLVE_AREA_SHAPE_INDEX_BIT) != (index & SOLVE_AREA_SHAPE_INDEX_BIT))) {
            return false;
        }
        if (solve_puzzle[x][y - 2] != AREA_NORMAL && solve_puzzle[x][y - 2] != AREA_BLOCK && puzzle[x][y - 1] & LINE_DIFFERENT &&
            ((solve_puzzle[x][y - 2] & SOLVE_AREA_SHAPE_INDEX_BIT) == (index & SOLVE_AREA_SHAPE_INDEX_BIT))) {
            return false;
        }
        if (solve_puzzle[x][y - 2] != AREA_NORMAL && solve_puzzle[x][y - 2] != AREA_BLOCK &&
            (puzzle[x][y - 1] & LINE_LARGER || puzzle[x][y - 1] & LINE_SMALLER || (puzzle[x][y - 1] & LINE_SIZE_DIFF_BIT) != 0)) {
            int up_size = shape_size_by_index[(solve_puzzle[x][y - 2] & SOLVE_AREA_SHAPE_INDEX_BIT) >> SOLVE_AREA_SHAPE_INDEX_BIT_SHIFT];
            int down_size = shape_size_by_index[my_key];
            if (puzzle[x][y - 1] & LINE_LARGER && up_size <= down_size) return false;
            if (puzzle[x][y - 1] & LINE_SMALLER && up_size >= down_size) return false;
            if (puzzle[x][y - 1] & LINE_SIZE_DIFF_BIT) {
                int diff_val = ((puzzle[x][y - 1] & LINE_SIZE_DIFF_BIT) >> LINE_SIZE_DIFF_BIT_SHIFT) - 1;
                if ((std::max(up_size, down_size) - std::min(up_size, down_size)) != diff_val) return false;
            }
        }
    }
    // Right
    if (area_in_puzzle_range(x, y + 2, n_row, n_col)) {
        if (solve_puzzle[x][y + 2] != AREA_NORMAL && solve_puzzle[x][y + 2] != AREA_BLOCK && puzzle[x][y + 1] & LINE_EQUAL &&
            ((solve_puzzle[x][y + 2] & SOLVE_AREA_SHAPE_INDEX_BIT) != (index & SOLVE_AREA_SHAPE_INDEX_BIT))) {
            return false;
        }
        if (solve_puzzle[x][y + 2] != AREA_NORMAL && solve_puzzle[x][y + 2] != AREA_BLOCK && puzzle[x][y + 1] & LINE_DIFFERENT &&
            ((solve_puzzle[x][y + 2] & SOLVE_AREA_SHAPE_INDEX_BIT) == (index & SOLVE_AREA_SHAPE_INDEX_BIT))) {
            return false;
        }
        if (solve_puzzle[x][y + 2] != AREA_NORMAL && solve_puzzle[x][y + 2] != AREA_BLOCK &&
            (puzzle[x][y + 1] & LINE_LARGER || puzzle[x][y + 1] & LINE_SMALLER || (puzzle[x][y + 1] & LINE_SIZE_DIFF_BIT) != 0)) {
            int up_size = shape_size_by_index[my_key];
            int down_size = shape_size_by_index[(solve_puzzle[x][y + 2] & SOLVE_AREA_SHAPE_INDEX_BIT) >> SOLVE_AREA_SHAPE_INDEX_BIT_SHIFT];
            if (puzzle[x][y + 1] & LINE_LARGER && up_size <= down_size) return false;
            if (puzzle[x][y + 1] & LINE_SMALLER && up_size >= down_size) return false;
            if (puzzle[x][y + 1] & LINE_SIZE_DIFF_BIT) {
                int diff_val = ((puzzle[x][y + 1] & LINE_SIZE_DIFF_BIT) >> LINE_SIZE_DIFF_BIT_SHIFT) - 1;
                if ((std::max(up_size, down_size) - std::min(up_size, down_size)) != diff_val) return false;
            }
        }
    }
    return true;
}

// ------------------------------------------------------------
// check_edge
// ------------------------------------------------------------
bool check_edge(uint32_t x, uint32_t y, uint32_t n_row, uint32_t n_col, uint32_t** solve_puzzle) {
    uint32_t index = solve_puzzle[x][y];
    if (area_in_puzzle_range(x - 2, y, n_row, n_col)) {
        if (solve_puzzle[x - 2][y] == index && (puzzle[x - 1][y] & LINE_BLOCK)) {
            return false;
        }
    }
    if (area_in_puzzle_range(x + 2, y, n_row, n_col)) {
        if (solve_puzzle[x + 2][y] == index && (puzzle[x + 1][y] & LINE_BLOCK)) {
            return false;
        }
    }
    if (area_in_puzzle_range(x, y - 2, n_row, n_col)) {
        if (solve_puzzle[x][y - 2] == index && (puzzle[x][y - 1] & LINE_BLOCK)) {
            return false;
        }
    }
    if (area_in_puzzle_range(x, y + 2, n_row, n_col)) {
        if (solve_puzzle[x][y + 2] == index && (puzzle[x][y + 1] & LINE_BLOCK)) {
            return false;
        }
    }
    return true;
}

// ------------------------------------------------------------
// check_palisade_type2 (final check after shape placement)
// ------------------------------------------------------------
bool check_palisade_type2(uint32_t x, uint32_t y, uint32_t n_row, uint32_t n_col, uint32_t** solve_puzzle) {
    uint32_t index = solve_puzzle[x][y];
    int palisade_type = (puzzle[x][y] & AREA_PALISADE_INDEX_BIT) >> AREA_PALISADE_INDEX_BIT_SHIFT;

    bool up = false, down = false, left = false, right = false;
    if (area_in_puzzle_range(x - 2, y, n_row, n_col)) {
        if (solve_puzzle[x - 2][y] == index) left = true;
    }
    if (area_in_puzzle_range(x + 2, y, n_row, n_col)) {
        if (solve_puzzle[x + 2][y] == index) right = true;
    }
    if (area_in_puzzle_range(x, y - 2, n_row, n_col)) {
        if (solve_puzzle[x][y - 2] == index) up = true;
    }
    if (area_in_puzzle_range(x, y + 2, n_row, n_col)) {
        if (solve_puzzle[x][y + 2] == index) down = true;
    }

    if (palisade_type == 1) {
        if (!up || !down || !left || !right) return false;
    }
    else if (palisade_type == 2) {
        if ((up + down + left + right) != 3) return false;
    }
    else if (palisade_type == 3) {
        if ((up + down + left + right) != 2) return false;
        if (!((up && down) || (left && right))) return false;
    }
    else if (palisade_type == 4) {
        if ((up + down + left + right) != 1) return false;
    }
    else if (palisade_type == 5) {
        int sum = up + down + left + right;
        if (sum != 2) return false;
        if (!((up && right) || (right && down) || (down && left) || (left && up))) return false;
    }
    else if (palisade_type == 6) {
        if ((up + down + left + right) > 0) return false;
    }

    return true;
}

// ------------------------------------------------------------
// check_palisade_type1 (delta check during shape expansion)
// ------------------------------------------------------------
bool check_palisade_type1(uint32_t x, uint32_t y, uint32_t n_row, uint32_t n_col, uint32_t** solve_puzzle) {
    uint32_t index = solve_puzzle[x][y];
    int palisade_type = (puzzle[x][y] & AREA_PALISADE_INDEX_BIT) >> AREA_PALISADE_INDEX_BIT_SHIFT;

    bool up = false, down = false, left = false, right = false;
    if (area_in_puzzle_range(x - 2, y, n_row, n_col)) {
        if (solve_puzzle[x - 2][y] == index) left = true;
    }
    if (area_in_puzzle_range(x + 2, y, n_row, n_col)) {
        if (solve_puzzle[x + 2][y] == index) right = true;
    }
    if (area_in_puzzle_range(x, y - 2, n_row, n_col)) {
        if (solve_puzzle[x][y - 2] == index) up = true;
    }
    if (area_in_puzzle_range(x, y + 2, n_row, n_col)) {
        if (solve_puzzle[x][y + 2] == index) down = true;
    }

    if (palisade_type == 1) {
        // Always valid during delta check
    }
    else if (palisade_type == 2) {
        int sum = up + down + left + right;
        if (sum > 3) return false;
    }
    else if (palisade_type == 3) {
        int sum = up + down + left + right;
        if (sum > 2) return false;
        if (sum == 2 && !((up && down) || (left && right))) return false;
    }
    else if (palisade_type == 4) {
        int sum = up + down + left + right;
        if (sum > 1) return false;
    }
    else if (palisade_type == 5) {
        int sum = up + down + left + right;
        if (sum > 2) return false;
        if (sum == 2 && !((up && right) || (right && down) || (down && left) || (left && up))) return false;
    }
    else if (palisade_type == 6) {
        int sum = up + down + left + right;
        if (sum > 0) return false;
    }

    return true;
}

// ------------------------------------------------------------
// check_tatami (no 4-way intersections)
// ------------------------------------------------------------
bool check_tatami(uint32_t x, uint32_t y, uint32_t n_row, uint32_t n_col, uint32_t** solve_puzzle) {
    uint32_t index = solve_puzzle[x][y];

    if (area_in_puzzle_range(x - 2, y - 2, n_row, n_col)) {
        if (solve_puzzle[x - 2][y] != index && solve_puzzle[x][y - 2] != index &&
            solve_puzzle[x - 2][y - 2] != solve_puzzle[x][y - 2] && solve_puzzle[x - 2][y - 2] != solve_puzzle[x - 2][y]) {
            return false;
        }
    }
    if (area_in_puzzle_range(x - 2, y + 2, n_row, n_col)) {
        if (solve_puzzle[x - 2][y] != index && solve_puzzle[x][y + 2] != index &&
            solve_puzzle[x - 2][y + 2] != solve_puzzle[x][y + 2] && solve_puzzle[x - 2][y + 2] != solve_puzzle[x - 2][y]) {
            return false;
        }
    }
    if (area_in_puzzle_range(x + 2, y - 2, n_row, n_col)) {
        if (solve_puzzle[x + 2][y] != index && solve_puzzle[x][y - 2] != index &&
            solve_puzzle[x + 2][y - 2] != solve_puzzle[x][y - 2] && solve_puzzle[x + 2][y - 2] != solve_puzzle[x + 2][y]) {
            return false;
        }
    }
    if (area_in_puzzle_range(x + 2, y + 2, n_row, n_col)) {
        if (solve_puzzle[x + 2][y] != index && solve_puzzle[x][y + 2] != index &&
            solve_puzzle[x + 2][y + 2] != solve_puzzle[x][y + 2] && solve_puzzle[x + 2][y + 2] != solve_puzzle[x + 2][y]) {
            return false;
        }
    }

    return true;
}

// ------------------------------------------------------------
// check_loopy (no 3-way intersections)
// ------------------------------------------------------------
bool check_loopy(uint32_t x, uint32_t y, uint32_t /*n_row*/, uint32_t /*n_col*/, uint32_t** solve_puzzle) {
    uint32_t index = solve_puzzle[x][y];

    int count = (solve_puzzle[x - 2][y] != index) + (solve_puzzle[x][y - 2] != index) +
                (solve_puzzle[x - 2][y - 2] != solve_puzzle[x][y - 2]) + (solve_puzzle[x - 2][y - 2] != solve_puzzle[x - 2][y]);
    int zero_count = (solve_puzzle[x - 2][y] == 0) + (solve_puzzle[x][y - 2] == 0) + (solve_puzzle[x - 2][y - 2] == 0);
    if (count == 3 && zero_count != 2) return false;

    count = (solve_puzzle[x - 2][y] != index) + (solve_puzzle[x][y + 2] != index) +
            (solve_puzzle[x - 2][y + 2] != solve_puzzle[x][y + 2]) + (solve_puzzle[x - 2][y + 2] != solve_puzzle[x - 2][y]);
    zero_count = (solve_puzzle[x - 2][y] == 0) + (solve_puzzle[x][y + 2] == 0) + (solve_puzzle[x - 2][y + 2] == 0);
    if (count == 3 && zero_count != 2) return false;

    count = (solve_puzzle[x + 2][y] != index) + (solve_puzzle[x][y - 2] != index) +
            (solve_puzzle[x + 2][y - 2] != solve_puzzle[x][y - 2]) + (solve_puzzle[x + 2][y - 2] != solve_puzzle[x + 2][y]);
    zero_count = (solve_puzzle[x + 2][y] == 0) + (solve_puzzle[x][y - 2] == 0) + (solve_puzzle[x + 2][y - 2] == 0);
    if (count == 3 && zero_count != 2) return false;

    count = (solve_puzzle[x + 2][y] != index) + (solve_puzzle[x][y + 2] != index) +
            (solve_puzzle[x + 2][y + 2] != solve_puzzle[x][y + 2]) + (solve_puzzle[x + 2][y + 2] != solve_puzzle[x + 2][y]);
    zero_count = (solve_puzzle[x + 2][y] == 0) + (solve_puzzle[x][y + 2] == 0) + (solve_puzzle[x + 2][y + 2] == 0);
    if (count == 3 && zero_count != 2) return false;

    return true;
}

// ------------------------------------------------------------
// check_radar
// ------------------------------------------------------------
bool check_radar(uint32_t x, uint32_t y, uint32_t /*n_row*/, uint32_t /*n_col*/, uint32_t** solve_puzzle) {

    uint32_t index = solve_puzzle[x][y];
    auto check_corner = [&](int dx, int dy) {
        if (puzzle[x + dx][y + dy] & VERTEX_RADAR_BIT) {
            int radar_value = (puzzle[x + dx][y + dy] & VERTEX_RADAR_BIT) >> VERTEX_RADAR_BIT_SHIFT;
            int filled_count = 0;
            uint32_t regions[4];
            int region_cnt = 0;
            regions[region_cnt++] = index;
            filled_count += 1;

            int offsets[3][2] = {{dx * 2, 0}, {0, dy * 2}, {dx * 2, dy * 2}};
            for (int k = 0; k < 3; ++k) {
                int nx = offsets[k][0], ny = offsets[k][1];
                if (puzzle[x + nx][y + ny] != AREA_BLOCK) {
                    if (solve_puzzle[x + nx][y + ny] != AREA_NORMAL) {
                        uint32_t r = solve_puzzle[x + nx][y + ny];
                        bool dup = false;
                        for (int m = 0; m < region_cnt; ++m)
                            if (regions[m] == r) { dup = true; break; }
                        if (!dup) regions[region_cnt++] = r;
                        filled_count++;
                    }
                }
                else {
                    filled_count += 1;
                }
            }
            if (filled_count == 4) {
                if (region_cnt != radar_value) return false;
            }
            else {
                if (region_cnt + (4 - filled_count) < radar_value) return false;
            }
        }
        return true;
    };

    if (!check_corner(-1, -1)) return false;
    if (!check_corner(-1, +1)) return false;
    if (!check_corner(+1, -1)) return false;
    if (!check_corner(+1, +1)) return false;

    return true;
}

// ------------------------------------------------------------
// palisade_derive - forced/forbidden neighbor propagation
// ------------------------------------------------------------
// Neighbor slots: 0=up, 1=down, 2=left, 3=right
// nb_state: 0=open, 1=in_shape, 2=dead
PalisadeDeriveResult palisade_derive(int palisade_type, const uint8_t nb_state[4]) {
    PalisadeDeriveResult res{false, 0, 0};

    int in_cnt = 0, open_cnt = 0;
    for (int i = 0; i < 4; ++i) {
        if (nb_state[i] == 1) in_cnt++;
        else if (nb_state[i] == 0) open_cnt++;
    }

    // Quota-based types: exactly `quota` neighbors in shape
    auto derive_quota = [&](int quota) {
        int need = quota - in_cnt;
        if (need < 0 || need > open_cnt) { res.fail = true; return; }
        if (need == 0) {
            for (int i = 0; i < 4; ++i)
                if (nb_state[i] == 0) res.forbidden_mask |= (1 << i);
        }
        else if (need == open_cnt) {
            for (int i = 0; i < 4; ++i)
                if (nb_state[i] == 0) res.forced_mask |= (1 << i);
        }
    };

    // Pair-based types: exactly one of the given neighbor pairs in shape
    auto derive_pair = [&](const uint8_t pairs[][2], int pair_cnt) {
        uint8_t remain_intersect = 0xf; // cells in every remaining pair
        uint8_t remain_union = 0;       // cells in some remaining pair
        int remain_cnt = 0;
        for (int p = 0; p < pair_cnt; ++p) {
            int a = pairs[p][0], b = pairs[p][1];
            // pair impossible if an endpoint is dead, or an in-shape cell
            // exists outside this pair (the final pair must contain it)
            if (nb_state[a] == 2 || nb_state[b] == 2) continue;
            bool covers_all_in = true;
            for (int i = 0; i < 4; ++i)
                if (nb_state[i] == 1 && i != a && i != b) { covers_all_in = false; break; }
            if (!covers_all_in) continue;
            remain_cnt++;
            remain_intersect &= (uint8_t)((1 << a) | (1 << b));
            remain_union |= (uint8_t)((1 << a) | (1 << b));
        }
        if (remain_cnt == 0) { res.fail = true; return; }
        for (int i = 0; i < 4; ++i) {
            if (remain_intersect & (1 << i)) {
                if (nb_state[i] == 0) res.forced_mask |= (1 << i);
            }
            else if (!(remain_union & (1 << i))) {
                if (nb_state[i] == 1) { res.fail = true; return; }
                if (nb_state[i] == 0) res.forbidden_mask |= (1 << i);
            }
        }
    };

    static const uint8_t straight_pairs[2][2] = {{0, 1}, {2, 3}};
    static const uint8_t corner_pairs[4][2] = {{0, 3}, {3, 1}, {1, 2}, {2, 0}};

    switch (palisade_type) {
        case 1: // F0: all 4 neighbors in shape
            for (int i = 0; i < 4; ++i) {
                if (nb_state[i] == 2) { res.fail = true; return res; }
                if (nb_state[i] == 0) res.forced_mask |= (1 << i);
            }
            break;
        case 6: // F4: no neighbor in shape
            if (in_cnt > 0) { res.fail = true; return res; }
            for (int i = 0; i < 4; ++i)
                if (nb_state[i] == 0) res.forbidden_mask |= (1 << i);
            break;
        case 2: // F1: exactly 3 neighbors in shape
            derive_quota(3);
            break;
        case 4: // F3: exactly 1 neighbor in shape
            derive_quota(1);
            break;
        case 3: // F2: exactly 2 neighbors in shape, straight line
            derive_pair(straight_pairs, 2);
            break;
        case 5: // F7: exactly 2 neighbors in shape, corner
            derive_pair(corner_pairs, 4);
            break;
    }
    return res;
}

// ------------------------------------------------------------
// loopy_derive_vertex - forced/forbidden propagation for one vertex
// ------------------------------------------------------------
// Cell cycle around the vertex: E - P - O - Q - E. E is empty, P/O/Q resolved.
// deg(E joins cur)  = (P != cur) + (Q != cur) + t
// deg(E fresh region) = 2 + t   (fresh differs from both resolved neighbors)
// where t = (P != O) + (O != Q) is fixed.
LoopyVertexResult loopy_derive_vertex(uint32_t id_P, uint32_t id_O, uint32_t id_Q, uint32_t cur) {
    int t = (id_P != id_O) + (id_O != id_Q);
    bool joinable = (id_P == cur) || (id_Q == cur);
    bool fresh_ok = (2 + t) != 3;
    bool join_ok = joinable && ((id_P != cur) + (id_Q != cur) + t) != 3;
    if (!fresh_ok && !join_ok) return LOOPY_FAIL;
    if (!fresh_ok) return LOOPY_FORCE;
    if (joinable && !join_ok) return LOOPY_FORBID;
    return LOOPY_NONE;
}

// ------------------------------------------------------------
// tatami_derive_vertex - forced propagation for one vertex
// ------------------------------------------------------------
// Cell cycle around the vertex: E - P - O - Q - E. E is empty, P/O/Q resolved.
// deg(E joins cur)    = (P != cur) + (Q != cur) + t   (never 4 when joinable)
// deg(E fresh region) = 2 + t                         (4 exactly when t == 2)
// where t = (P != O) + (O != Q) is fixed.
TatamiVertexResult tatami_derive_vertex(uint32_t id_P, uint32_t id_O, uint32_t id_Q, uint32_t cur) {
    int t = (id_P != id_O) + (id_O != id_Q);
    if (t != 2) return TATAMI_NONE; // fresh outcome valid, join never invalid
    bool joinable = (id_P == cur) || (id_Q == cur);
    // join outcome is valid iff at most one of P/Q is outside the shape
    bool join_ok = joinable && ((id_P != cur) + (id_Q != cur) + t) != 4;
    if (!join_ok) return TATAMI_FAIL;
    return TATAMI_FORCE;
}
