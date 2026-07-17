#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

#include "defines.h"
#include "types.h"
#include "puzzle.h"
#include "shapes.h"
#include "dfs.h"

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
            config.predefine_shapes_only = true;
            continue;
        }
        if (line.find("ADJACENT_SHAPES_DIFFERENT") != std::string::npos) {
            config.adjacent_shapes_different = true;
            continue;
        }
        if (line.find("ADJACENT_SIZES_DIFFERENT") != std::string::npos) {
            config.adjacent_sizes_different = true;
            continue;
        }
        if (line.find("ALL_SHAPES_DIFFERENT") != std::string::npos) {
            config.all_shapes_different = true;
            continue;
        }
        if (line.find("ONLY_RECTANGLES") != std::string::npos) {
            config.only_rectangles = true;
            continue;
        }
        if (line.find("NO_RECTANGLES") != std::string::npos) {
            config.no_rectangles = true;
            continue;
        }
        if (line.find("ONE_SYMBOL_PER_REGION") != std::string::npos) {
            config.one_symbol_per_region = true;
            continue;
        }
        if (line.find("ALL_SHAPES_SAME") != std::string::npos) {
            config.all_shapes_same = true;
            continue;
        }
        if (line.find("NO_4_WAY_INTERSECTIONS") != std::string::npos) {
            config.no_4_way_intersections = true;
            continue;
        }
        if (line.find("NO_3_WAY_INTERSECTIONS") != std::string::npos) {
            config.no_3_way_intersections = true;
            continue;
        }
        if (line.find("AREA_EQUALS") != std::string::npos) {
            std::cin >> config.shape_size_lower_bound;
            config.shape_size_upper_bound = config.shape_size_lower_bound;
            continue;
        }
        if (line.find("AREA_AT_LEAST") != std::string::npos) {
            std::cin >> config.shape_size_lower_bound;
            continue;
        }
        if (line.find("AREA_AT_MOST") != std::string::npos) {
            std::cin >> config.shape_size_upper_bound;
            continue;
        }
        if (line.find("SHAPE") != std::string::npos) {
            uint32_t temp_shape_n_row;
            uint32_t temp_shape_size;

            std::cin >> line;
            std::cin >> temp_shape_n_row;

            uint32_t** temp_shape = new uint32_t*[MAX_SHAPE_SIZE];
            for (int i = 0; i < MAX_SHAPE_SIZE; ++i) {
                temp_shape[i] = new uint32_t[MAX_SHAPE_SIZE];
            }

            std::getline(std::cin, line);

            for (uint32_t i = 0; i < temp_shape_n_row; ++i) {
                std::getline(std::cin, line);
                for (int j = 0; j < (int)line.size(); ++j) {
                    temp_shape[i][j] = (line[j] == '#') ? 1 : 0;
                }
                for (int j = (int)line.size(); j < (int)temp_shape_n_row; ++j) {
                    temp_shape[i][j] = 0;
                }
            }

            temp_shape_size = std::max(static_cast<uint32_t>(line.size()), temp_shape_n_row);

            for (uint32_t i = temp_shape_n_row; i < temp_shape_size; ++i) {
                for (uint32_t j = 0; j < temp_shape_size; ++j) {
                    temp_shape[i][j] = 0;
                }
            }
            shapes_insert(temp_shape, temp_shape_size);

            for (int i = 0; i < MAX_SHAPE_SIZE; ++i) {
                delete[] temp_shape[i];
            }
            delete[] temp_shape;

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

    uint32_t** solve_puzzle = new uint32_t*[MAX_PUZZLE_SIZE];
    for (int i = 0; i < MAX_PUZZLE_SIZE; ++i) {
        solve_puzzle[i] = new uint32_t[MAX_PUZZLE_SIZE];
        for (int j = 0; j < MAX_PUZZLE_SIZE; ++j) {
            solve_puzzle[i][j] = AREA_NORMAL;
        }
    }

    build_solve_puzzle(solve_puzzle);

    int empty_area_cnt = 0;
    for (int x = 1; x <= (int)puzzle_n_row; ++x) {
        for (int y = 1; y <= (int)puzzle_n_col; ++y) {
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
        slash_nodes[x] = std::vector<Node>();
    }

    for (int x = 1; x <= (int)puzzle_n_row; ++x) {
        for (int y = 1; y <= (int)puzzle_n_col; ++y) {
            if (puzzle[to_puzzle_x(x)][to_puzzle_y(y)] & AREA_SLASH_INDEX_BIT) {
                int slash_index = (puzzle[to_puzzle_x(x)][to_puzzle_y(y)] & AREA_SLASH_INDEX_BIT) >> AREA_SLASH_INDEX_BIT_SHIFT;
                slash_nodes[slash_index].push_back(Node(x, y));
            }
            if (puzzle[to_puzzle_x(x)][to_puzzle_y(y)] & AREA_SHAPE_SIZE_BIT) {
                shape_size_nodes.push_back(Node(x, y));
            }
        }
    }

    std::sort(shape_size_nodes.begin(), shape_size_nodes.end(), [](const Node& a, const Node& b) {
        return ((puzzle[to_puzzle_x(a.x)][to_puzzle_y(a.y)] & AREA_SHAPE_SIZE_BIT) >> AREA_SHAPE_SIZE_BIT_SHIFT) <
               ((puzzle[to_puzzle_x(b.x)][to_puzzle_y(b.y)] & AREA_SHAPE_SIZE_BIT) >> AREA_SHAPE_SIZE_BIT_SHIFT);
    });

    if (config.shape_size_lower_bound == -1) {
        config.shape_size_lower_bound = 1;
    }
    if (config.shape_size_upper_bound == -1) {
        config.shape_size_upper_bound = empty_area_cnt;
    }
    if (slash_check_enable) {
        config.shape_size_lower_bound = std::max(config.shape_size_lower_bound, slash_check_slash_cnt);
    }

    if (config.only_rectangles) {

        int up, down, left, right;
        for (int k = 0; k < (int)shapes.size(); ++k) {
            up = 0, down = 0, left = 0, right = 0;
            for (int l = 0; l < (int)shapes[k].nodes.size(); ++l) {
                up = std::min(up, shapes[k].nodes[l].x);
                down = std::max(down, shapes[k].nodes[l].x);
                left = std::min(left, shapes[k].nodes[l].y);
                right = std::max(right, shapes[k].nodes[l].y);
            }
            int rectangle_width = right - left + 1;
            int rectangle_height = down - up + 1;
            if (rectangle_width * rectangle_height > (int)shapes[k].nodes.size()) {
                shapes[k].shape_index = 0;
            }
        }

        uint32_t** temp_shape = new uint32_t*[100];
        for (int i = 0; i < 100; ++i) {
            temp_shape[i] = new uint32_t[100];
        }
        for (int size = config.shape_size_lower_bound; size <= config.shape_size_upper_bound; ++size) {
            for (int l = 1; l <= (size / l); ++l) {
                if (size % l != 0) continue;
                int h = size / l;
                if (l > (int)puzzle_n_row || h > (int)puzzle_n_col) continue;
                int shape_size = std::max(h, l);
                for (int i = 0; i < shape_size; ++i) {
                    for (int j = 0; j < shape_size; ++j) {
                        temp_shape[i][j] = (i < l && j < h) ? 1 : 0;
                    }
                }
                shapes_insert(temp_shape, shape_size);
            }
        }
        for (int i = 0; i < 100; ++i) {
            delete[] temp_shape[i];
        }
        delete[] temp_shape;
    }

    if (config.predefine_shapes_only) {
        config.shape_size_lower_bound = shapes[0].nodes.size();
        config.shape_size_upper_bound = shapes[0].nodes.size();
        for (const auto& shape : shapes) {
            config.shape_size_lower_bound = std::min(config.shape_size_lower_bound, (int)shape.nodes.size());
            config.shape_size_upper_bound = std::max(config.shape_size_upper_bound, (int)shape.nodes.size());
        }
    }

    DFS(1, solve_puzzle);

    std::cout << "SOLUTION" << std::endl;
    print_puzzle(solve_puzzle);

    for (int i = 0; i < MAX_PUZZLE_SIZE; ++i) {
        delete[] solve_puzzle[i];
    }
    delete[] solve_puzzle;

    return 0;
}
