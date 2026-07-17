#ifndef SHAPES_H
#define SHAPES_H

#include <vector>
#include <map>
#include <unordered_map>
#include <cstddef>

#include "defines.h"
#include "types.h"

// ------------------------------------------------------------
// Global shape state
// ------------------------------------------------------------
extern std::vector<Shape> shapes;
// O(1) array lookup for shape sizes (index is small int, starts at 1)
extern std::vector<int> shape_size_by_index;
extern std::unordered_map<uint32_t, std::vector<size_t>> shape_digest_index;
extern uint32_t next_shape_index;

extern std::unordered_map<Node, std::vector<int>> node_to_shape_index;

extern std::unordered_map<int, std::vector<Node>> slash_nodes;
extern std::vector<Node> shape_size_nodes;

extern uint32_t _temp_shape[MAX_SHAPE_SIZE][MAX_SHAPE_SIZE];

// ------------------------------------------------------------
// Shape operations
// ------------------------------------------------------------
void _shape_rotate(uint32_t** shape, uint32_t shape_size);
void _shape_mirror(uint32_t** shape, uint32_t shape_size);
bool _add_shape_to_shapes(uint32_t shape_index, uint32_t** shape, uint32_t shape_size);
uint32_t shapes_search(uint32_t** shape, uint32_t shape_size);
uint32_t shapes_insert(uint32_t** shape, uint32_t shape_size);

#endif // SHAPES_H
