#include "puzzle.h"

// ------------------------------------------------------------
// Global puzzle configuration flags
// ------------------------------------------------------------
PuzzleConfig config;

// ------------------------------------------------------------
// Puzzle grid state
// ------------------------------------------------------------
uint32_t puzzle_n_row;
uint32_t puzzle_n_col;

uint32_t puzzle[MAX_PUZZLE_SIZE][MAX_PUZZLE_SIZE];
int puzzle_compass_up[MAX_PUZZLE_SIZE][MAX_PUZZLE_SIZE];
int puzzle_compass_down[MAX_PUZZLE_SIZE][MAX_PUZZLE_SIZE];
int puzzle_compass_left[MAX_PUZZLE_SIZE][MAX_PUZZLE_SIZE];
int puzzle_compass_right[MAX_PUZZLE_SIZE][MAX_PUZZLE_SIZE];

bool slash_check_enable = false;
int slash_check_slash_cnt = 0;

bool shape_compare_enable = false;

int all_shapes_same_check_shape_index = -1;

std::set<uint32_t> all_shapes_different_check_shape_index_pool;

std::map<uint32_t, uint32_t> shape_index_modify_map;

// ------------------------------------------------------------
// Coordinate conversion helpers
// ------------------------------------------------------------

// ------------------------------------------------------------
// Parse functions
// ------------------------------------------------------------

uint32_t parse_vertex(char c) {
    switch (c) {
        case '+':
            return VERTEX_NORMAL;
        case ' ':
            return VERTEX_BLOCK;
        case '1':
        case '2':
        case '3':
        case '4':
            return (c - '0') << VERTEX_RADAR_BIT_SHIFT;
        default:
            return VERTEX_PARSE_ERROR;
    }
}

uint32_t parse_line(char c) {
    switch (c) {
        case '|':
            return LINE_NORMAL;
        case '-':
            return LINE_NORMAL;
        case ' ':
            return LINE_BLOCK;
        case '#':
            return LINE_BLOCK;
        case '=':
            return LINE_BLOCK | LINE_EQUAL;
        case '!':
            return LINE_BLOCK | LINE_DIFFERENT;
        case '<':
            return LINE_BLOCK | LINE_SMALLER;
        case '>':
            return LINE_BLOCK | LINE_LARGER;
        case '^':
            return LINE_BLOCK | LINE_SMALLER;
        case 'v':
            return LINE_BLOCK | LINE_LARGER;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            return LINE_BLOCK | ((c - '0' + 1) << LINE_SIZE_DIFF_BIT_SHIFT);
        default:
            return LINE_PARSE_ERROR;
    }
}

uint32_t parse_area(char c1, char c2, char c3) {
    switch (c1) {
        case '.':
            return AREA_NORMAL;
        case ' ':
            return AREA_BLOCK;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            return ((c1 - '0') * 10 + (c2 - '0')) << AREA_SHAPE_SIZE_BIT_SHIFT;
        case 'S':
            if (c3 != 'X') {
                return ((c2 - '0') * 10 + (c3 - '0')) << AREA_SHAPE_INDEX_BIT_SHIFT;
            }
            return (c2 - '0') << AREA_SHAPE_INDEX_BIT_SHIFT;
        case 'P':
            return (c2 - '0') << AREA_SLASH_INDEX_BIT_SHIFT;
        case 'F':
            if (c2 == '0') {
                return 1 << AREA_PALISADE_INDEX_BIT_SHIFT;
            }
            if (c2 == '1') {
                return 2 << AREA_PALISADE_INDEX_BIT_SHIFT;
            }
            if (c2 == '2') {
                return 3 << AREA_PALISADE_INDEX_BIT_SHIFT;
            }
            if (c2 == '3') {
                return 4 << AREA_PALISADE_INDEX_BIT_SHIFT;
            }
            if (c2 == '7') {
                return 5 << AREA_PALISADE_INDEX_BIT_SHIFT;
            }
            if (c2 == '4') {
                return 6 << AREA_PALISADE_INDEX_BIT_SHIFT;
            }
            [[fallthrough]];
        case 'U':
            return AREA_COMPASS_ENABLE;
        default:
            return AREA_PARSE_ERROR;
    }
}

// ------------------------------------------------------------
// Puzzle I/O
// ------------------------------------------------------------

void modify_shape_index_in_puzzle(int original_index, int new_index) {
    for (int i = 1; i < (int)(puzzle_n_row * 2) + 1 + 2; i += 2) {
        for (int j = 1; j < (int)(puzzle_n_col * 2) + 1 + 2; j += 2) {
            if ((puzzle[i][j] & AREA_SHAPE_INDEX_BIT) &&
                ((puzzle[i][j] & AREA_SHAPE_INDEX_BIT) >> AREA_SHAPE_INDEX_BIT_SHIFT) == (uint32_t)original_index) {
                puzzle[i][j] = (puzzle[i][j] & ~AREA_SHAPE_INDEX_BIT) | (new_index << AREA_SHAPE_INDEX_BIT_SHIFT);
            }
        }
    }
}

void read_puzzle() {

    // Puzzle Fence
    for (int j = 0; j < (int)(puzzle_n_col * 2) + 1 + 4; ++j) {
        puzzle[0][j] = LINE_BLOCK;
        puzzle[1][j] = LINE_BLOCK;
        puzzle[(puzzle_n_row * 2) + 3][j] = LINE_BLOCK;
        puzzle[(puzzle_n_row * 2) + 4][j] = LINE_BLOCK;
    }
    for (int i = 0; i < (int)(puzzle_n_row * 2) + 1 + 4; ++i) {
        puzzle[i][0] = LINE_BLOCK;
        puzzle[i][1] = LINE_BLOCK;
        puzzle[i][(puzzle_n_col * 2) + 3] = LINE_BLOCK;
        puzzle[i][(puzzle_n_col * 2) + 4] = LINE_BLOCK;
    }

    std::string line;
    for (int i = 0; i < (int)(puzzle_n_row * 2) + 1; ++i) {
        std::getline(std::cin, line);
        if (i & 1) { // area line
            int next_status = 0; // 0 for line, 1 for area
            for (int j = 0; j < (int)(puzzle_n_col * 3) + 1; j = j + 3) {
                puzzle[i + 2][(j / 3) * 2 + 2] = parse_line(line[j]);
            }
            int size = (puzzle_n_col * 3) + 1;
            int j = 0;
            int index_j = 2;
            while (j < size) {
                if (next_status == 0) {
                    puzzle[i + 2][index_j] = parse_line(line[j]);
                    index_j += 1;
                    j += 1;
                    next_status = 1;
                }
                else {
                    if (line[j] == 'S' && line[j + 2] >= '0' && line[j + 2] <= '9') {
                        puzzle[i + 2][index_j] = parse_area(line[j], line[j + 1], line[j + 2]);
                        index_j += 1;
                        j += 3;
                        size += 1;
                        next_status = 0;
                    }
                    else {
                        puzzle[i + 2][index_j] = parse_area(line[j], line[j + 1]);
                        if (puzzle[i + 2][index_j] & AREA_COMPASS_ENABLE) {
                            int compass_index = j;
                            puzzle_compass_up[i + 2][index_j] = -1;
                            if (line[compass_index + 1] != 'D' && line[compass_index + 2] != 'D') {
                                puzzle_compass_up[i + 2][index_j] = (line[compass_index + 1] - '0') * 10 + line[compass_index + 2] - '0';
                                compass_index += 2;
                            }
                            else if (line[compass_index + 1] != 'D') {
                                puzzle_compass_up[i + 2][index_j] = line[compass_index + 1] - '0';
                                compass_index += 1;
                            }
                            compass_index += 1;
                            puzzle_compass_down[i + 2][index_j] = -1;
                            if (line[compass_index + 1] != 'L' && line[compass_index + 2] != 'L') {
                                puzzle_compass_down[i + 2][index_j] = (line[compass_index + 1] - '0') * 10 + line[compass_index + 2] - '0';
                                compass_index += 2;
                            }
                            else if (line[compass_index + 1] != 'L') {
                                puzzle_compass_down[i + 2][index_j] = line[compass_index + 1] - '0';
                                compass_index += 1;
                            }
                            compass_index += 1;
                            puzzle_compass_left[i + 2][index_j] = -1;
                            if (line[compass_index + 1] != 'R' && line[compass_index + 2] != 'R') {
                                puzzle_compass_left[i + 2][index_j] = (line[compass_index + 1] - '0') * 10 + line[compass_index + 2] - '0';
                                compass_index += 2;
                            }
                            else if (line[compass_index + 1] != 'R') {
                                puzzle_compass_left[i + 2][index_j] = line[compass_index + 1] - '0';
                                compass_index += 1;
                            }
                            compass_index += 1;
                            puzzle_compass_right[i + 2][index_j] = -1;
                            if (line[compass_index + 1] >= '0' && line[compass_index + 1] <= '9' &&
                                line[compass_index + 2] >= '0' && line[compass_index + 2] <= '9') {
                                puzzle_compass_right[i + 2][index_j] = (line[compass_index + 1] - '0') * 10 + line[compass_index + 2] - '0';
                                compass_index += 2;
                            }
                            else if (line[compass_index + 1] >= '0' && line[compass_index + 1] <= '9') {
                                puzzle_compass_right[i + 2][index_j] = line[compass_index + 1] - '0';
                                compass_index += 1;
                            }
                            compass_index += 1;

                            index_j += 1;
                            size += (compass_index - j - 2);
                            j = compass_index;
                            next_status = 0;
                        }
                        else {
                            index_j += 1;
                            j += 2;
                            next_status = 0;
                        }
                    }
                }
            }
        }
        else { // node line
            for (int j = 0; j < (int)(puzzle_n_col * 3) + 1; j = j + 3) {
                puzzle[i + 2][(j / 3) * 2 + 2] = parse_vertex(line[j]);
            }
            for (int j = 2; j < (int)(puzzle_n_col * 3) + 1; j = j + 3) {
                puzzle[i + 2][((j / 3) * 2) + 1 + 2] = parse_line(line[j]);
            }
        }
    }

    for (auto &entry : shape_index_modify_map) {
        modify_shape_index_in_puzzle(entry.first, entry.second);
    }
}

void build_solve_puzzle(uint32_t** solve_puzzle) {

    // Puzzle Fence
    for (int j = 0; j < (int)(puzzle_n_col * 2) + 1 + 4; ++j) {
        solve_puzzle[0][j] = LINE_BLOCK;
        solve_puzzle[1][j] = LINE_BLOCK;
        solve_puzzle[(puzzle_n_row * 2) + 3][j] = LINE_BLOCK;
        solve_puzzle[(puzzle_n_row * 2) + 4][j] = LINE_BLOCK;
    }
    for (int i = 0; i < (int)(puzzle_n_row * 2) + 1 + 4; ++i) {
        solve_puzzle[i][0] = LINE_BLOCK;
        solve_puzzle[i][1] = LINE_BLOCK;
        solve_puzzle[i][(puzzle_n_col * 2) + 3] = LINE_BLOCK;
        solve_puzzle[i][(puzzle_n_col * 2) + 4] = LINE_BLOCK;
    }

    for (int i = 1; i < (int)(puzzle_n_row * 2) + 1 + 2; i += 2) {
        for (int j = 1; j < (int)(puzzle_n_col * 2) + 1 + 2; j += 2) {
            if (puzzle[i][j] == AREA_BLOCK) {
                solve_puzzle[i][j] = AREA_BLOCK;
            }
            else {
                solve_puzzle[i][j] = AREA_NORMAL;
            }
        }
    }
}

void print_puzzle(uint32_t** solve_puzzle) {
    std::string line;
    for (int i = 2; i < (int)(puzzle_n_row * 2) + 1 + 2; ++i) {
        line.clear();
        if (i & 1) { // area line
            for (int j = 2; j < (int)(puzzle_n_col * 2) + 1 + 2; ++j) {
                if (j & 1) {
                    line += "  ";
                }
                else {
                    if (solve_puzzle[i][j - 1] == solve_puzzle[i][j + 1] ||
                        (puzzle[i][j - 1] == puzzle[i][j + 1] && puzzle[i][j - 1] == AREA_BLOCK)) {
                        line += " ";
                    }
                    else {
                        line += "#";
                    }
                }
            }
        }
        else { // node line
            for (int j = 2; j < (int)(puzzle_n_col * 2) + 1 + 2; ++j) {
                if (j & 1) {
                    if (solve_puzzle[i - 1][j] == solve_puzzle[i + 1][j] ||
                        (puzzle[i - 1][j] == puzzle[i + 1][j] && puzzle[i - 1][j] == AREA_BLOCK)) {
                        line += "  ";
                    }
                    else {
                        line += "##";
                    }
                }
                else {
                    if (puzzle[i][j] == LINE_BLOCK) {
                        line += " ";
                    }
                    else {
                        line += "+";
                    }
                }
            }
        }
        std::cout << line << std::endl;
    }
}

void print_DFS_puzzle(uint32_t** solve_puzzle) {

    int index;

    std::cout << "Encoded puzzle:" << std::endl;
    std::cout << "  ";
    for (int j = 0; j < (int)(puzzle_n_col * 2) + 1 + 4; ++j) {
        if (j & 1) {
            index = (j - 1) / 2;
            if (index >= 10) {
                std::cout << index;
            }
            else {
                std::cout << index << " ";
            }
        }
        else {
            std::cout << "  ";
        }
    }
    std::cout << std::endl;
    for (int i = 0; i < (int)(puzzle_n_row * 2) + 1 + 4; ++i) {
        if (i & 1) {
            index = (i - 1) / 2;
            if (index >= 10) {
                std::cout << index << " ";
            }
            else {
                std::cout << index << " ";
            }
        }
        else {
            std::cout << "  ";
        }
        for (int j = 0; j < (int)(puzzle_n_col * 2) + 1 + 4; ++j) {
            if (puzzle[i][j] == 0xffffffffu || puzzle[i][j] == AREA_BLOCK || puzzle[i][j] == LINE_BLOCK) {
                std::cout << "X ";
            }
            else {
                if ((solve_puzzle[i][j] & SOLVE_AREA_BIT) >= 10) {
                    std::cout << (solve_puzzle[i][j] & SOLVE_AREA_BIT);
                }
                else {
                    std::cout << (solve_puzzle[i][j] & SOLVE_AREA_BIT) << " ";
                }
            }
        }
        std::cout << std::endl;
    }
    std::cout << "Encoded puzzle (SHAPE INDEX):" << std::endl;
    std::cout << "  ";
    for (int j = 0; j < (int)(puzzle_n_col * 2) + 1 + 4; ++j) {
        if (j & 1) {
            index = (j - 1) / 2;
            if (index >= 10) {
                std::cout << index;
            }
            else {
                std::cout << index << " ";
            }
        }
        else {
            std::cout << "  ";
        }
    }
    std::cout << std::endl;
    for (int i = 0; i < (int)(puzzle_n_row * 2) + 1 + 4; ++i) {
        if (i & 1) {
            index = (i - 1) / 2;
            if (index >= 10) {
                std::cout << index << " ";
            }
            else {
                std::cout << index << " ";
            }
        }
        else {
            std::cout << "  ";
        }
        for (int j = 0; j < (int)(puzzle_n_col * 2) + 1 + 4; ++j) {
            if (puzzle[i][j] == 0xffffffffu || puzzle[i][j] == AREA_BLOCK || puzzle[i][j] == LINE_BLOCK) {
                std::cout << "X ";
            }
            else {
                if (((solve_puzzle[i][j] & SOLVE_AREA_SHAPE_INDEX_BIT) >> SOLVE_AREA_SHAPE_INDEX_BIT_SHIFT) >= 10) {
                    std::cout << ((solve_puzzle[i][j] & SOLVE_AREA_SHAPE_INDEX_BIT) >> SOLVE_AREA_SHAPE_INDEX_BIT_SHIFT);
                }
                else {
                    std::cout << ((solve_puzzle[i][j] & SOLVE_AREA_SHAPE_INDEX_BIT) >> SOLVE_AREA_SHAPE_INDEX_BIT_SHIFT) << " ";
                }
            }
        }
        std::cout << std::endl;
    }
}

// ------------------------------------------------------------
// Area helpers
// ------------------------------------------------------------

bool area_in_puzzle_range(uint32_t x, uint32_t y, uint32_t n_row, uint32_t n_col) {
    return x > 2 && x < (n_row * 2) + 1 + 2 && y > 2 && y < (n_col * 2) + 1 + 2;
}

bool area_contain_symbol(int x, int y) {
    int puzzle_x = to_puzzle_x(x);
    int puzzle_y = to_puzzle_y(y);
    if (puzzle[puzzle_x][puzzle_y] & AREA_PALISADE_INDEX_BIT) {
        return true;
    }
    if (puzzle[puzzle_x][puzzle_y] & AREA_SLASH_INDEX_BIT) {
        return true;
    }
    if (puzzle[puzzle_x][puzzle_y] & AREA_SHAPE_INDEX_BIT) {
        return true;
    }
    if (puzzle[puzzle_x][puzzle_y] & AREA_SHAPE_SIZE_BIT) {
        return true;
    }
    if (puzzle[puzzle_x][puzzle_y] & AREA_COMPASS_ENABLE) {
        return true;
    }
    return false;
}
