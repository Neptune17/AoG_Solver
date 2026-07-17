# The Artisan of Glimmith Puzzle Solver

Automatically search for puzzle answers for game "The Artisan of Glimmith".

- Using the official puzzle encoding: Can be used to solve official and community's puzzles.

- Supports all types of puzzles.

- Current Status on Official Puzzles (time 10s+ marked as fail)

    - Total: 977 / 1231

        - Zone1: 301 / 312

        - Zone2: 377 / 438

        - Zone3: 299 / 481

## Example (single run)

### Input File

puzzles/Zone1/1-single-shape/0008.puz

```
VERSION 1
PUZZLE_VERSION 2
DIFFICULTY 1
SHAPE 1 2
  #
###
SHAPE_BANK 1
DIMENSIONS 5 6
PUZZLE
                
                
+--+  +--+--+   
|..|  |..|..|   
+--+--+--+--+--+
|..|..|..|..|..|
+--+--+--+--+--+
   |..|  |..|..|
   +--+--+--+--+
   |..|..|..|..|
+--+--+--+--+--+
|..|..|..|..|..|
+--+--+--+--+--+
SOLUTION
                
                
+##+  +##+##+   
#  #  #     #   
+  +##+##+  +##+
#        #  #  #
+##+##+##+  +  +
   #  #  #  #  #
   +  +##+##+  +
   #  #  #     #
+##+  +  +##+##+
#     #        #
+##+##+##+##+##+
```

### Bash

```
./main < puzzles/Zone1/1-single-shape/0008.puz
SOLUTION
                
                
+##+  +##+##+   
#  #  #     #   
+  +##+##+  +##+
#        #  #  #
+##+##+##+  +  +
   #  #  #  #  #
   +  +##+##+  +
   #  #  #     #
+##+  +  +##+##+
#     #        #
+##+##+##+##+##+
```

## Example (batch run)

You can directly use it on the puzzle in the game files.

```
./batch_run.sh puzzles/Zone1 > Zone1.ansi
开始批量处理...
Puzzle 目录: puzzles/Zone1
日志目录: puzzles/Zone1/logs
扫描 .puz 文件...
----------------------------------------
找到 312 个 puzzle 文件
----------------------------------------
[1] 处理: 10-same-shape-no-touch/0079.puz
    ✓ 程序运行成功 - 耗时: .008009185s
    日志: puzzles/Zone1/logs/10-same-shape-no-touch/0079.log
    ✓✓ 答案正确 - 与标准答案一致
[2] 处理: 10-same-shape-no-touch/0080.puz
    ✓ 程序运行成功 - 耗时: .007831798s
    日志: puzzles/Zone1/logs/10-same-shape-no-touch/0080.log
    ✓✓ 答案正确 - 与标准答案一致
[3] 处理: 10-same-shape-no-touch/0157.puz
    ✓ 程序运行成功 - 耗时: .010795051s
    日志: puzzles/Zone1/logs/10-same-shape-no-touch/0157.log
    ✓✓ 答案正确 - 与标准答案一致
[4] 处理: 10-same-shape-no-touch/0159.puz
    ✓ 程序运行成功 - 耗时: .007463527s
    日志: puzzles/Zone1/logs/10-same-shape-no-touch/0159.log
    ✓✓ 答案正确 - 与标准答案一致
...
```

## Note

I'm continuously improving the program's efficiency.