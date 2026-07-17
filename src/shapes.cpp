#include "shapes.h"
#include "puzzle.h"

// ------------------------------------------------------------
// Global shape state
// ------------------------------------------------------------
std::vector<Shape> shapes;
std::map<uint32_t, int> shape_index_to_shape_size_map;
std::unordered_map<uint32_t, std::vector<size_t>> shape_digest_index;
uint32_t next_shape_index = 1;

std::map<Node, std::vector<int>> node_to_shape_index;

std::map<int, std::vector<Node>> slash_nodes;
std::vector<Node> shape_size_nodes;

uint32_t _temp_shape[MAX_SHAPE_SIZE][MAX_SHAPE_SIZE];

// ------------------------------------------------------------
// Shape operations
// ------------------------------------------------------------

void _shape_rotate(uint32_t** shape, uint32_t shape_size) {
    for (int i = 0; i < (int)shape_size; ++i) {
        for (int j = 0; j < (int)shape_size; ++j) {
            _temp_shape[j][shape_size - 1 - i] = shape[i][j];
        }
    }
    for (int i = 0; i < (int)shape_size; ++i) {
        for (int j = 0; j < (int)shape_size; ++j) {
            shape[i][j] = _temp_shape[i][j];
        }
    }
}

void _shape_mirror(uint32_t** shape, uint32_t shape_size) {
    for (int i = 0; i < (int)shape_size; ++i) {
        for (int j = 0; j < (int)shape_size; ++j) {
            _temp_shape[i][shape_size - 1 - j] = shape[i][j];
        }
    }
    for (int i = 0; i < (int)shape_size; ++i) {
        for (int j = 0; j < (int)shape_size; ++j) {
            shape[i][j] = _temp_shape[i][j];
        }
    }
}

bool _add_shape_to_shapes(uint32_t shape_index, uint32_t** shape, uint32_t shape_size) {

    if (shapes_search(shape, shape_size) == 0xffffffffu) {
        Shape new_shape(shape, shape_size);
        new_shape.shape_index = shape_index;
        shapes.push_back(new_shape);

        size_t idx = shapes.size() - 1;
        shape_digest_index[new_shape.digest].push_back(idx);

        for (int i = 0; i < (int)new_shape.nodes.size(); ++i) {
            node_to_shape_index[new_shape.nodes[i]].push_back(idx);
        }

        return true;
    }

    return false;
}

uint32_t shapes_search(uint32_t** shape, uint32_t shape_size) {
    Shape new_shape(shape, shape_size);

    auto it = shape_digest_index.find(new_shape.digest);
    if (it == shape_digest_index.end()) {
        return 0xffffffffu;
    }

    for (size_t idx : it->second) {
        if (shapes[idx] == new_shape) {
            return shapes[idx].shape_index;
        }
    }
    return 0xffffffffu;
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
        next_shape_index++;
    }

    return insert_success_count;
}
