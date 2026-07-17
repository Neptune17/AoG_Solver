#!/bin/bash

# 批量运行 main 程序的脚本（并行版）
# 用法: ./batch_run.sh <puzzles_folder_path>
# 环境变量 NCPU 可覆盖并行数，默认使用全部逻辑核

# 定义颜色
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

if [ $# -ne 1 ]; then
    echo "用法: $0 <puzzles_folder_path>"
    echo "示例: $0 puzzles/Zone1/1-single-shape"
    echo "       $0 puzzles/Zone1"
    echo ""
    echo "环境变量:"
    echo "  NCPU=4  并行任务数（默认: nproc）"
    exit 1
fi

PUZZLE_DIR="$1"
NCPU=${NCPU:-$(nproc)}

# 检查目录是否存在
if [ ! -d "$PUZZLE_DIR" ]; then
    echo "错误: 目录 '$PUZZLE_DIR' 不存在"
    exit 1
fi

# 检查 main 是否存在
if [ ! -x "./build/main" ]; then
    echo "错误: ./build/main 不存在或不可执行"
    echo "请先编译: cmake -B build && cmake --build build"
    exit 1
fi

# 创建日志输出目录
LOG_DIR="${PUZZLE_DIR}/logs"
mkdir -p "$LOG_DIR"

echo "开始批量处理..."
echo "Puzzle 目录: $PUZZLE_DIR"
echo "日志目录:   $LOG_DIR"
echo "并行任务数: $NCPU"
echo "扫描 .puz 文件..."
echo "----------------------------------------"

# 使用 find 命令递归查找所有 .puz 文件
direct_puz_count=$(find "$PUZZLE_DIR" -maxdepth 1 -name "*.puz" | wc -l)

if [ $direct_puz_count -gt 0 ]; then
    mapfile -t puz_files < <(find "$PUZZLE_DIR" -maxdepth 1 -name "*.puz" | sort)
else
    mapfile -t puz_files < <(find "$PUZZLE_DIR" -name "*.puz" | sort)
fi

puz_count=${#puz_files[@]}
if [ $puz_count -eq 0 ]; then
    echo "警告: 在 $PUZZLE_DIR 及其子目录中没有找到 .puz 文件"
    exit 0
fi

echo "找到 $puz_count 个 puzzle 文件"
echo "----------------------------------------"

# 临时目录存放并行结果
RESULT_DIR=$(mktemp -d /tmp/batch_run_XXXXXX)
trap "rm -rf $RESULT_DIR" EXIT

# ------------------------------------------------------------
# Worker: 处理单个 puzzle，结果写入 $RESULT_DIR/result_<idx>
# ------------------------------------------------------------
run_one() {
    local idx=$1
    local puz_file=$2
    local result_file="$RESULT_DIR/result_$idx"

    # 日志文件路径（保持原有目录结构）
    local filename=$(basename "$puz_file")
    local basename="${filename%.puz}"
    local rel_path="${puz_file#$PUZZLE_DIR/}"
    local file_dir=$(dirname "$rel_path")

    if [ "$file_dir" != "." ]; then
        local log_dir="$LOG_DIR/$file_dir"
        mkdir -p "$log_dir"
        local log_file="$log_dir/${basename}.log"
    else
        local log_file="$LOG_DIR/${basename}.log"
    fi

    # 记录开始时间
    local start_time=$(date +%s.%N)

    # 运行 main，10秒超时
    timeout 10s ./build/main < "$puz_file" > "$log_file" 2>&1
    local exit_code=$?

    # 记录结束时间
    local end_time=$(date +%s.%N)
    local elapsed=$(echo "$end_time - $start_time" | bc)

    # 判断结果
    local status
    local extra=""

    if [ $exit_code -eq 124 ]; then
        status="timeout"
    elif [ $exit_code -eq 0 ]; then
        if grep -q "^SOLUTION$" "$log_file"; then
            local puz_solution=$(sed -n '/^SOLUTION$/,$p' "$puz_file" | tail -n +2)
            local log_solution=$(sed -n '/^SOLUTION$/,$p' "$log_file" | tail -n +2)
            if [ "$puz_solution" = "$log_solution" ]; then
                status="correct"
            else
                status="wrong"
                extra="expected_lines=$(echo "$puz_solution" | wc -l)|actual_lines=$(echo "$log_solution" | wc -l)"
            fi
        else
            status="no_solution"
        fi
    else
        status="error"
        extra="exit_code=$exit_code"
    fi

    # 写入结果（一行，用 | 分隔字段）
    echo "$idx|$status|$elapsed|$extra" > "$result_file"
}

# ------------------------------------------------------------
# 并行启动所有 worker
# ------------------------------------------------------------
total_start_time=$(date +%s.%N)

running=0
for i in "${!puz_files[@]}"; do
    run_one "$i" "${puz_files[$i]}" &
    running=$((running + 1))

    if [ $running -ge $NCPU ]; then
        wait -n 2>/dev/null
        running=$((running - 1))
    fi
done
wait

total_end_time=$(date +%s.%N)
total_elapsed=$(echo "$total_end_time - $total_start_time" | bc)

# ------------------------------------------------------------
# 按序汇总结果
# ------------------------------------------------------------
total=0
success=0
failed=0

for i in "${!puz_files[@]}"; do
    puz_file="${puz_files[$i]}"
    result_file="$RESULT_DIR/result_$i"

    # 解析结果行: idx|status|elapsed|extra
    IFS='|' read -r _idx status elapsed extra < "$result_file"

    total=$((total + 1))

    filename=$(basename "$puz_file")
    rel_path="${puz_file#$PUZZLE_DIR/}"
    file_dir=$(dirname "$rel_path")

    if [ "$file_dir" != "." ]; then
        echo "[$total] 处理: $file_dir/$filename"
    else
        echo "[$total] 处理: $filename"
    fi

    case "$status" in
        timeout)
            printf "    ${RED}✗ 超时终止 - 运行时间超过10秒${NC} - 耗时: ${elapsed}s\n"
            failed=$((failed + 1))
            ;;
        correct)
            printf "    ${GREEN}✓ 程序运行成功${NC} - 耗时: ${elapsed}s\n"
            printf "    ${GREEN}✓✓ 答案正确 - 与标准答案一致${NC}\n"
            success=$((success + 1))
            ;;
        wrong)
            printf "    ${GREEN}✓ 程序运行成功${NC} - 耗时: ${elapsed}s\n"
            # 解析 extra 字段: expected_lines=N|actual_lines=M
            expected_lines=$(echo "$extra" | grep -oP 'expected_lines=\K[0-9]+')
            actual_lines=$(echo "$extra" | grep -oP 'actual_lines=\K[0-9]+')
            printf "    ${RED}✗✗ 答案错误 - 与标准答案不一致${NC}\n"
            printf "       标准答案行数: ${expected_lines}, 输出答案行数: ${actual_lines}\n"
            failed=$((failed + 1))
            ;;
        no_solution)
            printf "    ${RED}✗ 程序未输出 SOLUTION - 可能解题失败${NC} - 耗时: ${elapsed}s\n"
            failed=$((failed + 1))
            ;;
        error)
            actual_code=$(echo "$extra" | grep -oP 'exit_code=\K[0-9]+')
            printf "    ${RED}✗ 程序运行失败 (退出码: ${actual_code})${NC} - 耗时: ${elapsed}s\n"
            failed=$((failed + 1))
            ;;
    esac
done

echo "----------------------------------------"
printf "批量处理完成!\n"
printf "总计: $total | ${GREEN}成功: $success${NC} | ${RED}失败: $failed${NC}\n"
echo "总耗时: ${total_elapsed}s"
echo "所有日志保存在: $LOG_DIR"
