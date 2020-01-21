g++ sudoku.c -Ofast -std=c++11 -o ofast_sudoku.out
g++ sudoku.c -std=c++11 -o sudoku.out
g++ sudoku.c -std=c++11 -DDEBUGMODE=1 -o sudoku_debug.out

echo "usage: use ofast_sudoku.out <file_name> to run the test on that file"


