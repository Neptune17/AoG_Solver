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
    if (area_in_puzzle_range(x - 2, y, n_row, n_col)) {
        if (solve_puzzle[x - 2][y] != AREA_NORMAL && solve_puzzle[x - 2][y] != solve_puzzle[x][y]) {
            int my_size = shape_index_to_shape_size_map[(index & SOLVE_AREA_SHAPE_INDEX_BIT) >> SOLVE_AREA_SHAPE_INDEX_BIT_SHIFT];
            int nearby_size = shape_index_to_shape_size_map[(solve_puzzle[x - 2][y] & SOLVE_AREA_SHAPE_INDEX_BIT) >> SOLVE_AREA_SHAPE_INDEX_BIT_SHIFT];
            if (my_size == nearby_size) {
                return false;
            }
        }
    }
    if (area_in_puzzle_range(x + 2, y, n_row, n_col)) {
        if (solve_puzzle[x + 2][y] != AREA_NORMAL && solve_puzzle[x + 2][y] != solve_puzzle[x][y]) {
            int my_size = shape_index_to_shape_size_map[(index & SOLVE_AREA_SHAPE_INDEX_BIT) >> SOLVE_AREA_SHAPE_INDEX_BIT_SHIFT];
            int nearby_size = shape_index_to_shape_size_map[(solve_puzzle[x + 2][y] & SOLVE_AREA_SHAPE_INDEX_BIT) >> SOLVE_AREA_SHAPE_INDEX_BIT_SHIFT];
            if (my_size == nearby_size) {
                return false;
            }
        }
    }
    if (area_in_puzzle_range(x, y - 2, n_row, n_col)) {
        if (solve_puzzle[x][y - 2] != AREA_NORMAL && solve_puzzle[x][y - 2] != solve_puzzle[x][y]) {
            int my_size = shape_index_to_shape_size_map[(index & SOLVE_AREA_SHAPE_INDEX_BIT) >> SOLVE_AREA_SHAPE_INDEX_BIT_SHIFT];
            int nearby_size = shape_index_to_shape_size_map[(solve_puzzle[x][y - 2] & SOLVE_AREA_SHAPE_INDEX_BIT) >> SOLVE_AREA_SHAPE_INDEX_BIT_SHIFT];
            if (my_size == nearby_size) {
                return false;
            }
        }
    }
    if (area_in_puzzle_range(x, y + 2, n_row, n_col)) {
        if (solve_puzzle[x][y + 2] != AREA_NORMAL && solve_puzzle[x][y + 2] != solve_puzzle[x][y]) {
            int my_size = shape_index_to_shape_size_map[(index & SOLVE_AREA_SHAPE_INDEX_BIT) >> SOLVE_AREA_SHAPE_INDEX_BIT_SHIFT];
            int nearby_size = shape_index_to_shape_size_map[(solve_puzzle[x][y + 2] & SOLVE_AREA_SHAPE_INDEX_BIT) >> SOLVE_AREA_SHAPE_INDEX_BIT_SHIFT];
            if (my_size == nearby_size) {
                return false;
            }
        }
    }
    return true;
}

// ------------------------------------------------------------
// check_edge_shape
// ------------------------------------------------------------
bool check_edge_shape(uint32_t x, uint32_t y, uint32_t n_row, uint32_t n_col, uint32_t** solve_puzzle) {
    uint32_t index = solve_puzzle[x][y];
    // Up
    if (area_in_puzzle_range(x - 2, y, n_row, n_col)) {
        if (solve_puzzle[x - 2][y] != AREA_NORMAL && puzzle[x - 1][y] & LINE_EQUAL &&
            ((solve_puzzle[x - 2][y] & SOLVE_AREA_SHAPE_INDEX_BIT) != (index & SOLVE_AREA_SHAPE_INDEX_BIT))) {
            return false;
        }
        if (solve_puzzle[x - 2][y] != AREA_NORMAL && puzzle[x - 1][y] & LINE_DIFFERENT &&
            ((solve_puzzle[x - 2][y] & SOLVE_AREA_SHAPE_INDEX_BIT) == (index & SOLVE_AREA_SHAPE_INDEX_BIT))) {
            return false;
        }
        if (solve_puzzle[x - 2][y] != AREA_NORMAL &&
            (puzzle[x - 1][y] & LINE_LARGER || puzzle[x - 1][y] & LINE_SMALLER || (puzzle[x - 1][y] & LINE_SIZE_DIFF_BIT) != 0)) {
            int left_size = shape_index_to_shape_size_map[(solve_puzzle[x - 2][y] & SOLVE_AREA_SHAPE_INDEX_BIT) >> SOLVE_AREA_SHAPE_INDEX_BIT_SHIFT];
            int right_size = shape_index_to_shape_size_map[(solve_puzzle[x][y] & SOLVE_AREA_SHAPE_INDEX_BIT) >> SOLVE_AREA_SHAPE_INDEX_BIT_SHIFT];
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
        if (solve_puzzle[x + 2][y] != AREA_NORMAL && puzzle[x + 1][y] & LINE_EQUAL &&
            ((solve_puzzle[x + 2][y] & SOLVE_AREA_SHAPE_INDEX_BIT) != (index & SOLVE_AREA_SHAPE_INDEX_BIT))) {
            return false;
        }
        if (solve_puzzle[x + 2][y] != AREA_NORMAL && puzzle[x + 1][y] & LINE_DIFFERENT &&
            ((solve_puzzle[x + 2][y] & SOLVE_AREA_SHAPE_INDEX_BIT) == (index & SOLVE_AREA_SHAPE_INDEX_BIT))) {
            return false;
        }
        if (solve_puzzle[x + 2][y] != AREA_NORMAL &&
            (puzzle[x + 1][y] & LINE_LARGER || puzzle[x + 1][y] & LINE_SMALLER || (puzzle[x + 1][y] & LINE_SIZE_DIFF_BIT) != 0)) {
            int left_size = shape_index_to_shape_size_map[(solve_puzzle[x][y] & SOLVE_AREA_SHAPE_INDEX_BIT) >> SOLVE_AREA_SHAPE_INDEX_BIT_SHIFT];
            int right_size = shape_index_to_shape_size_map[(solve_puzzle[x + 2][y] & SOLVE_AREA_SHAPE_INDEX_BIT) >> SOLVE_AREA_SHAPE_INDEX_BIT_SHIFT];
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
        if (solve_puzzle[x][y - 2] != AREA_NORMAL && puzzle[x][y - 1] & LINE_EQUAL &&
            ((solve_puzzle[x][y - 2] & SOLVE_AREA_SHAPE_INDEX_BIT) != (index & SOLVE_AREA_SHAPE_INDEX_BIT))) {
            return false;
        }
        if (solve_puzzle[x][y - 2] != AREA_NORMAL && puzzle[x][y - 1] & LINE_DIFFERENT &&
            ((solve_puzzle[x][y - 2] & SOLVE_AREA_SHAPE_INDEX_BIT) == (index & SOLVE_AREA_SHAPE_INDEX_BIT))) {
            return false;
        }
        if (solve_puzzle[x][y - 2] != AREA_NORMAL &&
            (puzzle[x][y - 1] & LINE_LARGER || puzzle[x][y - 1] & LINE_SMALLER || (puzzle[x][y - 1] & LINE_SIZE_DIFF_BIT) != 0)) {
            int up_size = shape_index_to_shape_size_map[(solve_puzzle[x][y - 2] & SOLVE_AREA_SHAPE_INDEX_BIT) >> SOLVE_AREA_SHAPE_INDEX_BIT_SHIFT];
            int down_size = shape_index_to_shape_size_map[(solve_puzzle[x][y] & SOLVE_AREA_SHAPE_INDEX_BIT) >> SOLVE_AREA_SHAPE_INDEX_BIT_SHIFT];
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
        if (solve_puzzle[x][y + 2] != AREA_NORMAL && puzzle[x][y + 1] & LINE_EQUAL &&
            ((solve_puzzle[x][y + 2] & SOLVE_AREA_SHAPE_INDEX_BIT) != (index & SOLVE_AREA_SHAPE_INDEX_BIT))) {
            return false;
        }
        if (solve_puzzle[x][y + 2] != AREA_NORMAL && puzzle[x][y + 1] & LINE_DIFFERENT &&
            ((solve_puzzle[x][y + 2] & SOLVE_AREA_SHAPE_INDEX_BIT) == (index & SOLVE_AREA_SHAPE_INDEX_BIT))) {
            return false;
        }
        if (solve_puzzle[x][y + 2] != AREA_NORMAL &&
            (puzzle[x][y + 1] & LINE_LARGER || puzzle[x][y + 1] & LINE_SMALLER || (puzzle[x][y + 1] & LINE_SIZE_DIFF_BIT) != 0)) {
            int up_size = shape_index_to_shape_size_map[(solve_puzzle[x][y] & SOLVE_AREA_SHAPE_INDEX_BIT) >> SOLVE_AREA_SHAPE_INDEX_BIT_SHIFT];
            int down_size = shape_index_to_shape_size_map[(solve_puzzle[x][y + 2] & SOLVE_AREA_SHAPE_INDEX_BIT) >> SOLVE_AREA_SHAPE_INDEX_BIT_SHIFT];
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
            std::set<uint32_t> unique_region;
            unique_region.insert(index);
            filled_count += 1;

            int offsets[3][2] = {{dx * 2, 0}, {0, dy * 2}, {dx * 2, dy * 2}};
            for (int k = 0; k < 3; ++k) {
                int nx = offsets[k][0], ny = offsets[k][1];
                if (puzzle[x + nx][y + ny] != AREA_BLOCK) {
                    if (solve_puzzle[x + nx][y + ny] != AREA_NORMAL) {
                        unique_region.insert(solve_puzzle[x + nx][y + ny]);
                        filled_count++;
                    }
                }
                else {
                    filled_count += 1;
                }
            }
            if (filled_count == 4) {
                if (unique_region.size() != (size_t)radar_value) return false;
            }
            else {
                if (unique_region.size() + (4 - filled_count) < (size_t)radar_value) return false;
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
