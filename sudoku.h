
#ifndef _SUDOKU_IO_H_
#define _SUDOKU_IO_H_

#include <stdio.h>


/**
 *
 */
typedef struct
{
    int n_cols;
    int n_rows;
    int n_cells;
    int n_values;

    int region_n_cols;
    int region_n_rows;

    int n_fixed_cells;

    int **cells;
} Sudoku;

enum ErrorCode;

/**
 *
 */
Sudoku* sudoku_new();

/**
 *
 */
int sudoku_init(Sudoku*, int region_n_cols, int region_n_rows);

/**
 *
 */
void sudoku_delete(Sudoku* sudoku);

/**
 *
 */
int sudoku_parse_file(const char* path, Sudoku* sudoku);

/**
 *
 */
const char* sudoku_translate_error_code(int error_code);

/**
 *
 */
void sudoku_print(FILE* outf, const Sudoku* sudoku);

#endif
