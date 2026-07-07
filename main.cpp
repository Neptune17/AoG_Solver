#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <tuple>
#include <set>
#include <cstring>
#include <algorithm>

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

#define RET_CODE_BLOCK_AREA 0x00000001u
#define RET_CODE_FILLED_AREA 0x00000002u
#define RET_CODE_AREA_SHAPE_INDEX 0x00000004u
#define RET_CODE_AREA_SHAPE_SIZE 0x00000008u
#define RET_CODE_SLASH 0x00000010u
#define RET_CODE_ALL_SHAPE_SAME 0x00000020u
#define RET_CODE_ALL_SHAPE_DIFFERENT 0x00000040u
#define RET_CODE_ONE_SYMBOL_PER_REGION 0x00000080u
#define RET_CODE_COMPASS 0x00000100u
#define RET_CODE_SHAPE_SIZE_RANGE 0x00000200u
#define RET_CODE_PALISADE 0x00000400u
#define RET_CODE_TATAMI 0x00000800u
#define RET_CODE_LOOPY 0x00001000u
#define RET_CODE_RADAR 0x00002000u
#define RET_CODE_EMPTY_CHECK 0x00004000u
#define RET_CODE_ADJACENT_SHAPE_DIFFERENT 0x00008000u
#define RET_CODE_ADJACENT_SIZE_DIFFERENT 0x00010000u
#define RET_CODE_IN_SHAPE_EDGE 0x00020000u
#define RET_CODE_EDGE_CONSTRAINT 0x00040000u
#define RET_CODE_SHAPE_CONTAIN_BLOCK_AREA 0x00080000u

#define SPECIAL_START_DEFAULT 0x00000000u
#define SPECIAL_START_SIZE_1_REGION 0x00000001u
#define SPECIAL_START_SIZE_MATCH_REGION 0x00000002u
#define SPECIAL_START_LINE_SAME 0x00000003u
#define SPECIAL_START_LINE_SMALLER_OR_LARGER 0x00000004u
#define SPECIAL_START_AREA_INDEX 0x00000005u
#define SPECIAL_START_AREA_SIZE 0x00000006u
#define SPECIAL_START_CORNER 0x00000007u
#define SPECIAL_START_COMPASS 0x00000008u
#define SPECIAL_START_LINE_CONSTRAINT 0x00000009u

#define MAX_PUZZLE_SIZE 1000
#define MAX_SHAPE_SIZE 100

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

struct Node {
    int x, y;

    Node() : Node(0, 0) {}

    Node(int x, int y) : x(x), y(y) {}

    bool operator<(const Node& other) const {
        if (x != other.x)
            return x < other.x;
        return y < other.y;
    }
    
    bool operator==(const Node& other) const {
        return x == other.x && y == other.y;
    }

    bool operator!=(const Node& other) const {
        return x != other.x || y != other.y;
    }
};

struct Shape {
    uint32_t shape_index;
    std::vector<Node> nodes;

    uint32_t digest;
    std::vector<uint32_t> preview;

    Shape() {}
    
    Shape(uint32_t** shape, uint32_t shape_size) {
        this->shape_index = 0;
        this->nodes = std::vector<Node>(0);
        this->digest = 0;
        this->preview = std::vector<uint32_t>(0);

        int start_x = -1, start_y = -1;
        uint32_t line_preview;
        int most_left_j = shape_size;
        int most_up_i = shape_size;
        for (int i = 0; i < shape_size; ++i) {
            line_preview = 0;
            for (int j = 0; j < shape_size; ++j) {
                line_preview <<= 1;
                if (shape[i][j] == 1) {
                    most_left_j = std::min(most_left_j, j);
                    most_up_i = std::min(most_up_i, i);
                    if (start_x == -1) {
                        start_x = i;
                        start_y = j;
                    }
                    this->nodes.push_back({i - start_x, j - start_y});
                    line_preview += 1;
                }
            }
            this->preview.push_back(line_preview);
        }

        for (int i = 0; i < this->preview.size(); ++i) {
            this->preview[i] <<= most_left_j;
        }
        
        for (int i = most_up_i; i < this->preview.size(); ++i) {
            this->preview[i - most_up_i] = this->preview[i];
        }

        for (int i = 0; i < most_up_i; ++i) {
            this->preview[i + this->preview.size() - most_up_i] = 0;
        }

        for (int i = 0; i < this->preview.size(); ++i) {
            this->digest = (this->digest * 131) + this->preview[i];
        }
    }

    bool operator==(const Shape& other) const {
        if (digest != other.digest) {
            return false;
        }
        if (preview.size() != other.preview.size()) {
            return false;
        }
        for (int i = 0; i < preview.size(); ++i) {
            if (preview[i] != other.preview[i]) {
                return false;
            }
        }
        return true;
    }
};

std::vector<Shape> shapes;
std::map<uint32_t, int> shape_index_to_shape_size_map;
uint32_t next_shape_index = 1;

std::map<Node, std::vector<int>> node_to_shape_index;

std::map<int, std::vector<Node>> slash_nodes;

std::vector<Node> shape_size_nodes;

// uint32_t shape_digest(std::vector<uint32_t> shape_preview) {
//     uint32_t digest = 0;
//     for (int i = 0; i < shape_preview.size(); ++i) {
//         digest = (digest * 131) + shape_preview[i];
//     }
//     return digest;
// }

void _shape_rotate(uint32_t** shape, uint32_t shape_size);
void _shape_mirror(uint32_t** shape, uint32_t shape_size);
bool _add_shape_to_shapes(uint32_t shape_index, uint32_t** shape, uint32_t shape_size);
uint32_t shapes_search(uint32_t** shape, uint32_t shape_size);
uint32_t shapes_insert(uint32_t** shape, uint32_t shape_size);

uint32_t _temp_shape[MAX_SHAPE_SIZE][MAX_SHAPE_SIZE];
void _shape_rotate(uint32_t** shape, uint32_t shape_size) {
    for (int i = 0; i < shape_size; ++i) {
        for (int j = 0; j < shape_size; ++j) {
            _temp_shape[j][shape_size - 1 - i] = shape[i][j];
        }
    }
    for (int i = 0; i < shape_size; ++i) {
        for (int j = 0; j < shape_size; ++j) {
            shape[i][j] = _temp_shape[i][j];
        }
    }
}

void _shape_mirror(uint32_t** shape, uint32_t shape_size) {
    for (int i = 0; i < shape_size; ++i) {
        for (int j = 0; j < shape_size; ++j) {
            _temp_shape[i][shape_size - 1 - j] = shape[i][j];
        }
    }
    for (int i = 0; i < shape_size; ++i) {
        for (int j = 0; j < shape_size; ++j) {
            shape[i][j] = _temp_shape[i][j];
        }
    }
}

bool _add_shape_to_shapes(uint32_t shape_index, uint32_t** shape, uint32_t shape_size) {

    if (shapes_search(shape, shape_size) == 0xffffffffu) {
        Shape new_shape(shape, shape_size);
        new_shape.shape_index = shape_index;
        shapes.push_back(new_shape);

        for (int i = 0; i < new_shape.nodes.size(); ++i) {
            node_to_shape_index[new_shape.nodes[i]].push_back(shapes.size() - 1);
        }

        return true;
    }

    return false;
}

uint32_t shapes_search(uint32_t** shape, uint32_t shape_size) {
    Shape new_shape(shape, shape_size);
    for (auto shape : shapes) {
        if (shape == new_shape) {
            return shape.shape_index;
        }
    }
    return 0xffffffffu; // No duplicate shape found
}

uint32_t shapes_insert(uint32_t** shape, uint32_t shape_size) {
    uint32_t insert_success_count = 0;
    
    for (int i = 0; i < 4; ++i) {
        insert_success_count += _add_shape_to_shapes(next_shape_index, shape, shape_size);
        _shape_rotate(shape, shape_size);
    }
    _shape_mirror(shape, shape_size);
    for (int i = 0; i < 4; ++i) {
        insert_success_count += _add_shape_to_shapes(next_shape_index, shape, shape_size);
        _shape_rotate(shape, shape_size);
    }

    if (insert_success_count != 0) {
        shape_index_to_shape_size_map[next_shape_index] = shapes[shapes.size() - 1].nodes.size();
        next_shape_index ++;
    }
    
    return insert_success_count;
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
            int my_size = shape_index_to_shape_size_map[(index & SOLVE_AREA_SHAPE_INDEX_BIT) >> SOLVE_AREA_SHAPE_INDEX_BIT_SHIFT];
            int nearby_size = shape_index_to_shape_size_map[(solve_puzzle[x - 2][y] & SOLVE_AREA_SHAPE_INDEX_BIT) >> SOLVE_AREA_SHAPE_INDEX_BIT_SHIFT];
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
            int my_size = shape_index_to_shape_size_map[(index & SOLVE_AREA_SHAPE_INDEX_BIT) >> SOLVE_AREA_SHAPE_INDEX_BIT_SHIFT];
            int nearby_size = shape_index_to_shape_size_map[(solve_puzzle[x + 2][y] & SOLVE_AREA_SHAPE_INDEX_BIT) >> SOLVE_AREA_SHAPE_INDEX_BIT_SHIFT];
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
            int my_size = shape_index_to_shape_size_map[(index & SOLVE_AREA_SHAPE_INDEX_BIT) >> SOLVE_AREA_SHAPE_INDEX_BIT_SHIFT];
            int nearby_size = shape_index_to_shape_size_map[(solve_puzzle[x][y - 2] & SOLVE_AREA_SHAPE_INDEX_BIT) >> SOLVE_AREA_SHAPE_INDEX_BIT_SHIFT];
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
            int my_size = shape_index_to_shape_size_map[(index & SOLVE_AREA_SHAPE_INDEX_BIT) >> SOLVE_AREA_SHAPE_INDEX_BIT_SHIFT];
            int nearby_size = shape_index_to_shape_size_map[(solve_puzzle[x][y + 2] & SOLVE_AREA_SHAPE_INDEX_BIT) >> SOLVE_AREA_SHAPE_INDEX_BIT_SHIFT];
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
            int left_size = shape_index_to_shape_size_map[(solve_puzzle[x - 2][y] & SOLVE_AREA_SHAPE_INDEX_BIT) >> SOLVE_AREA_SHAPE_INDEX_BIT_SHIFT];
            int right_size = shape_index_to_shape_size_map[(solve_puzzle[x][y] & SOLVE_AREA_SHAPE_INDEX_BIT) >> SOLVE_AREA_SHAPE_INDEX_BIT_SHIFT];
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
        // std::cout << "Checking edge shapes for cell (" << x << ", " << y << ") with index " << solve_puzzle[x][y] << std::endl;
        // std::cout << solve_puzzle[x + 2][y] << std::endl;
        // std::cout << puzzle[x + 1][y] << std::endl;
        // std::cout << index << std::endl;
        if (solve_puzzle[x + 2][y] != AREA_NORMAL && puzzle[x + 1][y] & LINE_EQUAL && ((solve_puzzle[x + 2][y] & SOLVE_AREA_SHAPE_INDEX_BIT) != (index & SOLVE_AREA_SHAPE_INDEX_BIT))) {
            // std::cout << "Edge check failed at (" << x << ", " << y << "): LINE_EQUAL with different index below." << std::endl;
            return false;
        }
        if (solve_puzzle[x + 2][y] != AREA_NORMAL && puzzle[x + 1][y] & LINE_DIFFERENT && ((solve_puzzle[x + 2][y] & SOLVE_AREA_SHAPE_INDEX_BIT) == (index & SOLVE_AREA_SHAPE_INDEX_BIT))) {
            // std::cout << "Edge check failed at (" << x << ", " << y << "): LINE_DIFFERENT with same index below." << std::endl;
            return false;
        }
        if (solve_puzzle[x + 2][y] != AREA_NORMAL && (puzzle[x + 1][y] & LINE_LARGER || puzzle[x + 1][y] & LINE_SMALLER || (puzzle[x + 1][y] & LINE_SIZE_DIFF_BIT) != 0)) {
            int left_size = shape_index_to_shape_size_map[(solve_puzzle[x][y] & SOLVE_AREA_SHAPE_INDEX_BIT) >> SOLVE_AREA_SHAPE_INDEX_BIT_SHIFT];
            int right_size = shape_index_to_shape_size_map[(solve_puzzle[x + 2][y] & SOLVE_AREA_SHAPE_INDEX_BIT) >> SOLVE_AREA_SHAPE_INDEX_BIT_SHIFT];
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
            int up_size = shape_index_to_shape_size_map[(solve_puzzle[x][y - 2] & SOLVE_AREA_SHAPE_INDEX_BIT) >> SOLVE_AREA_SHAPE_INDEX_BIT_SHIFT];
            int down_size = shape_index_to_shape_size_map[(solve_puzzle[x][y] & SOLVE_AREA_SHAPE_INDEX_BIT) >> SOLVE_AREA_SHAPE_INDEX_BIT_SHIFT];
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
            int up_size = shape_index_to_shape_size_map[(solve_puzzle[x][y] & SOLVE_AREA_SHAPE_INDEX_BIT) >> SOLVE_AREA_SHAPE_INDEX_BIT_SHIFT];
            int down_size = shape_index_to_shape_size_map[(solve_puzzle[x][y + 2] & SOLVE_AREA_SHAPE_INDEX_BIT) >> SOLVE_AREA_SHAPE_INDEX_BIT_SHIFT];
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

bool check_palisade_type2(uint32_t x, uint32_t y, uint32_t n_row, uint32_t n_col, uint32_t** solve_puzzle) {
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

bool check_palisade_type1(uint32_t x, uint32_t y, uint32_t n_row, uint32_t n_col, uint32_t** solve_puzzle) {
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
        int radar_value = (puzzle[x - 1][y - 1] & VERTEX_RADAR_BIT) >> VERTEX_RADAR_BIT_SHIFT;
        int filled_count = 0;
        std::set<uint32_t> unique_region;
        unique_region.insert(index);
        filled_count += 1;
        if (puzzle[x - 2][y] != AREA_BLOCK) {
            if (solve_puzzle[x - 2][y] != AREA_NORMAL) {
                unique_region.insert(solve_puzzle[x - 2][y]);
                filled_count++;
            }
        }
        else {
            filled_count += 1;
        }
        if (puzzle[x][y - 2] != AREA_BLOCK) {
            if (solve_puzzle[x][y - 2] != AREA_NORMAL) {
                unique_region.insert(solve_puzzle[x][y - 2]);
                filled_count++;
            }
        }
        else {
            filled_count += 1;
        }
        if (puzzle[x - 2][y - 2] != AREA_BLOCK) {
            if (solve_puzzle[x - 2][y - 2] != AREA_NORMAL) {
                unique_region.insert(solve_puzzle[x - 2][y - 2]);
                filled_count++;
            }
        }
        else {
            filled_count += 1;
        }
        if (filled_count == 4) {
            if (unique_region.size() != radar_value) {
                return false;
            }
        }
        else {
            if (unique_region.size() + (4 - filled_count) < radar_value) {
                return false;
            }
        }
    }
    if (puzzle[x - 1][y + 1] & VERTEX_RADAR_BIT) {
        int radar_value = (puzzle[x - 1][y + 1] & VERTEX_RADAR_BIT) >> VERTEX_RADAR_BIT_SHIFT;
        int filled_count = 0;
        std::set<uint32_t> unique_region;
        unique_region.insert(index);
        filled_count += 1;
        if (puzzle[x - 2][y] != AREA_BLOCK) {
            if (solve_puzzle[x - 2][y] != AREA_NORMAL) {
                unique_region.insert(solve_puzzle[x - 2][y]);
                filled_count++;
            }
        }
        else {
            filled_count += 1;
        }
        if (puzzle[x][y + 2] != AREA_BLOCK) {
            if (solve_puzzle[x][y + 2] != AREA_NORMAL) {
                unique_region.insert(solve_puzzle[x][y + 2]);
                filled_count++;
            }
        }
        else {
            filled_count += 1;
        }
        if (puzzle[x - 2][y + 2] != AREA_BLOCK) {
            if (solve_puzzle[x - 2][y + 2] != AREA_NORMAL) {
                unique_region.insert(solve_puzzle[x - 2][y + 2]);
                filled_count++;
            }
        }
        else {
            filled_count += 1;
        }
        if (filled_count == 4) {
            if (unique_region.size() != radar_value) {
                return false;
            }
        }
        else {
            if (unique_region.size() + (4 - filled_count) < radar_value) {
                return false;
            }
        }
    }
    if (puzzle[x + 1][y - 1] & VERTEX_RADAR_BIT) {
        int radar_value = (puzzle[x + 1][y - 1] & VERTEX_RADAR_BIT) >> VERTEX_RADAR_BIT_SHIFT;
        int filled_count = 0;
        std::set<uint32_t> unique_region;
        unique_region.insert(index);
        filled_count += 1;
        if (puzzle[x + 2][y] != AREA_BLOCK) {
            if (solve_puzzle[x + 2][y] != AREA_NORMAL) {
                unique_region.insert(solve_puzzle[x + 2][y]);
                filled_count++;
            }
        }
        else {
            filled_count += 1;
        }
        if (puzzle[x][y - 2] != AREA_BLOCK) {
            if (solve_puzzle[x][y - 2] != AREA_NORMAL) {
                unique_region.insert(solve_puzzle[x][y - 2]);
                filled_count++;
            }
        }
        else {
            filled_count += 1;
        }
        if (puzzle[x + 2][y - 2] != AREA_BLOCK) {
            if (solve_puzzle[x + 2][y - 2] != AREA_NORMAL) {
                unique_region.insert(solve_puzzle[x + 2][y - 2]);
                filled_count++;
            }
        }
        else {
            filled_count += 1;
        }
        if (filled_count == 4) {
            if (unique_region.size() != radar_value) {
                return false;
            }
        }
        else {
            if (unique_region.size() + (4 - filled_count) < radar_value) {
                return false;
            }
        }
    }
    if (puzzle[x + 1][y + 1] & VERTEX_RADAR_BIT) {
        int radar_value = (puzzle[x + 1][y + 1] & VERTEX_RADAR_BIT) >> VERTEX_RADAR_BIT_SHIFT;
        int filled_count = 0;
        std::set<uint32_t> unique_region;
        unique_region.insert(index);
        filled_count += 1;
        if (puzzle[x + 2][y] != AREA_BLOCK) {
            if (solve_puzzle[x + 2][y] != AREA_NORMAL) {
                unique_region.insert(solve_puzzle[x + 2][y]);
                filled_count++;
            }
        }
        else {
            filled_count += 1;
        }
        if (puzzle[x][y + 2] != AREA_BLOCK) {
            if (solve_puzzle[x][y + 2] != AREA_NORMAL) {
                unique_region.insert(solve_puzzle[x][y + 2]);
                filled_count++;
            }
        }
        else {
            filled_count += 1;
        }
        if (puzzle[x + 2][y + 2] != AREA_BLOCK) {
            if (solve_puzzle[x + 2][y + 2] != AREA_NORMAL) {
                unique_region.insert(solve_puzzle[x + 2][y + 2]);
                filled_count++;
            }
        }
        else {
            filled_count += 1;
        }
        if (filled_count == 4) {
            if (unique_region.size() != radar_value) {
                return false;
            }
        }
        else {
            if (unique_region.size() + (4 - filled_count) < radar_value) {
                return false;
            }
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

bool area_contain_symbol(int x, int y) {
    bool contain_symbol = false;
    int puzzle_x = to_puzzle_x(x);
    int puzzle_y = to_puzzle_y(y);
    if (puzzle[puzzle_x][puzzle_y] & AREA_PALISADE_INDEX_BIT) {
        contain_symbol = true;
    }
    if (puzzle[puzzle_x][puzzle_y] & AREA_SLASH_INDEX_BIT) {
        contain_symbol = true;
    }
    if (puzzle[puzzle_x][puzzle_y] & AREA_SHAPE_INDEX_BIT) {
        contain_symbol = true;
    }
    if (puzzle[puzzle_x][puzzle_y] & AREA_SHAPE_SIZE_BIT) {
        contain_symbol = true;
    }
    if (puzzle[puzzle_x][puzzle_y] & AREA_COMPASS_ENABLE) {
        contain_symbol = true;
    }
    return contain_symbol;
}

struct CompassStates {
    int up, down, left, right;

    CompassStates() : CompassStates(0, 0, 0, 0) {
        
    }

    CompassStates(int up, int down, int left, int right) {
        this->up = up;
        this->down = down;
        this->left = left;
        this->right = right;
    }
};

int dfs_visited[MAX_PUZZLE_SIZE][MAX_PUZZLE_SIZE];
int dfs_visited_index = 0;
int dfs_empty_count = 0;
std::set<std::pair<Node, Node>> dfs_empty_block_line_node_pair;
int dfs_empty_block_line_count = 0;
int dfs_symbol_count = 0;
int dfs_slash_count[10];
std::vector<Node> dfs_compass_nodes;
std::vector<CompassStates> dfs_compass_node_states;
std::vector<int> dfs_area_shape_sizes;

void dfs_empty(int x, int y, uint32_t** solve_puzzle) {
    // std::cout << "dfs_empty" << " " << x << " " << y << std::endl;

    int puzzle_x = to_puzzle_x(x);
    int puzzle_y = to_puzzle_y(y);

    // std::cout << "dfs_empty" << " " << puzzle_x << " " << puzzle_y << std::endl;
    
    if (puzzle[puzzle_x][puzzle_y] == AREA_BLOCK) {
        // std::cout << "puzzle[puzzle_x][puzzle_y] == AREA_BLOCK" << std::endl;
        return ;
    }
    if (solve_puzzle[puzzle_x][puzzle_y] != AREA_NORMAL) {
        // std::cout << "solve_puzzle[puzzle_x][puzzle_y] != AREA_NORMAL" << std::endl;
        return ;
    }
    if (dfs_visited[puzzle_x][puzzle_y] == dfs_visited_index) {
        // std::cout << "dfs_visited[puzzle_x][puzzle_y] == dfs_visited_index" << std::endl;
        return ;
    }

    // std::cout << "dfs_empty" << " " << x << " " << y << std::endl;

    dfs_visited[puzzle_x][puzzle_y] = dfs_visited_index;
    
    dfs_empty_count += 1;

    // if there is a block line between connected empty areas.
    bool block_line_flag = false;
    if ((puzzle[puzzle_x - 1][puzzle_y] & LINE_BLOCK) && (dfs_visited[puzzle_x - 2][puzzle_y] == dfs_visited_index)) {
        // dfs_empty_block_line_node.insert(Node(x - 1, y));
        // dfs_empty_block_line_node.insert(Node(x, y));
        dfs_empty_block_line_node_pair.insert(std::make_pair(Node(x - 1, y), Node(x, y)));
        block_line_flag = true;
    }
    if ((puzzle[puzzle_x + 1][puzzle_y] & LINE_BLOCK) && (dfs_visited[puzzle_x + 2][puzzle_y] == dfs_visited_index)) {
        // dfs_empty_block_line_node.insert(Node(x + 1, y));
        // dfs_empty_block_line_node.insert(Node(x, y));
        dfs_empty_block_line_node_pair.insert(std::make_pair(Node(x + 1, y), Node(x, y)));
        block_line_flag = true;
    }
    if ((puzzle[puzzle_x][puzzle_y - 1] & LINE_BLOCK) && (dfs_visited[puzzle_x][puzzle_y - 2] == dfs_visited_index)) {
        // dfs_empty_block_line_node.insert(Node(x, y - 1));
        // dfs_empty_block_line_node.insert(Node(x, y));
        dfs_empty_block_line_node_pair.insert(std::make_pair(Node(x, y), Node(x, y - 1)));
        block_line_flag = true;
    }
    if ((puzzle[puzzle_x][puzzle_y + 1] & LINE_BLOCK) && (dfs_visited[puzzle_x][puzzle_y + 2] == dfs_visited_index)) {
        // dfs_empty_block_line_node.insert(Node(x, y + 1));
        // dfs_empty_block_line_node.insert(Node(x, y));
        dfs_empty_block_line_node_pair.insert(std::make_pair(Node(x, y), Node(x, y + 1)));
        block_line_flag = true;
    }
    if (block_line_flag) {
        // std::cout << "block_line_flag" << std::endl;
        dfs_empty_block_line_count += 1;
        // dfs_empty_block_line_node.insert(node(x, y));
    }

    if (area_contain_symbol(x, y)) {
        dfs_symbol_count += 1;
    }

    if (puzzle[puzzle_x][puzzle_y] & AREA_SLASH_INDEX_BIT) {
        dfs_slash_count[puzzle[puzzle_x][puzzle_y] >> AREA_SLASH_INDEX_BIT_SHIFT] += 1;
    }

    if (puzzle[puzzle_x][puzzle_y] & AREA_SHAPE_SIZE_BIT) {
        dfs_area_shape_sizes.push_back((puzzle[puzzle_x][puzzle_y] & AREA_SHAPE_SIZE_BIT) >> AREA_SHAPE_SIZE_BIT_SHIFT);
    }

    if (puzzle[puzzle_x][puzzle_y] & AREA_COMPASS_ENABLE) {
        dfs_compass_nodes.push_back(Node(x, y));
        dfs_compass_node_states.push_back(CompassStates(0, 0, 0, 0));
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

    return ;
}

void dfs_empty_compass(int x, int y, uint32_t** solve_puzzle) {
    // std::cout << "dfs_empty_compass" << " " << x << " " << y << std::endl;

    int puzzle_x = to_puzzle_x(x);
    int puzzle_y = to_puzzle_y(y);

    // std::cout << "dfs_empty_compass" << " " << puzzle_x << " " << puzzle_y << std::endl;
    
    if (puzzle[puzzle_x][puzzle_y] == AREA_BLOCK) {
        // std::cout << "puzzle[puzzle_x][puzzle_y] == AREA_BLOCK" << std::endl;
        return ;
    }
    if (solve_puzzle[puzzle_x][puzzle_y] != AREA_NORMAL) {
        // std::cout << "solve_puzzle[puzzle_x][puzzle_y] != AREA_NORMAL" << std::endl;
        return ;
    }
    if (dfs_visited[puzzle_x][puzzle_y] == dfs_visited_index) {
        // std::cout << "dfs_visited[puzzle_x][puzzle_y] == dfs_visited_index" << std::endl;
        return ;
    }

    // std::cout << "dfs_empty_compass" << " " << x << " " << y << std::endl;

    dfs_visited[puzzle_x][puzzle_y] = dfs_visited_index;

    for (int i = 0; i < dfs_compass_nodes.size(); ++i) {
        if (x < dfs_compass_nodes[i].x) {
            dfs_compass_node_states[i].up ++;
        }
        if (x > dfs_compass_nodes[i].x) {
            dfs_compass_node_states[i].down ++;
        }
        if (y < dfs_compass_nodes[i].y) {
            dfs_compass_node_states[i].left ++;
        }
        if (y > dfs_compass_nodes[i].y) {
            dfs_compass_node_states[i].right ++;
        }
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

    return ;
}

bool DFS_empty_compass_check(int x, int y, uint32_t** solve_puzzle) {
    // std::cout << "DFS_empty_compass_check" << " " << x << " " << y << std::endl;
    
    dfs_visited_index += 1;
    dfs_empty_compass(x, y, solve_puzzle);

    for (int i = 0; i < dfs_compass_nodes.size(); ++i) {
        int puzzle_x = to_puzzle_x(dfs_compass_nodes[i].x);
        int puzzle_y = to_puzzle_y(dfs_compass_nodes[i].y);
        if ((puzzle_compass_up[puzzle_x][puzzle_y] != -1) && (puzzle_compass_up[puzzle_x][puzzle_y] > dfs_compass_node_states[i].up)) {
            // std::cout << "1 fail " << puzzle_compass_up[puzzle_x][puzzle_y] << " " << dfs_compass_node_states[i].up << std::endl;
            return false;
        }
        if ((puzzle_compass_down[puzzle_x][puzzle_y] != -1) && (puzzle_compass_down[puzzle_x][puzzle_y] > dfs_compass_node_states[i].down)) {
            // std::cout << "2 fail " << puzzle_compass_down[puzzle_x][puzzle_y] << " " << dfs_compass_node_states[i].down << std::endl;
            return false;
        }
        if ((puzzle_compass_left[puzzle_x][puzzle_y] != -1) && (puzzle_compass_left[puzzle_x][puzzle_y] > dfs_compass_node_states[i].left)) {
            // std::cout << "3 fail " << puzzle_compass_left[puzzle_x][puzzle_y] << " " << dfs_compass_node_states[i].left << std::endl;
            return false;
        }
        if ((puzzle_compass_right[puzzle_x][puzzle_y] != -1) && (puzzle_compass_right[puzzle_x][puzzle_y] > dfs_compass_node_states[i].right)) {
            // std::cout << "4 fail " << puzzle_compass_right[puzzle_x][puzzle_y] << " " << dfs_compass_node_states[i].right << std::endl;
            return false;
        }
    }

    return true;
}

// int fa[MAX_PUZZLE_SIZE];
// int find(int x) {
//     if (fa[x] == x) {
//         return x;
//     }
//     else {
//         fa[x] = find(fa[x]);
//         return fa[x];
//     }
// }

std::map<Node, int> place_visited;
int try_place_id(int x, int y, bool value, int visited_value) {
    if (place_visited.find(Node(x, y)) == place_visited.end()) {
        place_visited[Node(x, y)] = 0;
    }

    if (place_visited[Node(x, y)] == visited_value) {
        return 0;
    }
    
    place_visited[Node(x, y)] = visited_value;
    int count = value;

    for (auto pair : dfs_empty_block_line_node_pair) {
        if (pair.first == Node(x, y)) {
            count += try_place_id(pair.second.x, pair.second.y, !value, visited_value);
        }
        if (pair.second == Node(x, y)) {
            count += try_place_id(pair.first.x, pair.first.y, !value, visited_value);
        }
    }

    return count;
}

void DFS_empty(int x, int y, uint32_t** solve_puzzle) {
    dfs_empty_count = 0;
    dfs_empty_block_line_count = 0;
    dfs_empty_block_line_node_pair.clear();
    dfs_symbol_count = 0;
    memset(dfs_slash_count, 0, sizeof(dfs_slash_count));
    dfs_compass_nodes.clear();
    dfs_compass_node_states.clear();
    dfs_area_shape_sizes.clear();

    dfs_visited_index += 1;
    dfs_empty(x, y, solve_puzzle);

    place_visited.clear();
    dfs_empty_block_line_count = 0;
    for (auto pair : dfs_empty_block_line_node_pair) {
        if (place_visited.find(pair.first) == place_visited.end()) {
            dfs_empty_block_line_count += std::min(try_place_id(pair.first.x, pair.first.y, 0, 1), try_place_id(pair.first.x, pair.first.y, 1, 2));
            // count += try_place_id(pair.second.x, pair.second.y, !value);
        }
        if (place_visited.find(pair.second) == place_visited.end()) {
            dfs_empty_block_line_count += std::min(try_place_id(pair.second.x, pair.second.y, 0, 1), try_place_id(pair.second.x, pair.second.y, 1, 2));
            // count += try_place_id(pair.first.x, pair.first.y, !value);
        }
    }



    // std::vector<node> vecByX(dfs_empty_block_line_node.begin(), dfs_empty_block_line_node.end());
    // std::sort(vecByX.begin(), vecByX.end(), [](const node& a, const node& b) {
    //     if (a.x != b.x) return a.x < b.x;
    //     return a.y < b.y;
    // });
    // std::vector<node> vecByY(dfs_empty_block_line_node.begin(), dfs_empty_block_line_node.end());
    // std::sort(vecByY.begin(), vecByY.end(), [](const node& a, const node& b) {
    //     if (a.y != b.y) return a.y < b.y;
    //     return a.x < b.x;
    // });
    
    // std::map<node, int> map_id;
    // for (int i = 0; i < vecByX.size(); ++i) {
    //     map_id[vecByX[i]] = i + 1;
    // }
    // for (int i = 0; i < vecByX.size(); ++i) {
    //     // std::cout << vecByX[i].x << " " << vecByX[i].y << " " << map_id[vecByX[i]] << std::endl;
    //     fa[i + 1] = i + 1;
    // }
    // for (int i = 1; i < vecByX.size(); ++i) {
    //     if (vecByX[i - 1].x == vecByX[i].x && (std::abs(vecByX[i - 1].y - vecByX[i].y) <= 1)) {
    //         int index_a = map_id[vecByX[i - 1]];
    //         int index_b = map_id[vecByX[i]];
    //         if (find(index_a) != find(index_b)) {
    //             fa[find(index_a)] = find(index_b);
    //         }
    //     }
    // }
    // for (int i = 1; i < vecByY.size(); ++i) {
    //     if (vecByY[i - 1].y == vecByY[i].y && (std::abs(vecByY[i - 1].x - vecByY[i].x) <= 1)) {
    //         int index_a = map_id[vecByY[i - 1]];
    //         int index_b = map_id[vecByY[i]];
    //         if (find(index_a) != find(index_b)) {
    //             fa[find(index_a)] = find(index_b);
    //         }
    //     }
    // }
    // std::map<int, int> result_count;
    // for (int i = 1; i <= vecByX.size(); ++i) {
    //     // std::cout << map_id[vecByX[i - 1]] << " " << fa[i] << std::endl;
    //     if (result_count.find(find(i)) == result_count.end()) {
    //         result_count[find(i)] = 0;
    //     }
    //     result_count[find(i)] += 1;
    // }
    // dfs_empty_block_line_count = 0;
    // for (auto entry : result_count) {
    //     dfs_empty_block_line_count += (entry.second <= 3) ? 1 : 2;
    // }
    // std::cout << "new dfs_empty_block_line_count" << " " << dfs_empty_block_line_count << std::endl;

    // std::cout << "DFS empty" << std::endl;
    // std::cout << "dfs_empty_count" << " " << dfs_empty_count << std::endl;
    // std::cout << "dfs_empty_block_line_count" << " " << dfs_empty_block_line_count << std::endl;
    // std::cout << "dfs_symbol_count" << " " << dfs_symbol_count << std::endl;
    // std::cout << "dfs_slash_count" << " ";
    // for (int i = 1; i <= slash_check_slash_cnt; ++i) {
    //     std::cout << dfs_slash_count[i] << " ";
    // }
    // std::cout << std::endl;
    // std::cout << "dfs_area_shape_sizes" << " ";
    // for (auto shape_size : dfs_area_shape_sizes) {
    //     std::cout << shape_size << " ";
    // }
    // std::cout << std::endl;
}

int dfs_group_mark_index = 0;

void DFS_group_mark() {
    dfs_group_mark_index = dfs_visited_index;
}

bool DFS_in_group_mark(int x, int y, uint32_t** solve_puzzle) {
    int puzzle_x = to_puzzle_x(x);
    int puzzle_y = to_puzzle_y(y);
    return dfs_visited[puzzle_x][puzzle_y] > dfs_group_mark_index;
}

bool empty_area_check(uint32_t** solve_puzzle) {
    // std::cout << "empty_area_check" << std::endl;
    DFS_group_mark();
    for (int x = 1; x <= puzzle_n_row; ++x) {
        for (int y = 1; y <= puzzle_n_col; ++y) {
            if ((puzzle[to_puzzle_x(x)][to_puzzle_y(y)] != AREA_BLOCK) && (solve_puzzle[to_puzzle_x(x)][to_puzzle_y(y)] == AREA_NORMAL) && !DFS_in_group_mark(x, y, solve_puzzle)) {
                DFS_empty(x, y, solve_puzzle);

                int max_area_size = dfs_empty_count - dfs_empty_block_line_count;
                if (max_area_size < puzzle_shape_size_lower_bound) {
                    // std::cout << "max_area_size < puzzle_shape_size_lower_bound" << std::endl;
                    return false;
                }

                std::set<int> unique_val_set;
                int area_shape_sizes_required_size = 0;
                for (int val : dfs_area_shape_sizes) {
                    if (unique_val_set.insert(val).second) {
                        area_shape_sizes_required_size += val;
                    }
                }
                if (area_shape_sizes_required_size > dfs_empty_count) {
                    // std::cout << "area_shape_sizes_required_size > dfs_empty_count" << std::endl;
                    return false;
                }

                if (puzzle_one_symbol_per_region) {
                    if (dfs_symbol_count == 0) {
                        // std::cout << "dfs_symbol_count == 0" << std::endl;
                        return false;
                    }
                    if ((dfs_symbol_count == 1) && (dfs_empty_block_line_count != 0)) {
                        return false;
                    }
                }

                if (slash_check_enable) {
                    for (int i = 1; i <= slash_check_slash_cnt; ++i) {
                        if (dfs_slash_count[i] != dfs_slash_count[1]) {
                            // std::cout << "dfs_slash_count[i] != dfs_slash_count[1]" << std::endl;
                            return false;
                        }
                    }
                    if (dfs_slash_count[1] == 0) {
                        // std::cout << "dfs_slash_count[1] == 0" << std::endl;
                        return false;
                    }
                    if ((dfs_slash_count[1] == 1) && (dfs_empty_block_line_count != 0)) {
                        return false;
                    }
                }

                if (puzzle_shape_size_lower_bound == puzzle_shape_size_upper_bound) {
                    if (dfs_empty_count % puzzle_shape_size_lower_bound != 0) {
                        return false;
                    }
                    if ((dfs_empty_count == puzzle_shape_size_lower_bound) && (dfs_empty_block_line_count != 0)) {
                        return false;
                    }
                }

                if (puzzle_all_shapes_same && all_shapes_same_check_shape_index != -1) {
                    int shape_size = shape_index_to_shape_size_map[all_shapes_same_check_shape_index];
                    if (dfs_empty_count % shape_size != 0) {
                        return false;
                    }
                }

                if (dfs_compass_nodes.size() != 0) {
                    if (!DFS_empty_compass_check(x, y, solve_puzzle)) {
                        return false;
                    }
                }
            }
            if ((puzzle[to_puzzle_x(x)][to_puzzle_y(y)] != AREA_BLOCK) && (solve_puzzle[to_puzzle_x(x)][to_puzzle_y(y)] == AREA_NORMAL) && (puzzle[to_puzzle_x(x)][to_puzzle_y(y)] & AREA_PALISADE_INDEX_BIT)) {
                int palisade_type = (puzzle[to_puzzle_x(x)][to_puzzle_y(y)] & AREA_PALISADE_INDEX_BIT) >> AREA_PALISADE_INDEX_BIT_SHIFT;

                bool up = false, down = false, left = false, right = false;
                if ((puzzle[to_puzzle_x(x) - 2][to_puzzle_y(y)] != AREA_BLOCK) && (solve_puzzle[to_puzzle_x(x) - 2][to_puzzle_y(y)] == AREA_NORMAL)) {
                    left = true;
                }
                if ((puzzle[to_puzzle_x(x) + 2][to_puzzle_y(y)] != AREA_BLOCK) && (solve_puzzle[to_puzzle_x(x) + 2][to_puzzle_y(y)] == AREA_NORMAL)) {
                    right = true;
                }
                if ((puzzle[to_puzzle_x(x)][to_puzzle_y(y) - 2] != AREA_BLOCK) && (solve_puzzle[to_puzzle_x(x)][to_puzzle_y(y) - 2] == AREA_NORMAL)) {
                    up = true;
                }
                if ((puzzle[to_puzzle_x(x)][to_puzzle_y(y) + 2] != AREA_BLOCK) && (solve_puzzle[to_puzzle_x(x)][to_puzzle_y(y) + 2] == AREA_NORMAL)) {
                    down = true;
                }

                if (palisade_type == 1) {
                    if (!up || !down || !left || !right) {
                        // std::cout << "palisade fail" << " " << palisade_type << " " << x << " " << y << std::endl;
                        return false;
                    }
                }
                else if (palisade_type == 2) {
                    int sum = up + down + left + right;
                    if (sum < 3) {
                        // std::cout << up << down << left << right << std::endl;
                        // std::cout << "palisade fail" << " " << palisade_type << " " << x << " " << y << std::endl;
                        return false;
                    }
                }
                else if (palisade_type == 3) {
                    if ((!up && !right) || (!right && !down) || (!down && !left) || (!left && !up)) {
                        // std::cout << "palisade fail" << " " << palisade_type << " " << x << " " << y << std::endl;
                        return false;
                    }
                }
                else if (palisade_type == 4) {
                    if (!up && !down && !left && !right) {
                        // std::cout << "palisade fail" << " " << palisade_type << " " << x << " " << y << std::endl;
                        return false;
                    }
                }
                else if (palisade_type == 5) {
                    if ((!up && !down) || (!right && !left)) {
                        // std::cout << "palisade fail" << " " << palisade_type << " " << x << " " << y << std::endl;
                        return false;
                    }
                }
                else if (palisade_type == 6) {

                }
            }
        }
    }
    return true;
}

// bool empty_area_size_check(uint32_t min_shape_size, uint32_t max_shape_size, uint32_t n_row, uint32_t n_col, uint32_t** solve_puzzle) {
//     // std::cout << "Empty area size check..." << std::endl;
    
//     memset(visited, 0, sizeof(visited));

//     for (int i = 1; i <= n_row; ++i) {
//         for (int j = 1; j <= n_col; ++j) {
//             if (puzzle[(i << 1) + 1][(j << 1) + 1] != AREA_BLOCK && solve_puzzle[(i << 1) + 1][(j << 1) + 1] == AREA_NORMAL && !visited[(i << 1) + 1][(j << 1) + 1]) {
//                 area_shape_sizes.clear();
//                 contain_symbol = false;
//                 visited_nodes.clear();
//                 int empty_area_size = DFS_count_empty((i << 1) + 1, (j << 1) + 1, n_row, n_col, solve_puzzle);
//                 if (empty_area_size < min_shape_size) {
//                     // std::cout << "Empty area size check failed: found an empty area of size " << empty_area_size << " which is smaller than the minimum shape size " << min_shape_size << "." << std::endl;
//                     return false;
//                 }
//                 int area_shape_sizes_required_size = 0;
//                 for (auto shape_size : area_shape_sizes) {
//                     area_shape_sizes_required_size += shape_size;
//                 }
//                 if (empty_area_size < area_shape_sizes_required_size) {
//                     // std::cout << "Empty area size check failed: found an empty area of size " << empty_area_size << " which is smaller than the total required size " << area_shape_sizes_required_size << " of the shapes that must be placed in this area." << std::endl;
//                     return false;
//                 }
//                 if (puzzle_one_symbol_per_region && !contain_symbol) {
//                     return false;
//                 }
//             }
//         }
//     }

//     // std::cout << "Empty area size check passed." << std::endl;
//     return true;
// }

// int slash_count[10];

// void DFS_slash_count_empty(int x, int y, uint32_t n_row, uint32_t n_col, uint32_t** solve_puzzle) {

//     // print_DFS_puzzle(solve_puzzle);

//     // std::cout << "DFS_slash_count_empty: visiting cell (" << x << ", " << y << ")" << std::endl;
//     // std::cout << "DFS_slash_count_empty: visiting cell (" << puzzle[x][y] << ")" << std::endl;

//     if (puzzle[x][y] == AREA_BLOCK) {
//         return ;
//     }
//     if (solve_puzzle[x][y] != AREA_NORMAL) {
//         return ;
//     }
//     if (visited[x][y]) {
//         return ;
//     }
//     if (puzzle[x][y] & AREA_SLASH_INDEX_BIT) {
//         int slash_index = puzzle[x][y] >> AREA_SLASH_INDEX_BIT_SHIFT;
//         slash_count[slash_index] += 1;
//     }

//     visited[x][y] = true;

//     if ((puzzle[x - 1][y] & LINE_BLOCK) == 0) {
//         DFS_slash_count_empty(x - 2, y, n_row, n_col, solve_puzzle);
//     }
//     if ((puzzle[x + 1][y] & LINE_BLOCK) == 0) {
//         DFS_slash_count_empty(x + 2, y, n_row, n_col, solve_puzzle);
//     }
//     if ((puzzle[x][y - 1] & LINE_BLOCK) == 0) {
//         DFS_slash_count_empty(x, y - 2, n_row, n_col, solve_puzzle);
//     }
//     if ((puzzle[x][y + 1] & LINE_BLOCK) == 0) {
//         DFS_slash_count_empty(x, y + 2, n_row, n_col, solve_puzzle);
//     }
//     return ;
// }

// bool empty_area_slash_check(uint32_t n_row, uint32_t n_col, uint32_t** solve_puzzle) {
//     // std::cout << "Empty area slash check..." << std::endl;
    
//     memset(visited, 0, sizeof(visited));

//     for (int i = 1; i <= n_row; ++i) {
//         for (int j = 1; j <= n_col; ++j) {
//             if (puzzle[(i << 1) + 1][(j << 1) + 1] != AREA_BLOCK && solve_puzzle[(i << 1) + 1][(j << 1) + 1] == AREA_NORMAL && !visited[(i << 1) + 1][(j << 1) + 1]) {
//                 memset(slash_count, 0, sizeof(slash_count));
//                 DFS_slash_count_empty((i << 1) + 1, (j << 1) + 1, n_row, n_col, solve_puzzle);

//                 int unique_slash_size_value = slash_count[1];
//                 for (int k = 1; k <= slash_check_slash_cnt; k ++) {
//                     if (unique_slash_size_value != slash_count[k]) {
//                         // std::cout << "Empty area slash check failed: found an empty area with non-unique slashes." << std::endl;
//                         return false;
//                     }
//                 }
//                 if (unique_slash_size_value == 0) {
//                     // std::cout << "Empty area slash check failed: found an empty area with no slashes." << std::endl;
//                     return false;
//                 }
//             }
//         }
//     }

//     // std::cout << "Empty area slash check passed." << std::endl;
//     return true;
// }

// bool delta_empty_area_slash_check(uint32_t n_row, uint32_t n_col, uint32_t** solve_puzzle) {
//     // std::cout << "Empty area slash check..." << std::endl;
    
//     memset(visited, 0, sizeof(visited));

//     for (int i = 1; i <= n_row; ++i) {
//         for (int j = 1; j <= n_col; ++j) {
//             if (puzzle[(i << 1) + 1][(j << 1) + 1] != AREA_BLOCK && solve_puzzle[(i << 1) + 1][(j << 1) + 1] == AREA_NORMAL && !visited[(i << 1) + 1][(j << 1) + 1]) {
//                 memset(slash_count, 0, sizeof(slash_count));
//                 DFS_slash_count_empty((i << 1) + 1, (j << 1) + 1, n_row, n_col, solve_puzzle);

//                 int unique_slash_size_value = slash_count[1];
//                 for (int k = 1; k <= slash_check_slash_cnt; k ++) {
//                     if (std::abs(unique_slash_size_value - slash_count[k]) > 1) {
//                         // std::cout << "Empty area slash check failed: found an empty area with non-unique slashes." << std::endl;
//                         return false;
//                     }
//                 }
//                 // if (unique_slash_size_value == 0) {
//                 //     // std::cout << "Empty area slash check failed: found an empty area with no slashes." << std::endl;
//                 //     return false;
//                 // }
//             }
//         }
//     }

//     // std::cout << "Empty area slash check passed." << std::endl;
//     return true;
// }

int _empty_area_shape_count(int x, int y, uint32_t** solve_puzzle) {

    if (dfs_empty_block_line_count != 0) {
        return 0;
    }

    if (dfs_empty_count <= puzzle_shape_size_lower_bound) {
        return 1;
    }

    if ((dfs_area_shape_sizes.size() == 1) && (dfs_area_shape_sizes[0] == dfs_empty_count)) {
        return 1;
    }

    if (puzzle_one_symbol_per_region && dfs_symbol_count == 1) {
        return 1;
    }

    if (slash_check_enable) {
        return dfs_slash_count[1];
    }

    return 0;
}

std::tuple<int, int, int> empty_area_size_range(int x, int y, uint32_t** solve_puzzle) {
    
    DFS_empty(x, y, solve_puzzle);
    
    int shape_size_lower_bound = puzzle_shape_size_lower_bound;
    int shape_size_upper_bound = puzzle_shape_size_upper_bound;

    int max_area_size = dfs_empty_count - dfs_empty_block_line_count;
    if (max_area_size < puzzle_shape_size_lower_bound) {
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

std::tuple<int, int, int> find_empty_compass_area(uint32_t** solve_puzzle) {
    // std::cout << "find_empty_compass_area" << std::endl;
    for (int i = 1; i <= puzzle_n_row; ++i) {
        for (int j = 1; j <= puzzle_n_col; ++j) {
            if (((puzzle[to_puzzle_x(i)][to_puzzle_y(j)] & AREA_COMPASS_ENABLE) != 0) && (solve_puzzle[to_puzzle_x(i)][to_puzzle_y(j)] == AREA_NORMAL)) {
                return std::make_tuple(0, i, j);
            }
        }
    }
    return std::make_tuple(-1, -1, -1);
}

std::tuple<int, int, int> find_empty_alone_area(uint32_t** solve_puzzle) {
    // std::cout << "find_empty_alone_area" << std::endl;
    for (int i = 1; i <= puzzle_n_row; ++i) {
        for (int j = 1; j <= puzzle_n_col; ++j) {
            if ((solve_puzzle[to_puzzle_x(i)][to_puzzle_y(j)] == AREA_NORMAL)) {
                int block_status = 0;
                if ((puzzle[to_puzzle_x(i - 1)][to_puzzle_y(j)] & AREA_BLOCK) || (solve_puzzle[to_puzzle_x(i - 1)][to_puzzle_y(j)] != AREA_NORMAL) || (puzzle[to_puzzle_x(i) - 1][to_puzzle_y(j)] & LINE_BLOCK)) {
                    block_status |= (1 << 3);
                }
                if ((puzzle[to_puzzle_x(i + 1)][to_puzzle_y(j)] & AREA_BLOCK) || (solve_puzzle[to_puzzle_x(i + 1)][to_puzzle_y(j)] != AREA_NORMAL) || (puzzle[to_puzzle_x(i) + 1][to_puzzle_y(j)] & LINE_BLOCK)) {
                    block_status |= (1 << 2);
                }
                if ((puzzle[to_puzzle_x(i)][to_puzzle_y(j - 1)] & AREA_BLOCK) || (solve_puzzle[to_puzzle_x(i)][to_puzzle_y(j - 1)] != AREA_NORMAL) || (puzzle[to_puzzle_x(i)][to_puzzle_y(j) - 1] & LINE_BLOCK)) {
                    block_status |= (1 << 1);
                }
                if ((puzzle[to_puzzle_x(i)][to_puzzle_y(j + 1)] & AREA_BLOCK) || (solve_puzzle[to_puzzle_x(i)][to_puzzle_y(j + 1)] != AREA_NORMAL) || (puzzle[to_puzzle_x(i)][to_puzzle_y(j) + 1] & LINE_BLOCK)) {
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
    // std::cout << "find_size_limit_small_area" << std::endl;
    DFS_group_mark();
    for (int x = 1; x <= puzzle_n_row; ++x) {
        for (int y = 1; y <= puzzle_n_col; ++y) {
            if ((puzzle[to_puzzle_x(x)][to_puzzle_y(y)] != AREA_BLOCK) && (solve_puzzle[to_puzzle_x(x)][to_puzzle_y(y)] == AREA_NORMAL) && !DFS_in_group_mark(x, y, solve_puzzle)) {
                DFS_empty(x, y, solve_puzzle);

                if (puzzle_shape_size_lower_bound == puzzle_shape_size_upper_bound && (dfs_empty_count == puzzle_shape_size_lower_bound)) {
                    return std::make_tuple(0, x, y);
                }
            }
        }
    }
    return std::make_tuple(-1, -1, -1);
}

std::tuple<int, int, int> find_empty_shape_index_area(uint32_t** solve_puzzle) {
    // std::cout << "find_empty_shape_index_area" << std::endl;
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
    // std::cout << "find_empty_shape_size_area" << std::endl;
    for (auto _node : shape_size_nodes) {
        if (((puzzle[to_puzzle_x(_node.x)][to_puzzle_y(_node.y)] & AREA_SHAPE_SIZE_BIT) != 0) && (solve_puzzle[to_puzzle_x(_node.x)][to_puzzle_y(_node.y)] == AREA_NORMAL)) {
            return std::make_tuple(0, _node.x, _node.y);
        }
    }
    // for (int i = 1; i <= puzzle_n_row; ++i) {
    //     for (int j = 1; j <= puzzle_n_col; ++j) {
    //         if (((puzzle[to_puzzle_x(i)][to_puzzle_y(j)] & AREA_SHAPE_SIZE_BIT) != 0) && (solve_puzzle[to_puzzle_x(i)][to_puzzle_y(j)] == AREA_NORMAL)) {
    //             return std::make_tuple(0, i, j);
    //         }
    //     }
    // }
    return std::make_tuple(-1, -1, -1);
}

std::tuple<int, int, int> find_empty_corner_area(uint32_t** solve_puzzle) {
    // std::cout << "find_empty_corner_area" << std::endl;
    for (int i = 1; i <= puzzle_n_row; ++i) {
        for (int j = 1; j <= puzzle_n_col; ++j) {
            if (puzzle[to_puzzle_x(i)][to_puzzle_y(j)] != AREA_BLOCK && solve_puzzle[to_puzzle_x(i)][to_puzzle_y(j)] == AREA_NORMAL) {
                int block_line_count = ((puzzle[to_puzzle_x(i) - 1][to_puzzle_y(j)] & LINE_BLOCK) != 0 || puzzle[to_puzzle_x(i) - 2][to_puzzle_y(j)] == AREA_BLOCK) + ((puzzle[to_puzzle_x(i) + 1][to_puzzle_y(j)] & LINE_BLOCK) != 0 || puzzle[to_puzzle_x(i) + 2][to_puzzle_y(j)] == AREA_BLOCK) + ((puzzle[to_puzzle_x(i)][to_puzzle_y(j) - 1] & LINE_BLOCK) != 0 || puzzle[to_puzzle_x(i)][to_puzzle_y(j) - 2] == AREA_BLOCK) + ((puzzle[to_puzzle_x(i)][to_puzzle_y(j) + 1] & LINE_BLOCK) != 0 || puzzle[to_puzzle_x(i)][to_puzzle_y(j) + 2] == AREA_BLOCK);
                if (slash_check_enable && (puzzle[to_puzzle_x(i)][to_puzzle_y(j)] & AREA_SLASH_INDEX_BIT)) {
                    block_line_count += 
                        (((puzzle[to_puzzle_x(i) - 1][to_puzzle_y(j)] & LINE_BLOCK) == 0) && ((puzzle[to_puzzle_x(i) - 2][to_puzzle_y(j)] & AREA_SLASH_INDEX_BIT) == (puzzle[to_puzzle_x(i)][to_puzzle_y(j)] & AREA_SLASH_INDEX_BIT))) 
                        + (((puzzle[to_puzzle_x(i) + 1][to_puzzle_y(j)] & LINE_BLOCK) == 0) && ((puzzle[to_puzzle_x(i) + 2][to_puzzle_y(j)] & AREA_SLASH_INDEX_BIT) == (puzzle[to_puzzle_x(i)][to_puzzle_y(j)] & AREA_SLASH_INDEX_BIT))) 
                        + (((puzzle[to_puzzle_x(i)][to_puzzle_y(j) - 1] & LINE_BLOCK) == 0) && ((puzzle[to_puzzle_x(i)][to_puzzle_y(j) - 2] & AREA_SLASH_INDEX_BIT) == (puzzle[to_puzzle_x(i)][to_puzzle_y(j)] & AREA_SLASH_INDEX_BIT))) 
                        + (((puzzle[to_puzzle_x(i)][to_puzzle_y(j) + 1] & LINE_BLOCK) == 0) && ((puzzle[to_puzzle_x(i)][to_puzzle_y(j) + 2] & AREA_SLASH_INDEX_BIT) == (puzzle[to_puzzle_x(i)][to_puzzle_y(j)] & AREA_SLASH_INDEX_BIT)));
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
    // std::cout << "find_empty_line_equal_area" << std::endl;
    for (int i = 1; i <= puzzle_n_row; ++i) {
        for (int j = 1; j <= puzzle_n_col; ++j) {
            if (puzzle[to_puzzle_x(i)][to_puzzle_y(j)] != AREA_BLOCK && solve_puzzle[to_puzzle_x(i)][to_puzzle_y(j)] == AREA_NORMAL) {
                uint32_t line[4];
                if ((puzzle[to_puzzle_x(i) - 1][to_puzzle_y(j)] & LINE_EQUAL) && (solve_puzzle[to_puzzle_x(i) - 2][to_puzzle_y(j)] != AREA_NORMAL) && (puzzle[to_puzzle_x(i) - 2][to_puzzle_y(j)] != AREA_BLOCK)) {
                    return std::make_tuple(0, i, j);
                }
                if ((puzzle[to_puzzle_x(i) + 1][to_puzzle_y(j)] & LINE_EQUAL) && (solve_puzzle[to_puzzle_x(i) + 2][to_puzzle_y(j)] != AREA_NORMAL) && (puzzle[to_puzzle_x(i) + 2][to_puzzle_y(j)] != AREA_BLOCK)) {
                    return std::make_tuple(0, i, j);
                }
                if ((puzzle[to_puzzle_x(i)][to_puzzle_y(j) - 1] & LINE_EQUAL) && (solve_puzzle[to_puzzle_x(i)][to_puzzle_y(j) - 2] != AREA_NORMAL) && (puzzle[to_puzzle_x(i)][to_puzzle_y(j) - 2] != AREA_BLOCK)) {
                    return std::make_tuple(0, i, j);
                }
                if ((puzzle[to_puzzle_x(i)][to_puzzle_y(j) + 1] & LINE_EQUAL) && (solve_puzzle[to_puzzle_x(i)][to_puzzle_y(j) + 2] != AREA_NORMAL) && (puzzle[to_puzzle_x(i)][to_puzzle_y(j) + 2] != AREA_BLOCK)) {
                    return std::make_tuple(0, i, j);
                }
            }
        }
    }
    return std::make_tuple(-1, -1, -1);
}

std::tuple<int, int, int> find_empty_line_larger_or_smaller_area(uint32_t** solve_puzzle) {
    // std::cout << "find_empty_line_larger_or_smaller_area" << std::endl;
    for (int i = 1; i <= puzzle_n_row; ++i) {
        for (int j = 1; j <= puzzle_n_col; ++j) {
            if (puzzle[to_puzzle_x(i)][to_puzzle_y(j)] != AREA_BLOCK && solve_puzzle[to_puzzle_x(i)][to_puzzle_y(j)] == AREA_NORMAL) {
                uint32_t line[4];
                if (((puzzle[to_puzzle_x(i) - 1][to_puzzle_y(j)] & LINE_LARGER) || (puzzle[to_puzzle_x(i) - 1][to_puzzle_y(j)] & LINE_SMALLER)) && (solve_puzzle[to_puzzle_x(i) - 2][to_puzzle_y(j)] != AREA_NORMAL) && (puzzle[to_puzzle_x(i) - 2][to_puzzle_y(j)] != AREA_BLOCK)) {
                    return std::make_tuple(0, i, j);
                }
                if (((puzzle[to_puzzle_x(i) + 1][to_puzzle_y(j)] & LINE_LARGER) || (puzzle[to_puzzle_x(i) + 1][to_puzzle_y(j)] & LINE_SMALLER)) && (solve_puzzle[to_puzzle_x(i) + 2][to_puzzle_y(j)] != AREA_NORMAL) && (puzzle[to_puzzle_x(i) + 2][to_puzzle_y(j)] != AREA_BLOCK)) {
                    return std::make_tuple(0, i, j);
                }
                if (((puzzle[to_puzzle_x(i)][to_puzzle_y(j) - 1] & LINE_LARGER) || (puzzle[to_puzzle_x(i)][to_puzzle_y(j) - 1] & LINE_SMALLER)) && (solve_puzzle[to_puzzle_x(i)][to_puzzle_y(j) - 2] != AREA_NORMAL) && (puzzle[to_puzzle_x(i)][to_puzzle_y(j) - 2] != AREA_BLOCK)) {
                    return std::make_tuple(0, i, j);
                }
                if (((puzzle[to_puzzle_x(i)][to_puzzle_y(j) + 1] & LINE_LARGER) || (puzzle[to_puzzle_x(i)][to_puzzle_y(j) + 1] & LINE_SMALLER)) && (solve_puzzle[to_puzzle_x(i)][to_puzzle_y(j) + 2] != AREA_NORMAL) && (puzzle[to_puzzle_x(i)][to_puzzle_y(j) + 2] != AREA_BLOCK)) {
                    return std::make_tuple(0, i, j);
                }
            }
        }
    }
    return std::make_tuple(-1, -1, -1);
}

std::tuple<int, int, int> find_empty_line_constraint_area(uint32_t** solve_puzzle) {
    // std::cout << "find_empty_line_constraint_area" << std::endl;
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
    // std::cout << "find_empty_area" << std::endl;
    for (int i = 1; i <= puzzle_n_row; ++i) {
        for (int j = 1; j <= puzzle_n_col; ++j) {
            if (puzzle[to_puzzle_x(i)][to_puzzle_y(j)] != AREA_BLOCK && solve_puzzle[to_puzzle_x(i)][to_puzzle_y(j)] == AREA_NORMAL) {
                return std::make_tuple(0, i, j);
            }
        }
    }
    return std::make_tuple(-1, -1, -1);
}

std::tuple<uint32_t, int, int> find_special_start_area(uint32_t** solve_puzzle) {
    
    std::tuple<uint32_t, int, int> ret_data;

    uint32_t special_start_type = 0xffffffffu;
    int ret, x, y;

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

    x = std::get<1>(ret_data);
    y = std::get<2>(ret_data);

    return std::make_tuple(special_start_type, x, y);
}

int place_non_predifined_shape(int index, int x, int y, uint32_t size, bool up_left_seq, int known_shape_index, uint32_t** solve_puzzle) {

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

    Node symbol_loc = {-233, -666};

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
            temp_x = x + dfs_current_shape[dfs_current_shape_cnt - 1].x;
            temp_y = y + dfs_current_shape[dfs_current_shape_cnt - 1].y;
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

            if (puzzle_one_symbol_per_region && symbol_loc.x == temp_x && symbol_loc.y == temp_y) {
                symbol_loc.x = -233;
                symbol_loc.y = -666;
            }

            for (int i = 0; i < compass_visited_cnt; ++i) {
                if (dfs_current_shape[dfs_current_shape_cnt - 1].x < compass_visited[i].x) {
                    compass_visited_up_cnt[i] --;
                }
                if (dfs_current_shape[dfs_current_shape_cnt - 1].x > compass_visited[i].x) {
                    compass_visited_down_cnt[i] --;
                }
                if (dfs_current_shape[dfs_current_shape_cnt - 1].y < compass_visited[i].y) {
                    compass_visited_left_cnt[i] --;
                }
                if (dfs_current_shape[dfs_current_shape_cnt - 1].y > compass_visited[i].y) {
                    compass_visited_right_cnt[i] --;
                }
            }


            dfs_current_shape_cnt -= 1;
        }
        dfs_expand_candidates_cnt = candidates_size;

        if (current_size == size) {
            bool slash_check_fail_flag = false;
            for (int i = 1; i <= slash_check_slash_cnt; i ++){
                if (!mark_slash[i]) {
                    slash_check_fail_flag = true;
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
                if (symbol_loc.x == -233 && symbol_loc.y == -666) {
                    one_symbol_per_region_check_fail_flag = true;
                }
            }

            if (slash_check_fail_flag || no_rectangle_check_fail_flag || one_symbol_per_region_check_fail_flag) {
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
                start_x = std::min(start_x, dfs_current_shape[i].x);
                start_y = std::min(start_y, dfs_current_shape[i].y);
            }
            for (int i = 0; i < current_size; ++i) {
                int x = dfs_current_shape[i].x - start_x;
                int y = dfs_current_shape[i].y - start_y;
                temp_shape[x][y] = 1;
                shape_size = std::max(shape_size, std::max(x, y) + 1);
            }

            uint32_t shape_index = shapes_search(temp_shape, shape_size);
            if (shape_index != 0xffffffffu) {
                if (up_left_seq && (int)shape_index <= known_shape_index) {
                    continue;
                }
                if (puzzle_all_shapes_different && all_shapes_different_check_shape_index_pool.find(shape_index) != all_shapes_different_check_shape_index_pool.end()) {
                    continue;
                }
            }
            else {
                shapes_insert(temp_shape, shape_size);
                shape_index = shapes_search(temp_shape, shape_size);
            }

            for (int i = 0; i < current_size; ++i) {
                solve_puzzle[((x + dfs_current_shape[i].x) << 1) + 1][((y + dfs_current_shape[i].y) << 1) + 1] |= (shape_index << SOLVE_AREA_SHAPE_INDEX_BIT_SHIFT);
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
                    compass_fail_flag = true;
                    break;
                }
                if (puzzle_compass_down[new_x][new_y] != -1 && puzzle_compass_down[new_x][new_y] != compass_visited_down_cnt[i]) {
                    compass_fail_flag = true;
                    break;
                }
                if (puzzle_compass_left[new_x][new_y] != -1 && puzzle_compass_left[new_x][new_y] != compass_visited_left_cnt[i]) {
                    compass_fail_flag = true;
                    break;
                }
                if (puzzle_compass_right[new_x][new_y] != -1 && puzzle_compass_right[new_x][new_y] != compass_visited_right_cnt[i]) {
                    compass_fail_flag = true;
                    break;
                }
            }

            if (shape_check_fail_flag || nearby_shape_check_fail_flag || shape_in_puzzle_fail_flag || nearby_size_check_fail_flag || palisade_fail_flag || tatami_check_fail_flag || loopy_check_fail_flag || radar_check_fail_flag || compass_fail_flag) {
                for (int i = 0; i < current_size; ++i) {
                    solve_puzzle[((x + dfs_current_shape[i].x) << 1) + 1][((y + dfs_current_shape[i].y) << 1) + 1] &= (~SOLVE_AREA_SHAPE_INDEX_BIT);
                } 
                continue;
            }

            int ret;

            if (!empty_area_check(solve_puzzle)) {
                ret = -1;
            }
            else {
                if (puzzle_all_shapes_same) {
                    all_shapes_same_check_shape_index = shape_index;
                }
                all_shapes_different_check_shape_index_pool.insert(shape_index);
                ret = DFS(index + 1, solve_puzzle);
                if (puzzle_all_shapes_same) {
                    all_shapes_same_check_shape_index = -1;
                }
            }

            if (ret != -1) {
                return ret;
            }
            else {
                all_shapes_different_check_shape_index_pool.erase(shape_index);
                for (int i = 0; i < current_size; ++i) {
                    solve_puzzle[((x + dfs_current_shape[i].x) << 1) + 1][((y + dfs_current_shape[i].y) << 1) + 1] &= (~SOLVE_AREA_SHAPE_INDEX_BIT);
                }                    
            }
            continue;
        }

        bool delta_size_check = false;

        while (candidates_i < candidates_size) {

            expand_x = x + dfs_expand_candidates[candidates_i].x;
            expand_y = y + dfs_expand_candidates[candidates_i].y;
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
                    if (dfs_current_shape[j].x == dfs_expand_candidates[candidates_i - 1].x && dfs_current_shape[j].y == dfs_expand_candidates[candidates_i - 1].y) {
                        in_current_shape = true;
                        break;
                    }
                }
                if (!in_current_shape) {
                    DFS_empty(expand_x, expand_y, solve_puzzle);
                    int max_area_size = dfs_empty_count - dfs_empty_block_line_count;
                    if (max_area_size < puzzle_shape_size_lower_bound) {
                        break;
                    }
                    if (slash_check_enable) {
                        bool check_flag = false;
                        for (int i = 1; i <= slash_check_slash_cnt; i ++) {
                            if (std::abs(dfs_slash_count[i] - dfs_slash_count[1]) > 1) {
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
                    candidates_i ++;
                    continue;
                }
            }

            if (puzzle[to_puzzle_x(expand_x)][to_puzzle_y(expand_y)] & AREA_SHAPE_SIZE_BIT) {
                int target_size = puzzle[to_puzzle_x(expand_x)][to_puzzle_y(expand_y)] >> AREA_SHAPE_SIZE_BIT_SHIFT;
                if (target_size != size) {
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

            mark_slash[slash_index] = true;

            dfs_current_shape[dfs_current_shape_cnt].x = dfs_expand_candidates[candidates_i].x;
            dfs_current_shape[dfs_current_shape_cnt].y = dfs_expand_candidates[candidates_i].y;
            if (puzzle_only_rectangles || puzzle_no_rectangles) {
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
                if (dfs_current_shape[dfs_current_shape_cnt].x < compass_visited[i].x) {
                    compass_visited_up_cnt[i] ++;
                }
                if (dfs_current_shape[dfs_current_shape_cnt].x > compass_visited[i].x) {
                    compass_visited_down_cnt[i] ++;
                }
                if (dfs_current_shape[dfs_current_shape_cnt].y < compass_visited[i].y) {
                    compass_visited_left_cnt[i] ++;
                }
                if (dfs_current_shape[dfs_current_shape_cnt].y > compass_visited[i].y) {
                    compass_visited_right_cnt[i] ++;
                }
            }
            for (int i = 1; i <= slash_check_slash_cnt; ++i) {
                for (int j = 0; j < slash_nodes[1].size(); ++j) {
                    
                    if (dfs_current_shape_cnt == 0) {
                        slash_distance[dfs_current_shape_cnt][i][j] = dfs_current_shape_cnt;
                        continue;
                    }
                    else {
                        slash_distance[dfs_current_shape_cnt][i][j] = slash_distance[dfs_current_shape_cnt - 1][i][j];
                    }

                    int new_distance = std::abs(x + dfs_current_shape[dfs_current_shape_cnt].x - slash_nodes[i][j].x) + std::abs(y + dfs_current_shape[dfs_current_shape_cnt].y - slash_nodes[i][j].y);
                    int old_distance = std::abs(x + dfs_current_shape[slash_distance[dfs_current_shape_cnt][i][j]].x - slash_nodes[i][j].x) + std::abs(y + dfs_current_shape[slash_distance[dfs_current_shape_cnt][i][j]].y - slash_nodes[i][j].y);

                    if (new_distance < old_distance) {
                        slash_distance[dfs_current_shape_cnt][i][j] = dfs_current_shape_cnt;
                    }
                }
            }

            dfs_current_shape_cnt ++;

            if (puzzle[to_puzzle_x(expand_x)][to_puzzle_y(expand_y)] & AREA_PALISADE_INDEX_BIT) {
                palisade_visited[palisade_visited_cnt] = {dfs_expand_candidates[candidates_i].x, dfs_expand_candidates[candidates_i].y};
                palisade_visited_cnt ++;
            }
            if (puzzle[to_puzzle_x(expand_x)][to_puzzle_y(expand_y)] & AREA_COMPASS_ENABLE) {
                compass_visited[compass_visited_cnt] = {dfs_expand_candidates[candidates_i].x, dfs_expand_candidates[candidates_i].y};
                compass_visited_up_cnt[compass_visited_cnt] = 0;
                compass_visited_down_cnt[compass_visited_cnt] = 0;
                compass_visited_left_cnt[compass_visited_cnt] = 0;
                compass_visited_right_cnt[compass_visited_cnt] = 0;
                for (int w = 0; w < dfs_current_shape_cnt; ++w) {
                    if (dfs_current_shape[w].x < compass_visited[compass_visited_cnt].x) {
                        compass_visited_up_cnt[compass_visited_cnt] ++;
                    }
                    if (dfs_current_shape[w].x > compass_visited[compass_visited_cnt].x) {
                        compass_visited_down_cnt[compass_visited_cnt] ++;
                    }
                    if (dfs_current_shape[w].y < compass_visited[compass_visited_cnt].y) {
                        compass_visited_left_cnt[compass_visited_cnt] ++;
                    }
                    if (dfs_current_shape[w].y > compass_visited[compass_visited_cnt].y) {
                        compass_visited_right_cnt[compass_visited_cnt] ++;
                    }
                }
                compass_visited_cnt ++;
            }

            bool rectangle_fail_flag = false;
            if (puzzle_only_rectangles) {
                int rectangle_width = dfs_rectangle_right[dfs_current_shape_cnt - 1] - dfs_rectangle_left[dfs_current_shape_cnt - 1] + 1;
                int rectangle_height = dfs_rectangle_down[dfs_current_shape_cnt - 1] - dfs_rectangle_up[dfs_current_shape_cnt - 1] + 1;
                if (rectangle_width * rectangle_height > size) {
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
                    compass_fail_flag = true;
                    break;
                }
                if (puzzle_compass_down[new_x][new_y] != -1 && puzzle_compass_down[new_x][new_y] < compass_visited_down_cnt[i]) {
                    compass_fail_flag = true;
                    break;
                }
                if (puzzle_compass_left[new_x][new_y] != -1 && puzzle_compass_left[new_x][new_y] < compass_visited_left_cnt[i]) {
                    compass_fail_flag = true;
                    break;
                }
                if (puzzle_compass_right[new_x][new_y] != -1 && puzzle_compass_right[new_x][new_y] < compass_visited_right_cnt[i]) {
                    compass_fail_flag = true;
                    break;
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
                        if (mark_slash[i]) {
                            continue;
                        }
                        int _x = x + dfs_current_shape[slash_distance[dfs_current_shape_cnt - 1][i][slash_node_indexs[i]]].x - slash_nodes[i][slash_node_indexs[i]].x;
                        int _y = y + dfs_current_shape[slash_distance[dfs_current_shape_cnt - 1][i][slash_node_indexs[i]]].y - slash_nodes[i][slash_node_indexs[i]].y;
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
                        if (slash_node_indexs[temp_loc] == slash_nodes[1].size()) {
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
                    if (final_flag) {
                        break;
                    }
                }

                int remain_size = size - current_size - 1;

                if (distance_predict > remain_size) {
                    slash_distance_fail_flag = true;
                }

            }

            if (palisade_fail_flag || rectangle_fail_flag || compass_fail_flag || slash_distance_fail_flag) {

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
                    if (dfs_current_shape[dfs_current_shape_cnt].x < compass_visited[i].x) {
                        compass_visited_up_cnt[i] --;
                    }
                    if (dfs_current_shape[dfs_current_shape_cnt].x > compass_visited[i].x) {
                        compass_visited_down_cnt[i] --;
                    }
                    if (dfs_current_shape[dfs_current_shape_cnt].y < compass_visited[i].y) {
                        compass_visited_left_cnt[i] --;
                    }
                    if (dfs_current_shape[dfs_current_shape_cnt].y > compass_visited[i].y) {
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
                    if (symbol_loc.x == -233 && symbol_loc.y == -666) {
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
                            if (dfs_current_shape[dfs_current_shape_cnt].x < compass_visited[i].x) {
                                compass_visited_up_cnt[i] --;
                            }
                            if (dfs_current_shape[dfs_current_shape_cnt].x > compass_visited[i].x) {
                                compass_visited_down_cnt[i] --;
                            }
                            if (dfs_current_shape[dfs_current_shape_cnt].y < compass_visited[i].y) {
                                compass_visited_left_cnt[i] --;
                            }
                            if (dfs_current_shape[dfs_current_shape_cnt].y > compass_visited[i].y) {
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
                int new_x = dfs_expand_candidates[candidates_i].x + expand_dx[d];
                int new_y = dfs_expand_candidates[candidates_i].y + expand_dy[d];
                
                int puzzle_value = puzzle[((x + new_x) << 1) + 1][((y + new_y) << 1) + 1];
                if (puzzle_value == AREA_BLOCK) {
                    continue;
                }

                int solve_puzzle_value = solve_puzzle[((x + new_x) << 1) + 1][((y + new_y) << 1) + 1];
                if (solve_puzzle_value != AREA_NORMAL) {
                    continue;
                }
                
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
                if (alreay_in_candidates) {
                    continue;
                }
                
                dfs_expand_candidates[dfs_expand_candidates_cnt] = {new_x, new_y};
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
    }

    return -1;
}

int DFS(uint32_t index, uint32_t** solve_puzzle) {

    auto [ret, x, y] = find_special_start_area(solve_puzzle);

    if (ret == SPECIAL_START_DEFAULT && x == -1 && y == -1) {
        return 0; // Solved
    }

    bool mark_skip_shape[65536];
    memset(mark_skip_shape, 0, sizeof(mark_skip_shape));

    bool mark_slash[slash_check_slash_cnt + 1];

    Node compass_visited[MAX_SHAPE_SIZE];
    int compass_visited_cnt = 0;

    uint32_t ret_code;

    bool one_symbol_per_region_check;

    auto [range_check, shape_size_lower_bound, shape_size_upper_bound] = empty_area_size_range(x, y, solve_puzzle);

    // Type -1 check
    if (range_check == -1) {
        return -1;
    }

    if (ret == SPECIAL_START_AREA_SIZE) {
        shape_size_lower_bound = (puzzle[to_puzzle_x(x)][to_puzzle_y(y)] & AREA_SHAPE_SIZE_BIT) >> AREA_SHAPE_SIZE_BIT_SHIFT;
        shape_size_upper_bound = shape_size_lower_bound;
    }

    for (int i = 0; i < shapes.size(); ++i) {

        ret_code = 0;

        // ----------------------------------------------------
        // Type 0 check

        if (ret == SPECIAL_START_DEFAULT && mark_skip_shape[i]) {
            ret_code |= RET_CODE_SHAPE_CONTAIN_BLOCK_AREA;
            continue;
        }

        // Type 0 check: all shapes different
        if (puzzle_all_shapes_different) {
            if (all_shapes_different_check_shape_index_pool.find(shapes[i].shape_index) != all_shapes_different_check_shape_index_pool.end()) {
                ret_code |= RET_CODE_ALL_SHAPE_DIFFERENT;
                continue;
            }
        }

        // Type 0 check: all shapes same
        if (puzzle_all_shapes_same) {
            if ((all_shapes_same_check_shape_index != -1) && (shapes[i].shape_index != all_shapes_same_check_shape_index)) {
                ret_code |= RET_CODE_ALL_SHAPE_SAME;
                continue;
            }
        }

        // Type 0 check: shape size range
        if (shapes[i].nodes.size() < puzzle_shape_size_lower_bound || shapes[i].nodes.size() > puzzle_shape_size_upper_bound) {
            ret_code |= RET_CODE_SHAPE_SIZE_RANGE;
            continue;
        }

        // Type 0 check finished
        if (ret_code != 0) {
            continue;
        }
        // ----------------------------------------------------

        for (int p = 0; p < shapes[i].nodes.size(); ++p) {
            
            ret_code = 0;

            if (ret == SPECIAL_START_DEFAULT && p != 0) {
                break;
            }

            int start_x = shapes[i].nodes[p].x;
            int start_y = shapes[i].nodes[p].y;

            // ----------------------------------------------------
            // Type 1 check

            one_symbol_per_region_check = false;
            memset(mark_slash, 0, sizeof(mark_slash));
            compass_visited_cnt = 0;

            for (int j = 0; j < shapes[i].nodes.size(); ++j) {
                int new_x = x + shapes[i].nodes[j].x - start_x;
                int new_y = y + shapes[i].nodes[j].y - start_y;
                int puzzle_new_x = to_puzzle_x(new_x);
                int puzzle_new_y = to_puzzle_y(new_y);

                if (new_x < 1 || new_x > puzzle_n_row || new_y < 1 || new_y > puzzle_n_col) {
                    ret_code |= RET_CODE_BLOCK_AREA;
                    break;
                }

                // Type 1 check: BLOCK
                if (puzzle[puzzle_new_x][puzzle_new_y] == AREA_BLOCK) {
                    ret_code |= RET_CODE_BLOCK_AREA;
                    Node error_node = {shapes[i].nodes[j].x, shapes[i].nodes[j].y};
                    for (int skip_index : node_to_shape_index[error_node]) {
                        mark_skip_shape[skip_index] = true;
                    }
                    break;
                }
                
                // Type 1 check: FILLED
                if (solve_puzzle[puzzle_new_x][puzzle_new_y] != AREA_NORMAL) {
                    ret_code |= RET_CODE_FILLED_AREA;
                    Node error_node = {shapes[i].nodes[j].x, shapes[i].nodes[j].y};
                    for (int skip_index : node_to_shape_index[error_node]) {
                        mark_skip_shape[skip_index] = true;
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
                    if (target_index != shapes[i].shape_index) {
                        ret_code |= RET_CODE_AREA_SHAPE_INDEX;
                        break;
                    }
                }
                
                // Type 1 check: AREA_SHAPE_SIZE_CHECK
                if (puzzle[puzzle_new_x][puzzle_new_y] & AREA_SHAPE_SIZE_BIT) {
                    int target_size = puzzle[puzzle_new_x][puzzle_new_y] >> AREA_SHAPE_SIZE_BIT_SHIFT;
                    if (target_size != shapes[i].nodes.size()) {
                        ret_code |= RET_CODE_AREA_SHAPE_SIZE;
                        break;
                    }
                }

                // Type 1 check: ONE_SYMBOL_PER_REGION
                if (puzzle_one_symbol_per_region) {
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
                    compass_visited_cnt ++;
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
            if (puzzle_one_symbol_per_region) {
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
                for (int k = 0; k < shapes[i].nodes.size(); ++k) {
                    int new_x = x + shapes[i].nodes[k].x - start_x;
                    int new_y = y + shapes[i].nodes[k].y - start_y;
                    if (new_x < compass_x) {
                        compass_states.up ++;
                    }
                    if (new_x > compass_x) {
                        compass_states.down ++;
                    }
                    if (new_y < compass_y) {
                        compass_states.left ++;
                    }
                    if (new_y > compass_y) {
                        compass_states.right ++;
                    }
                }

                if (puzzle_compass_up[compass_puzzle_x][compass_puzzle_y] != -1 && puzzle_compass_up[compass_puzzle_x][compass_puzzle_y] != compass_states.up) {
                    ret_code |= RET_CODE_COMPASS;
                    break;
                }
                if (puzzle_compass_down[compass_puzzle_x][compass_puzzle_y] != -1 && puzzle_compass_down[compass_puzzle_x][compass_puzzle_y] != compass_states.down) {
                    ret_code |= RET_CODE_COMPASS;
                    break;
                }
                if (puzzle_compass_left[compass_puzzle_x][compass_puzzle_y] != -1 && puzzle_compass_left[compass_puzzle_x][compass_puzzle_y] != compass_states.left) {
                    ret_code |= RET_CODE_COMPASS;
                    break;
                }
                if (puzzle_compass_right[compass_puzzle_x][compass_puzzle_y] != -1 && puzzle_compass_right[compass_puzzle_x][compass_puzzle_y] != compass_states.right) {
                    ret_code |= RET_CODE_COMPASS;
                    break;
                }
            }

            // Type 1 check finished
            if (ret_code != 0) {
                continue;
            }
            // ----------------------------------------------------

            // ----------------------------------------------------
            // Type 2 check

            int l = 0;

            for (; l < shapes[i].nodes.size(); ++l) {
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

                if (puzzle_adjacent_shapes_different && !check_nearby_shape(puzzle_new_x, puzzle_new_y, puzzle_n_row, puzzle_n_col, solve_puzzle)) {
                    ret_code |= RET_CODE_ADJACENT_SHAPE_DIFFERENT;
                    break;
                }

                if (puzzle_adjacent_sizes_different && !check_nearby_size(puzzle_new_x, puzzle_new_y, puzzle_n_row, puzzle_n_col, solve_puzzle)) {
                    ret_code |= RET_CODE_ADJACENT_SIZE_DIFFERENT;
                    break;
                }
            }

            // Type 2 check finished
            if (ret_code != 0) {
                for (int j = 0; j <= l; ++j) {
                    int new_x = x + shapes[i].nodes[j].x - start_x;
                    int new_y = y + shapes[i].nodes[j].y - start_y;
                    int puzzle_new_x = to_puzzle_x(new_x);
                    int puzzle_new_y = to_puzzle_y(new_y);
                    solve_puzzle[puzzle_new_x][puzzle_new_y] = AREA_NORMAL;
                }
                continue;
            }
            // ----------------------------------------------------

            // ----------------------------------------------------
            // Type 3 check

            for (int j = 0; j < shapes[i].nodes.size(); ++j) {
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
                
                if (puzzle_no_4_way_intersections) {
                    if (!check_tatami(new_puzzle_x, new_puzzle_y, puzzle_n_row, puzzle_n_col, solve_puzzle)) {
                        ret_code |= RET_CODE_TATAMI;
                        break;
                    }
                }

                if (puzzle_no_3_way_intersections) {
                    if (!check_loopy(new_puzzle_x, new_puzzle_y, puzzle_n_row, puzzle_n_col, solve_puzzle)) {
                        ret_code |= RET_CODE_LOOPY;
                        break;
                    }
                }
            }

            // Type 3 check finished
            if (ret_code != 0) {
                for (int j = 0; j < shapes[i].nodes.size(); ++j) {
                    int new_x = x + shapes[i].nodes[j].x - start_x;
                    int new_y = y + shapes[i].nodes[j].y - start_y;
                    int puzzle_new_x = to_puzzle_x(new_x);
                    int puzzle_new_y = to_puzzle_y(new_y);
                    solve_puzzle[puzzle_new_x][puzzle_new_y] = AREA_NORMAL;
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
                for (int j = 0; j < shapes[i].nodes.size(); ++j) {
                    int new_x = x + shapes[i].nodes[j].x - start_x;
                    int new_y = y + shapes[i].nodes[j].y - start_y;
                    int puzzle_new_x = to_puzzle_x(new_x);
                    int puzzle_new_y = to_puzzle_y(new_y);
                    solve_puzzle[puzzle_new_x][puzzle_new_y] = AREA_NORMAL;
                }
                continue;
            }
            // ----------------------------------------------------

            // ----------------------------------------------------
            // DFS

            bool first_shape_flag = false;
            if (puzzle_all_shapes_same && all_shapes_same_check_shape_index == -1) {
                all_shapes_same_check_shape_index = shapes[i].shape_index;
                first_shape_flag = true;
            }

            if (puzzle_all_shapes_different) {
                all_shapes_different_check_shape_index_pool.insert(shapes[i].shape_index);
            }

            int ret = DFS(index + 1, solve_puzzle);
            if (ret != -1) {
                return ret;
            }

            if (first_shape_flag) {
                all_shapes_same_check_shape_index = -1;
            }

            if (puzzle_all_shapes_different) {
                all_shapes_different_check_shape_index_pool.erase(shapes[i].shape_index);
            }

            // DFS finished
            for (int j = 0; j < shapes[i].nodes.size(); ++j) {
                int new_x = x + shapes[i].nodes[j].x - start_x;
                int new_y = y + shapes[i].nodes[j].y - start_y;
                int puzzle_new_x = to_puzzle_x(new_x);
                int puzzle_new_y = to_puzzle_y(new_y);
                solve_puzzle[puzzle_new_x][puzzle_new_y] = AREA_NORMAL;
            }
            // ----------------------------------------------------
        }
    }

    if (ret == SPECIAL_START_AREA_INDEX) {
        return -1;
    }

    if (ret == SPECIAL_START_LINE_SAME) {
        return -1;
    }

    if (puzzle_predefine_shapes_only || puzzle_only_rectangles) {
        return -1;
    }

    if (puzzle_all_shapes_same && all_shapes_same_check_shape_index != -1) {
        return -1;
    }

    int known_shape_index = -1;
    if (!shapes.empty()) {
        known_shape_index = shapes[shapes.size() - 1].shape_index;
    }

    for (int size = shape_size_lower_bound; size <= shape_size_upper_bound; ++size) {
        int ret = place_non_predifined_shape(index, x, y, size, true, known_shape_index, solve_puzzle);
        if (ret != -1) {
            return ret;
        }
    }

    return -1;
}

int main() {
    std::string line;

    while (true) {
        std::cin >> line;
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
            continue;
        }
        if (line.find("AREA_AT_LEAST") != std::string::npos) {
            std::cin >> puzzle_shape_size_lower_bound;
            continue;
        }
        if (line.find("AREA_AT_MOST") != std::string::npos) {
            std::cin >> puzzle_shape_size_upper_bound;
            continue;
        }
        if (line.find("SHAPE") != std::string::npos) {
            uint32_t temp_shape_n_row;
            uint32_t temp_shape_size;
            // uint32_t** temp_shape;

            std::cin >> line;
            std::cin >> temp_shape_n_row;
            
            uint32_t** temp_shape = new uint32_t*[MAX_SHAPE_SIZE];
            for (int i = 0; i < MAX_SHAPE_SIZE; ++i) {
                temp_shape[i] = new uint32_t[MAX_SHAPE_SIZE];
            }

            std::getline(std::cin, line);

            for (int i = 0; i < temp_shape_n_row; ++i) {
                std::getline(std::cin, line);
                for (int j = 0; j < line.size(); ++j) {
                    temp_shape[i][j] = (line[j] == '#') ? 1 : 0;
                }
            }

            temp_shape_size = std::max(static_cast<uint32_t>(line.size()), temp_shape_n_row);
            
            for (int i = temp_shape_n_row; i < temp_shape_size; ++i) {
                for (int j = 0; j < temp_shape_size; ++j) {
                    temp_shape[i][j] = 0;
                }
            }
            shapes_insert(temp_shape, temp_shape_size);
            
            continue;
        }
        if (line.find("DIMENSIONS") != std::string::npos) {
            std::cin >> puzzle_n_col >> puzzle_n_row;
            continue;
        }
        if (line.find("PUZZLE") != std::string::npos) {
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
                slash_check_slash_cnt = std::max(slash_check_slash_cnt, slash_index);
            }
        }
    }

    for (int x = 1; x <= slash_check_slash_cnt; ++x) {
        std::vector<Node> temp;
        slash_nodes[x] = temp;
    }

    for (int x = 1; x <= puzzle_n_row; ++x) {
        for (int y = 1; y <= puzzle_n_col; ++y) {
            if (puzzle[to_puzzle_x(x)][to_puzzle_y(y)] & AREA_SLASH_INDEX_BIT) {
                int slash_index = (puzzle[to_puzzle_x(x)][to_puzzle_y(y)] & AREA_SLASH_INDEX_BIT) >> AREA_SLASH_INDEX_BIT_SHIFT;
                slash_nodes[slash_index].push_back(Node(x, y));
            }
            if (puzzle[to_puzzle_x(x)][to_puzzle_y(y)] & AREA_SHAPE_SIZE_BIT) {
                // int shape_size = (puzzle[to_puzzle_x(x)][to_puzzle_y(y)] & AREA_SHAPE_SIZE_BIT) >> AREA_SHAPE_SIZE_BIT_SHIFT;
                shape_size_nodes.push_back(Node(x, y));
            }
        }
    }

    std::sort(shape_size_nodes.begin(), shape_size_nodes.end(), [](const Node& a, const Node& b) {
        return ((puzzle[to_puzzle_x(a.x)][to_puzzle_y(a.y)] & AREA_SHAPE_SIZE_BIT) >> AREA_SHAPE_SIZE_BIT_SHIFT) < ((puzzle[to_puzzle_x(b.x)][to_puzzle_y(b.y)] & AREA_SHAPE_SIZE_BIT) >> AREA_SHAPE_SIZE_BIT_SHIFT);
    });

    if (puzzle_shape_size_lower_bound == -1) {
        puzzle_shape_size_lower_bound = 1;
    }
    if (puzzle_shape_size_upper_bound == -1) {
        puzzle_shape_size_upper_bound = empty_area_cnt;
    }
    if (slash_check_enable) {
        puzzle_shape_size_lower_bound = std::max(puzzle_shape_size_lower_bound, slash_check_slash_cnt);
    }

    if (puzzle_only_rectangles) {
        
        int up, down, left, right;
        for (int k = 0; k < shapes.size(); ++k) {
            up = 0, down = 0, left = 0, right = 0;
            for (int l = 0; l < shapes[k].nodes.size(); ++l) {
                up = std::min(up, shapes[k].nodes[l].x);
                down = std::max(down, shapes[k].nodes[l].x);
                left = std::min(left, shapes[k].nodes[l].y);
                right = std::max(right, shapes[k].nodes[l].y);
            }
            int rectangle_width = right - left + 1;
            int rectangle_height = down - up + 1;
            if (rectangle_width * rectangle_height > shapes[k].nodes.size()) {
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
                // add_shape_to_shapes(next_shape_index, shape_size, temp_shape);
                // rotate_shape(shape_size, temp_shape);
                // for (int i = 0; i < shape_size; ++i) {
                //     for (int j = 0; j < shape_size; ++j) {
                //         std::cout << temp_shape[i][j] << " ";
                //     }
                //     std::cout << std::endl;
                // }
                // add_shape_to_shapes(next_shape_index, shape_size, temp_shape);
                // shape_index_to_shape_size_map[next_shape_index] = shapes[shapes.size() - 1].nodes.size();
                // next_shape_index ++;
                shapes_insert(temp_shape, shape_size);
            }
        }
    }

    // std::cout << "puzzle_shape_size_lower_bound = " << puzzle_shape_size_lower_bound << std::endl;
    // std::cout << "puzzle_shape_size_upper_bound = " << puzzle_shape_size_upper_bound << std::endl;
    // std::cout << "slash_size = " << slash_size << std::endl;

    // DFS_special_start(1, solve_puzzle);
    DFS(1, solve_puzzle);

    std::cout << "SOLUTION" << std::endl;
    print_puzzle(solve_puzzle);

    return 0;
}