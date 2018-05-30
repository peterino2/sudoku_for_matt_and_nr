// Code kata exercise, fast sudoku solver
// Maintain fast cache-safe code
//
#include<string.h>
#include<stdio.h>
#include<stdint.h>

#ifdef __cplusplus
#include<chrono>
#include<iostream>
#endif


#define DEBUGMODE
#undef DEBUGMODE
#ifdef DEBUGMODE
#endif
#define SETBIT(X, N) (X |= (1 << N) )
#define CLEARBIT(X, N) (X &= ~(1 << N))

char g_puzzleString[128] = {0};

typedef struct {
	int col;
	int row;
	int square;
} CRS_t;

CRS_t calc_crs_from_index(int indx){
	CRS_t ret = {indx % 9, indx / 9, 0};
	// Get the square it resides in;
	ret.square = (indx / 27) * 3 + (indx % 9) / 3;
	return ret;
}

void printInterlaced_grid_moves(int8_t *grid_moves, char* solutionString){
	int i = 0;
	while(*solutionString){
		if(*solutionString != '\n'){
			printf("%c%c%c",i%9? ' ': '\n',*solutionString, *grid_moves + '0');

			grid_moves += 1;
			i += 1;
		}
		solutionString += 1;
	}
}

void build_solution(int nsteps, CRS_t *steps, int *values, char *puzzleString, char* solutionString){
	memcpy(solutionString, puzzleString, 128);
	for(int i = 0; i < nsteps; i++){
		int stringOffset = steps[i].row * 10 + steps[i].col;
		solutionString[stringOffset] = values[i] + '0';
	}
}

void print16Bit(int val){
	char cstring[17];
	for ( int i = 15; i >= 0 ; i--) {
		cstring[15-i] = (1 << i) & val ? '1' : '0';
	}
	cstring[16] = 0;
	printf("val %d : %s\n",val, cstring);
}
void print_crs_constraint(int *constraint_array){
    for(int i = 0; i < 9; i++) {
        print16Bit(constraint_array[i]);
    }
}

int calc_free_moves(int index, int *colStats, int *rowStats, int *squareStats, int8_t *grid_moves){
	int filled = 0;
	if(grid_moves[index] != 0){
		CRS_t crs = calc_crs_from_index(index);
		for (int value  = 1; value < 10 ; value++) {
			if(
				// Constraints for wether or not a value is free or nto
				((1 << value) & colStats[crs.col]) ||
				((1 << value) & rowStats[crs.row]) ||
				((1 << value) & squareStats[crs.square])
				){
				} else {
					
					filled += 1;		
				}
		}
		if (filled == 0){
			return -1;
		} else{  
			return filled;
		}
	}else{
		return 0;
	}

}

void unmake_move(
	CRS_t crs, 
	int value, 	
	int *colStats, 
	int *rowStats,
	int *squareStats,
	int8_t *grid_moves
){
	CLEARBIT(colStats[crs.col], value);
	CLEARBIT(rowStats[crs.row], value);
	CLEARBIT(squareStats[crs.square], value);
	int indx =  crs.row * 9 + crs.col;
	// Now traverse through all others and calculate the free moves
	// Traverse same row
	grid_moves[indx] = 9;
	for( int i =  crs.row * 9 ; i < crs.row * 9 + 9; i++){
		grid_moves[i] = calc_free_moves(i, colStats, rowStats, squareStats, grid_moves);
	}

	// Traverse same column
	for( int i =  crs.col ; i< 81; i += 9){
		grid_moves[i] = calc_free_moves(i, colStats, rowStats, squareStats, grid_moves);
	}

	// Traverse all from same square
	int tl_row = crs.row / 3;
	tl_row = tl_row * 3;
	int tl_col = crs.col / 3;
	tl_col = tl_col * 3;

	for( int i = tl_row * 9 + tl_col; i < tl_row * 9 + tl_col + 3; i++ ){
		grid_moves[i] = calc_free_moves(i, colStats, rowStats, squareStats, grid_moves);
	}
	for( int i = tl_row * 9 + tl_col + 9; i < tl_row * 9 + tl_col + 3 + 9; i++ ){
		grid_moves[i] = calc_free_moves(i, colStats, rowStats, squareStats, grid_moves);
	}
	for( int i = tl_row * 9 + tl_col + 18; i < tl_row * 9 + tl_col + 3 + 18; i++ ){
		grid_moves[i] = calc_free_moves(i, colStats, rowStats, squareStats, grid_moves);
	}
#ifdef DEBUGMODE
	printf("unmove indx: %d value: %d\n", indx, value);
	printf("cols:\n");
	print_crs_constraint(colStats);
	printf("rows:\n");
	print_crs_constraint(rowStats);
	printf("squares:\n");
	print_crs_constraint(squareStats);
#endif
}

int make_move(
	CRS_t crs, 
	int value, 	
	int *colStats, 
	int *rowStats,
	int *squareStats,
	int8_t *grid_moves
){
	int brokenValue = 0;
	int indx =  crs.row * 9 + crs.col;
    grid_moves[indx] = 0;
	SETBIT(colStats[crs.col], value);
	SETBIT(rowStats[crs.row], value);
	SETBIT(squareStats[crs.square], value);
	
	int free_moves = 0;
	// Now traverse through all others and calculate the free moves
	// Traverse same row
	for( int i =  crs.row * 9 ; i < crs.row * 9 + 9; i++){
		grid_moves[i] = calc_free_moves(i, colStats, rowStats, squareStats, grid_moves);
		if(grid_moves[i] < 0){
			brokenValue = 1;
		}
	}
	// Traverse same column
	for( int i =  crs.col ; i< 81; i += 9){
		grid_moves[i] = calc_free_moves(i, colStats, rowStats, squareStats, grid_moves);
		if(grid_moves[i] < 0){
			brokenValue = 1;
		}
	}
	// Traverse all from same square
	int tl_row = crs.row / 3;
	tl_row = tl_row * 3;
	int tl_col = crs.col / 3;
	tl_col = tl_col * 3;

	for( int i = tl_row * 9 + tl_col; i < tl_row * 9 + tl_col + 3; i++ ){
		grid_moves[i] = calc_free_moves(i, colStats, rowStats, squareStats, grid_moves);
		if(grid_moves[i] < 0){
			brokenValue = 1;
		}
	}
	for( int i = tl_row * 9 + tl_col + 9; i < tl_row * 9 + tl_col + 3 + 9; i++ ){
		grid_moves[i] = calc_free_moves(i, colStats, rowStats, squareStats, grid_moves);
		if(grid_moves[i] < 0){
			brokenValue = 1;
		}
	}
	for( int i = tl_row * 9 + tl_col + 18; i < tl_row * 9 + tl_col + 3 + 18; i++ ){
		grid_moves[i] = calc_free_moves(i, colStats, rowStats, squareStats, grid_moves);
		if(grid_moves[i] < 0){
			brokenValue = 1;
		}
	}
#ifdef DEBUGMODE
        printf("move indx: %d value: %d\n", indx, value);
printf("cols:\n");
		print_crs_constraint(colStats);
		printf("rows:\n");
		print_crs_constraint(rowStats);
		printf("squares:\n");
		print_crs_constraint(squareStats);
#endif
	grid_moves[indx] = 0;
	return brokenValue;
}

int load_puzzle(
	int *colStats, 
	int *rowStats, 
	int *squareStats, 
	int8_t  *grid_moves,
	char* puzzleString,
	const char* filename
){
	FILE* f; 
	char c;
	int indx = 0;
	CRS_t crs;
	f = fopen(filename, "r");
	if(f){
		while((c = getc(f)) != EOF){
			printf("%c", c);
			*puzzleString = c;
			puzzleString += 1;
			if(c != '\n'){
				if( c!= '0'){
					crs = calc_crs_from_index(indx);
	#ifdef DEBUGMODE/*
					printf("indx: %d\n", indx);
					*/
	#endif
					make_move(
						crs, 
						c - '0',
						colStats,
						rowStats,
						squareStats,
						grid_moves
					);
				}
				indx++;	
			}	
		}
	}

	for (int i = 0; i < 81; i++){
		grid_moves[i] = calc_free_moves(i,colStats,rowStats,squareStats, grid_moves);
	}
}

CRS_t calc_lowest_move_step(int8_t *grid_moves){
	int lowest= 10;
	int lowest_i= 0;
	int filled = 0;
	
	for( int i = 0; i < 81; i++){
		if (grid_moves[i] == 1){
			return calc_crs_from_index(i);// Autoreturn guaranteed move
		}else if(grid_moves[i] > 1){
			if(grid_moves[i] < lowest){
				lowest = grid_moves[i];
				lowest_i = i;
			}
		}else if(grid_moves[i] == 0){
			filled += 1;
		}
	}
	if( filled >= 81) {
		CRS_t ret = { -1, -1, - 1};
		return ret;
	}
	return calc_crs_from_index(lowest_i);
}


void print_all_free(int *colStats, int *rowStats, int * squareStats){
	for(int stepnum = 0; stepnum < 81; stepnum++){
		CRS_t crs = calc_crs_from_index(stepnum);
#ifdef DEBUGMODE
		printf("step %d: ", stepnum);
#endif
		for (int value = 1; value < 10 ; value++) {
            if( // Search for earliest free value and make that move
                ((1 << value) & colStats[ crs.col ]) ||
                ((1 << value) & rowStats[ crs.row ]) ||
                ((1 << value) & squareStats[ crs.square ])
            ){
                } else {
					printf("%d, ", value);
				}
		}
#ifdef DEBUGMODE
		printf(" are all valid\n");
#endif
	}
}


char g_solutionString[128];
int main( int argc, char **argv){


	int colStats[9] = {0}; // investigate if uint16 is much faster than 32
	int rowStats[9] = {0};
	int squareStats[9] = {0};
	// number of moves available for each square, 0 means square is filled.
	int8_t grid_moves[81] = {9};
	int walkbacks[81];
	memset(walkbacks, 0, sizeof(walkbacks));
	memset(grid_moves, 9, sizeof(grid_moves));
	CRS_t steps[81]; // should never be more than 81 steps
	int values[81];  
	int stepnum = 0; 
	int stepend;
	int end = 0;
	memset(g_solutionString,0, sizeof(g_solutionString));
	load_puzzle(colStats, rowStats, squareStats, grid_moves, g_puzzleString, argv[1]);
#ifdef __cplusplus
	// If compiled with cpp, then x platform time logging features are available via std::chrono
	// Also note you're gonna need C++ 11 for auto and chrono
	using namespace std::chrono;
	high_resolution_clock::time_point t1 = high_resolution_clock::now();
#endif
    	while (stepnum < 81){
		int this_walkback = 0;
#ifdef DEBUGMODE
		build_solution(stepnum, steps, values, g_puzzleString, g_solutionString);
		printf("stepnum: %d\n%s\n freegrid\n-------",stepnum,g_solutionString);
		printInterlaced_grid_moves(grid_moves, g_solutionString); 
#endif
		steps[stepnum] = calc_lowest_move_step(grid_moves);
        if(steps[stepnum].row < 0)
			break; // returns -1 if the puzzle is complete
		int value;
		int stepIndex = steps[stepnum].row * 9 + steps[stepnum].col;
		int brokenValue = 0;
		int found_value = 0;
        for (value = 1; value < 10 ; value++) {
            if( // Search for earliest free value and make that move
				((1 << value) & colStats[ steps[stepnum].col ]) ||
				((1 << value) & rowStats[ steps[stepnum].row ]) ||
				((1 << value) & squareStats[ steps[stepnum].square ]) 
			){
			
			} else {
					if(this_walkback >= walkbacks[stepnum]){
					found_value = 1;
					brokenValue = make_move(
						steps[stepnum], 
						value,
						colStats,
						rowStats,
						squareStats,
						grid_moves
					);
					
					values[stepnum] = value;
					break;
				} else {
					this_walkback += 1;
				}
				
            }
		}
		if(brokenValue){
			// walkback due broken value
			unmake_move(
				steps[stepnum], 
				values[stepnum],
				colStats, 
				rowStats, 
				squareStats, 
				grid_moves
			);
			walkbacks[stepnum] += 1;
		}else if(!found_value){
        	// walkback here due to unfound value
#ifdef DEBUGMODE
			printf("walkbacking due to unfound value, row: %d, col: %d\n", steps[stepnum].row, steps[stepnum].col);
#endif
			walkbacks[stepnum] = 0;
			walkbacks[stepnum - 1] += 1;
			stepnum -= 1;
			unmake_move(
				steps[stepnum], 
				values[stepnum],
				colStats, 
				rowStats, 
				squareStats, 
				grid_moves
			);
			steps[stepnum].col = 0;
			steps[stepnum].row = 0;
			steps[stepnum].square = 0;
#ifdef DEBUGMODE
			print_all_free(colStats,rowStats,squareStats);
#endif
        }else{
        	stepnum += 1;
		}
	}
#ifdef __cplusplus
	high_resolution_clock::time_point t2 = high_resolution_clock::now();
	auto duration =  duration_cast<std::chrono::microseconds>(t2 -t1).count();
#endif
	build_solution(stepnum, steps, values, g_puzzleString, g_solutionString);
	printf("\n THE SOLUTION IS :\n%s\n",g_solutionString);
#ifdef __cplusplus
	std::cout << "Solution found in "<< duration<< " us\n";
	
#endif
	return 0;
}
