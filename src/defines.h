#ifndef DEFINES_H
#define DEFINES_H

#include <cstdint>

// ------------------------------------------------------------
// Line flags (bits in puzzle cells for lines between areas)
// ------------------------------------------------------------
constexpr uint32_t LINE_NORMAL           = 0x00000000u;
constexpr uint32_t LINE_BLOCK            = 0x80000000u;
constexpr uint32_t LINE_DIFFERENT        = 0x40000000u;
constexpr uint32_t LINE_EQUAL            = 0x20000000u;
constexpr uint32_t LINE_SMALLER          = 0x10000000u;
constexpr uint32_t LINE_LARGER           = 0x08000000u;
constexpr uint32_t LINE_SIZE_DIFF_BIT    = 0x000f0000u;
constexpr uint32_t LINE_SIZE_DIFF_BIT_SHIFT = 16;
constexpr uint32_t LINE_PARSE_ERROR      = 0xffffffffu;

// ------------------------------------------------------------
// Area flags (bits in puzzle cells for area contents)
// ------------------------------------------------------------
constexpr uint32_t AREA_NORMAL               = 0x00000000u;
constexpr uint32_t AREA_BLOCK                = 0x80000000u;
constexpr uint32_t AREA_PALISADE_INDEX_BIT   = 0x70000000u;
constexpr uint32_t AREA_PALISADE_INDEX_BIT_SHIFT = 28;
constexpr uint32_t AREA_SHAPE_INDEX_BIT      = 0x0f000000u;
constexpr uint32_t AREA_SHAPE_INDEX_BIT_SHIFT = 24;
constexpr uint32_t AREA_SHAPE_SIZE_BIT       = 0x00ff0000u;
constexpr uint32_t AREA_SHAPE_SIZE_BIT_SHIFT = 16;
constexpr uint32_t AREA_SLASH_INDEX_BIT      = 0x0000f000u;
constexpr uint32_t AREA_SLASH_INDEX_BIT_SHIFT = 12;
constexpr uint32_t AREA_COMPASS_ENABLE       = 0x00000800u;
constexpr uint32_t AREA_PARSE_ERROR          = 0xffffffffu;

// ------------------------------------------------------------
// Solve puzzle area flags
// ------------------------------------------------------------
constexpr uint32_t SOLVE_AREA_SHAPE_INDEX_BIT      = 0xffff0000u;
constexpr uint32_t SOLVE_AREA_SHAPE_INDEX_BIT_SHIFT = 16;
constexpr uint32_t SOLVE_AREA_BIT                   = 0x0000ffffu;

// ------------------------------------------------------------
// Vertex flags
// ------------------------------------------------------------
constexpr uint32_t VERTEX_NORMAL       = 0x00000000u;
constexpr uint32_t VERTEX_BLOCK        = 0x80000000u;
constexpr uint32_t VERTEX_RADAR_BIT    = 0x0000000Fu;
constexpr uint32_t VERTEX_RADAR_BIT_SHIFT = 0;
constexpr uint32_t VERTEX_PARSE_ERROR  = 0xffffffffu;

// ------------------------------------------------------------
// Return codes for DFS type checks
// ------------------------------------------------------------
constexpr uint32_t RET_CODE_BLOCK_AREA             = 0x00000001u;
constexpr uint32_t RET_CODE_FILLED_AREA            = 0x00000002u;
constexpr uint32_t RET_CODE_AREA_SHAPE_INDEX       = 0x00000004u;
constexpr uint32_t RET_CODE_AREA_SHAPE_SIZE        = 0x00000008u;
constexpr uint32_t RET_CODE_SLASH                  = 0x00000010u;
constexpr uint32_t RET_CODE_ALL_SHAPE_SAME         = 0x00000020u;
constexpr uint32_t RET_CODE_ALL_SHAPE_DIFFERENT    = 0x00000040u;
constexpr uint32_t RET_CODE_ONE_SYMBOL_PER_REGION  = 0x00000080u;
constexpr uint32_t RET_CODE_COMPASS                = 0x00000100u;
constexpr uint32_t RET_CODE_SHAPE_SIZE_RANGE       = 0x00000200u;
constexpr uint32_t RET_CODE_PALISADE               = 0x00000400u;
constexpr uint32_t RET_CODE_TATAMI                 = 0x00000800u;
constexpr uint32_t RET_CODE_LOOPY                  = 0x00001000u;
constexpr uint32_t RET_CODE_RADAR                  = 0x00002000u;
constexpr uint32_t RET_CODE_EMPTY_CHECK            = 0x00004000u;
constexpr uint32_t RET_CODE_ADJACENT_SHAPE_DIFFERENT  = 0x00008000u;
constexpr uint32_t RET_CODE_ADJACENT_SIZE_DIFFERENT   = 0x00010000u;
constexpr uint32_t RET_CODE_IN_SHAPE_EDGE          = 0x00020000u;
constexpr uint32_t RET_CODE_EDGE_CONSTRAINT        = 0x00040000u;
constexpr uint32_t RET_CODE_SHAPE_CONTAIN_BLOCK_AREA = 0x00080000u;

// ------------------------------------------------------------
// Special start area types
// ------------------------------------------------------------
constexpr uint32_t SPECIAL_START_DEFAULT               = 0x00000000u;
constexpr uint32_t SPECIAL_START_SIZE_1_REGION          = 0x00000001u;
constexpr uint32_t SPECIAL_START_SIZE_MATCH_REGION      = 0x00000002u;
constexpr uint32_t SPECIAL_START_LINE_SAME              = 0x00000003u;
constexpr uint32_t SPECIAL_START_LINE_SMALLER_OR_LARGER = 0x00000004u;
constexpr uint32_t SPECIAL_START_AREA_INDEX             = 0x00000005u;
constexpr uint32_t SPECIAL_START_AREA_SIZE              = 0x00000006u;
constexpr uint32_t SPECIAL_START_CORNER                 = 0x00000007u;
constexpr uint32_t SPECIAL_START_COMPASS                = 0x00000008u;
constexpr uint32_t SPECIAL_START_LINE_CONSTRAINT        = 0x00000009u;
constexpr uint32_t SPECIAL_START_LINE_SIZE_DIFF         = 0x0000000au;

// ------------------------------------------------------------
// Size limits
// ------------------------------------------------------------
constexpr int MAX_PUZZLE_SIZE = 1000;
constexpr int MAX_SHAPE_SIZE  = 100;

#endif // DEFINES_H
