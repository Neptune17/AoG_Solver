#!/bin/bash

# 批量运行 main 程序的脚本
# 用法: ./batch_run.sh <puzzles_folder_path>

# 定义颜色
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

if [ $# -ne 1 ]; then
    echo "用法: $0 <puzzles_folder_path>"
    echo "示例: $0 puzzles/Zone1/1-single-shape"
    echo "       $0 puzzles/Zone1"
    exit 1
fi

PUZZLE_DIR="$1"

# 检查目录是否存在
if [ ! -d "$PUZZLE_DIR" ]; then
    echo "错误: 目录 '$PUZZLE_DIR' 不存在"
    exit 1
fi

# 检查 main 是否存在
if [ ! -x "./main" ]; then
    echo "错误: ./main 不存在或不可执行"
    echo "请先编译: make"
    exit 1
fi

# 创建日志输出目录
LOG_DIR="${PUZZLE_DIR}/logs"
mkdir -p "$LOG_DIR"

echo "开始批量处理..."
echo "Puzzle 目录: $PUZZLE_DIR"
echo "日志目录: $LOG_DIR"
echo "扫描 .puz 文件..."
echo "----------------------------------------"

# 使用 find 命令递归查找所有 .puz 文件
# 首先检查是否直接包含 .puz 文件
direct_puz_count=$(find "$PUZZLE_DIR" -maxdepth 1 -name "*.puz" | wc -l)

if [ $direct_puz_count -gt 0 ]; then
    # 如果直接包含 .puz 文件，只处理这些文件
    puz_files=$(find "$PUZZLE_DIR" -maxdepth 1 -name "*.puz" | sort)
else
    # 否则递归查找所有子目录中的 .puz 文件
    puz_files=$(find "$PUZZLE_DIR" -name "*.puz" | sort)
fi

# 检查是否找到了 .puz 文件
puz_count=$(echo "$puz_files" | grep -c "\.puz" || true)
if [ $puz_count -eq 0 ]; then
    echo "警告: 在 $PUZZLE_DIR 及其子目录中没有找到 .puz 文件"
    exit 0
fi

echo "找到 $puz_count 个 puzzle 文件"
echo "----------------------------------------"

# 计数器
total=0
success=0
failed=0

# 计数器
total=0
success=0
failed=0

# 记录总体开始时间
total_start_time=$(date +%s.%N)

# 遍历所有找到的 .puz 文件
while IFS= read -r puz_file; do
    # 检查文件是否存在
    if [ ! -f "$puz_file" ]; then
        continue
    fi

    # 获取文件名（不含路径）
    filename=$(basename "$puz_file")
    # 获取不含扩展名的文件名
    basename="${filename%.puz}"

    # 获取相对于 PUZZLE_DIR 的路径
    rel_path="${puz_file#$PUZZLE_DIR/}"
    # 获取文件的目录路径（相对于 PUZZLE_DIR）
    file_dir=$(dirname "$rel_path")

    # 如果文件在子目录中，创建对应的日志目录结构
    if [ "$file_dir" != "." ]; then
        log_dir="$LOG_DIR/$file_dir"
        mkdir -p "$log_dir"
        log_file="$log_dir/${basename}.log"
    else
        log_file="$LOG_DIR/${basename}.log"
    fi

    total=$((total + 1))

    # 显示相对路径作为标识
    if [ "$file_dir" != "." ]; then
        echo "[$total] 处理: $file_dir/$filename"
    else
        echo "[$total] 处理: $filename"
    fi

    # 记录开始时间
    start_time=$(date +%s.%N)

    # 运行 main 并保存输出，设置10秒超时
    timeout 10s ./main < "$puz_file" > "$log_file" 2>&1
    exit_code=$?

    # 记录结束时间
    end_time=$(date +%s.%N)
    # 计算运行时间（秒）
    elapsed=$(echo "$end_time - $start_time" | bc)

    # 检查是否超时
    if [ $exit_code -eq 124 ]; then
        printf "    \033[0;31m✗ 超时终止 - 运行时间超过10秒\033[0m - 耗时: ${elapsed}s\n"
        echo "    日志: $log_file"
        failed=$((failed + 1))
        continue
    fi

    # 检查程序是否正常完成
    if [ $exit_code -eq 0 ]; then
        # 检查 .log 文件中是否包含 SOLUTION
        if grep -q "^SOLUTION$" "$log_file"; then
            printf "    \033[0;32m✓ 程序运行成功\033[0m - 耗时: ${elapsed}s\n"
            printf "    日志: $log_file\n"

            # 对比 SOLUTION 部分
            # 提取 .puz 文件中的 SOLUTION 部分
            puz_solution=$(sed -n '/^SOLUTION$/,$p' "$puz_file" | tail -n +2)

            # 提取 .log 文件中的 SOLUTION 部分
            log_solution=$(sed -n '/^SOLUTION$/,$p' "$log_file" | tail -n +2)

            # 对比两个 SOLUTION 部分
            if [ "$puz_solution" = "$log_solution" ]; then
                printf "    \033[0;32m✓✓ 答案正确 - 与标准答案一致\033[0m\n"
                success=$((success + 1))
            else
                printf "    \033[0;31m✗✗ 答案错误 - 与标准答案不一致\033[0m\n"
                echo "       标准答案行数: $(echo "$puz_solution" | wc -l), 输出答案行数: $(echo "$log_solution" | wc -l)"
                failed=$((failed + 1))
            fi
        else
            printf "    \033[0;31m✗ 程序未输出 SOLUTION - 可能解题失败\033[0m - 耗时: ${elapsed}s\n"
            echo "    日志: $log_file"
            failed=$((failed + 1))
        fi
    else
        printf "    \033[0;31m✗ 程序运行失败 (退出码: $exit_code)\033[0m - 耗时: ${elapsed}s\n"
        echo "    日志: $log_file"
        failed=$((failed + 1))
    fi
done <<< "$puz_files"

echo "----------------------------------------"
# 记录总体结束时间
total_end_time=$(date +%s.%N)
total_elapsed=$(echo "$total_end_time - $total_start_time" | bc)

printf "批量处理完成!\n"
printf "总计: $total | \033[0;32m成功: $success\033[0m | \033[0;31m失败: $failed\033[0m\n"
echo "总耗时: ${total_elapsed}s"
echo "所有日志保存在: $LOG_DIR"
