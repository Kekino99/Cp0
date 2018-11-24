
#ifndef _RUN_SOLVER_H_
#define _RUN_SOLVER_H_

typedef enum {
    RUN_SOLVER_SAT,
    RUN_SOLVER_UNSAT,
    RUN_SOLVER_UNKNOWN,
    RUN_SOLVER_ERR_MEMORY,   /* could not allocate memory */
    RUN_SOLVER_ERR_STREAM,   /* could not create/read the result file */
    RUN_SOLVER_ERR_NO_SHELL, /* system() cannot be executed (not supported) */
    RUN_SOLVER_ERR_CHILD,    /* system() could not run the command */
} RunSolverCode;

/**
 * Runs the provided solver on `instance`. If `instance` is satisfiable
 * and `model` is not NULL, the variables assignment is stored in `model`.
 *
 * `model` will always be terminated with the dummy value '0', thereby it must
 * have enough room to fit all the variables plus 1.
 *
 * Any return code different from RUN_SOLVER_SAT, RUN_SOLVER_UNSAT and
 * RUN_SOLVER_UNKNOWN, indicates that an error ocurred.
 */
RunSolverCode run_solver(const char* solver, const char* instance, int *model);

#endif
