#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sudoku.h"

enum ErrorCode {
    NO_ERROR = 0,
    ERR_ERRNO,
    ERR_IO,
    ERR_EOF,
    ERR_MEMORY,
    ERR_NULL_OUTPUT_PARAM,
    ERR_INVALID_SIZE,
    ERR_INVALID_HEADER,
    ERR_INVALID_NUM_CELLS,
    ERR_INVALID_CELL_VALUE
};


/***** Private functions *****/

static int _parse_sudoku_file_header(FILE* file, int* region_n_rows,
                                     int* region_n_cols)
{
    /* read first non-empty line */
    char dummy;
    int n_matches = fscanf(file, "%d %d%[\n\t\r ]",
                           region_n_rows, region_n_cols, &dummy);
    if (n_matches == EOF) {
        return ERR_EOF;
    } else if (n_matches != 3) {
        if (ferror(file)) {
            return ERR_IO;
        } else if (feof(file)) {
            return ERR_EOF;
        }
        return ERR_INVALID_HEADER;
    }

    return NO_ERROR;
}


static int _parse_sudoku_file_cells(FILE* file, Sudoku* sudoku)
{
    int value = 0;
    unsigned sudoku_size = sudoku->region_n_rows * sudoku->region_n_cols;

    for (unsigned i = 0; i < sudoku_size; ++i) {
        for (unsigned j = 0; j < sudoku_size; ++j) {
            errno = 0;
            int n_matches = fscanf(file, "%d", &value);
            if (n_matches == EOF) {
                return ERR_INVALID_NUM_CELLS;
            } else if (n_matches != 1) {
                if (ferror(file)) {
                    return ERR_IO;
                } else if (feof(file)) {
                    return ERR_EOF;
                }
                return ERR_INVALID_CELL_VALUE;
            }
            sudoku->cells[i][j] = value;
            if (value > 0) {
                sudoku->n_fixed_cells += 1;
            }
        }
    }

    return NO_ERROR;
}


int _parse_sudoku_file(FILE* file, Sudoku* sudoku)
{
    int region_n_rows, region_n_cols;

    int ret = _parse_sudoku_file_header(file, &region_n_rows, &region_n_cols);
    if (ret != NO_ERROR) { return ret; }

    ret = sudoku_init(sudoku, region_n_rows, region_n_cols);
    if (ret != NO_ERROR) { return ret; }

    if (ret == NO_ERROR) {
        ret = _parse_sudoku_file_cells(file, sudoku);
    }

    return ret;
}


/****************************/
/***** Public functions *****/
/****************************/


Sudoku* sudoku_new(void)
{
    errno = 0;
    Sudoku* sudoku = (Sudoku*)malloc(sizeof(Sudoku));
    if (sudoku != NULL) {
        sudoku->n_rows = 0;
        sudoku->n_cols = 0;
        sudoku->n_cells = 0;
        sudoku->n_values = 0;

        sudoku->region_n_rows = 0;
        sudoku->region_n_cols = 0;

        sudoku->n_fixed_cells = 0;

        sudoku->cells = NULL;
    }

    return sudoku;
}


int sudoku_init(Sudoku* sudoku, int region_n_rows, int region_n_cols)
{
    errno = 0;  /* clear error state */

    if (region_n_rows <= 0 || region_n_cols <= 0) {
        return ERR_INVALID_SIZE;
    }

    /* allocate rows */
    int sudoku_size = region_n_rows * region_n_cols;
    sudoku->cells = (int**)malloc(sudoku_size * sizeof(int*));
    if (sudoku->cells == NULL) {
        return ERR_MEMORY;
    }

    /* allocate columns */
    for (int i = 0; i < sudoku_size; ++i) {
        sudoku->cells[i] = (int*)malloc(sudoku_size * sizeof(int));

        if (sudoku->cells[i] == NULL) {
            while (i >= 0) {
                free(sudoku->cells[i]);
                sudoku->cells[i--] = NULL;
            }
            free(sudoku->cells);
            return ERR_MEMORY;
        }

        memset(sudoku->cells[i], 0, sudoku_size * sizeof(int));
    }

    sudoku->n_rows = sudoku_size;
    sudoku->n_cols = sudoku_size;
    sudoku->n_cells = sudoku->n_rows * sudoku->n_cols;
    sudoku->n_values = sudoku_size;
    sudoku->region_n_rows = region_n_rows;
    sudoku->region_n_cols = region_n_cols;
    sudoku->n_fixed_cells = 0;
    return NO_ERROR;
}

void sudoku_delete(Sudoku* sudoku)
{
    for (int i = 0; i < sudoku->n_rows; ++i) {
        free(sudoku->cells[i]);
    }
    free(sudoku->cells);
    free(sudoku);
}


int sudoku_parse_file(const char* path, Sudoku* sudoku)
{
    if (sudoku == NULL) {
        return ERR_NULL_OUTPUT_PARAM;
    }

    FILE* f = fopen(path, "r");
    if (f == NULL) {
        return ERR_ERRNO;
    }

    int ret = _parse_sudoku_file(f, sudoku);
    fclose(f);

    return ret;
}


const char* sudoku_translate_error_code(int error_code)
{
    switch (error_code) {
        case NO_ERROR:
            return "No error";
        case ERR_ERRNO:
            return errno == 0 ? "Unknown error state" : strerror(errno);
        case ERR_IO:
            return "Error while performing input/output operations";
        case ERR_EOF:
            return "End of file found and more data was expected";
        case ERR_MEMORY:
            return "Error allocating memory";
        case ERR_NULL_OUTPUT_PARAM:
            return "Expected output parameter cannot be NULL";
        case ERR_INVALID_SIZE:
            return "Invalid size, some dimension is less than or equal to 0";
        case ERR_INVALID_HEADER:
            return "Invalid sudoku file header.\n"
                "It must be a line with format: <num_rows> <num_cols>";
        case ERR_INVALID_NUM_CELLS:
            return "Number of rows/cols does not match the header";
        case ERR_INVALID_CELL_VALUE:
            return "One or more cells contain an invalid value";
        default:
            return "Unkown error (This should not happen)";
    }
}


static void _print_region_separator(FILE* outf, const Sudoku* sudoku,
                                    unsigned line_len)
{
    const unsigned sep_pos = (line_len - 1) / sudoku->region_n_rows;
    for (unsigned j = 0; j < line_len; ++j) {
        if (j % sep_pos == 0) {
            fputc('+', outf);
        } else {
            fputc('-', outf);
        }
    }
}

void sudoku_print(FILE* outf, const Sudoku* sudoku)
{
    /* number of digits to represent biggest number in base 10 */
    unsigned n_digits = 0;
    unsigned temp = sudoku->n_rows;
    while (temp > 0) {
        n_digits += 1;
        temp /= 10;
    }

    const unsigned line_len = sudoku->n_cols * n_digits /* cells content */
                            + sudoku->n_cols            /* space after value */
                            + sudoku->region_n_rows * 2 /* region separator */
                            + 1;                        /* closing char '|' */

    /* print sudoku */
    for (int i = 0; i < sudoku->n_rows; ++i) {
        if (i % sudoku->region_n_rows == 0) {
            // horizontal region separator
            _print_region_separator(outf, sudoku, line_len);
            fputc('\n', outf);
        }

        for (int j = 0; j < sudoku->n_cols; ++j) {
            // vertical region separator
            if (j % sudoku->region_n_cols == 0) {
                fputs("| ", outf);
            }

            // cell value
            if (sudoku->cells[i][j] == 0) {
                fprintf(outf, "%*c", n_digits, ' ');
            } else {
                fprintf(outf, "%*d", n_digits, sudoku->cells[i][j]);
            }

            fputs(" ", outf);
        }
        fputs("|\n", outf);
    }

    _print_region_separator(outf, sudoku, line_len);
    fputc('\n', outf);
}
