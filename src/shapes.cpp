#include "shapes.h"
#include "puzzle.h"

// ------------------------------------------------------------
// Global shape state
// ------------------------------------------------------------
std::vector<Shape> shapes;
// O(1) array lookup for shape sizes (index is small int, starts at 1)
std::vector<int> shape_size_by_index(1, 0); // [0] unused
std::unordered_map<uint32_t, std::vector<size_t>> shape_digest_index;
uint32_t next_shape_index = 1;

std::unordered_map<Node, std::vector<int>> node_to_shape_index;

std::unordered_map<int, std::vector<Node>> slash_nodes;
std::vector<Node> shape_size_nodes;

uint32_t _temp_shape[MAX_SHAPE_SIZE][MAX_SHAPE_SIZE];

// ------------------------------------------------------------
// Shape operations
// ------------------------------------------------------------

void _shape_rotate(uint32_t** shape, uint32_t shape_size) {
    // In-place 4-way ring rotation
    int n = shape_size;
    for (int layer = 0; layer < n / 2; ++layer) {
        int first = layer, last = n - 1 - layer;
        for (int i = first; i < last; ++i) {
            int offset = i - first;
            uint32_t top = shape[first][i];
            shape[first][i]          = shape[last - offset][first];   // left -> top
            shape[last - offset][first]  = shape[last][last - offset]; // bottom -> left
            shape[last][last - offset]   = shape[i][last];             // right -> bottom
            shape[i][last]               = top;                       // top -> right
        }
    }
}

void _shape_mirror(uint32_t** shape, uint32_t shape_size) {
    // In-place column swap (horizontal flip)
    int n = shape_size;
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n / 2; ++j)
            std::swap(shape[i][j], shape[i][n - 1 - j]);
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

// Compute digest without building full Shape (avoids allocation on cache miss)
// Compute digest matching Shape constructor exactly
static uint32_t compute_digest(uint32_t** shape, uint32_t shape_size) {
    int n = (int)shape_size;
    uint32_t preview[128]; // shape_size <= MAX_SHAPE_SIZE = 100 (< 128)
    int most_left_j = n, most_up_i = n;

    for (int i = 0; i < n; ++i) {
        uint32_t line = 0;
        for (int j = 0; j < n; ++j) {
            line <<= 1;
            if (shape[i][j] == 1) {
                most_left_j = std::min(most_left_j, j);
                most_up_i = std::min(most_up_i, i);
                line += 1;
            }
        }
        preview[i] = line;
    }

    for (int i = 0; i < n; ++i) preview[i] <<= most_left_j;
    for (int i = most_up_i; i < n; ++i) preview[i - most_up_i] = preview[i];
    for (int i = 0; i < most_up_i; ++i) preview[i + n - most_up_i] = 0;

    uint32_t d = 0;
    for (int i = 0; i < n; ++i) d = d * 131 + preview[i];
    return d;
}

uint32_t shapes_search(uint32_t** shape, uint32_t shape_size) {
    uint32_t d = compute_digest(shape, shape_size);

    auto it = shape_digest_index.find(d);
    if (it == shape_digest_index.end()) {
        return 0xffffffffu;
    }

    Shape new_shape(shape, shape_size);
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
        shape_size_by_index.push_back(shapes[shapes.size() - 1].nodes.size());
        next_shape_index++;
    }

    return insert_success_count;
}
