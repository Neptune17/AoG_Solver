#ifndef TYPES_H
#define TYPES_H

#include <cstdint>
#include <vector>
#include <functional>

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

namespace std {
    template <>
    struct hash<Node> {
        std::size_t operator()(const Node& node) const noexcept {
            return node.x * 131 + node.y;
        }
    };
}

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
        for (int i = 0; i < (int)shape_size; ++i) {
            line_preview = 0;
            for (int j = 0; j < (int)shape_size; ++j) {
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

        for (int i = 0; i < (int)this->preview.size(); ++i) {
            this->preview[i] <<= most_left_j;
        }

        for (int i = most_up_i; i < (int)this->preview.size(); ++i) {
            this->preview[i - most_up_i] = this->preview[i];
        }

        for (int i = 0; i < most_up_i; ++i) {
            this->preview[i + this->preview.size() - most_up_i] = 0;
        }

        for (int i = 0; i < (int)this->preview.size(); ++i) {
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
        for (int i = 0; i < (int)preview.size(); ++i) {
            if (preview[i] != other.preview[i]) {
                return false;
            }
        }
        return true;
    }
};

struct CompassStates {
    int up, down, left, right;

    CompassStates() : CompassStates(0, 0, 0, 0) {}

    CompassStates(int up, int down, int left, int right) {
        this->up = up;
        this->down = down;
        this->left = left;
        this->right = right;
    }
};

#endif // TYPES_H
