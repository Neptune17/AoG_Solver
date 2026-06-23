#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <tuple>
#include <set>
#include <cstring>

#define LINE_NORMAL 0x00000000u
#define LINE_BLOCK 0x80000000u
#define LINE_DIFFERENT 0x40000000u
#define LINE_EQUAL 0x20000000u
#define LINE_SMALLER 0x10000000u
#define LINE_LARGER 0x08000000u
#define LINE_SIZE_DIFF_BIT 0x000f0000u
#define LINE_SIZE_DIFF_BIT_SHIFT 16
#define LINE_PARSE_ERROR 0xffffffffu

#define AREA_NORMAL 0x00000000u
#define AREA_BLOCK 0x80000000u
#define AREA_PALISADE_INDEX_BIT 0x70000000u
#define AREA_PALISADE_INDEX_BIT_SHIFT 28
#define AREA_SHAPE_INDEX_BIT 0x0f000000u
#define AREA_SHAPE_INDEX_BIT_SHIFT 24
#define AREA_SHAPE_SIZE_BIT 0x00ff0000u
#define AREA_SHAPE_SIZE_BIT_SHIFT 16
#define AREA_SLASH_INDEX_BIT 0x0000f000u
#define AREA_SLASH_INDEX_BIT_SHIFT 12
#define AREA_COMPASS_ENABLE 0x00000800u
#define AREA_PARSE_ERROR 0xffffffffu

#define SOLVE_AREA_SHAPE_INDEX_BIT 0xffff0000u
#define SOLVE_AREA_SHAPE_INDEX_BIT_SHIFT 16
#define SOLVE_AREA_BIT 0x0000ffffu

#define VERTEX_NORMAL 0x00000000u
#define VERTEX_BLOCK 0x80000000u
#define VERTEX_RADAR_BIT 0x0000000Fu
#define VERTEX_RADAR_BIT_SHIFT 0
#define VERTEX_PARSE_ERROR 0xffffffffu

uint32_t parse_vertex(char c);
uint32_t parse_line(char c);
uint32_t parse_area(char c1, char c2, char c3);
int DFS(uint32_t index, uint32_t** solve_puzzle);
int DFS_special_start(uint32_t index, uint32_t** solve_puzzle);

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

uint32_t parse_line(char c) { // Horizontal line should use the second bit
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

uint32_t parse_area(char c1, char c2, char c3 = 'X') {
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
        case 'U':
            return AREA_COMPASS_ENABLE;
        default:
            return AREA_PARSE_ERROR;
    }
}

int puzzle_compass_up[1000][1000];
int puzzle_compass_down[1000][1000];
int puzzle_compass_left[1000][1000];
int puzzle_compass_right[1000][1000];

bool puzzle_only_rectangles = false;
bool puzzle_no_rectangles = false;
bool puzzle_adjacent_shapes_different = false;
bool puzzle_adjacent_sizes_different = false;
bool puzzle_all_shapes_different = false;
bool puzzle_all_shapes_same = false;
bool puzzle_one_symbol_per_region = false;
bool puzzle_predefine_shapes_only = false;
bool puzzle_no_4_way_intersections = false;
bool puzzle_no_3_way_intersections = false;

int puzzle_shape_size_lower_bound = -1;
int puzzle_shape_size_upper_bound = -1;

uint32_t puzzle_n_row;
uint32_t puzzle_n_col;
uint32_t puzzle[1000][1000];

bool slash_check_enable = false;
int slash_size = 0;

bool shape_compare_enable = false;

int all_shapes_same_index = -1;

std::map<uint32_t, int> shape_index_size_map;

std::map<uint32_t, uint32_t> shape_index_modify_map;

void modify_shape_index_in_puzzle(int original_index, int new_index) {
    std::cout << "Modifying shape index in puzzle from " << original_index << " to " << new_index << std::endl;
    for (int i = 1; i < (puzzle_n_row * 2) + 1 + 2; i += 2) {
        for (int j = 1; j < (puzzle_n_col * 2) + 1 + 2; j += 2) {
            if ((puzzle[i][j] & AREA_SHAPE_INDEX_BIT) && ((puzzle[i][j] & AREA_SHAPE_INDEX_BIT) >> AREA_SHAPE_INDEX_BIT_SHIFT) == original_index) {
                puzzle[i][j] = (puzzle[i][j] & ~AREA_SHAPE_INDEX_BIT) | (new_index << AREA_SHAPE_INDEX_BIT_SHIFT);
            }
        }
    }
}

int to_puzzle_x(int x) {
    return (x << 1) + 1;
}

int to_puzzle_y(int y) {
    return (y << 1) + 1;
}

void read_puzzle() {

    // Puzzle Fence
    for (int j = 0; j < (puzzle_n_col * 2) + 1 + 4; ++j) {
        puzzle[0][j] = LINE_BLOCK;
        puzzle[1][j] = LINE_BLOCK;
        puzzle[(puzzle_n_row * 2) + 3][j] = LINE_BLOCK;
        puzzle[(puzzle_n_row * 2) + 4][j] = LINE_BLOCK;
    }
    for (int i = 0; i < (puzzle_n_row * 2) + 1 + 4; ++i) {
        puzzle[i][0] = LINE_BLOCK;
        puzzle[i][1] = LINE_BLOCK;
        puzzle[i][(puzzle_n_col * 2) + 3] = LINE_BLOCK;
        puzzle[i][(puzzle_n_col * 2) + 4] = LINE_BLOCK;
    }

    std::string line;
    for (int i = 0; i < (puzzle_n_row * 2) + 1; ++i) {
        std::getline(std::cin, line);
        // std::cout << line << std::endl;
        if (i & 1) { // area line
            int next_status = 0; // 0 for line, 1 for area
            for (int j = 0; j < (puzzle_n_col * 3) + 1; j = j + 3) {
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
                            if (line[compass_index + 1] >= '0' && line[compass_index + 1] <= '9' && line[compass_index + 2] >= '0' && line[compass_index + 2] <= '9') {
                                puzzle_compass_right[i + 2][index_j] = (line[compass_index + 1] - '0') * 10 + line[compass_index + 2] - '0';
                                compass_index += 2;
                            }
                            else if (line[compass_index + 1] >= '0' && line[compass_index + 1] <= '9') {
                                puzzle_compass_right[i + 2][index_j] = line[compass_index + 1] - '0';
                                compass_index += 1;
                            }
                            compass_index += 1;
                            // std::cout << "compass (" << (i + 2) << "," << j << ")" << std::endl;
                            // std::cout << puzzle_compass_up[i + 2][index_j] << " " << puzzle_compass_down[i + 2][index_j] << " " << puzzle_compass_left[i + 2][index_j] << " " << puzzle_compass_right[i + 2][index_j] << " " << std::endl;

                            index_j += 1;
                            // std::cout << line[compass_index] << " " << line[compass_index - 1] << std::endl;
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
            for (int j = 0; j < (puzzle_n_col * 3) + 1; j = j + 3) {
                puzzle[i + 2][(j / 3) * 2 + 2] = parse_vertex(line[j]);
            }
            for (int j = 2; j < (puzzle_n_col * 3) + 1; j = j + 3) {
                puzzle[i + 2][((j / 3) * 2) + 1 + 2] = parse_line(line[j]);
            }
        }
    }

    for (auto &entry : shape_index_modify_map) {
        modify_shape_index_in_puzzle(entry.first, entry.second);
    }

    // std::cout << "Encoded puzzle:" << std::endl;
    // for (int i = 0; i < (puzzle_n_row * 2) + 1 + 4; ++i) {
    //     for (int j = 0; j < (puzzle_n_col * 2) + 1 + 4; ++j) {
    //         if (puzzle[i][j] == 0xffffffffu || puzzle[i][j] == AREA_BLOCK || puzzle[i][j] == LINE_BLOCK) {
    //             std::cout << "X ";
    //         }
    //         else {
    //             std::cout << puzzle[i][j] << " ";
    //         }
    //     }
    //     std::cout << std::endl;
    // }
}

void build_solve_puzzle(uint32_t** solve_puzzle) {

    // Puzzle Fence
    for (int j = 0; j < (puzzle_n_col * 2) + 1 + 4; ++j) {
        solve_puzzle[0][j] = LINE_BLOCK;
        solve_puzzle[1][j] = LINE_BLOCK;
        solve_puzzle[(puzzle_n_row * 2) + 3][j] = LINE_BLOCK;
        solve_puzzle[(puzzle_n_row * 2) + 4][j] = LINE_BLOCK;
    }
    for (int i = 0; i < (puzzle_n_row * 2) + 1 + 4; ++i) {
        solve_puzzle[i][0] = LINE_BLOCK;
        solve_puzzle[i][1] = LINE_BLOCK;
        solve_puzzle[i][(puzzle_n_col * 2) + 3] = LINE_BLOCK;
        solve_puzzle[i][(puzzle_n_col * 2) + 4] = LINE_BLOCK;
    }

    for (int i = 1; i < (puzzle_n_row * 2) + 1 + 2; i += 2) {
        for (int j = 1; j < (puzzle_n_col * 2) + 1 + 2; j += 2) {
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
    for (int i = 2; i < (puzzle_n_row * 2) + 1 + 2; ++i) {
        line.clear();
        if (i & 1) { // area line
            for (int j = 2; j < (puzzle_n_col * 2) + 1 + 2; ++j) {
                if (j & 1) {
                    line += "  ";
                }
                else {
                    if (solve_puzzle[i][j - 1] == solve_puzzle[i][j + 1] || (puzzle[i][j - 1] == puzzle[i][j + 1] && puzzle[i][j - 1] == AREA_BLOCK)) {
                        line += " ";
                    }
                    else {
                        line += "#";
                    }
                }
            }
        } 
        else { // node line
            for (int j = 2; j < (puzzle_n_col * 2) + 1 + 2; ++j) {
                if (j & 1) {
                    if (solve_puzzle[i - 1][j] == solve_puzzle[i + 1][j] || (puzzle[i - 1][j] == puzzle[i + 1][j] && puzzle[i - 1][j] == AREA_BLOCK)) {
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

std::set<uint32_t> current_shape_index_pool;

struct node {
    int dx;
    int dy;

    node () {}

    node (int a, int b) {
        this->dx = a;
        this->dy = b;
    }

    bool operator<(const node& other) const {
        if (dx != other.dx)
            return dx < other.dx;
        return dy < other.dy;
    }
    
    bool operator==(const node& other) const {
        return dx == other.dx && dy == other.dy;
    }

    bool operator!=(const node& other) const {
        return dx != other.dx || dy != other.dy;
    }
};

struct Shape {
    uint32_t shape_digest;
    uint32_t shape_index;
    std::vector<uint32_t> shape_preview;
    std::vector<node> search_order;
};

std::vector<Shape> shapes;
std::vector<Shape> solve_shapes;

uint32_t next_shape_index = 1;

std::map<node, std::vector<int>> node_to_shape_index;

std::map<int, std::vector<node>> slash_nodes;

uint32_t shape_digest(std::vector<uint32_t> shape_preview) {
    uint32_t digest = 0;
    for (int i = 0; i < shape_preview.size(); ++i) {
        digest = (digest * 131) + shape_preview[i];
    }
    return digest;
}

void rotate_shape(uint32_t shape_size, uint32_t** shape) {
    uint32_t rotated_shape[100][100];
    for (int i = 0; i < shape_size; ++i) {
        for (int j = 0; j < shape_size; ++j) {
            rotated_shape[j][shape_size - 1 - i] = shape[i][j];
        }
    }
    for (int i = 0; i < shape_size; ++i) {
        for (int j = 0; j < shape_size; ++j) {
            shape[i][j] = rotated_shape[i][j];
        }
    }
}

void mirror_shape(uint32_t shape_size, uint32_t** shape) {
    uint32_t mirrored_shape[100][100];
    for (int i = 0; i < shape_size; ++i) {
        for (int j = 0; j < shape_size; ++j) {
            mirrored_shape[i][shape_size - 1 - j] = shape[i][j];
        }
    }
    for (int i = 0; i < shape_size; ++i) {
        for (int j = 0; j < shape_size; ++j) {
            shape[i][j] = mirrored_shape[i][j];
        }
    }
}

int shape_in_shapes(uint32_t shape_size, uint32_t** shape) {
    Shape new_shape;
    int start_x = -1, start_y = -1;
    for (int i = 0; i < shape_size; ++i) {
        for (int j = 0; j < shape_size; ++j) {
            if (shape[i][j] == 1) {
                if (start_x == -1) {
                    start_x = i;
                    start_y = j;
                }
                new_shape.search_order.push_back({i - start_x, j - start_y});
            }
        }
    }
    for (auto shape : shapes) {
        if (shape.search_order.size() == new_shape.search_order.size()) {
            bool is_duplicate = true;
            for (int i = 0; i < shape.search_order.size(); ++i) {
                if (shape.search_order[i] != new_shape.search_order[i]) {
                    is_duplicate = false;
                    break;
                }
            }
            if (is_duplicate) {
                return shape.shape_index; // Duplicate shape found
            }
        }
    }
    return -1; // No duplicate shape found
}

void add_shape_to_shapes(uint32_t shape_index, uint32_t shape_size, uint32_t** shape) {
    Shape new_shape;
    int start_x = -1, start_y = -1;
    for (int i = 0; i < shape_size; ++i) {
        for (int j = 0; j < shape_size; ++j) {
            if (shape[i][j] == 1) {
                if (start_x == -1) {
                    start_x = i;
                    start_y = j;
                }
                new_shape.search_order.push_back({i - start_x, j - start_y});
            }
        }
    }

    new_shape.shape_index = shape_index;
    for (auto shape : shapes) {
        // if (shape.shape_digest == shape_digest(new_shape.shape_preview)) {
        //     return; // Duplicate shape, skip adding
        // }
        if (shape.search_order.size() == new_shape.search_order.size()) {
            bool is_duplicate = true;
            for (int i = 0; i < shape.search_order.size(); ++i) {
                if (shape.search_order[i] != new_shape.search_order[i]) {
                    is_duplicate = false;
                    break;
                }
            }
            if (is_duplicate) {
                // std::cout << "Duplicate shape found, skipping addition." << std::endl;
                return; // Duplicate shape, skip adding
            }
        }
    }

    for (int i = 0; i < new_shape.search_order.size(); ++i) {
        node_to_shape_index[new_shape.search_order[i]].push_back(shapes.size());
    }
    shapes.push_back(new_shape);
    // std::cout << (shapes.size() - 1) << std::endl;
}

bool area_in_puzzle_range(uint32_t x, uint32_t y, uint32_t n_row, uint32_t n_col) {
    return x > 2 && x < (n_row * 2) + 1 + 2 && y > 2 && y < (n_col * 2) + 1 + 2;
}

bool check_nearby_shape(uint32_t x, uint32_t y, uint32_t n_row, uint32_t n_col, uint32_t** solve_puzzle) {
    // std::cout << "Checking nearby shapes for cell (" << x << ", " << y << ") with index " << solve_puzzle[x][y] << std::endl;
    int index = solve_puzzle[x][y];
    if (area_in_puzzle_range(x - 2, y, n_row, n_col)) {
        if (solve_puzzle[x - 2][y] != AREA_NORMAL && solve_puzzle[x - 2][y] != solve_puzzle[x][y] && ((solve_puzzle[x - 2][y] & SOLVE_AREA_SHAPE_INDEX_BIT) == (index & SOLVE_AREA_SHAPE_INDEX_BIT))) {
            // std::cout << "Edge check failed at (" << x << ", " << y << "): LINE_DIFFERENT with same index above." << std::endl;
            return false;
        }
    }
    if (area_in_puzzle_range(x + 2, y, n_row, n_col)) {
        if (solve_puzzle[x + 2][y] != AREA_NORMAL && solve_puzzle[x + 2][y] != solve_puzzle[x][y] && ((solve_puzzle[x + 2][y] & SOLVE_AREA_SHAPE_INDEX_BIT) == (index & SOLVE_AREA_SHAPE_INDEX_BIT))) {
            // std::cout << "Edge check failed at (" << x << ", " << y << "): LINE_DIFFERENT with same index below." << std::endl;
            return false;
        }
    }
    if (area_in_puzzle_range(x, y - 2, n_row, n_col)) {
        if (solve_puzzle[x][y - 2] != AREA_NORMAL && solve_puzzle[x][y - 2] != solve_puzzle[x][y] && ((solve_puzzle[x][y - 2] & SOLVE_AREA_SHAPE_INDEX_BIT) == (index & SOLVE_AREA_SHAPE_INDEX_BIT))) {
            // std::cout << "Edge check failed at (" << x << ", " << y << "): LINE_DIFFERENT with same index on the left." << std::endl;
            return false;
        }
    }
    if (area_in_puzzle_range(x, y + 2, n_row, n_col)) {
        if (solve_puzzle[x][y + 2] != AREA_NORMAL && solve_puzzle[x][y + 2] != solve_puzzle[x][y] && ((solve_puzzle[x][y + 2] & SOLVE_AREA_SHAPE_INDEX_BIT) == (index & SOLVE_AREA_SHAPE_INDEX_BIT))) {
            // std::cout << "Edge check failed at (" << x << ", " << y << "): LINE_DIFFERENT with same index on the right." << std::endl;
            return false;
        }
    }
    // std::cout << "Nearby shape check passed for cell (" << x << ", " << y << ")." << std::endl;
    return true;
}

bool check_nearby_size(uint32_t x, uint32_t y, uint32_t n_row, uint32_t n_col, uint32_t** solve_puzzle) {
    // std::cout << "Checking nearby sizes for cell (" << x << ", " << y << ") with index " << solve_puzzle[x][y] << std::endl;
    int index = solve_puzzle[x][y];
    if (area_in_puzzle_range(x - 2, y, n_row, n_col)) {
        if (solve_puzzle[x - 2][y] != AREA_NORMAL && solve_puzzle[x - 2][y] != solve_puzzle[x][y]) {
            int my_size = shape_index_size_map[(index & SOLVE_AREA_SHAPE_INDEX_BIT) >> SOLVE_AREA_SHAPE_INDEX_BIT_SHIFT];
            int nearby_size = shape_index_size_map[(solve_puzzle[x - 2][y] & SOLVE_AREA_SHAPE_INDEX_BIT) >> SOLVE_AREA_SHAPE_INDEX_BIT_SHIFT];
            // std::cout << index << " vs " << solve_puzzle[x - 2][y] << std::endl;
            // std::cout << "Current cell size: " << my_size << ", Nearby cell size: " << nearby_size << std::endl;
            if (my_size == nearby_size) {
                // std::cout << "Edge check failed at (" << x << ", " << y << "): LINE_DIFFERENT with same size above." << std::endl;
                return false;
            }
        }
    }
    if (area_in_puzzle_range(x + 2, y, n_row, n_col)) {
        if (solve_puzzle[x + 2][y] != AREA_NORMAL && solve_puzzle[x + 2][y] != solve_puzzle[x][y]) {
            int my_size = shape_index_size_map[(index & SOLVE_AREA_SHAPE_INDEX_BIT) >> SOLVE_AREA_SHAPE_INDEX_BIT_SHIFT];
            int nearby_size = shape_index_size_map[(solve_puzzle[x + 2][y] & SOLVE_AREA_SHAPE_INDEX_BIT) >> SOLVE_AREA_SHAPE_INDEX_BIT_SHIFT];
            // std::cout << index << " vs " << solve_puzzle[x + 2][y] << std::endl;
            // std::cout << "Current cell size: " << my_size << ", Nearby cell size: " << nearby_size << std::endl;
            if (my_size == nearby_size) {
                // std::cout << "Edge check failed at (" << x << ", " << y << "): LINE_DIFFERENT with same size below." << std::endl;
                return false;
            }
        }
    }
    if (area_in_puzzle_range(x, y - 2, n_row, n_col)) {
        if (solve_puzzle[x][y - 2] != AREA_NORMAL && solve_puzzle[x][y - 2] != solve_puzzle[x][y]) {
            int my_size = shape_index_size_map[(index & SOLVE_AREA_SHAPE_INDEX_BIT) >> SOLVE_AREA_SHAPE_INDEX_BIT_SHIFT];
            int nearby_size = shape_index_size_map[(solve_puzzle[x][y - 2] & SOLVE_AREA_SHAPE_INDEX_BIT) >> SOLVE_AREA_SHAPE_INDEX_BIT_SHIFT];
            // std::cout << index << " vs " << solve_puzzle[x][y - 2] << std::endl;
            // std::cout << "Current cell size: " << my_size << ", Nearby cell size: " << nearby_size << std::endl;
            if (my_size == nearby_size) {
                // std::cout << "Edge check failed at (" << x << ", " << y << "): LINE_DIFFERENT with same size on the left." << std::endl;
                return false;
            }
        }
    }
    if (area_in_puzzle_range(x, y + 2, n_row, n_col)) {
        if (solve_puzzle[x][y + 2] != AREA_NORMAL && solve_puzzle[x][y + 2] != solve_puzzle[x][y]) {
            int my_size = shape_index_size_map[(index & SOLVE_AREA_SHAPE_INDEX_BIT) >> SOLVE_AREA_SHAPE_INDEX_BIT_SHIFT];
            int nearby_size = shape_index_size_map[(solve_puzzle[x][y + 2] & SOLVE_AREA_SHAPE_INDEX_BIT) >> SOLVE_AREA_SHAPE_INDEX_BIT_SHIFT];
            // std::cout << index << " vs " << solve_puzzle[x][y + 2] << std::endl;
            // std::cout << "Current cell size: " << my_size << ", Nearby cell size: " << nearby_size << std::endl;
            if (my_size == nearby_size) {
                // std::cout << "Edge check failed at (" << x << ", " << y << "): LINE_DIFFERENT with same size on the right." << std::endl;
                return false;
            }
        }
    }
    // std::cout << "Nearby size check passed for cell (" << x << ", " << y << ")." << std::endl;
    return true;
}

bool check_edge_shape(uint32_t x, uint32_t y, uint32_t n_row, uint32_t n_col, uint32_t** solve_puzzle) {
    // std::cout << "Checking edge shapes for cell (" << x << ", " << y << ") with index " << solve_puzzle[x][y] << std::endl;
    int index = solve_puzzle[x][y];
    if (area_in_puzzle_range(x - 2, y, n_row, n_col)) {
        if (solve_puzzle[x - 2][y] != AREA_NORMAL && puzzle[x - 1][y] & LINE_EQUAL && ((solve_puzzle[x - 2][y] & SOLVE_AREA_SHAPE_INDEX_BIT) != (index & SOLVE_AREA_SHAPE_INDEX_BIT))) {
            // std::cout << "Edge check failed at (" << x << ", " << y << "): LINE_EQUAL with different index above." << std::endl;
            return false;
        }
        if (solve_puzzle[x - 2][y] != AREA_NORMAL && puzzle[x - 1][y] & LINE_DIFFERENT && ((solve_puzzle[x - 2][y] & SOLVE_AREA_SHAPE_INDEX_BIT) == (index & SOLVE_AREA_SHAPE_INDEX_BIT))) {
            // std::cout << "Edge check failed at (" << x << ", " << y << "): LINE_DIFFERENT with same index above." << std::endl;
            return false;
        }
        if (solve_puzzle[x - 2][y] != AREA_NORMAL && (puzzle[x - 1][y] & LINE_LARGER || puzzle[x - 1][y] & LINE_SMALLER || (puzzle[x - 1][y] & LINE_SIZE_DIFF_BIT) != 0)) {
            int left_size = shape_index_size_map[(solve_puzzle[x - 2][y] & SOLVE_AREA_SHAPE_INDEX_BIT) >> SOLVE_AREA_SHAPE_INDEX_BIT_SHIFT];
            int right_size = shape_index_size_map[(solve_puzzle[x][y] & SOLVE_AREA_SHAPE_INDEX_BIT) >> SOLVE_AREA_SHAPE_INDEX_BIT_SHIFT];
            if (puzzle[x - 1][y] & LINE_LARGER && left_size <= right_size) {
                return false;
            }
            if (puzzle[x - 1][y] & LINE_SMALLER && left_size >= right_size) {
                return false;
            }
            if (puzzle[x - 1][y] & LINE_SIZE_DIFF_BIT) {
                int diff_val = ((puzzle[x - 1][y] & LINE_SIZE_DIFF_BIT) >> LINE_SIZE_DIFF_BIT_SHIFT) - 1;
                if ((std::max(left_size, right_size) - std::min(left_size, right_size)) != diff_val) {
                    return false;
                }
            }
        }
    }
    if (area_in_puzzle_range(x + 2, y, n_row, n_col)) {
        if (solve_puzzle[x + 2][y] != AREA_NORMAL && puzzle[x + 1][y] & LINE_EQUAL && ((solve_puzzle[x + 2][y] & SOLVE_AREA_SHAPE_INDEX_BIT) != (index & SOLVE_AREA_SHAPE_INDEX_BIT))) {
            // std::cout << "Edge check failed at (" << x << ", " << y << "): LINE_EQUAL with different index below." << std::endl;
            return false;
        }
        if (solve_puzzle[x + 2][y] != AREA_NORMAL && puzzle[x + 1][y] & LINE_DIFFERENT && ((solve_puzzle[x + 2][y] & SOLVE_AREA_SHAPE_INDEX_BIT) == (index & SOLVE_AREA_SHAPE_INDEX_BIT))) {
            // std::cout << "Edge check failed at (" << x << ", " << y << "): LINE_DIFFERENT with same index below." << std::endl;
            return false;
        }
        if (solve_puzzle[x + 2][y] != AREA_NORMAL && (puzzle[x + 1][y] & LINE_LARGER || puzzle[x + 1][y] & LINE_SMALLER || (puzzle[x + 1][y] & LINE_SIZE_DIFF_BIT) != 0)) {
            int left_size = shape_index_size_map[(solve_puzzle[x][y] & SOLVE_AREA_SHAPE_INDEX_BIT) >> SOLVE_AREA_SHAPE_INDEX_BIT_SHIFT];
            int right_size = shape_index_size_map[(solve_puzzle[x + 2][y] & SOLVE_AREA_SHAPE_INDEX_BIT) >> SOLVE_AREA_SHAPE_INDEX_BIT_SHIFT];
            if (puzzle[x + 1][y] & LINE_LARGER && left_size <= right_size) {
                return false;
            }
            if (puzzle[x + 1][y] & LINE_SMALLER && left_size >= right_size) {
                return false;
            }
            if (puzzle[x + 1][y] & LINE_SIZE_DIFF_BIT) {
                int diff_val = ((puzzle[x + 1][y] & LINE_SIZE_DIFF_BIT) >> LINE_SIZE_DIFF_BIT_SHIFT) - 1;
                if ((std::max(left_size, right_size) - std::min(left_size, right_size)) != diff_val) {
                    return false;
                }
            }
        }
    }
    if (area_in_puzzle_range(x, y - 2, n_row, n_col)) {
        if (solve_puzzle[x][y - 2] != AREA_NORMAL && puzzle[x][y - 1] & LINE_EQUAL && ((solve_puzzle[x][y - 2] & SOLVE_AREA_SHAPE_INDEX_BIT) != (index & SOLVE_AREA_SHAPE_INDEX_BIT))) {
            // std::cout << "Edge check failed at (" << x << ", " << y << "): LINE_EQUAL with different index on the left." << std::endl;
            return false;
        }
        if (solve_puzzle[x][y - 2] != AREA_NORMAL && puzzle[x][y - 1] & LINE_DIFFERENT && ((solve_puzzle[x][y - 2] & SOLVE_AREA_SHAPE_INDEX_BIT) == (index & SOLVE_AREA_SHAPE_INDEX_BIT))) {
            // std::cout << "Edge check failed at (" << x << ", " << y << "): LINE_DIFFERENT with same index on the left." << std::endl;
            return false;
        }
        if (solve_puzzle[x][y - 2] != AREA_NORMAL && (puzzle[x][y - 1] & LINE_LARGER || puzzle[x][y - 1] & LINE_SMALLER || (puzzle[x][y - 1] & LINE_SIZE_DIFF_BIT) != 0)) {
            int up_size = shape_index_size_map[(solve_puzzle[x][y - 2] & SOLVE_AREA_SHAPE_INDEX_BIT) >> SOLVE_AREA_SHAPE_INDEX_BIT_SHIFT];
            int down_size = shape_index_size_map[(solve_puzzle[x][y] & SOLVE_AREA_SHAPE_INDEX_BIT) >> SOLVE_AREA_SHAPE_INDEX_BIT_SHIFT];
            if (puzzle[x][y - 1] & LINE_LARGER && up_size <= down_size) {
                return false;
            }
            if (puzzle[x][y - 1] & LINE_SMALLER && up_size >= down_size) {
                return false;
            }
            if (puzzle[x][y - 1] & LINE_SIZE_DIFF_BIT) {
                int diff_val = ((puzzle[x][y - 1] & LINE_SIZE_DIFF_BIT) >> LINE_SIZE_DIFF_BIT_SHIFT) - 1;
                if ((std::max(up_size, down_size) - std::min(up_size, down_size)) != diff_val) {
                    return false;
                }
            }
        }
    }
    if (area_in_puzzle_range(x, y + 2, n_row, n_col)) {
        if (solve_puzzle[x][y + 2] != AREA_NORMAL && puzzle[x][y + 1] & LINE_EQUAL && ((solve_puzzle[x][y + 2] & SOLVE_AREA_SHAPE_INDEX_BIT) != (index & SOLVE_AREA_SHAPE_INDEX_BIT))) {
            // std::cout << "Edge check failed at (" << x << ", " << y << "): LINE_EQUAL with different index on the right." << std::endl;
            return false;
        }
        if (solve_puzzle[x][y + 2] != AREA_NORMAL && puzzle[x][y + 1] & LINE_DIFFERENT && ((solve_puzzle[x][y + 2] & SOLVE_AREA_SHAPE_INDEX_BIT) == (index & SOLVE_AREA_SHAPE_INDEX_BIT))) {
            // std::cout << "Edge check failed at (" << x << ", " << y << "): LINE_DIFFERENT with same index on the right." << std::endl;
            return false;
        }
        if (solve_puzzle[x][y + 2] != AREA_NORMAL && (puzzle[x][y + 1] & LINE_LARGER || puzzle[x][y + 1] & LINE_SMALLER || (puzzle[x][y + 1] & LINE_SIZE_DIFF_BIT) != 0)) {
            int up_size = shape_index_size_map[(solve_puzzle[x][y] & SOLVE_AREA_SHAPE_INDEX_BIT) >> SOLVE_AREA_SHAPE_INDEX_BIT_SHIFT];
            int down_size = shape_index_size_map[(solve_puzzle[x][y + 2] & SOLVE_AREA_SHAPE_INDEX_BIT) >> SOLVE_AREA_SHAPE_INDEX_BIT_SHIFT];
            if (puzzle[x][y + 1] & LINE_LARGER && up_size <= down_size) {
                return false;
            }
            if (puzzle[x][y + 1] & LINE_SMALLER && up_size >= down_size) {
                return false;
            }
            if (puzzle[x][y + 1] & LINE_SIZE_DIFF_BIT) {
                int diff_val = ((puzzle[x][y + 1] & LINE_SIZE_DIFF_BIT) >> LINE_SIZE_DIFF_BIT_SHIFT) - 1;
                if ((std::max(up_size, down_size) - std::min(up_size, down_size)) != diff_val) {
                    return false;
                }
            }
        }
    }
    // std::cout << "Edge shape check passed for cell (" << x << ", " << y << ")." << std::endl;
    return true;
}

bool check_edge(uint32_t x, uint32_t y, uint32_t n_row, uint32_t n_col, uint32_t** solve_puzzle) {
    // std::cout << "Checking edges for cell (" << x << ", " << y << ") with index " << solve_puzzle[x][y] << std::endl;
    int index = solve_puzzle[x][y];
    if (area_in_puzzle_range(x - 2, y, n_row, n_col)) {
        if (solve_puzzle[x - 2][y] == index && (puzzle[x - 1][y] & LINE_BLOCK)) {
            // std::cout << "Edge check failed at (" << x << ", " << y << "): LINE_BLOCK with same index above." << std::endl;
            return false;
        }
    }
    if (area_in_puzzle_range(x + 2, y, n_row, n_col)) {
        if (solve_puzzle[x + 2][y] == index && (puzzle[x + 1][y] & LINE_BLOCK)) {
            // std::cout << "Edge check failed at (" << x << ", " << y << "): LINE_BLOCK with same index below." << std::endl;
            return false;
        }
    }
    if (area_in_puzzle_range(x, y - 2, n_row, n_col)) {
        if (solve_puzzle[x][y - 2] == index && (puzzle[x][y - 1] & LINE_BLOCK)) {
            // std::cout << "Edge check failed at (" << x << ", " << y << "): LINE_BLOCK with same index on the left." << std::endl;
            return false;
        }
    }
    if (area_in_puzzle_range(x, y + 2, n_row, n_col)) {
        if (solve_puzzle[x][y + 2] == index && (puzzle[x][y + 1] & LINE_BLOCK)) {
            // std::cout << "Edge check failed at (" << x << ", " << y << "): LINE_BLOCK with same index on the right." << std::endl;
            return false;
        }
    }
    // std::cout << "Edge check passed for cell (" << x << ", " << y << ")." << std::endl; 
    return true;
}

bool check_palisade(uint32_t x, uint32_t y, uint32_t n_row, uint32_t n_col, uint32_t** solve_puzzle) {
    // std::cout << "Checking palisade for cell (" << x << ", " << y << ") with index " << solve_puzzle[x][y] << std::endl;

    int index = solve_puzzle[x][y];
    int palisade_type = (puzzle[x][y] & AREA_PALISADE_INDEX_BIT) >> AREA_PALISADE_INDEX_BIT_SHIFT;

    bool up = false, down = false, left = false, right = false;
    if (area_in_puzzle_range(x - 2, y, n_row, n_col)) {
        if (solve_puzzle[x - 2][y] == index) {
            left = true;
        }
    }
    if (area_in_puzzle_range(x + 2, y, n_row, n_col)) {
        if (solve_puzzle[x + 2][y] == index) {
            right = true;
        }
    }
    if (area_in_puzzle_range(x, y - 2, n_row, n_col)) {
        if (solve_puzzle[x][y - 2] == index) {
            up = true;
        }
    }
    if (area_in_puzzle_range(x, y + 2, n_row, n_col)) {
        if (solve_puzzle[x][y + 2] == index) {
            down = true;
        }
    }

    if (palisade_type == 1) {
        if (!up || !down || !left || !right) {
            return false;
        }
    }
    else if (palisade_type == 2) {
        int sum = up + down + left + right;
        if (sum != 3) {
            return false;
        }
    }
    else if (palisade_type == 3) {
        int sum = up + down + left + right;
        if (sum != 2) {
            return false;
        }
        if (!((up && down) || (left && right))) {
            return false;
        }
    }
    else if (palisade_type == 4) {
        int sum = up + down + left + right;
        if (sum != 1) {
            return false;
        }
    }
    else if (palisade_type == 5) {
        int sum = up + down + left + right;
        if (sum != 2) {
            return false;
        }
        if (!((up && right) || (right && down) || (down && left) || (left && up))) {
            return false;
        }
    }
    else if (palisade_type == 6) {
        int sum = up + down + left + right;
        if (sum > 0) {
            return false;
        }
    }

    // std::cout << "Palisade check passed for cell (" << x << ", " << y << ")." << std::endl;
    return true;
}

bool check_palisade_delta(uint32_t x, uint32_t y, uint32_t n_row, uint32_t n_col, uint32_t** solve_puzzle) {
    // std::cout << "Checking palisade delta for cell (" << x << ", " << y << ") with index " << solve_puzzle[x][y] << std::endl;

    int index = solve_puzzle[x][y];
    int palisade_type = (puzzle[x][y] & AREA_PALISADE_INDEX_BIT) >> AREA_PALISADE_INDEX_BIT_SHIFT;

    bool up = false, down = false, left = false, right = false;
    if (area_in_puzzle_range(x - 2, y, n_row, n_col)) {
        if (solve_puzzle[x - 2][y] == index) {
            left = true;
        }
    }
    if (area_in_puzzle_range(x + 2, y, n_row, n_col)) {
        if (solve_puzzle[x + 2][y] == index) {
            right = true;
        }
    }
    if (area_in_puzzle_range(x, y - 2, n_row, n_col)) {
        if (solve_puzzle[x][y - 2] == index) {
            up = true;
        }
    }
    if (area_in_puzzle_range(x, y + 2, n_row, n_col)) {
        if (solve_puzzle[x][y + 2] == index) {
            down = true;
        }
    }

    if (palisade_type == 1) {
        
    }
    else if (palisade_type == 2) {
        int sum = up + down + left + right;
        if (sum > 3) {
            return false;
        }
    }
    else if (palisade_type == 3) {
        int sum = up + down + left + right;
        if (sum > 2) {
            return false;
        }
        if (sum == 2 && !((up && down) || (left && right))) {
            return false;
        }
    }
    else if (palisade_type == 4) {
        int sum = up + down + left + right;
        if (sum > 1) {
            return false;
        }
    }
    else if (palisade_type == 5) {
        int sum = up + down + left + right;
        if (sum > 2) {
            return false;
        }
        if (sum == 2 && !((up && right) || (right && down) || (down && left) || (left && up))) {
            return false;
        }
    }
    else if (palisade_type == 6) {
        int sum = up + down + left + right;
        if (sum > 0) {
            return false;
        }
    }

    // std::cout << "Palisade check delta passed for cell (" << x << ", " << y << ")." << std::endl;
    return true;
}

bool check_tatami(uint32_t x, uint32_t y, uint32_t n_row, uint32_t n_col, uint32_t** solve_puzzle) {
    // std::cout << "Checking tatami for cell (" << x << ", " << y << ") with index " << solve_puzzle[x][y] << std::endl;

    int index = solve_puzzle[x][y];

    if (area_in_puzzle_range(x - 2, y - 2, n_row, n_col)) {
        if (solve_puzzle[x - 2][y] != index && solve_puzzle[x][y - 2] != index && solve_puzzle[x - 2][y - 2] != solve_puzzle[x][y - 2] && solve_puzzle[x - 2][y - 2] != solve_puzzle[x - 2][y]) {
            return false;
        }
    }
    if (area_in_puzzle_range(x - 2, y + 2, n_row, n_col)) {
        if (solve_puzzle[x - 2][y] != index && solve_puzzle[x][y + 2] != index && solve_puzzle[x - 2][y + 2] != solve_puzzle[x][y + 2] && solve_puzzle[x - 2][y + 2] != solve_puzzle[x - 2][y]) {
            return false;
        }
    }
    if (area_in_puzzle_range(x + 2, y - 2, n_row, n_col)) {
        if (solve_puzzle[x + 2][y] != index && solve_puzzle[x][y - 2] != index && solve_puzzle[x + 2][y - 2] != solve_puzzle[x][y - 2] && solve_puzzle[x + 2][y - 2] != solve_puzzle[x + 2][y]) {
            return false;
        }
    }
    if (area_in_puzzle_range(x + 2, y + 2, n_row, n_col)) {
        if (solve_puzzle[x + 2][y] != index && solve_puzzle[x][y + 2] != index && solve_puzzle[x + 2][y + 2] != solve_puzzle[x][y + 2] && solve_puzzle[x + 2][y + 2] != solve_puzzle[x + 2][y]) {
            return false;
        }
    }

    // std::cout << "Tatami check passed for cell (" << x << ", " << y << ")." << std::endl;
    return true;
}

bool check_loopy(uint32_t x, uint32_t y, uint32_t n_row, uint32_t n_col, uint32_t** solve_puzzle) {
    // std::cout << "Checking loopy for cell (" << x << ", " << y << ") with index " << solve_puzzle[x][y] << std::endl;

    int index = solve_puzzle[x][y];

    int count = (solve_puzzle[x - 2][y] != index) + (solve_puzzle[x][y - 2] != index) + (solve_puzzle[x - 2][y - 2] != solve_puzzle[x][y - 2]) + (solve_puzzle[x - 2][y - 2] != solve_puzzle[x - 2][y]);
    int zero_count = (solve_puzzle[x - 2][y] == 0) + (solve_puzzle[x][y - 2] == 0) + (solve_puzzle[x - 2][y - 2] == 0);
    if (count == 3 && zero_count != 2) {
        // std::cout << index << " " << solve_puzzle[x - 2][y] << " " << solve_puzzle[x][y - 2] << " " << solve_puzzle[x - 2][y - 2] << std::endl;
        // std::cout << "Loopy check failed at 1" << std::endl;
        return false;
    }

    count = (solve_puzzle[x - 2][y] != index) + (solve_puzzle[x][y + 2] != index) + (solve_puzzle[x - 2][y + 2] != solve_puzzle[x][y + 2]) + (solve_puzzle[x - 2][y + 2] != solve_puzzle[x - 2][y]);
    zero_count = (solve_puzzle[x - 2][y] == 0) + (solve_puzzle[x][y + 2] == 0) + (solve_puzzle[x - 2][y + 2] == 0);
    if (count == 3 && zero_count != 2) {
        // std::cout << "Loopy check failed at 2" << std::endl;
        return false;
    }

    count = (solve_puzzle[x + 2][y] != index) + (solve_puzzle[x][y - 2] != index) + (solve_puzzle[x + 2][y - 2] != solve_puzzle[x][y - 2]) + (solve_puzzle[x + 2][y - 2] != solve_puzzle[x + 2][y]);
    zero_count = (solve_puzzle[x + 2][y] == 0) + (solve_puzzle[x][y - 2] == 0) + (solve_puzzle[x + 2][y - 2] == 0);
    if (count == 3 && zero_count != 2) {
        // std::cout << "Loopy check failed at 3" << std::endl;
        return false;
    }

    count = (solve_puzzle[x + 2][y] != index) + (solve_puzzle[x][y + 2] != index) + (solve_puzzle[x + 2][y + 2] != solve_puzzle[x][y + 2]) + (solve_puzzle[x + 2][y + 2] != solve_puzzle[x + 2][y]);
    zero_count = (solve_puzzle[x + 2][y] == 0) + (solve_puzzle[x][y + 2] == 0) + (solve_puzzle[x + 2][y + 2] == 0);
    if (count == 3 && zero_count != 2) {
        // std::cout << "Loopy check failed at 4" << std::endl;
        return false;
    }

    // std::cout << "Loopy check passed for cell (" << x << ", " << y << ")." << std::endl;
    return true;
}

bool check_radar(uint32_t x, uint32_t y, uint32_t n_row, uint32_t n_col, uint32_t** solve_puzzle) {

    int index = solve_puzzle[x][y];

    if (puzzle[x - 1][y - 1] & VERTEX_RADAR_BIT) {
        // std::cout << "check " << "(" << (x - 1) << "," << (y - 1) << ")" << std::endl;
        int radar_value = (puzzle[x - 1][y - 1] & VERTEX_RADAR_BIT) >> VERTEX_RADAR_BIT_SHIFT;
        std::set<uint32_t> unique_region;
        unique_region.insert(index);
        if (solve_puzzle[x - 2][y] != AREA_BLOCK) {
            unique_region.insert(solve_puzzle[x - 2][y]);
        }
        if (solve_puzzle[x][y - 2] != AREA_BLOCK) {
            unique_region.insert(solve_puzzle[x][y - 2]);
        }
        if (solve_puzzle[x - 2][y - 2] != AREA_BLOCK) {
            unique_region.insert(solve_puzzle[x - 2][y - 2]);
        }
        if (unique_region.size() > radar_value || (unique_region.find(AREA_NORMAL) == unique_region.end() && unique_region.size() != radar_value)) {
            // std::cout << "check fail" << std::endl;
            return false;
        }
    }
    if (puzzle[x - 1][y + 1] & VERTEX_RADAR_BIT) {
        // std::cout << "check " << "(" << (x - 1) << "," << (y + 1) << ")" << std::endl;
        int radar_value = (puzzle[x - 1][y + 1] & VERTEX_RADAR_BIT) >> VERTEX_RADAR_BIT_SHIFT;
        std::set<uint32_t> unique_region;
        unique_region.insert(index);
        if (solve_puzzle[x - 2][y] != AREA_BLOCK) {
            unique_region.insert(solve_puzzle[x - 2][y]);
        }
        if (solve_puzzle[x][y + 2] != AREA_BLOCK) {
            unique_region.insert(solve_puzzle[x][y + 2]);
        }
        if (solve_puzzle[x - 2][y + 2] != AREA_BLOCK) {
            unique_region.insert(solve_puzzle[x - 2][y + 2]);
        }
        if (unique_region.size() > radar_value || (unique_region.find(AREA_NORMAL) == unique_region.end() && unique_region.size() != radar_value)) {
            // std::cout << "check fail" << std::endl;
            return false;
        }
    }
    if (puzzle[x + 1][y - 1] & VERTEX_RADAR_BIT) {
        // std::cout << "check " << "(" << (x + 1) << "," << (y - 1) << ")" << std::endl;
        int radar_value = (puzzle[x + 1][y - 1] & VERTEX_RADAR_BIT) >> VERTEX_RADAR_BIT_SHIFT;
        std::set<uint32_t> unique_region;
        unique_region.insert(index);
        if (solve_puzzle[x + 2][y] != AREA_BLOCK) {
            unique_region.insert(solve_puzzle[x + 2][y]);
        }
        if (solve_puzzle[x][y - 2] != AREA_BLOCK) {
            unique_region.insert(solve_puzzle[x][y - 2]);
        }
        if (solve_puzzle[x + 2][y - 2] != AREA_BLOCK) {
            unique_region.insert(solve_puzzle[x + 2][y - 2]);
        }
        if (unique_region.size() > radar_value || (unique_region.find(AREA_NORMAL) == unique_region.end() && unique_region.size() != radar_value)) {
            // std::cout << "check fail" << std::endl;
            return false;
        }
    }
    if (puzzle[x + 1][y + 1] & VERTEX_RADAR_BIT) {
        // std::cout << "check " << "(" << (x + 1) << "," << (y + 1) << ")" << std::endl;
        int radar_value = (puzzle[x + 1][y + 1] & VERTEX_RADAR_BIT) >> VERTEX_RADAR_BIT_SHIFT;
        std::set<uint32_t> unique_region;
        unique_region.insert(index);
        if (solve_puzzle[x + 2][y] != AREA_BLOCK) {
            unique_region.insert(solve_puzzle[x + 2][y]);
        }
        if (solve_puzzle[x][y + 2] != AREA_BLOCK) {
            unique_region.insert(solve_puzzle[x][y + 2]);
        }
        if (solve_puzzle[x + 2][y + 2] != AREA_BLOCK) {
            unique_region.insert(solve_puzzle[x + 2][y + 2]);
        }
        if (unique_region.size() > radar_value || (unique_region.find(AREA_NORMAL) == unique_region.end() && unique_region.size() != radar_value)) {
            // std::cout << "check fail" << std::endl;
            return false;
        }
    }

    return true;
}


void print_DFS_puzzle(uint32_t** solve_puzzle) {
    
    int index;

    std::cout << "Encoded puzzle:" << std::endl;
    std::cout << "  ";
    for (int j = 0; j < (puzzle_n_col * 2) + 1 + 4; ++j) {
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
    for (int i = 0; i < (puzzle_n_row * 2) + 1 + 4; ++i) {
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
        for (int j = 0; j < (puzzle_n_col * 2) + 1 + 4; ++j) {
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
    for (int j = 0; j < (puzzle_n_col * 2) + 1 + 4; ++j) {
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
    for (int i = 0; i < (puzzle_n_row * 2) + 1 + 4; ++i) {
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
        for (int j = 0; j < (puzzle_n_col * 2) + 1 + 4; ++j) {
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

bool visited[100][100];
std::set<int> area_shape_sizes;
bool contain_symbol = false;
std::set<node> visited_nodes;

int DFS_count_empty(int x, int y, uint32_t n_row, uint32_t n_col, uint32_t** solve_puzzle) {
    if (puzzle[x][y] == AREA_BLOCK) {
        return 0;
    }
    if (solve_puzzle[x][y] != AREA_NORMAL) {
        return 0;
    }
    if (visited[x][y]) {
        return 0;
    }
    visited[x][y] = true;
    visited_nodes.insert(node(x, y));
    int count = 1;
    if (puzzle[x][y] & AREA_SHAPE_SIZE_BIT) {
        area_shape_sizes.insert((puzzle[x][y] & AREA_SHAPE_SIZE_BIT) >> AREA_SHAPE_SIZE_BIT_SHIFT);
    }

    if (puzzle_one_symbol_per_region) {
        if (puzzle[x][y] & AREA_PALISADE_INDEX_BIT) {
            contain_symbol = true;
        }
        if (puzzle[x][y] & AREA_SLASH_INDEX_BIT) {
            contain_symbol = true;
        }
        if (puzzle[x][y] & AREA_SHAPE_INDEX_BIT) {
            contain_symbol = true;
        }
        if (puzzle[x][y] & AREA_SHAPE_SIZE_BIT) {
            contain_symbol = true;
        }
        if (puzzle[x][y] & AREA_COMPASS_ENABLE) {
            contain_symbol = true;
        }
    }

    if ((puzzle[x - 1][y] & LINE_BLOCK) == 0) {
        count += DFS_count_empty(x - 2, y, n_row, n_col, solve_puzzle);
    }
    if ((puzzle[x + 1][y] & LINE_BLOCK) == 0) {
        count += DFS_count_empty(x + 2, y, n_row, n_col, solve_puzzle);
    }
    if ((puzzle[x][y - 1] & LINE_BLOCK) == 0) {
        count += DFS_count_empty(x, y - 2, n_row, n_col, solve_puzzle);
    }
    if ((puzzle[x][y + 1] & LINE_BLOCK) == 0) {
        count += DFS_count_empty(x, y + 2, n_row, n_col, solve_puzzle);
    }

    if ((puzzle[x + 1][y] & LINE_BLOCK) && visited_nodes.find(node(x + 2, y)) != visited_nodes.end()) {
        count -= 1;
    }
    if ((puzzle[x][y + 1] & LINE_BLOCK) && visited_nodes.find(node(x, y + 1)) != visited_nodes.end()) {
        count -= 1;
    } 

    return count;
}

bool empty_area_size_check(uint32_t min_shape_size, uint32_t max_shape_size, uint32_t n_row, uint32_t n_col, uint32_t** solve_puzzle) {
    // std::cout << "Empty area size check..." << std::endl;
    
    memset(visited, 0, sizeof(visited));

    for (int i = 1; i <= n_row; ++i) {
        for (int j = 1; j <= n_col; ++j) {
            if (puzzle[(i << 1) + 1][(j << 1) + 1] != AREA_BLOCK && solve_puzzle[(i << 1) + 1][(j << 1) + 1] == AREA_NORMAL && !visited[(i << 1) + 1][(j << 1) + 1]) {
                area_shape_sizes.clear();
                contain_symbol = false;
                visited_nodes.clear();
                int empty_area_size = DFS_count_empty((i << 1) + 1, (j << 1) + 1, n_row, n_col, solve_puzzle);
                if (empty_area_size < min_shape_size) {
                    // std::cout << "Empty area size check failed: found an empty area of size " << empty_area_size << " which is smaller than the minimum shape size " << min_shape_size << "." << std::endl;
                    return false;
                }
                int area_shape_sizes_required_size = 0;
                for (auto shape_size : area_shape_sizes) {
                    area_shape_sizes_required_size += shape_size;
                }
                if (empty_area_size < area_shape_sizes_required_size) {
                    // std::cout << "Empty area size check failed: found an empty area of size " << empty_area_size << " which is smaller than the total required size " << area_shape_sizes_required_size << " of the shapes that must be placed in this area." << std::endl;
                    return false;
                }
                if (puzzle_one_symbol_per_region && !contain_symbol) {
                    return false;
                }
            }
        }
    }

    // std::cout << "Empty area size check passed." << std::endl;
    return true;
}

int slash_count[10];

void DFS_slash_count_empty(int x, int y, uint32_t n_row, uint32_t n_col, uint32_t** solve_puzzle) {

    // print_DFS_puzzle(solve_puzzle);

    // std::cout << "DFS_slash_count_empty: visiting cell (" << x << ", " << y << ")" << std::endl;
    // std::cout << "DFS_slash_count_empty: visiting cell (" << puzzle[x][y] << ")" << std::endl;

    if (puzzle[x][y] == AREA_BLOCK) {
        return ;
    }
    if (solve_puzzle[x][y] != AREA_NORMAL) {
        return ;
    }
    if (visited[x][y]) {
        return ;
    }
    if (puzzle[x][y] & AREA_SLASH_INDEX_BIT) {
        int slash_index = puzzle[x][y] >> AREA_SLASH_INDEX_BIT_SHIFT;
        slash_count[slash_index] += 1;
    }

    visited[x][y] = true;

    if ((puzzle[x - 1][y] & LINE_BLOCK) == 0) {
        DFS_slash_count_empty(x - 2, y, n_row, n_col, solve_puzzle);
    }
    if ((puzzle[x + 1][y] & LINE_BLOCK) == 0) {
        DFS_slash_count_empty(x + 2, y, n_row, n_col, solve_puzzle);
    }
    if ((puzzle[x][y - 1] & LINE_BLOCK) == 0) {
        DFS_slash_count_empty(x, y - 2, n_row, n_col, solve_puzzle);
    }
    if ((puzzle[x][y + 1] & LINE_BLOCK) == 0) {
        DFS_slash_count_empty(x, y + 2, n_row, n_col, solve_puzzle);
    }
    return ;
}

bool empty_area_slash_check(uint32_t n_row, uint32_t n_col, uint32_t** solve_puzzle) {
    // std::cout << "Empty area slash check..." << std::endl;
    
    memset(visited, 0, sizeof(visited));

    for (int i = 1; i <= n_row; ++i) {
        for (int j = 1; j <= n_col; ++j) {
            if (puzzle[(i << 1) + 1][(j << 1) + 1] != AREA_BLOCK && solve_puzzle[(i << 1) + 1][(j << 1) + 1] == AREA_NORMAL && !visited[(i << 1) + 1][(j << 1) + 1]) {
                memset(slash_count, 0, sizeof(slash_count));
                DFS_slash_count_empty((i << 1) + 1, (j << 1) + 1, n_row, n_col, solve_puzzle);

                int unique_slash_size_value = slash_count[1];
                for (int k = 1; k <= slash_size; k ++) {
                    if (unique_slash_size_value != slash_count[k]) {
                        // std::cout << "Empty area slash check failed: found an empty area with non-unique slashes." << std::endl;
                        return false;
                    }
                }
                if (unique_slash_size_value == 0) {
                    // std::cout << "Empty area slash check failed: found an empty area with no slashes." << std::endl;
                    return false;
                }
            }
        }
    }

    // std::cout << "Empty area slash check passed." << std::endl;
    return true;
}

int empty_area_shape_count(int x, int y, uint32_t n_row, uint32_t n_col, uint32_t** solve_puzzle) {
    
    memset(visited, 0, sizeof(visited));

    memset(slash_count, 0, sizeof(slash_count));

    // std::cout << "check" << std::endl;

    DFS_slash_count_empty(x, y, n_row, n_col, solve_puzzle);

    // std::cout << "check" << std::endl;

    int unique_slash_size_value = slash_count[1];

    return unique_slash_size_value;
}

std::tuple<int, int, int> find_empty_compass_area(uint32_t** solve_puzzle) {
    for (int i = 1; i <= puzzle_n_row; ++i) {
        for (int j = 1; j <= puzzle_n_col; ++j) {
            if (((puzzle[to_puzzle_x(i)][to_puzzle_y(j)] & AREA_COMPASS_ENABLE) != 0) && (solve_puzzle[to_puzzle_x(i)][to_puzzle_y(j)] == AREA_NORMAL)) {
                return std::make_tuple(0, i, j);
            }
        }
    }
    return std::make_tuple(-1, -1, -1);
}

std::tuple<int, int, int> find_empty_shape_index_area(uint32_t** solve_puzzle) {
    for (int i = 1; i <= puzzle_n_row; ++i) {
        for (int j = 1; j <= puzzle_n_col; ++j) {
            if (((puzzle[to_puzzle_x(i)][to_puzzle_y(j)] & AREA_SHAPE_INDEX_BIT) != 0) && (solve_puzzle[to_puzzle_x(i)][to_puzzle_y(j)] == AREA_NORMAL)) {
                return std::make_tuple(0, i, j);
            }
        }
    }
    return std::make_tuple(-1, -1, -1);
}

std::tuple<int, int, int> find_empty_shape_size_area(uint32_t** solve_puzzle) {
    for (int i = 1; i <= puzzle_n_row; ++i) {
        for (int j = 1; j <= puzzle_n_col; ++j) {
            if (((puzzle[to_puzzle_x(i)][to_puzzle_y(j)] & AREA_SHAPE_SIZE_BIT) != 0) && (solve_puzzle[to_puzzle_x(i)][to_puzzle_y(j)] == AREA_NORMAL)) {
                return std::make_tuple(0, i, j);
            }
        }
    }
    return std::make_tuple(-1, -1, -1);
}

std::tuple<int, int, int> find_empty_corner_area(uint32_t** solve_puzzle) {
    for (int i = 1; i <= puzzle_n_row; ++i) {
        for (int j = 1; j <= puzzle_n_col; ++j) {
            if (puzzle[to_puzzle_x(i)][to_puzzle_y(j)] != AREA_BLOCK && solve_puzzle[to_puzzle_x(i)][to_puzzle_y(j)] == AREA_NORMAL) {
                int block_line_count = ((puzzle[to_puzzle_x(i) - 1][to_puzzle_y(j)] & LINE_BLOCK) != 0 || puzzle[to_puzzle_x(i) - 2][to_puzzle_y(j)] == AREA_BLOCK) + ((puzzle[to_puzzle_x(i) + 1][to_puzzle_y(j)] & LINE_BLOCK) != 0 || puzzle[to_puzzle_x(i) + 2][to_puzzle_y(j)] == AREA_BLOCK) + ((puzzle[to_puzzle_x(i)][to_puzzle_y(j) - 1] & LINE_BLOCK) != 0 || puzzle[to_puzzle_x(i)][to_puzzle_y(j) - 2] == AREA_BLOCK) + ((puzzle[to_puzzle_x(i)][to_puzzle_y(j) + 1] & LINE_BLOCK) != 0 || puzzle[to_puzzle_x(i)][to_puzzle_y(j) + 2] == AREA_BLOCK);
                if (block_line_count >= 3) {
                    return std::make_tuple(0, i, j);
                }
            }
        }
    }
    return std::make_tuple(-1, -1, -1);
}

std::tuple<int, int, int> find_empty_line_constraint_area(uint32_t** solve_puzzle) {
    for (int i = 1; i <= puzzle_n_row; ++i) {
        for (int j = 1; j <= puzzle_n_col; ++j) {
            if (puzzle[to_puzzle_x(i)][to_puzzle_y(j)] != AREA_BLOCK && solve_puzzle[to_puzzle_x(i)][to_puzzle_y(j)] == AREA_NORMAL) {
                uint32_t line[4];
                line[0] = puzzle[to_puzzle_x(i) - 1][to_puzzle_y(j)];
                line[1] = puzzle[to_puzzle_x(i) + 1][to_puzzle_y(j)];
                line[2] = puzzle[to_puzzle_x(i)][to_puzzle_y(j) - 1];
                line[3] = puzzle[to_puzzle_x(i)][to_puzzle_y(j) + 1];
                for (int k = 0; k <= 3; ++k) {
                    if ((line[k] & LINE_EQUAL) || (line[k] & LINE_LARGER) || (line[k] & LINE_SMALLER) || (line[k] & LINE_DIFFERENT) || (line[k] & LINE_SIZE_DIFF_BIT)) {
                        return std::make_tuple(0, i, j);
                    }
                }
            }
        }
    }
    return std::make_tuple(-1, -1, -1);
}

std::tuple<int, int, int> find_empty_area(uint32_t** solve_puzzle) {
    for (int i = 1; i <= puzzle_n_row; ++i) {
        for (int j = 1; j <= puzzle_n_col; ++j) {
            if (puzzle[to_puzzle_x(i)][to_puzzle_y(j)] != AREA_BLOCK && solve_puzzle[to_puzzle_x(i)][to_puzzle_y(j)] == AREA_NORMAL) {
                return std::make_tuple(0, i, j);
            }
        }
    }
    return std::make_tuple(-1, -1, -1);
}

int place_non_predifined_shape(int index, int x, int y, uint32_t size, bool up_left_seq, int known_shape_index, uint32_t** solve_puzzle) {

    // std::cout << "Trying to place non-predefined shape of size " << size << " at (" << x << ", " << y << ")" << std::endl;

    const int MAX_SHAPE_SIZE = 100;
    const int MAX_EXPAND_CANDIDATES = (MAX_SHAPE_SIZE + 2) * 3;

    bool mark_slash[slash_size + 1];
    memset(mark_slash, 0, sizeof(mark_slash));
    int slash_distance[MAX_SHAPE_SIZE][slash_size + 1][slash_nodes[1].size() + 1];

    node dfs_current_shape[MAX_SHAPE_SIZE];
    int dfs_current_shape_cnt = 0;
    node dfs_expand_candidates[MAX_EXPAND_CANDIDATES];
    int dfs_expand_candidates_distance[MAX_EXPAND_CANDIDATES];
    int dfs_expand_candidates_cnt = 0;

    int dfs_rectangle_up[MAX_SHAPE_SIZE];
    int dfs_rectangle_down[MAX_SHAPE_SIZE];
    int dfs_rectangle_left[MAX_SHAPE_SIZE];
    int dfs_rectangle_right[MAX_SHAPE_SIZE];

    node palisade_visited[MAX_SHAPE_SIZE];
    int palisade_visited_cnt = 0;

    node compass_visited[MAX_SHAPE_SIZE];
    int compass_visited_up_cnt[MAX_SHAPE_SIZE];
    int compass_visited_down_cnt[MAX_SHAPE_SIZE];
    int compass_visited_left_cnt[MAX_SHAPE_SIZE];
    int compass_visited_right_cnt[MAX_SHAPE_SIZE];
    int compass_visited_cnt = 0;

    node symbol_loc = {-233, -666};

    int stack_size[MAX_SHAPE_SIZE];
    int stack_expand_distance_lb[MAX_SHAPE_SIZE];
    int stack_expand_x_lb[MAX_SHAPE_SIZE];
    int stack_expand_y_lb[MAX_SHAPE_SIZE];
    int stack_candidates_i[MAX_SHAPE_SIZE];
    int stack_candidates_size[MAX_SHAPE_SIZE];
    int stack_target_shape_index[MAX_SHAPE_SIZE];
    int stack_top = 0;

    dfs_expand_candidates[dfs_expand_candidates_cnt] = {0, 0};
    dfs_expand_candidates_distance[dfs_expand_candidates_cnt] = 0;
    dfs_expand_candidates_cnt ++;

    stack_size[stack_top] = 0;
    stack_expand_distance_lb[stack_top] = 0;
    stack_expand_x_lb[stack_top] = 0;
    stack_expand_y_lb[stack_top] = 0;
    stack_candidates_i[stack_top] = 0;
    stack_candidates_size[stack_top] = 1;
    stack_target_shape_index[stack_top] = 0;
    stack_top ++;

    int current_size;
    int expand_distance_lb;
    int expand_x_lb;
    int expand_y_lb;
    int candidates_i;
    int candidates_size;
    int target_shape_index;
    
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
        target_shape_index = stack_target_shape_index[stack_top - 1];

        stack_top --;

        while (dfs_current_shape_cnt > current_size) {
            temp_x = x + dfs_current_shape[dfs_current_shape_cnt - 1].dx;
            temp_y = y + dfs_current_shape[dfs_current_shape_cnt - 1].dy;
            solve_puzzle[to_puzzle_x(temp_x)][to_puzzle_y(temp_y)] = AREA_NORMAL;
            if (puzzle[to_puzzle_x(temp_x)][to_puzzle_y(temp_y)] & AREA_SLASH_INDEX_BIT) {
                int slash_index = puzzle[to_puzzle_x(temp_x)][to_puzzle_y(temp_y)] >> AREA_SLASH_INDEX_BIT_SHIFT;
                mark_slash[slash_index] = false;
            }
            if (puzzle[to_puzzle_x(temp_x)][to_puzzle_y(temp_y)] & AREA_PALISADE_INDEX_BIT) {
                palisade_visited_cnt --;
            }
            if (puzzle[to_puzzle_x(temp_x)][to_puzzle_y(temp_y)] & AREA_COMPASS_ENABLE) {
                compass_visited_cnt --;
            }

            if (puzzle_one_symbol_per_region && symbol_loc.dx == temp_x && symbol_loc.dy == temp_y) {
                symbol_loc.dx = -233;
                symbol_loc.dy = -666;
            }

            for (int i = 0; i < compass_visited_cnt; ++i) {
                if (dfs_current_shape[dfs_current_shape_cnt - 1].dx < compass_visited[i].dx) {
                    compass_visited_up_cnt[i] --;
                }
                if (dfs_current_shape[dfs_current_shape_cnt - 1].dx > compass_visited[i].dx) {
                    compass_visited_down_cnt[i] --;
                }
                if (dfs_current_shape[dfs_current_shape_cnt - 1].dy < compass_visited[i].dy) {
                    compass_visited_left_cnt[i] --;
                }
                if (dfs_current_shape[dfs_current_shape_cnt - 1].dy > compass_visited[i].dy) {
                    compass_visited_right_cnt[i] --;
                }
            }


            dfs_current_shape_cnt -= 1;
        }
        dfs_expand_candidates_cnt = candidates_size;

        // std::cout << "Index: " << index << ", Current shape size: " << current_size << ", expand_distance_lb: " << expand_distance_lb << ", expand_x_lb: " << expand_x_lb << ", expand_y_lb: " << expand_y_lb << ", candidates_i: " << candidates_i << ", candidates_size: " << candidates_size << ", target_shape_index: " << target_shape_index << std::endl;

        // for (int i = stack_top; i > 0; --i) {
        //     std::cout << "Stack: Index: " << index << ", Current shape size: " << stack_size[i - 1] << ", expand_distance_lb: " << stack_expand_distance_lb[i - 1] << ", expand_x_lb: " << stack_expand_x_lb[i - 1] << ", expand_y_lb: " << stack_expand_y_lb[i - 1] << ", candidates_i: " << stack_candidates_i[i - 1] << ", candidates_size: " << stack_candidates_size[i - 1] << ", target_shape_index: " << stack_target_shape_index[i - 1] << std::endl;
        // }

        // for (int i = 0; i < dfs_current_shape_cnt; ++i) {
        //     std::cout << "(" << dfs_current_shape[i].dx << ", " << dfs_current_shape[i].dy << ") ";
        // }
        // std::cout << std::endl;
        // for (int i = candidates_i; i < candidates_size; ++i) {
        //     std::cout << "(" << dfs_expand_candidates[i].dx << ", " << dfs_expand_candidates[i].dy << ", " << dfs_expand_candidates_distance[i] << ") ";
        // }
        // std::cout << std::endl;
        // for (int i = 1; i <= slash_size; i ++) {
        //     std::cout << mark_slash[i] << " ";
        // }
        // std::cout << std::endl;

        if (current_size == size) {
            // std::cout << "checking" << std::endl;

            // print_DFS_puzzle(solve_puzzle);
            bool slash_check_fail_flag = false;
            for (int i = 1; i <= slash_size; i ++){
                if (!mark_slash[i]) {
                    slash_check_fail_flag = true;
                    // std::cout << "?" << std::endl;
                    break;
                }
            }
            bool no_rectangle_check_fail_flag = false;
            if (puzzle_no_rectangles) {
                int rectangle_width = dfs_rectangle_right[dfs_current_shape_cnt - 1] - dfs_rectangle_left[dfs_current_shape_cnt - 1] + 1;
                int rectangle_height = dfs_rectangle_down[dfs_current_shape_cnt - 1] - dfs_rectangle_up[dfs_current_shape_cnt - 1] + 1;
                if (rectangle_width * rectangle_height == size) {
                    no_rectangle_check_fail_flag = true;
                }
            }
            bool one_symbol_per_region_check_fail_flag = false;
            if (puzzle_one_symbol_per_region) {
                if (symbol_loc.dx == -233 && symbol_loc.dy == -666) {
                    one_symbol_per_region_check_fail_flag = true;
                }
            }

            if (slash_check_fail_flag || no_rectangle_check_fail_flag || one_symbol_per_region_check_fail_flag) {
                // std::cout << "Error check failed" << std::endl;
                continue;
            }

            uint32_t** temp_shape = new uint32_t*[100];
            for (int i = 0; i < 100; ++i) {
                temp_shape[i] = new uint32_t[100];
                for (int j = 0; j < 100; ++j) {
                    temp_shape[i][j] = 0;
                }
            }
            int start_x = 1000, start_y = 1000;
            int shape_size = 0;
            for (int i = 0; i < current_size; ++i) {
                start_x = std::min(start_x, dfs_current_shape[i].dx);
                start_y = std::min(start_y, dfs_current_shape[i].dy);
            }
            for (int i = 0; i < current_size; ++i) {
                int dx = dfs_current_shape[i].dx - start_x;
                int dy = dfs_current_shape[i].dy - start_y;
                temp_shape[dx][dy] = 1;
                shape_size = std::max(shape_size, std::max(dx, dy) + 1);
            }

            int shape_index = shape_in_shapes(shape_size, temp_shape);
            if (shape_index != -1) {
                // std::cout << "Shape already exists, skip adding. " << shape_index << " " << known_shape_index << std::endl;
                if (up_left_seq && shape_index <= known_shape_index) {
                    // std::cout << "skip checking." << std::endl;
                    continue;
                }
                if (puzzle_all_shapes_different && current_shape_index_pool.find(shape_index) != current_shape_index_pool.end()) {
                    continue;
                }
            }
            else {
                shape_index = next_shape_index;
                // std::cout << "add shape " << shape_index << std::endl;
                for (int k = 0; k < 4; ++k) {
                    // for (int i = 0; i < shape_size; ++i) {
                    //     for (int j = 0; j < shape_size; ++j) {
                    //         std::cout << temp_shape[i][j] << " ";
                    //     }
                    //     std::cout << std::endl;
                    // }
                    // std::cout << std::endl;
                    add_shape_to_shapes(next_shape_index, shape_size, temp_shape);
                    rotate_shape(shape_size, temp_shape);
                }
                mirror_shape(shape_size, temp_shape);
                for (int k = 0; k < 4; ++k) {
                    // for (int i = 0; i < shape_size; ++i) {
                    //     for (int j = 0; j < shape_size; ++j) {
                    //         std::cout << temp_shape[i][j] << " ";
                    //     }
                    //     std::cout << std::endl;
                    // }
                    // std::cout << std::endl;
                    add_shape_to_shapes(next_shape_index, shape_size, temp_shape);
                    rotate_shape(shape_size, temp_shape);
                }
                // std::cout << "Added shape " << next_shape_index << " with size " << shape_size << std::endl;
                shape_index_size_map[next_shape_index] = shapes[shapes.size() - 1].search_order.size();
                next_shape_index ++;
            }

            for (int i = 0; i < current_size; ++i) {
                solve_puzzle[((x + dfs_current_shape[i].dx) << 1) + 1][((y + dfs_current_shape[i].dy) << 1) + 1] |= (shape_index << SOLVE_AREA_SHAPE_INDEX_BIT_SHIFT);
            }

            bool shape_check_fail_flag = false;
            bool nearby_shape_check_fail_flag = false;
            bool nearby_size_check_fail_flag = false;
            bool shape_in_puzzle_fail_flag = false;
            bool tatami_check_fail_flag = false;
            bool loopy_check_fail_flag = false;
            bool radar_check_fail_flag = false;
            for (int i = 0; i < current_size; ++i) {
                int new_x = ((x + dfs_current_shape[i].dx) << 1) + 1;
                int new_y = ((y + dfs_current_shape[i].dy) << 1) + 1;
                if (!check_edge_shape(new_x, new_y, puzzle_n_row, puzzle_n_col, solve_puzzle)) {
                    shape_check_fail_flag = true;
                    break;
                }
                if (puzzle_adjacent_shapes_different && !check_nearby_shape(new_x, new_y, puzzle_n_row, puzzle_n_col, solve_puzzle)) {
                    nearby_shape_check_fail_flag = true;
                    break;
                }
                if (puzzle_adjacent_sizes_different && !check_nearby_size(new_x, new_y, puzzle_n_row, puzzle_n_col, solve_puzzle)) {
                    nearby_size_check_fail_flag = true;
                    break;
                }
                if (puzzle[new_x][new_y] & AREA_SHAPE_INDEX_BIT) {
                    if ((solve_puzzle[new_x][new_y] >> SOLVE_AREA_SHAPE_INDEX_BIT_SHIFT) != (puzzle[new_x][new_y] >> AREA_SHAPE_INDEX_BIT_SHIFT)) {
                        shape_in_puzzle_fail_flag = true;
                        break;
                    }
                }
                if (puzzle_no_4_way_intersections && !check_tatami(new_x, new_y, puzzle_n_row, puzzle_n_col, solve_puzzle)) {
                    tatami_check_fail_flag = true;
                    break;
                }
                if (puzzle_no_3_way_intersections && !check_loopy(new_x, new_y, puzzle_n_row, puzzle_n_col, solve_puzzle)) {
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
                int new_x = ((x + palisade_visited[i].dx) << 1) + 1;
                int new_y = ((y + palisade_visited[i].dy) << 1) + 1;
                if (!check_palisade(new_x, new_y, puzzle_n_row, puzzle_n_col, solve_puzzle)) {
                    palisade_fail_flag = true;
                    break;
                }
            }

            bool compass_fail_flag = false;
            for (int i = 0; i < compass_visited_cnt; ++i) {
                int new_x = ((x + compass_visited[i].dx) << 1) + 1;
                int new_y = ((y + compass_visited[i].dy) << 1) + 1;
                // int up_count = 0;
                // int down_count = 0;
                // int left_count = 0;
                // int right_count = 0;
                // for (int w = 0; w < current_size; ++w) {
                //     std::cout << dfs_current_shape[w].dx << " " << dfs_current_shape[w].dy << std::endl;
                //     if (dfs_current_shape[w].dx < compass_visited[i].dx) {
                //         up_count ++;
                //     }
                //     if (dfs_current_shape[w].dx > compass_visited[i].dx) {
                //         down_count ++;
                //     }
                //     if (dfs_current_shape[w].dy < compass_visited[i].dy) {
                //         left_count ++;
                //     }
                //     if (dfs_current_shape[w].dy > compass_visited[i].dy) {
                //         right_count ++;
                //     }
                // }
                if (puzzle_compass_up[new_x][new_y] != -1 && puzzle_compass_up[new_x][new_y] != compass_visited_up_cnt[i]) {
                    // std::cout << "1 fail " << puzzle_compass_up[new_x][new_y] << " " << compass_visited_up_cnt[i] << std::endl;
                    compass_fail_flag = true;
                    break;
                }
                if (puzzle_compass_down[new_x][new_y] != -1 && puzzle_compass_down[new_x][new_y] != compass_visited_down_cnt[i]) {
                    // std::cout << "2 fail " << puzzle_compass_down[new_x][new_y] << " " << compass_visited_down_cnt[i] << std::endl;
                    compass_fail_flag = true;
                    break;
                }
                if (puzzle_compass_left[new_x][new_y] != -1 && puzzle_compass_left[new_x][new_y] != compass_visited_left_cnt[i]) {
                    // std::cout << "3 fail " << puzzle_compass_left[new_x][new_y] << " " << compass_visited_left_cnt[i] << std::endl;
                    compass_fail_flag = true;
                    break;
                }
                if (puzzle_compass_right[new_x][new_y] != -1 && puzzle_compass_right[new_x][new_y] != compass_visited_right_cnt[i]) {
                    // std::cout << "4 fail " << puzzle_compass_right[new_x][new_y] << " " << compass_visited_right_cnt[i] << std::endl;
                    compass_fail_flag = true;
                    break;
                }
            }

            if (shape_check_fail_flag || nearby_shape_check_fail_flag || shape_in_puzzle_fail_flag || nearby_size_check_fail_flag || palisade_fail_flag || tatami_check_fail_flag || loopy_check_fail_flag || radar_check_fail_flag || compass_fail_flag) {
                // std::cout << "Shape check failed after placing shape " << shape_index << ", backtracking." << std::endl;
                for (int i = 0; i < current_size; ++i) {
                    solve_puzzle[((x + dfs_current_shape[i].dx) << 1) + 1][((y + dfs_current_shape[i].dy) << 1) + 1] &= (~SOLVE_AREA_SHAPE_INDEX_BIT);
                } 
                continue;
            }

            // std::cout << "Placed shape " << shape_index << " successfully." << std::endl;

            // print_DFS_puzzle(solve_puzzle);

            int ret;

            if (slash_size != 0 && !empty_area_slash_check(puzzle_n_row, puzzle_n_col, solve_puzzle)) {
                // std::cout << "??" << std::endl;
                ret = -1;
            }
            else if (!empty_area_size_check(puzzle_shape_size_lower_bound, puzzle_shape_size_upper_bound, puzzle_n_row, puzzle_n_col, solve_puzzle)) {
                // std::cout << "???" << std::endl;
                ret = -1;
                // std::cout << "Empty area size check failed" << std::endl;
            }
            else {
                if (puzzle_all_shapes_same) {
                    all_shapes_same_index = shape_index;
                }
                current_shape_index_pool.insert(shape_index);
                ret = DFS_special_start(index + 1, solve_puzzle);
                if (puzzle_all_shapes_same) {
                    all_shapes_same_index = -1;
                }
            }

            if (ret != -1) {
                return ret;
            }
            else {
                current_shape_index_pool.erase(shape_index);
                for (int i = 0; i < current_size; ++i) {
                    solve_puzzle[((x + dfs_current_shape[i].dx) << 1) + 1][((y + dfs_current_shape[i].dy) << 1) + 1] &= (~SOLVE_AREA_SHAPE_INDEX_BIT);
                }                    
            }
            continue;
        }

        bool delta_size_check = false;

        while (candidates_i < candidates_size) {

            // std::cout << "Index: " << index << " Try" << " candidate (" << dfs_expand_candidates[candidates_i].dx << ", " << dfs_expand_candidates[candidates_i].dy << ") with distance " << dfs_expand_candidates_distance[candidates_i] << std::endl;

            expand_x = x + dfs_expand_candidates[candidates_i].dx;
            expand_y = y + dfs_expand_candidates[candidates_i].dy;
            expand_distance = dfs_expand_candidates_distance[candidates_i];
            
            bool jump_for_ordered_search = false;
            if (expand_distance < expand_distance_lb) {
                candidates_i ++;
                jump_for_ordered_search = true;
            }
            else if (expand_distance == expand_distance_lb) {
                if (expand_x < expand_x_lb) {
                    candidates_i ++;
                    jump_for_ordered_search = true;
                }
                else if (expand_x == expand_x_lb) {
                    if (expand_y <= expand_y_lb) {
                        candidates_i ++;
                        jump_for_ordered_search = true;
                    }
                }
            }
            if (jump_for_ordered_search) {
                bool in_current_shape = false;
                for (int j = 0; j < dfs_current_shape_cnt; ++j) {
                    // std::cout << dfs_current_shape[j].dx << " " << dfs_current_shape[j].dy << std::endl;
                    // std::cout << dfs_expand_candidates[candidates_i - 1].dx << " " << dfs_expand_candidates[candidates_i - 1].dy << std::endl;
                    if (dfs_current_shape[j].dx == dfs_expand_candidates[candidates_i - 1].dx && dfs_current_shape[j].dy == dfs_expand_candidates[candidates_i - 1].dy) {
                        in_current_shape = true;
                        break;
                    }
                }
                if (!in_current_shape) {
                    // std::cout << expand_x << " " << expand_y << std::endl;
                    memset(visited, 0, sizeof(visited));
                    visited_nodes.clear();
                    int size = DFS_count_empty(to_puzzle_x(expand_x), to_puzzle_y(expand_y), puzzle_n_row, puzzle_n_col, solve_puzzle);
                    // std::cout << size << " " << puzzle_shape_size_lower_bound << std::endl;
                    if (size < puzzle_shape_size_lower_bound) {
                        // std::cout << "Delta Empty area size check failed" << std::endl;
                        // delta_size_check = true;
                        break;
                    }
                }
                continue;
            }

            int slash_index = 0;
            if (puzzle[to_puzzle_x(expand_x)][to_puzzle_y(expand_y)] & AREA_SLASH_INDEX_BIT) {
                slash_index = puzzle[to_puzzle_x(expand_x)][to_puzzle_y(expand_y)] >> AREA_SLASH_INDEX_BIT_SHIFT;
                if (mark_slash[slash_index]) {
                    // std::cout << "jump trying " << " candidate (" << dfs_expand_candidates[candidates_i].dx << ", " << dfs_expand_candidates[candidates_i].dy << ")" << " slash_index " << slash_index << std::endl;
                    candidates_i ++;
                    continue;
                }
            }

            if (puzzle[to_puzzle_x(expand_x)][to_puzzle_y(expand_y)] & AREA_SHAPE_SIZE_BIT) {
                int target_size = puzzle[to_puzzle_x(expand_x)][to_puzzle_y(expand_y)] >> AREA_SHAPE_SIZE_BIT_SHIFT;
                if (target_size != size) {
                    // std::cout << "jump trying " << " candidate (" << dfs_expand_candidates[candidates_i].dx << ", " << dfs_expand_candidates[candidates_i].dy << ")" << " target_size " << target_size << std::endl;
                    candidates_i ++;
                    continue;
                }
            }

            solve_puzzle[to_puzzle_x(expand_x)][to_puzzle_y(expand_y)] = index;
            if (!check_edge(to_puzzle_x(expand_x), to_puzzle_y(expand_y), puzzle_n_row, puzzle_n_col, solve_puzzle)) {
                solve_puzzle[to_puzzle_x(expand_x)][to_puzzle_y(expand_y)] = AREA_NORMAL;
                candidates_i ++;
                continue;
            }

            // std::cout << "Add candidate (" << dfs_expand_candidates[candidates_i].dx << ", " << dfs_expand_candidates[candidates_i].dy << ")" << " slash_index " << slash_index << std::endl;
            mark_slash[slash_index] = true;

            // std::cout << "Expand to (" << expand_x << ", " << expand_y << ") with distance " << expand_distance << std::endl;

            dfs_current_shape[dfs_current_shape_cnt].dx = dfs_expand_candidates[candidates_i].dx;
            dfs_current_shape[dfs_current_shape_cnt].dy = dfs_expand_candidates[candidates_i].dy;
            if (puzzle_only_rectangles || puzzle_no_rectangles) {
                if (dfs_current_shape_cnt == 0) {
                    dfs_rectangle_up[dfs_current_shape_cnt] = dfs_expand_candidates[candidates_i].dx;
                    dfs_rectangle_down[dfs_current_shape_cnt] = dfs_expand_candidates[candidates_i].dx;
                    dfs_rectangle_left[dfs_current_shape_cnt] = dfs_expand_candidates[candidates_i].dy;
                    dfs_rectangle_right[dfs_current_shape_cnt] = dfs_expand_candidates[candidates_i].dy;
                }
                else {
                    dfs_rectangle_up[dfs_current_shape_cnt] = std::min(dfs_rectangle_up[dfs_current_shape_cnt - 1], dfs_expand_candidates[candidates_i].dx);
                    dfs_rectangle_down[dfs_current_shape_cnt] = std::max(dfs_rectangle_down[dfs_current_shape_cnt - 1], dfs_expand_candidates[candidates_i].dx);
                    dfs_rectangle_left[dfs_current_shape_cnt] = std::min(dfs_rectangle_left[dfs_current_shape_cnt - 1], dfs_expand_candidates[candidates_i].dy);
                    dfs_rectangle_right[dfs_current_shape_cnt] = std::max(dfs_rectangle_right[dfs_current_shape_cnt - 1], dfs_expand_candidates[candidates_i].dy);
                }
            }
            for (int i = 0; i < compass_visited_cnt; ++i) {
                if (dfs_current_shape[dfs_current_shape_cnt].dx < compass_visited[i].dx) {
                    compass_visited_up_cnt[i] ++;
                }
                if (dfs_current_shape[dfs_current_shape_cnt].dx > compass_visited[i].dx) {
                    compass_visited_down_cnt[i] ++;
                }
                if (dfs_current_shape[dfs_current_shape_cnt].dy < compass_visited[i].dy) {
                    compass_visited_left_cnt[i] ++;
                }
                if (dfs_current_shape[dfs_current_shape_cnt].dy > compass_visited[i].dy) {
                    compass_visited_right_cnt[i] ++;
                }
            }
            // std::cout << dfs_current_shape[dfs_current_shape_cnt].dx << " " << dfs_current_shape[dfs_current_shape_cnt].dy  << " " << current_size << std::endl;
            for (int i = 1; i <= slash_size; ++i) {
                for (int j = 0; j < slash_nodes[1].size(); ++j) {
                    
                    if (dfs_current_shape_cnt == 0) {
                        slash_distance[dfs_current_shape_cnt][i][j] = dfs_current_shape_cnt;
                        continue;
                    }
                    else {
                        slash_distance[dfs_current_shape_cnt][i][j] = slash_distance[dfs_current_shape_cnt - 1][i][j];
                    }

                    int new_distance = std::abs(x + dfs_current_shape[dfs_current_shape_cnt].dx - slash_nodes[i][j].dx) + std::abs(y + dfs_current_shape[dfs_current_shape_cnt].dy - slash_nodes[i][j].dy);
                    int old_distance = std::abs(x + dfs_current_shape[slash_distance[dfs_current_shape_cnt][i][j]].dx - slash_nodes[i][j].dx) + std::abs(y + dfs_current_shape[slash_distance[dfs_current_shape_cnt][i][j]].dy - slash_nodes[i][j].dy);

                    if (new_distance < old_distance) {
                        slash_distance[dfs_current_shape_cnt][i][j] = dfs_current_shape_cnt;
                    }

                    // std::cout << slash_distance[dfs_current_shape_cnt][i] << " ";
                    // for (auto entry: slash_nodes[i]) {
                    //     int distance = std::abs(x + dfs_current_shape[dfs_current_shape_cnt].dx - entry.dx) + std::abs(y + dfs_current_shape[dfs_current_shape_cnt].dy - entry.dy);
                    //     // std::cout << entry.dx << " " << entry.dy << " " << distance << std::endl;
                    //     if (distance < slash_distance[dfs_current_shape_cnt][i]) {
                    //         slash_distance[dfs_current_shape_cnt][i] = distance;
                    //     }
                    // }
                }
            }

            dfs_current_shape_cnt ++;

            if (puzzle[to_puzzle_x(expand_x)][to_puzzle_y(expand_y)] & AREA_PALISADE_INDEX_BIT) {
                palisade_visited[palisade_visited_cnt] = {dfs_expand_candidates[candidates_i].dx, dfs_expand_candidates[candidates_i].dy};
                palisade_visited_cnt ++;
            }
            if (puzzle[to_puzzle_x(expand_x)][to_puzzle_y(expand_y)] & AREA_COMPASS_ENABLE) {
                compass_visited[compass_visited_cnt] = {dfs_expand_candidates[candidates_i].dx, dfs_expand_candidates[candidates_i].dy};
                compass_visited_up_cnt[compass_visited_cnt] = 0;
                compass_visited_down_cnt[compass_visited_cnt] = 0;
                compass_visited_left_cnt[compass_visited_cnt] = 0;
                compass_visited_right_cnt[compass_visited_cnt] = 0;
                for (int w = 0; w < dfs_current_shape_cnt; ++w) {
                    if (dfs_current_shape[w].dx < compass_visited[compass_visited_cnt].dx) {
                        compass_visited_up_cnt[compass_visited_cnt] ++;
                    }
                    if (dfs_current_shape[w].dx > compass_visited[compass_visited_cnt].dx) {
                        compass_visited_down_cnt[compass_visited_cnt] ++;
                    }
                    if (dfs_current_shape[w].dy < compass_visited[compass_visited_cnt].dy) {
                        compass_visited_left_cnt[compass_visited_cnt] ++;
                    }
                    if (dfs_current_shape[w].dy > compass_visited[compass_visited_cnt].dy) {
                        compass_visited_right_cnt[compass_visited_cnt] ++;
                    }
                }
                compass_visited_cnt ++;
            }

            bool rectangle_fail_flag = false;
            if (puzzle_only_rectangles) {
                int rectangle_width = dfs_rectangle_right[dfs_current_shape_cnt - 1] - dfs_rectangle_left[dfs_current_shape_cnt - 1] + 1;
                int rectangle_height = dfs_rectangle_down[dfs_current_shape_cnt - 1] - dfs_rectangle_up[dfs_current_shape_cnt - 1] + 1;
                // std::cout << rectangle_height << " " << rectangle_width << std::endl;
                // std::cout << size << std::endl;
                if (rectangle_width * rectangle_height > size) {
                    rectangle_fail_flag = true;
                }
            }

            bool palisade_fail_flag = false;
            for (int i = 0; i < palisade_visited_cnt; ++i) {
                int new_x = ((x + palisade_visited[i].dx) << 1) + 1;
                int new_y = ((y + palisade_visited[i].dy) << 1) + 1;
                if (!check_palisade_delta(new_x, new_y, puzzle_n_row, puzzle_n_col, solve_puzzle)) {
                    palisade_fail_flag = true;
                    break;
                }
            }

            bool compass_fail_flag = false;
            for (int i = 0; i < compass_visited_cnt; ++i) {
                int new_x = ((x + compass_visited[i].dx) << 1) + 1;
                int new_y = ((y + compass_visited[i].dy) << 1) + 1;
                // int up_count = 0;
                // int down_count = 0;
                // int left_count = 0;
                // int right_count = 0;
                // for (int w = 0; w < dfs_current_shape_cnt; ++w) {
                //     std::cout << dfs_current_shape[w].dx << " " << dfs_current_shape[w].dy << std::endl;
                //     if (dfs_current_shape[w].dx < compass_visited[i].dx) {
                //         up_count ++;
                //     }
                //     if (dfs_current_shape[w].dx > compass_visited[i].dx) {
                //         down_count ++;
                //     }
                //     if (dfs_current_shape[w].dy < compass_visited[i].dy) {
                //         left_count ++;
                //     }
                //     if (dfs_current_shape[w].dy > compass_visited[i].dy) {
                //         right_count ++;
                //     }
                // }
                if (puzzle_compass_up[new_x][new_y] != -1 && puzzle_compass_up[new_x][new_y] < compass_visited_up_cnt[i]) {
                    // std::cout << "up fail " << compass_visited_up_cnt[i] << " " << puzzle_compass_up[new_x][new_y] << std::endl;
                    compass_fail_flag = true;
                    break;
                }
                if (puzzle_compass_down[new_x][new_y] != -1 && puzzle_compass_down[new_x][new_y] < compass_visited_down_cnt[i]) {
                    // std::cout << "down fail " << compass_visited_down_cnt[i] << " " << puzzle_compass_down[new_x][new_y] << std::endl;
                    compass_fail_flag = true;
                    break;
                }
                if (puzzle_compass_left[new_x][new_y] != -1 && puzzle_compass_left[new_x][new_y] < compass_visited_left_cnt[i]) {
                    // std::cout << "left fail " << compass_visited_left_cnt[i] << " " << puzzle_compass_left[new_x][new_y] << std::endl;
                    compass_fail_flag = true;
                    break;
                }
                if (puzzle_compass_right[new_x][new_y] != -1 && puzzle_compass_right[new_x][new_y] < compass_visited_right_cnt[i]) {
                    // std::cout << "right fail " << compass_visited_right_cnt[i] << " " << puzzle_compass_right[new_x][new_y] << std::endl;
                    compass_fail_flag = true;
                    break;
                }
            }

            bool slash_distance_fail_flag = false;
            // print_DFS_puzzle(solve_puzzle);
            // std::cout << "check" << std::endl;
            if (slash_size != 0) {
                int distance_predict = 0x0fffffff;
                int slash_node_indexs[slash_size + 1];
                for (int i = 1; i <= slash_size; ++i) {
                    slash_node_indexs[i] = 0;
                }
                while (true) {
                    int minx = 0, maxx = 0, miny = 0, maxy = 0;
                    for (int i = 1; i <= slash_size; ++i) {
                        // std::cout << "select " << slash_node_indexs[i] << " " << slash_nodes[i][slash_node_indexs[i]].dx << " " << slash_nodes[i][slash_node_indexs[i]].dy << std::endl;
                        if (mark_slash[i]) {
                            continue;
                        }
                        // std::cout << "near " << dfs_current_shape[slash_distance[dfs_current_shape_cnt - 1][i][slash_node_indexs[i]]].dx << " " << dfs_current_shape[slash_distance[dfs_current_shape_cnt - 1][i][slash_node_indexs[i]]].dy << std::endl;
                        int dx = x + dfs_current_shape[slash_distance[dfs_current_shape_cnt - 1][i][slash_node_indexs[i]]].dx - slash_nodes[i][slash_node_indexs[i]].dx;
                        int dy = y + dfs_current_shape[slash_distance[dfs_current_shape_cnt - 1][i][slash_node_indexs[i]]].dy - slash_nodes[i][slash_node_indexs[i]].dy;
                        minx = std::min(dx, minx);
                        maxx = std::max(dx, maxx);
                        miny = std::min(dy, miny);
                        maxy = std::max(dy, maxy);
                    }
                    // std::cout << minx << " " << miny << " " << maxx << " " << maxy << std::endl;
                    distance_predict = std::min(distance_predict, maxx + maxy - minx - miny);
                    // std::cout << "distance predict " << distance_predict << std::endl;

                    slash_node_indexs[1] += 1;
                    int temp_loc = 1;
                    bool final_flag = false;
                    while (true) {
                        if (slash_node_indexs[temp_loc] == slash_nodes[1].size()) {
                            slash_node_indexs[temp_loc] = 0;
                            if (temp_loc < slash_size) {
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
                    if (final_flag) {
                        break;
                    }
                }

                int remain_size = size - current_size - 1;
                // std::cout << "remain_size " << remain_size << std::endl;

                if (distance_predict > remain_size) {
                    // std::cout << "remain_size check fail" << std::endl;
                    slash_distance_fail_flag = true;
                }

            }
            // for (int i = 1; i <= slash_size; ++i) {
            //     // std::cout << i << " " << mark_slash[i] << " " << slash_distance[dfs_current_shape_cnt - 1][i] << std::endl;
            //     int remain_size = size - current_size;
            //     if (!mark_slash[i] && remain_size < slash_distance[dfs_current_shape_cnt - 1][i]) {
            //         // std::cout << size << " " << remain_size << " " << slash_distance[dfs_current_shape_cnt - 1][i] << std::endl;
            //         slash_distance_fail_flag = true;
            //         break;
            //     }
            // }

            if (palisade_fail_flag || rectangle_fail_flag || compass_fail_flag || slash_distance_fail_flag) {
                // if (slash_distance_fail_flag) {
                // printf("!!!");
                //     print_DFS_puzzle(solve_puzzle);
                // }

                mark_slash[slash_index] = false;
                solve_puzzle[to_puzzle_x(expand_x)][to_puzzle_y(expand_y)] = AREA_NORMAL;
                dfs_current_shape_cnt --;
                candidates_i ++;
                if (puzzle[to_puzzle_x(expand_x)][to_puzzle_y(expand_y)] & AREA_PALISADE_INDEX_BIT) {
                    palisade_visited_cnt --;
                }
                if (puzzle[to_puzzle_x(expand_x)][to_puzzle_y(expand_y)] & AREA_COMPASS_ENABLE) {
                    compass_visited_cnt --;
                }       
                for (int i = 0; i < compass_visited_cnt; ++i) {
                    if (dfs_current_shape[dfs_current_shape_cnt].dx < compass_visited[i].dx) {
                        compass_visited_up_cnt[i] --;
                    }
                    if (dfs_current_shape[dfs_current_shape_cnt].dx > compass_visited[i].dx) {
                        compass_visited_down_cnt[i] --;
                    }
                    if (dfs_current_shape[dfs_current_shape_cnt].dy < compass_visited[i].dy) {
                        compass_visited_left_cnt[i] --;
                    }
                    if (dfs_current_shape[dfs_current_shape_cnt].dy > compass_visited[i].dy) {
                        compass_visited_right_cnt[i] --;
                    }
                }         
                continue;
            }

            if (puzzle_one_symbol_per_region) {
                bool symbol_found = false;
                if (puzzle[to_puzzle_x(expand_x)][to_puzzle_y(expand_y)] & AREA_PALISADE_INDEX_BIT) {
                    symbol_found = true;
                }
                if (puzzle[to_puzzle_x(expand_x)][to_puzzle_y(expand_y)] & AREA_SLASH_INDEX_BIT) {
                    symbol_found = true;
                }
                if (puzzle[to_puzzle_x(expand_x)][to_puzzle_y(expand_y)] & AREA_SHAPE_INDEX_BIT) {
                    symbol_found = true;
                }
                if (puzzle[to_puzzle_x(expand_x)][to_puzzle_y(expand_y)] & AREA_SHAPE_SIZE_BIT) {
                    symbol_found = true;
                }
                if (puzzle[to_puzzle_x(expand_x)][to_puzzle_y(expand_y)] & AREA_COMPASS_ENABLE) {
                    symbol_found = true;
                }
                if (symbol_found) {
                    if (symbol_loc.dx == -233 && symbol_loc.dy == -666) {
                        symbol_loc = {expand_x, expand_y};
                    }
                    else {
                        mark_slash[slash_index] = false;
                        solve_puzzle[to_puzzle_x(expand_x)][to_puzzle_y(expand_y)] = AREA_NORMAL;
                        dfs_current_shape_cnt --;
                        candidates_i ++;
                        if (puzzle[to_puzzle_x(expand_x)][to_puzzle_y(expand_y)] & AREA_PALISADE_INDEX_BIT) {
                            palisade_visited_cnt --;
                        }
                        if (puzzle[to_puzzle_x(expand_x)][to_puzzle_y(expand_y)] & AREA_COMPASS_ENABLE) {
                            compass_visited_cnt --;
                        }
                        for (int i = 0; i < compass_visited_cnt; ++i) {
                            if (dfs_current_shape[dfs_current_shape_cnt].dx < compass_visited[i].dx) {
                                compass_visited_up_cnt[i] --;
                            }
                            if (dfs_current_shape[dfs_current_shape_cnt].dx > compass_visited[i].dx) {
                                compass_visited_down_cnt[i] --;
                            }
                            if (dfs_current_shape[dfs_current_shape_cnt].dy < compass_visited[i].dy) {
                                compass_visited_left_cnt[i] --;
                            }
                            if (dfs_current_shape[dfs_current_shape_cnt].dy > compass_visited[i].dy) {
                                compass_visited_right_cnt[i] --;
                            }
                        }
                        continue;
                    }
                }
            }

            int expand_dx[] = {0, 0, -1, 1};
            int expand_dy[] = {-1, 1, 0, 0};

            for (int d = 0; d < 4; ++d) {
                int new_dx = dfs_expand_candidates[candidates_i].dx + expand_dx[d];
                int new_dy = dfs_expand_candidates[candidates_i].dy + expand_dy[d];

                // std::cout << "Try add candidate (" << new_dx << ", " << new_dy << ") with distance " << expand_distance + 1 << std::endl;
                
                int puzzle_value = puzzle[((x + new_dx) << 1) + 1][((y + new_dy) << 1) + 1];
                if (puzzle_value == AREA_BLOCK) {
                    // std::cout << "fail 1" << std::endl;
                    continue;
                }

                int solve_puzzle_value = solve_puzzle[((x + new_dx) << 1) + 1][((y + new_dy) << 1) + 1];
                if (solve_puzzle_value != AREA_NORMAL) {
                    // std::cout << "fail 2" << std::endl;
                    continue;
                }
                
                bool already_in_candidates = false;
                for (int j = 0; j < dfs_current_shape_cnt; ++j) {
                    if (dfs_current_shape[j].dx == new_dx && dfs_current_shape[j].dy == new_dy) {
                        already_in_candidates = true;
                        // std::cout << "fail 3" << std::endl;
                        break;
                    }
                }
                for (int j = 0; j < dfs_expand_candidates_cnt; ++j) {
                    if (!already_in_candidates && dfs_expand_candidates[j].dx == new_dx && dfs_expand_candidates[j].dy == new_dy) {
                        already_in_candidates = true;
                        // std::cout << "fail 4" << std::endl;
                        break;
                    }
                }
                if (already_in_candidates) {
                    continue;
                }

                // std::cout << "add candidate (" << new_dx << ", " << new_dy << ") with distance " << expand_distance + 1 << std::endl;
                
                dfs_expand_candidates[dfs_expand_candidates_cnt] = {new_dx, new_dy};
                dfs_expand_candidates_distance[dfs_expand_candidates_cnt] = expand_distance + 1;
                dfs_expand_candidates_cnt ++;
            }

            stack_size[stack_top] = current_size;
            stack_expand_distance_lb[stack_top] = expand_distance_lb;
            stack_expand_x_lb[stack_top] = expand_x_lb;
            stack_expand_y_lb[stack_top] = expand_y_lb;
            stack_candidates_i[stack_top] = candidates_i + 1;
            stack_candidates_size[stack_top] = candidates_size;
            stack_top ++;

            stack_size[stack_top] = current_size + 1;
            stack_expand_distance_lb[stack_top] = expand_distance;
            stack_expand_x_lb[stack_top] = expand_x;
            stack_expand_y_lb[stack_top] = expand_y;
            stack_candidates_i[stack_top] = 0;
            stack_candidates_size[stack_top] = dfs_expand_candidates_cnt;
            stack_top ++;
            break;
        }
        // if (delta_size_check) {
        //     continue;
        // }
    }

    return -1;
}

int DFS(uint32_t index, uint32_t** solve_puzzle) {

    // print_DFS_puzzle(solve_puzzle);

    auto [ret, x, y] = find_empty_area(solve_puzzle);

    if (ret == -1) {
        return 0; // Solved
    }
    // std::cout << "Index: " << index << " DFS at " << "(" << x << ", " << y << ")" << std::endl;

    int known_shape_size = shapes.size();

    bool mark_skip_shape_index[65536];
    memset(mark_skip_shape_index, 0, sizeof(mark_skip_shape_index));

    bool mark_skip_shape[65536];
    memset(mark_skip_shape, 0, sizeof(mark_skip_shape));

    bool mark_slash[slash_size + 1];

    const int MAX_SHAPE_SIZE = 100;

    node palisade_visited[MAX_SHAPE_SIZE];
    int palisade_visited_cnt = 0;

    node symbol_loc;

    ret = 0;
    for (int k = 0; k < shapes.size(); ++k) {

        // std::cout << "Index: " << index << " Trying existing shape " << k << " with index " << shapes[k].shape_index << std::endl;

        if (shapes[k].shape_index == 0) {
            continue;
        }

        if (puzzle_all_shapes_different && current_shape_index_pool.find(shapes[k].shape_index) != current_shape_index_pool.end()) {
            // std::cout << "skip shape " << k << " for all shape different."<< std::endl;
            continue;
        }

        if (puzzle_all_shapes_same && all_shapes_same_index != -1) {
            if (shapes[k].shape_index != all_shapes_same_index) {
                // std::cout << "skip shape " << k << " for all shape same."<< std::endl;
                continue;
            }
        }

        if (mark_skip_shape[k]) {
            // std::cout << "Index: " << index << " Skip existing shape " << k << " for similiar point block." << std::endl;
            continue;
        }
        mark_skip_shape_index[shapes[k].shape_index] = true;
        ret = 0;
        int l;
        memset(mark_slash, 0, sizeof(mark_slash));

        bool slash_check_fail_flag = false;
        // for (l = 0; l < shapes[k].search_order.size(); ++l) {
        //     int new_x = ((x + shapes[k].search_order[l].dx) << 1) + 1;
        //     int new_y = ((y + shapes[k].search_order[l].dy) << 1) + 1;
        //     if (puzzle[new_x][new_y] & AREA_SLASH_INDEX_BIT) {
        //         int slash_index = puzzle[new_x][new_y] >> AREA_SLASH_INDEX_BIT_SHIFT;
        //         if (mark_slash[slash_index]) {
        //             slash_check_fail_flag = true;
        //             break;
        //         }
        //         mark_slash[slash_index] = true;
        //     }
        // }
        // for (int i = 1; i <= slash_size; i ++){
        //     if (!mark_slash[i]) {
        //         slash_check_fail_flag = true;
        //         break;
        //     }
        // }
        // if (slash_check_fail_flag) {
        //     continue;
        // }


        palisade_visited_cnt = 0;

        symbol_loc = {-233, -666};

        for (l = 0; l < shapes[k].search_order.size(); ++l) {
            int new_x = ((x + shapes[k].search_order[l].dx) << 1) + 1;
            int new_y = ((y + shapes[k].search_order[l].dy) << 1) + 1;

            if (puzzle_one_symbol_per_region) {
                bool symbol_found = false;
                if (puzzle[new_x][new_y] & AREA_PALISADE_INDEX_BIT) {
                    symbol_found = true;
                }
                if (puzzle[new_x][new_y] & AREA_SLASH_INDEX_BIT) {
                    symbol_found = true;
                }
                if (puzzle[new_x][new_y] & AREA_SHAPE_INDEX_BIT) {
                    symbol_found = true;
                }
                if (puzzle[new_x][new_y] & AREA_SHAPE_SIZE_BIT) {
                    symbol_found = true;
                }
                if (puzzle[new_x][new_y] & AREA_COMPASS_ENABLE) {
                    symbol_found = true;
                }
                if (symbol_found) {
                    if (symbol_loc.dx == -233 && symbol_loc.dy == -666) {
                        symbol_loc = {shapes[k].search_order[l].dx, shapes[k].search_order[l].dy};
                    }
                    else {
                        ret = -1;
                        l --;
                        break;
                    }
                }
            }

            if (puzzle[new_x][new_y] & AREA_PALISADE_INDEX_BIT) {
                palisade_visited[palisade_visited_cnt] = {shapes[k].search_order[l].dx, shapes[k].search_order[l].dy};
                palisade_visited_cnt ++;
            }

            if (puzzle[new_x][new_y] & AREA_COMPASS_ENABLE) {
                // compass_visited[compass_visited_cnt] = {shapes[k].search_order[l].dx - shapes[k].search_order[m].dx, shapes[k].search_order[l].dy - shapes[k].search_order[m].dy};
                // compass_visited_cnt ++;
                int up_count = 0;
                int down_count = 0;
                int left_count = 0;
                int right_count = 0;
                for (int w = 0; w < shapes[k].search_order.size(); ++w) {
                    if (shapes[k].search_order[w].dx < shapes[k].search_order[l].dx) {
                        up_count ++;
                    }
                    if (shapes[k].search_order[w].dx > shapes[k].search_order[l].dx) {
                        down_count ++;
                    }
                    if (shapes[k].search_order[w].dy < shapes[k].search_order[l].dy) {
                        left_count ++;
                    }
                    if (shapes[k].search_order[w].dy > shapes[k].search_order[l].dy) {
                        right_count ++;
                    }
                }
                if (puzzle_compass_up[new_x][new_y] != -1 && puzzle_compass_up[new_x][new_y] != up_count) {
                    ret = -1;
                    l --;
                    break;
                }
                if (puzzle_compass_down[new_x][new_y] != -1 && puzzle_compass_down[new_x][new_y] != down_count) {
                    ret = -1;
                    l --;
                    break;
                }
                if (puzzle_compass_left[new_x][new_y] != -1 && puzzle_compass_left[new_x][new_y] != left_count) {
                    ret = -1;
                    l --;
                    break;
                }
                if (puzzle_compass_right[new_x][new_y] != -1 && puzzle_compass_right[new_x][new_y] != right_count) {
                    ret = -1;
                    l --;
                    break;
                }
            }

            // std::cout << "x =" << x << ", y = " << y << std::endl;
            // std::cout << "dx = " << shapes[k].search_order[l].dx << ", dy = " << shapes[k].search_order[l].dy << std::endl;
            // std::cout << "new_x = " << new_x << ", new_y = " << new_y << std::endl;
            // std::cout << "solve_puzzle[new_x][new_y] = " << solve_puzzle[new_x][new_y] << std::endl;
            if (!area_in_puzzle_range(new_x, new_y, puzzle_n_row, puzzle_n_col)) {
                // std::cout << "x = " << x << ", y = " << y << std::endl;
                // std::cout << "Cell (" << new_x << ", " << new_y << ") is not normal area." << puzzle[new_x][new_y] << std::endl;
                // std::cout << "Index: " << index << " Error placing shape " << k << " at node (" << shapes[k].search_order[l].dx << ", " << shapes[k].search_order[l].dy << ")" << " out of bounds." << std::endl;
                node error_node = {shapes[k].search_order[l].dx, shapes[k].search_order[l].dy};
                for (int skip_index : node_to_shape_index[error_node]) {
                    mark_skip_shape[skip_index] = true;
                }
                ret = -1;
                l --;
                break;
            }
            else if (puzzle[new_x][new_y] == AREA_BLOCK) {
                // std::cout << "x = " << x << ", y = " << y << std::endl;
                // std::cout << "Cell (" << new_x << ", " << new_y << ") is blocked area." << puzzle[new_x][new_y] << std::endl;
                // std::cout << "Index: " << index << " Error placing shape " << k << " at node (" << shapes[k].search_order[l].dx << ", " << shapes[k].search_order[l].dy << ")" << " on blocked cell." << std::endl;
                node error_node = {shapes[k].search_order[l].dx, shapes[k].search_order[l].dy};
                for (int skip_index : node_to_shape_index[error_node]) {
                    mark_skip_shape[skip_index] = true;
                }
                ret = -1;
                l --;
                break;
            }
            else if (solve_puzzle[new_x][new_y] != AREA_NORMAL) {
                // std::cout << "x = " << x << ", y = " << y << std::endl;
                // std::cout << "Cell (" << new_x << ", " << new_y << ") is not normal area." << puzzle[new_x][new_y] << std::endl;
                // std::cout << "Index: " << index << " Error placing shape " << k << " at node (" << shapes[k].search_order[l].dx << ", " << shapes[k].search_order[l].dy << ")" << " on occupied cell." << std::endl;
                node error_node = {shapes[k].search_order[l].dx, shapes[k].search_order[l].dy};
                for (int skip_index : node_to_shape_index[error_node]) {
                    mark_skip_shape[skip_index] = true;
                }
                ret = -1;
                l --;
                break;
            }
            else if (puzzle[new_x][new_y] & AREA_SLASH_INDEX_BIT) {
                int slash_index = puzzle[new_x][new_y] >> AREA_SLASH_INDEX_BIT_SHIFT;
                if (mark_slash[slash_index]) {
                    // std::cout << "Index: " << index << " Error placing shape " << k << " at node (" << shapes[k].search_order[l].dx << ", " << shapes[k].search_order[l].dy << ")" << " requested slash index: " << slash_index << " already placed." << std::endl;
                    ret = -1;
                    l --;
                    break;
                }
                mark_slash[slash_index] = true;
            }
            else if (puzzle[new_x][new_y] & AREA_SHAPE_INDEX_BIT) {
                int target_index = puzzle[new_x][new_y] >> AREA_SHAPE_INDEX_BIT_SHIFT;
                if (target_index != shapes[k].shape_index) {
                    // std::cout << "Index: " << index << " Error placing shape " << k << " at node (" << shapes[k].search_order[l].dx << ", " << shapes[k].search_order[l].dy << ")" << " requested index: " << target_index << " placing index: " << shapes[k].shape_index << std::endl;
                    ret = -1;
                    l --;
                    break;
                }
            }
            else if (puzzle[new_x][new_y] & AREA_SHAPE_SIZE_BIT) {
                int target_size = puzzle[new_x][new_y] >> AREA_SHAPE_SIZE_BIT_SHIFT;
                if (target_size != shapes[k].search_order.size()) {
                    // std::cout << "Index: " << index << " Error placing shape " << k << " at node (" << shapes[k].search_order[l].dx << ", " << shapes[k].search_order[l].dy << ")" << " requested size: " << target_size << " placing size: " << shapes[k].search_order.size() << std::endl;
                    ret = -1;
                    l --;
                    break;
                }
            }
            // std::cout << "add " << shapes[k].search_order[l].dx << " " << shapes[k].search_order[l].dy << std::endl;
            // std::cout << "add " << new_x << " " << new_y << std::endl;
            solve_puzzle[new_x][new_y] = index | (shapes[k].shape_index << SOLVE_AREA_SHAPE_INDEX_BIT_SHIFT);
            if (!check_edge(new_x, new_y, puzzle_n_row, puzzle_n_col, solve_puzzle) || !check_edge_shape(new_x, new_y, puzzle_n_row, puzzle_n_col, solve_puzzle)) {
                ret = -1;
                break;
            }
            if (puzzle_adjacent_shapes_different && !check_nearby_shape(new_x, new_y, puzzle_n_row, puzzle_n_col, solve_puzzle)) {
                ret = -1;
                break;
            }
            if (puzzle_adjacent_sizes_different && !check_nearby_size(new_x, new_y, puzzle_n_row, puzzle_n_col, solve_puzzle)) {
                ret = -1;
                break;
            }
        }
        // std::cout << "After placing shape " << k << ":" << ret << std::endl;
        if (ret == -1) {
            for (int m = 0; m <= l; ++m) {
                int new_x = ((x + shapes[k].search_order[m].dx) << 1) + 1;
                int new_y = ((y + shapes[k].search_order[m].dy) << 1) + 1;
                // std::cout << "remove " << shapes[k].search_order[m].dx << " " << shapes[k].search_order[m].dy << std::endl;
                // std::cout << "remove " << new_x << " " << new_y << std::endl;
                solve_puzzle[new_x][new_y] = AREA_NORMAL;
            }
        }
        else {
            // std::cout << "Placed shape " << shapes[k].shape_index << " successfully." << std::endl;

            // print_DFS_puzzle(solve_puzzle);
            if (puzzle_one_symbol_per_region) {
                if (symbol_loc.dx == -233 && symbol_loc.dy == -666) {
                    ret = -1;
                }
            }
            for (int i = 1; i <= slash_size; i ++){
                if (!mark_slash[i]) {
                    ret = -1;
                    break;
                }
            }
            for (int i = 0; i < palisade_visited_cnt; ++i) {
                int new_x = ((x + palisade_visited[i].dx) << 1) + 1;
                int new_y = ((y + palisade_visited[i].dy) << 1) + 1;
                if (!check_palisade(new_x, new_y, puzzle_n_row, puzzle_n_col, solve_puzzle)) {
                    ret = -1;
                    break;
                }
            }
            if (slash_size != 0 && !empty_area_slash_check(puzzle_n_row, puzzle_n_col, solve_puzzle)) {
                ret = -1;
            }
            if (!empty_area_size_check(puzzle_shape_size_lower_bound, puzzle_shape_size_upper_bound, puzzle_n_row, puzzle_n_col, solve_puzzle)) {
                // std::cout << "Empty area size check failed after placing shape " << k << "." << std::endl;
                // std::cout << "Index: " << index << " Empty area size check failed after placing shape " << k << "." << std::endl;
                ret = -1;
            }
            if (puzzle_no_4_way_intersections) {
                for (int m = 0; m < shapes[k].search_order.size(); ++m) {
                    int new_x = ((x + shapes[k].search_order[m].dx) << 1) + 1;
                    int new_y = ((y + shapes[k].search_order[m].dy) << 1) + 1;
                    if (!check_tatami(new_x, new_y, puzzle_n_row, puzzle_n_col, solve_puzzle)) {
                        ret = -1;
                        break;
                    }
                }
            }

            if (puzzle_no_3_way_intersections) {
                for (int m = 0; m < shapes[k].search_order.size(); ++m) {
                    int new_x = ((x + shapes[k].search_order[m].dx) << 1) + 1;
                    int new_y = ((y + shapes[k].search_order[m].dy) << 1) + 1;
                    if (!check_loopy(new_x, new_y, puzzle_n_row, puzzle_n_col, solve_puzzle)) {
                        ret = -1;
                        break;
                    }
                }
            }

            for (int m = 0; m < shapes[k].search_order.size(); ++m) {
                int new_x = ((x + shapes[k].search_order[m].dx) << 1) + 1;
                int new_y = ((y + shapes[k].search_order[m].dy) << 1) + 1;
                if (!check_radar(new_x, new_y, puzzle_n_row, puzzle_n_col, solve_puzzle)) {
                    ret = -1;
                    break;
                }
            }

            if (ret != -1) {
                bool flag = false;
                if (puzzle_all_shapes_same && all_shapes_same_index == -1) {
                    all_shapes_same_index = shapes[k].shape_index;
                    flag = true;
                }
                current_shape_index_pool.insert(shapes[k].shape_index);
                ret = DFS(index + 1, solve_puzzle);
                if (flag) {
                    all_shapes_same_index = -1;
                }
            }
            if (ret == -1) {
                current_shape_index_pool.erase(shapes[k].shape_index);
                for (int m = 0; m < shapes[k].search_order.size(); ++m) {
                    int new_x = ((x + shapes[k].search_order[m].dx) << 1) + 1;
                    int new_y = ((y + shapes[k].search_order[m].dy) << 1) + 1;
                    // std::cout << "remove " << shapes[k].search_order[m].dx << " " << shapes[k].search_order[m].dy << std::endl;
                    // std::cout << "remove " << new_x << " " << new_y << std::endl;
                    solve_puzzle[new_x][new_y] = AREA_NORMAL;
                }
            }
            else {
                return ret;
            }
        }
    }

    int known_shape_index = -1;
    if (!shapes.empty()) {
        known_shape_index = shapes[shapes.size() - 1].shape_index;
    }

    if (puzzle_predefine_shapes_only || puzzle_only_rectangles) {
        if (ret == -1) {
            // std::cout << "Fail to place shape at " << "(" << x << ", " << y << ")" << std::endl;
        }
        return ret;
    }

    if (puzzle_all_shapes_same && all_shapes_same_index != -1) {
        if (ret == -1) {
            // std::cout << "Fail to place shape at " << "(" << x << ", " << y << ")" << std::endl;
        }
        return ret;
    }

    int size = 0;

    int lower_bound = puzzle_shape_size_lower_bound;
    int upper_bound = puzzle_shape_size_upper_bound;

    int shape_count = -1;

    // std::cout << "Index: " << index << " Empty area shape count: " << shape_count << ", size lower bound: " << lower_bound << ", size upper bound: " << upper_bound << std::endl;

    if (slash_check_enable) {
        shape_count = empty_area_shape_count(to_puzzle_x(x), to_puzzle_y(y), puzzle_n_row, puzzle_n_col, solve_puzzle);
    }

    memset(visited, 0, sizeof(visited));
    visited_nodes.clear();
    size = DFS_count_empty(to_puzzle_x(x), to_puzzle_y(y), puzzle_n_row, puzzle_n_col, solve_puzzle);

    upper_bound = std::min(upper_bound, size);
    if (shape_count == 1) {
        if (size < lower_bound || size > upper_bound) {
            // std::cout << "Index: " << index << " Empty area shape count is 1 with size " << size << " out of bound, fail to place shape." << std::endl;
            return -1;
        }
        lower_bound = size;
    }

    // std::cout << "Index: " << index << " Empty area shape count: " << shape_count << ", size lower bound: " << lower_bound << ", size upper bound: " << upper_bound << std::endl;

    for (int size = lower_bound; size <= upper_bound; ++size) {
        int ret = place_non_predifined_shape(index, x, y, size, true, known_shape_index, solve_puzzle);
        if (ret != -1) {
            return ret;
        }
    }

    return -1;
}

int DFS_special_start(uint32_t index, uint32_t** solve_puzzle) {
    
    // print_DFS_puzzle(solve_puzzle);

    int special_start_type = 0;

    int ret, x, y;
    auto ret_data = find_empty_shape_index_area(solve_puzzle);
    special_start_type = 2;
    ret = std::get<0>(ret_data);
    if (ret == -1) {
        ret_data = find_empty_corner_area(solve_puzzle);
        special_start_type = 3;
    }
    ret = std::get<0>(ret_data);
    if (ret == -1) {
        ret_data = find_empty_shape_size_area(solve_puzzle);
        special_start_type = 1;
    }
    ret = std::get<0>(ret_data);
    if (ret == -1) {
        ret_data = find_empty_compass_area(solve_puzzle);
        special_start_type = 4;
    }
    ret = std::get<0>(ret_data);
    if (ret == -1) {
        ret_data = find_empty_line_constraint_area(solve_puzzle);
        special_start_type = 5;
    }
    ret = std::get<0>(ret_data);
    if (ret == -1) {
        return DFS(index, solve_puzzle);
    }

    x = std::get<1>(ret_data);
    y = std::get<2>(ret_data);

    uint32_t target_shape_size = (puzzle[to_puzzle_x(x)][to_puzzle_y(y)] & AREA_SHAPE_SIZE_BIT) >> AREA_SHAPE_SIZE_BIT_SHIFT;

    // std::cout << "Index: " << index << " DFS special at " << "(" << x << ", " << y << ")" << " with type " << special_start_type << std::endl;

    memset(visited, 0, sizeof(visited));
    visited_nodes.clear();
    int remain_empty_size = DFS_count_empty(to_puzzle_x(x), to_puzzle_y(y), puzzle_n_row, puzzle_n_col, solve_puzzle);

    if (remain_empty_size < target_shape_size) {
        // std::cout << "skip this level for remain empty size " << remain_empty_size << " < " << "target shape size " << target_shape_size << std::endl;
        return -1;
    }

    int known_shape_size = shapes.size();

    bool mark_skip_shape_index[65536];
    memset(mark_skip_shape_index, 0, sizeof(mark_skip_shape_index));

    bool mark_skip_shape[65536];
    memset(mark_skip_shape, 0, sizeof(mark_skip_shape));

    bool mark_slash[slash_size + 1];

    const int MAX_SHAPE_SIZE = 100;

    node palisade_visited[MAX_SHAPE_SIZE];
    int palisade_visited_cnt = 0;

    node symbol_loc;

    ret = 0;
    for (int k = 0; k < shapes.size(); ++k) {

        // std::cout << "Index: " << index << " Trying existing shape " << k << std::endl;

        if (shapes[k].shape_index == 0) {
            continue;
        }

        if (puzzle_all_shapes_different && current_shape_index_pool.find(shapes[k].shape_index) != current_shape_index_pool.end()) {
            // std::cout << "skip shape " << k << " for existing shape index in current pool."<< std::endl;
            continue;
        }

        if (special_start_type == 1 && shapes[k].search_order.size() != target_shape_size) {
            // std::cout << "skip shape " << k << " for size mismatch."<< std::endl;
            continue;
        }

        if (puzzle_all_shapes_same && all_shapes_same_index != -1) {
            if (shapes[k].shape_index != all_shapes_same_index) {
                // std::cout << "skip shape " << k << " for all shape same."<< std::endl;
                continue;
            }
        }

        // if (mark_skip_shape[k]) {
        //     std::cout << "Index: " << index << " Skip existing shape " << k << " for similiar point block." << std::endl;
        //     continue;
        // }
        // mark_skip_shape_index[shapes[k].shape_index] = true;

        for (int m = 0; m < shapes[k].search_order.size(); ++m) {

            // std::cout << "Index: " << index << " Trying existing shape " << k << " with offset (" << shapes[k].search_order[m].dx << "," << shapes[k].search_order[m].dy << ")" << std::endl;

            ret = 0;
            int l;
            memset(mark_slash, 0, sizeof(mark_slash));

            palisade_visited_cnt = 0;
            // compass_visited_cnt = 0;

            symbol_loc = {-233, -666};

            for (l = 0; l < shapes[k].search_order.size(); ++l) {
                int new_x = ((x + shapes[k].search_order[l].dx - shapes[k].search_order[m].dx) << 1) + 1;
                int new_y = ((y + shapes[k].search_order[l].dy - shapes[k].search_order[m].dy) << 1) + 1;

                if (puzzle_one_symbol_per_region) {
                    bool symbol_found = false;
                    if (puzzle[new_x][new_y] & AREA_PALISADE_INDEX_BIT) {
                        symbol_found = true;
                    }
                    if (puzzle[new_x][new_y] & AREA_SLASH_INDEX_BIT) {
                        symbol_found = true;
                    }
                    if (puzzle[new_x][new_y] & AREA_SHAPE_INDEX_BIT) {
                        symbol_found = true;
                    }
                    if (puzzle[new_x][new_y] & AREA_SHAPE_SIZE_BIT) {
                        symbol_found = true;
                    }
                    if (puzzle[new_x][new_y] & AREA_COMPASS_ENABLE) {
                        symbol_found = true;
                    }
                    if (symbol_found) {
                        if (symbol_loc.dx == -233 && symbol_loc.dy == -666) {
                            symbol_loc = {shapes[k].search_order[l].dx, shapes[k].search_order[l].dy};
                        }
                        else {
                            ret = -1;
                            l --;
                            break;
                        }
                    }
                }

                if (puzzle[new_x][new_y] & AREA_PALISADE_INDEX_BIT) {
                    palisade_visited[palisade_visited_cnt] = {shapes[k].search_order[l].dx - shapes[k].search_order[m].dx, shapes[k].search_order[l].dy - shapes[k].search_order[m].dy};
                    palisade_visited_cnt ++;
                }

                if (puzzle[new_x][new_y] & AREA_COMPASS_ENABLE) {
                    // compass_visited[compass_visited_cnt] = {shapes[k].search_order[l].dx - shapes[k].search_order[m].dx, shapes[k].search_order[l].dy - shapes[k].search_order[m].dy};
                    // compass_visited_cnt ++;
                    int up_count = 0;
                    int down_count = 0;
                    int left_count = 0;
                    int right_count = 0;
                    for (int w = 0; w < shapes[k].search_order.size(); ++w) {
                        if (shapes[k].search_order[w].dx - shapes[k].search_order[m].dx < shapes[k].search_order[l].dx - shapes[k].search_order[m].dx) {
                            up_count ++;
                        }
                        if (shapes[k].search_order[w].dx - shapes[k].search_order[m].dx > shapes[k].search_order[l].dx - shapes[k].search_order[m].dx) {
                            down_count ++;
                        }
                        if (shapes[k].search_order[w].dy - shapes[k].search_order[m].dy < shapes[k].search_order[l].dy - shapes[k].search_order[m].dy) {
                            left_count ++;
                        }
                        if (shapes[k].search_order[w].dy - shapes[k].search_order[m].dy > shapes[k].search_order[l].dy - shapes[k].search_order[m].dy) {
                            right_count ++;
                        }
                    }
                    if (puzzle_compass_up[new_x][new_y] != -1 && puzzle_compass_up[new_x][new_y] != up_count) {
                        ret = -1;
                        l --;
                        break;
                    }
                    if (puzzle_compass_down[new_x][new_y] != -1 && puzzle_compass_down[new_x][new_y] != down_count) {
                        ret = -1;
                        l --;
                        break;
                    }
                    if (puzzle_compass_left[new_x][new_y] != -1 && puzzle_compass_left[new_x][new_y] != left_count) {
                        ret = -1;
                        l --;
                        break;
                    }
                    if (puzzle_compass_right[new_x][new_y] != -1 && puzzle_compass_right[new_x][new_y] != right_count) {
                        ret = -1;
                        l --;
                        break;
                    }
                }

                // std::cout << "x =" << x << ", y = " << y << std::endl;
                // std::cout << "dx = " << shapes[k].search_order[l].dx << ", dy = " << shapes[k].search_order[l].dy << std::endl;
                // std::cout << "new_x = " << new_x << ", new_y = " << new_y << std::endl;
                // std::cout << "solve_puzzle[new_x][new_y] = " << solve_puzzle[new_x][new_y] << std::endl;
                if (!area_in_puzzle_range(new_x, new_y, puzzle_n_row, puzzle_n_col)) {
                    // std::cout << "x = " << x << ", y = " << y << std::endl;
                    // std::cout << "Cell (" << new_x << ", " << new_y << ") is not normal area." << puzzle[new_x][new_y] << std::endl;
                    // std::cout << "Index: " << index << " Error placing shape " << k << " at node (" << (shapes[k].search_order[l].dx - shapes[k].search_order[m].dx) << ", " << (shapes[k].search_order[l].dy - shapes[k].search_order[m].dy) << ")" << " out of bounds." << std::endl;
                    // node error_node = {shapes[k].search_order[l].dx, shapes[k].search_order[l].dy};
                    // for (int skip_index : node_to_shape_index[error_node]) {
                    //     mark_skip_shape[skip_index] = true;
                    // }
                    ret = -1;
                    l --;
                    break;
                }
                else if (puzzle[new_x][new_y] == AREA_BLOCK) {
                    // std::cout << "x = " << x << ", y = " << y << std::endl;
                    // std::cout << "Cell (" << new_x << ", " << new_y << ") is blocked area." << puzzle[new_x][new_y] << std::endl;
                    // std::cout << "Index: " << index << " Error placing shape " << k << " at node (" << (shapes[k].search_order[l].dx - shapes[k].search_order[m].dx) << ", " << (shapes[k].search_order[l].dy - shapes[k].search_order[m].dy) << ")" << " on blocked cell." << std::endl;
                    // node error_node = {shapes[k].search_order[l].dx, shapes[k].search_order[l].dy};
                    // for (int skip_index : node_to_shape_index[error_node]) {
                    //     mark_skip_shape[skip_index] = true;
                    // }
                    ret = -1;
                    l --;
                    break;
                }
                else if (solve_puzzle[new_x][new_y] != AREA_NORMAL) {
                    // std::cout << "x = " << x << ", y = " << y << std::endl;
                    // std::cout << "Cell (" << new_x << ", " << new_y << ") is not normal area." << puzzle[new_x][new_y] << std::endl;
                    // std::cout << "Index: " << index << " Error placing shape " << k << " at node (" << (shapes[k].search_order[l].dx - shapes[k].search_order[m].dx) << ", " << (shapes[k].search_order[l].dy - shapes[k].search_order[m].dy) << ")" << " on occupied cell." << std::endl;
                    // node error_node = {shapes[k].search_order[l].dx, shapes[k].search_order[l].dy};
                    // for (int skip_index : node_to_shape_index[error_node]) {
                    //     mark_skip_shape[skip_index] = true;
                    // }
                    ret = -1;
                    l --;
                    break;
                }
                else if (puzzle[new_x][new_y] & AREA_SLASH_INDEX_BIT) {
                    int slash_index = puzzle[new_x][new_y] >> AREA_SLASH_INDEX_BIT_SHIFT;
                    if (mark_slash[slash_index]) {
                        // std::cout << "Index: " << index << " Error placing shape " << k << " at node (" << (shapes[k].search_order[l].dx - shapes[k].search_order[m].dx) << ", " << (shapes[k].search_order[l].dy - shapes[k].search_order[m].dy) << ")" << " requested slash index: " << slash_index << " already placed." << std::endl;
                        ret = -1;
                        l --;
                        break;
                    }
                    mark_slash[slash_index] = true;
                }
                else if (puzzle[new_x][new_y] & AREA_SHAPE_INDEX_BIT) {
                    int target_index = puzzle[new_x][new_y] >> AREA_SHAPE_INDEX_BIT_SHIFT;
                    if (target_index != shapes[k].shape_index) {
                        // std::cout << "Index: " << index << " Error placing shape " << k << " at node (" << (shapes[k].search_order[l].dx - shapes[k].search_order[m].dx) << ", " << (shapes[k].search_order[l].dy - shapes[k].search_order[m].dy) << ")" << " requested index: " << target_index << " placing index: " << shapes[k].shape_index << std::endl;
                        ret = -1;
                        l --;
                        break;
                    }
                }
                else if (puzzle[new_x][new_y] & AREA_SHAPE_SIZE_BIT) {
                    int target_size = puzzle[new_x][new_y] >> AREA_SHAPE_SIZE_BIT_SHIFT;
                    if (target_size != shapes[k].search_order.size()) {
                        // std::cout << "Index: " << index << " Error placing shape " << k << " at node (" << (shapes[k].search_order[l].dx - shapes[k].search_order[m].dx) << ", " << (shapes[k].search_order[l].dy - shapes[k].search_order[m].dy) << ")" << " requested size: " << target_size << " placing size: " << shapes[k].search_order.size() << std::endl;
                        ret = -1;
                        l --;
                        break;
                    }
                }
                // std::cout << "add " << shapes[k].search_order[l].dx << " " << shapes[k].search_order[l].dy << std::endl;
                // std::cout << "add " << new_x << " " << new_y << std::endl;
                solve_puzzle[new_x][new_y] = index | (shapes[k].shape_index << SOLVE_AREA_SHAPE_INDEX_BIT_SHIFT);
                if (!check_edge(new_x, new_y, puzzle_n_row, puzzle_n_col, solve_puzzle) || !check_edge_shape(new_x, new_y, puzzle_n_row, puzzle_n_col, solve_puzzle)) {
                    // std::cout << "Index: " << index << " Error placing shape " << k << " at node (" << (shapes[k].search_order[l].dx - shapes[k].search_order[m].dx) << ", " << (shapes[k].search_order[l].dy - shapes[k].search_order[m].dy) << ")" << " edge error or edge shape error." << std::endl;
                    ret = -1;
                    break;
                }
                if (puzzle_adjacent_shapes_different && !check_nearby_shape(new_x, new_y, puzzle_n_row, puzzle_n_col, solve_puzzle)) {
                    // std::cout << "Index: " << index << " Error placing shape " << k << " at node (" << (shapes[k].search_order[l].dx - shapes[k].search_order[m].dx) << ", " << (shapes[k].search_order[l].dy - shapes[k].search_order[m].dy) << ")" << " nearby shape error." << std::endl;
                    ret = -1;
                    break;
                }
                if (puzzle_adjacent_sizes_different && !check_nearby_size(new_x, new_y, puzzle_n_row, puzzle_n_col, solve_puzzle)) {
                    // std::cout << "Index: " << index << " Error placing shape " << k << " at node (" << (shapes[k].search_order[l].dx - shapes[k].search_order[m].dx) << ", " << (shapes[k].search_order[l].dy - shapes[k].search_order[m].dy) << ")" << " nearby size error." << std::endl;
                    ret = -1;
                    break;
                }
            }
            // std::cout << "After placing shape " << k << ":" << ret << std::endl;
            if (ret == -1) {
                for (int n = 0; n <= l; ++n) {
                    int new_x = ((x + shapes[k].search_order[n].dx - shapes[k].search_order[m].dx) << 1) + 1;
                    int new_y = ((y + shapes[k].search_order[n].dy - shapes[k].search_order[m].dy) << 1) + 1;
                    // std::cout << "remove " << shapes[k].search_order[m].dx << " " << shapes[k].search_order[m].dy << std::endl;
                    // std::cout << "remove " << new_x << " " << new_y << std::endl;
                    solve_puzzle[new_x][new_y] = AREA_NORMAL;
                }
            }
            else {
                // std::cout << "Placed shape " << shapes[k].shape_index << " successfully." << std::endl;

                // print_DFS_puzzle(solve_puzzle);
                if (puzzle_one_symbol_per_region) {
                    if (symbol_loc.dx == -233 && symbol_loc.dy == -666) {
                        ret = -1;
                    }
                }
                for (int i = 1; i <= slash_size; i ++){
                    if (!mark_slash[i]) {
                        ret = -1;
                        break;
                    }
                }
                for (int i = 0; i < palisade_visited_cnt; ++i) {
                    int new_x = ((x + palisade_visited[i].dx) << 1) + 1;
                    int new_y = ((y + palisade_visited[i].dy) << 1) + 1;
                    if (!check_palisade(new_x, new_y, puzzle_n_row, puzzle_n_col, solve_puzzle)) {
                        ret = -1;
                        break;
                    }
                }
                if (slash_size != 0 && !empty_area_slash_check(puzzle_n_row, puzzle_n_col, solve_puzzle)) {
                    ret = -1;
                }
                if (!empty_area_size_check(puzzle_shape_size_lower_bound, puzzle_shape_size_upper_bound, puzzle_n_row, puzzle_n_col, solve_puzzle)) {
                    // std::cout << "Empty area size check failed after placing shape " << k << "." << std::endl;
                    // std::cout << "Index: " << index << " Empty area size check failed after placing shape " << k << "." << std::endl;
                    ret = -1;
                }
                if (puzzle_no_4_way_intersections) {
                    for (int n = 0; n < shapes[k].search_order.size(); ++n) {
                        int new_x = ((x + shapes[k].search_order[n].dx - shapes[k].search_order[m].dx) << 1) + 1;
                        int new_y = ((y + shapes[k].search_order[n].dy - shapes[k].search_order[m].dy) << 1) + 1;
                        if (!check_tatami(new_x, new_y, puzzle_n_row, puzzle_n_col, solve_puzzle)) {
                            ret = -1;
                            break;
                        }
                    }
                }

                if (puzzle_no_3_way_intersections) {
                    for (int n = 0; n < shapes[k].search_order.size(); ++n) {
                        int new_x = ((x + shapes[k].search_order[n].dx - shapes[k].search_order[m].dx) << 1) + 1;
                        int new_y = ((y + shapes[k].search_order[n].dy - shapes[k].search_order[m].dy) << 1) + 1;
                        if (!check_loopy(new_x, new_y, puzzle_n_row, puzzle_n_col, solve_puzzle)) {
                            ret = -1;
                            break;
                        }
                    }
                }

                for (int n = 0; n < shapes[k].search_order.size(); ++n) {
                    int new_x = ((x + shapes[k].search_order[n].dx - shapes[k].search_order[m].dx) << 1) + 1;
                    int new_y = ((y + shapes[k].search_order[n].dy - shapes[k].search_order[m].dy) << 1) + 1;
                    if (!check_radar(new_x, new_y, puzzle_n_row, puzzle_n_col, solve_puzzle)) {
                        ret = -1;
                        break;
                    }
                }

                if (ret != -1) {
                    bool flag = false;
                    if (puzzle_all_shapes_same && all_shapes_same_index == -1) {
                        all_shapes_same_index = shapes[k].shape_index;
                        flag = true;
                    }
                    current_shape_index_pool.insert(shapes[k].shape_index);
                    ret = DFS_special_start(index + 1, solve_puzzle);
                    if (flag) {
                        all_shapes_same_index = -1;
                    }
                }
                if (ret == -1) {
                    current_shape_index_pool.erase(shapes[k].shape_index);
                    for (int n = 0; n < shapes[k].search_order.size(); ++n) {
                        int new_x = ((x + shapes[k].search_order[n].dx - shapes[k].search_order[m].dx) << 1) + 1;
                        int new_y = ((y + shapes[k].search_order[n].dy - shapes[k].search_order[m].dy) << 1) + 1;
                        // std::cout << "remove " << (shapes[k].search_order[n].dx - shapes[k].search_order[m].dx) << " " << (shapes[k].search_order[n].dy - shapes[k].search_order[m].dy) << " " << new_x << " " << new_y << std::endl;
                        solve_puzzle[new_x][new_y] = AREA_NORMAL;
                    }
                }
                else {
                    return ret;
                }
            }   
        }
    }

    if (special_start_type == 2) {
        return -1;
    }

    if (puzzle_predefine_shapes_only || puzzle_only_rectangles) {
        return -1;
    }

    if (puzzle_all_shapes_same && all_shapes_same_index != -1) {
        return -1;
    }

    if (special_start_type == 1) {
        for (int size = target_shape_size; size <= target_shape_size; ++size) {
            int ret = place_non_predifined_shape(index, x, y, size, false, -1, solve_puzzle);
            if (ret != -1) {
                return ret;
            }
        }
    }
    if (special_start_type == 3 || special_start_type == 4 || special_start_type == 5) {

        int lower_bound = puzzle_shape_size_lower_bound;
        int upper_bound = puzzle_shape_size_upper_bound;

        int shape_count = -1;

        // std::cout << "Index: " << index << " Empty area shape count: " << shape_count << ", size lower bound: " << lower_bound << ", size upper bound: " << upper_bound << std::endl;

        if (slash_check_enable) {
            shape_count = empty_area_shape_count(to_puzzle_x(x), to_puzzle_y(y), puzzle_n_row, puzzle_n_col, solve_puzzle);
        }

        memset(visited, 0, sizeof(visited));
        visited_nodes.clear();
        int size = DFS_count_empty(to_puzzle_x(x), to_puzzle_y(y), puzzle_n_row, puzzle_n_col, solve_puzzle);

        upper_bound = std::min(upper_bound, size);
        if (shape_count == 1) {
            if (size < lower_bound || size > upper_bound) {
                // std::cout << "Index: " << index << " Empty area shape count is 1 with size " << size << " out of bound, fail to place shape." << std::endl;
                return -1;
            }
            lower_bound = size;
        }

        // std::cout << "Index: " << index << " Empty area shape count: " << shape_count << ", size lower bound: " << lower_bound << ", size upper bound: " << upper_bound << std::endl;

        for (int size = lower_bound; size <= upper_bound; ++size) {
            int ret = place_non_predifined_shape(index, x, y, size, false, -1, solve_puzzle);
            if (ret != -1) {
                return ret;
            }
        }
    }
    return -1;
}

int main() {
    std::string line;

    while (true) {
        std::cin >> line;
        // std::cout << "cooking " << line << std::endl;
        if (line.find("VERSION") != std::string::npos) {
            std::cin >> line;
            continue;
        }
        if (line.find("PUZZLE_VERSION") != std::string::npos) {
            std::cin >> line;
            continue;
        }
        if (line.find("DIFFICULTY") != std::string::npos) {
            std::cin >> line;
            continue;
        }
        if (line.find("SHAPE_BANK") != std::string::npos) {
            std::getline(std::cin, line);
            puzzle_predefine_shapes_only = true;
            puzzle_shape_size_lower_bound = shapes[0].search_order.size();
            puzzle_shape_size_upper_bound = shapes[0].search_order.size();
            for (int i = 0; i < shapes.size(); ++i) {
                if (shapes[i].search_order.size() < puzzle_shape_size_lower_bound) {
                    puzzle_shape_size_lower_bound = shapes[i].search_order.size();
                }
                if (shapes[i].search_order.size() > puzzle_shape_size_upper_bound) {
                    puzzle_shape_size_upper_bound = shapes[i].search_order.size();
                }
            }
            continue;
        }
        if (line.find("ADJACENT_SHAPES_DIFFERENT") != std::string::npos) {
            puzzle_adjacent_shapes_different = true;
            continue;
        }
        if (line.find("ADJACENT_SIZES_DIFFERENT") != std::string::npos) {
            puzzle_adjacent_sizes_different = true;
            continue;
        }
        if (line.find("ALL_SHAPES_DIFFERENT") != std::string::npos) {
            puzzle_all_shapes_different = true;
            continue;
        }
        if (line.find("ONLY_RECTANGLES") != std::string::npos) {
            puzzle_only_rectangles = true;
            continue;
        }
        if (line.find("NO_RECTANGLES") != std::string::npos) {
            puzzle_no_rectangles = true;
            continue;
        }
        if (line.find("ONE_SYMBOL_PER_REGION") != std::string::npos) {
            puzzle_one_symbol_per_region = true;
            continue;
        }
        if (line.find("ALL_SHAPES_SAME") != std::string::npos) {
            puzzle_all_shapes_same = true;
            continue;
        }
        if (line.find("NO_4_WAY_INTERSECTIONS") != std::string::npos) {
            puzzle_no_4_way_intersections = true;
            continue;
        }
        if (line.find("NO_3_WAY_INTERSECTIONS") != std::string::npos) {
            puzzle_no_3_way_intersections = true;
            continue;
        }
        if (line.find("AREA_EQUALS") != std::string::npos) {
            std::cin >> puzzle_shape_size_lower_bound;
            puzzle_shape_size_upper_bound = puzzle_shape_size_lower_bound;
            // std::cout << "puzzle_shape_size_lower_bound = puzzle_shape_size_upper_bound = " << puzzle_shape_size_lower_bound << std::endl;
            continue;
        }
        if (line.find("AREA_AT_LEAST") != std::string::npos) {
            std::cin >> puzzle_shape_size_lower_bound;
            // std::cout << "puzzle_shape_size_lower_bound = " << puzzle_shape_size_lower_bound << std::endl;
            continue;
        }
        if (line.find("AREA_AT_MOST") != std::string::npos) {
            std::cin >> puzzle_shape_size_upper_bound;
            // std::cout << "puzzle_shape_size_upper_bound = " << puzzle_shape_size_upper_bound << std::endl;
            continue;
        }
        if (line.find("SHAPE") != std::string::npos) {
            // std::cout << "Reading shape..." << std::endl;
            std::cin >> line;
            int n_shape_row;
            std::cin >> n_shape_row;
            uint32_t** temp_shape = new uint32_t*[100];
            for (int i = 0; i < 100; ++i) {
                temp_shape[i] = new uint32_t[100];
            }
            // std::cout << "n_shape_row = " << n_shape_row << std::endl;
            std::getline(std::cin, line);
            for (int i = 0; i < n_shape_row; ++i) {
                std::getline(std::cin, line);
                // std::cout << line << std::endl;
                for (int j = 0; j < line.size(); ++j) {
                    temp_shape[i][j] = (line[j] == '#') ? 1 : 0;
                    // std::cout << "line[" << j << "] = " << line[j] << std::endl;
                }
            }
            uint32_t shape_size = std::max(line.size(), static_cast<size_t>(n_shape_row));
            for (int i = n_shape_row; i < line.size(); ++i) {
                for (int j = 0; j < line.size(); ++j) {
                    temp_shape[i][j] = 0;
                }
            }
            int shape_index = shape_in_shapes(shape_size, temp_shape);
            if (shape_index != -1) {
                // std::cout << "Shape already exists, modify puzzle." << std::endl;
                shape_index_modify_map[next_shape_index] = shape_index;
                // modify_shape_index_in_puzzle(next_shape_index, shape_index);
            }

            for (int k = 0; k < 4; ++k) {
                // for (int i = 0; i < shape_size; ++i) {
                //     for (int j = 0; j < shape_size; ++j) {
                //         std::cout << temp_shape[i][j] << " ";
                //     }
                //     std::cout << std::endl;
                // }
                add_shape_to_shapes(next_shape_index, shape_size, temp_shape);
                rotate_shape(shape_size, temp_shape);
            }
            mirror_shape(shape_size, temp_shape);
            for (int k = 0; k < 4; ++k) {
                // for (int i = 0; i < shape_size; ++i) {
                //     for (int j = 0; j < shape_size; ++j) {
                //         std::cout << temp_shape[i][j] << " ";
                //     }
                //     std::cout << std::endl;
                // }
                add_shape_to_shapes(next_shape_index, shape_size, temp_shape);
                rotate_shape(shape_size, temp_shape);
            }
            // std::getline(std::cin, line);
            // std::cout << "Added shape " << next_shape_index << " with size " << shape_size << std::endl;
            shape_index_size_map[next_shape_index] = shapes[shapes.size() - 1].search_order.size();
            next_shape_index ++;
            // std::cout << "Total shapes: " << shapes.size() << std::endl;
            continue;
        }
        if (line.find("DIMENSIONS") != std::string::npos) {
            std::cin >> puzzle_n_col >> puzzle_n_row;
            // std::cout << "puzzle_n_col = " << puzzle_n_col << ", puzzle_n_row = " << puzzle_n_row << std::endl;
            continue;
        }
        if (line.find("PUZZLE") != std::string::npos) {
            // std::cout << "Reading puzzle..." << std::endl;
            std::getline(std::cin, line);   
            read_puzzle();
            break;
        }
    }
    
    uint32_t** solve_puzzle = new uint32_t*[1000];
    for (int i = 0; i < 1000; ++i) {
        solve_puzzle[i] = new uint32_t[1000];
        for (int j = 0; j < 1000; ++j) {
            solve_puzzle[i][j] = AREA_NORMAL;
        }
    }

    build_solve_puzzle(solve_puzzle);

    int empty_area_cnt = 0;
    for (int x = 1; x <= puzzle_n_row; ++x) {
        for (int y = 1; y <= puzzle_n_col; ++y) {
            if (puzzle[to_puzzle_x(x)][to_puzzle_y(y)] != AREA_BLOCK) {
                empty_area_cnt += 1;
            }
            if (puzzle[to_puzzle_x(x)][to_puzzle_y(y)] & AREA_SLASH_INDEX_BIT) {
                int slash_index = (puzzle[to_puzzle_x(x)][to_puzzle_y(y)] & AREA_SLASH_INDEX_BIT) >> AREA_SLASH_INDEX_BIT_SHIFT;
                slash_check_enable = true;
                slash_size = std::max(slash_size, slash_index);
            }
        }
    }

    for (int x = 1; x <= slash_size; ++x) {
        std::vector<node> temp;
        slash_nodes[x] = temp;
    }

    for (int x = 1; x <= puzzle_n_row; ++x) {
        for (int y = 1; y <= puzzle_n_col; ++y) {
            if (puzzle[to_puzzle_x(x)][to_puzzle_y(y)] & AREA_SLASH_INDEX_BIT) {
                int slash_index = (puzzle[to_puzzle_x(x)][to_puzzle_y(y)] & AREA_SLASH_INDEX_BIT) >> AREA_SLASH_INDEX_BIT_SHIFT;
                slash_nodes[slash_index].push_back(node(x, y));
            }
        }
    }

    if (puzzle_shape_size_lower_bound == -1) {
        puzzle_shape_size_lower_bound = 1;
    }
    if (puzzle_shape_size_upper_bound == -1) {
        puzzle_shape_size_upper_bound = empty_area_cnt;
    }
    if (slash_check_enable) {
        puzzle_shape_size_lower_bound = std::max(puzzle_shape_size_lower_bound, slash_size);
    }

    if (puzzle_only_rectangles) {
        
        int up, down, left, right;
        for (int k = 0; k < shapes.size(); ++k) {
            up = 0, down = 0, left = 0, right = 0;
            for (int l = 0; l < shapes[k].search_order.size(); ++l) {
                up = std::min(up, shapes[k].search_order[l].dx);
                down = std::max(down, shapes[k].search_order[l].dx);
                left = std::min(left, shapes[k].search_order[l].dy);
                right = std::max(right, shapes[k].search_order[l].dy);
            }
            int rectangle_width = right - left + 1;
            int rectangle_height = down - up + 1;
            if (rectangle_width * rectangle_height > shapes[k].search_order.size()) {
                shapes[k].shape_index = 0;
            }
        }

        uint32_t** temp_shape = new uint32_t*[100];
        for (int i = 0; i < 100; ++i) {
            temp_shape[i] = new uint32_t[100];
        }
        for (int size = puzzle_shape_size_lower_bound; size <= puzzle_shape_size_upper_bound; ++size) {
            for (int l = 1; l <= (size / l); ++l) {
                if (size % l != 0) {
                    continue;
                }
                int h = size / l;
                if (l > puzzle_n_row || h > puzzle_n_col) {
                    continue;
                }
                int shape_size = std::max(h, l);
                for (int i = 0; i < shape_size; ++i) {
                    for (int j = 0; j < shape_size; ++j) {
                        if (i < l && j < h) {
                            temp_shape[i][j] = 1;
                        }
                        else {
                            temp_shape[i][j] = 0;
                        }
                    }
                }
                // for (int i = 0; i < shape_size; ++i) {
                //     for (int j = 0; j < shape_size; ++j) {
                //         std::cout << temp_shape[i][j] << " ";
                //     }
                //     std::cout << std::endl;
                // }
                add_shape_to_shapes(next_shape_index, shape_size, temp_shape);
                rotate_shape(shape_size, temp_shape);
                // for (int i = 0; i < shape_size; ++i) {
                //     for (int j = 0; j < shape_size; ++j) {
                //         std::cout << temp_shape[i][j] << " ";
                //     }
                //     std::cout << std::endl;
                // }
                add_shape_to_shapes(next_shape_index, shape_size, temp_shape);
                shape_index_size_map[next_shape_index] = shapes[shapes.size() - 1].search_order.size();
                next_shape_index ++;
            }
        }
    }

    // std::cout << "puzzle_shape_size_lower_bound = " << puzzle_shape_size_lower_bound << std::endl;
    // std::cout << "puzzle_shape_size_upper_bound = " << puzzle_shape_size_upper_bound << std::endl;
    // std::cout << "slash_size = " << slash_size << std::endl;

    DFS_special_start(1, solve_puzzle);

    std::cout << "SOLUTION" << std::endl;
    print_puzzle(solve_puzzle);

    return 0;
}