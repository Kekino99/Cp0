#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "run_solver.h"

const char* OUT_FILE_TEMPLATE = "solver_out.XXXXXX";
const int PARSE_BUF_SIZE = 100;


static RunSolverCode _process_solver_result(FILE* solver_stream,
                                            int *model)
{
    if (fseek(solver_stream, 0L, SEEK_SET) == -1) {
        return RUN_SOLVER_ERR_STREAM;
    }

    if (model != NULL) { model[0] = 0; }  /* initialize model */

    RunSolverCode code = RUN_SOLVER_UNKNOWN;
    char buf[PARSE_BUF_SIZE];  /* more than enough for our purposes */
    int model_index = 0;
    int literal = 0;

    for (buf[0] = fgetc(solver_stream);
         !feof(solver_stream) && !ferror(solver_stream);
         buf[0] = fgetc(solver_stream))
    {
        if (buf[0] == 's') {                          /* solution line */
            fgets(buf, PARSE_BUF_SIZE, solver_stream);
            if (strstr(buf, "UNSATISFIABLE") != NULL) {
                code = RUN_SOLVER_UNSAT;
            } else if (strstr(buf, "SATISFIABLE") != NULL) {
                code = RUN_SOLVER_SAT;
            }
        } else if (buf[0] == 'v' && model != NULL) {  /* model line */
            int n_matches = fscanf(solver_stream, "%d", &literal);
            while (n_matches == 1) {
                if (literal == 0) {
                    break;
                } else {
                    model[model_index++] = literal;
                    n_matches = fscanf(solver_stream, "%d", &literal);
                }
            }
        } else if (!isspace(buf[0])) {                /* consume line */
            while (fgetc(solver_stream) != '\n') {
                if (feof(solver_stream) || ferror(solver_stream)) {
                    break;
                }
            }
        }
    }

    if (ferror(solver_stream)) {  /* correct code in case of IO error */
        code = RUN_SOLVER_ERR_STREAM;
    }
    return code;
}


static RunSolverCode _run_solver(const char* command, FILE* solver_stream,
                                 int *model)
{
    int sys_code = system(NULL);
    if (sys_code == 0) {
        return RUN_SOLVER_ERR_NO_SHELL;
    }

    sys_code = system(command);
    if (sys_code == -1) {
        return RUN_SOLVER_ERR_CHILD;
    }

    return _process_solver_result(solver_stream, model);
}


RunSolverCode run_solver(const char* solver, const char* instance, int *model)
{
    const size_t solver_len = strlen(solver);
    const size_t instance_len = strlen(instance);
    const size_t out_file_len = strlen(OUT_FILE_TEMPLATE);
    const size_t n_separators = 5;  // two spaces + " > "
    const size_t command_len = solver_len + instance_len
                             + out_file_len + n_separators;

    const size_t out_file_size = (out_file_len + 1) * sizeof(char);
    const size_t command_size = (command_len + 1) * sizeof(char);

    RunSolverCode code = RUN_SOLVER_UNKNOWN;
    FILE* solver_stream = NULL;

    /* allocate memory */
    char *out_file = (char*)malloc(out_file_size);
    char *command = (char*)malloc(command_size);
    if (out_file == NULL || command == NULL) {
        code = RUN_SOLVER_ERR_MEMORY;
        goto done;
    }

    /* create temporary file */
    strcpy(out_file, OUT_FILE_TEMPLATE);
    int solver_stream_fd = mkstemp(out_file);
    if (solver_stream_fd < 0) {
        code = RUN_SOLVER_ERR_STREAM;
        goto done;
    }
    solver_stream = fdopen(solver_stream_fd, "r");  /* should never fail */

    /* make command, enough memory was allocated previously */
    strcpy(command, solver);
    strcat(command, " ");
    strcat(command, instance);
    strcat(command, " ");
    strcat(command, " > ");
    strcat(command, out_file);

    /* run solver (for real) */
    code = _run_solver(command, solver_stream, model);

done:
    if (solver_stream != NULL) {
        fclose(solver_stream);  /* closes O.S level fd */
        remove(out_file);
    }

    free(out_file);
    free(command);
    return code;
}
