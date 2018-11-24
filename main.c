#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "run_solver.h"
#include "sudoku.h"
#include "teacher.h"


int x(Sudoku* s, int i, int j, int k) {
    const int n = s->n_values;
    return i * n * n + j * n + k + 1;
}


void alo(FILE* f, int *vars, int size) {
    /* YOUR CODE HERE */
}


void amo(FILE* f, int *vars, int size) {
    /* YOUR CODE HERE */
}


void eo(FILE* f, int *vars, int size){
    /* YOUR CODE HERE */
}


int main(int argc, char** argv)
{
    if (argc < 2) {
        printf("Usage: %s <sudoku_file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    /* creating and loading the sudoku */
    Sudoku* sudoku = sudoku_new();

    int error_code = sudoku_parse_file(argv[1], sudoku);
    if (error_code == 0) {
        printf("Loaded sudoku\n");
        sudoku_print(stdout, sudoku);
    } else {
        printf("Error: %s\n", sudoku_translate_error_code(error_code));
        return EXIT_FAILURE;
    }

    /* create your formula here. */
    const int n = sudoku->n_values;
    const int n_fixed = sudoku->n_fixed_cells;
    int num_vars = n * n * n;                        /* ADJUST AS NECESSARY */
    int num_clauses = n * n * (1 + (n * (n-1) / 2)); /* ADJUST AS NECESSARY */

    FILE* f = fopen("instance.cnf","w");      /* file to save the instance */
    fprintf(f, "p cnf %d %d\n", num_vars, num_clauses); /* instance header */

    int* vars = (int*) malloc(n * sizeof(int));

    /* only one value per cell */
    fprintf(f, "c Cell constraints\n");
    for(int i = 0; i < n; i++) {
        for(int j = 0; j < n; j++) {
            for(int k = 0; k < n; k++){
                vars[k] = x(sudoku, i, j, k);
            }
            teacher_eo(f, vars, n);
        }
    }

    fprintf(f, "c Row constraints.\n");
    /* YOUR CODE HERE */

    fprintf(f, "c Column constraints.\n");
    /* YOUR CODE HERE */

    fprintf(f, "c Region constraints.\n");
    /* YOUR CODE HERE */

    fprintf(f, "c Fixed number constraints.\n");
    /* YOUR CODE HERE */

    /* formula created: clean up & close the instance file */
    free(vars);
    fclose(f);

    /* solve the formula */
    int* model = (int*)malloc(sizeof(int) * num_vars);
    /*RunSolverCode rs_code = run_solver("./glucose -model", "instance.cnf",
                                         model);*/
    RunSolverCode rs_code = run_solver("./picosat", "instance.cnf",
                                       model);
    switch (rs_code) {
        case RUN_SOLVER_SAT:   /* formula is SAT, a solution has been found */
            printf("Formula is SAT. Model is:\n");
            for (int i = 0; i < num_vars; ++i) {
                printf("%d ", model[i]);
            }
            printf("\n");

            /* fill sudoku->cells using the model here */
            for(int i = 0; i < n; i++) {
                for(int j = 0; j < n; j++) {
                    for(int k = 0; k < n; k++) {
                        if (model[i * n * n + j * n + k] > 0) {
               	            sudoku->cells[i][j] = k+1;
                        }
                    }
                }
            }

            /* print the sudoku solution recovered from the model */
            sudoku_print(stdout, sudoku);
            break;
        case RUN_SOLVER_UNSAT:  /* formula is UNSAT, there is no solution */
            printf("Formula is UNSAT\n");
            break;
        case RUN_SOLVER_UNKNOWN:
            printf("Solver reported UNKNOWN\n");
            break;
        default:
            printf("something unexpected happened :(\n");
    }

    /* clean up and exit */
    free(model);
    sudoku_delete(sudoku);

    return EXIT_SUCCESS;
}