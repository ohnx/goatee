#include <stdio.h>
#include <stdlib.h>
#include "goatee_gen.h"
#include "goatee_run.h"
#include "goatee_logger.h"

/*
 * you expect() something to happen.
 * if it does not, error with the string given
 */
#define expect(x, y) if(!(x)) error(y);

goatee_logger *gl_cmd = NULL;

void error(char *str) {
    goatee_logger_log(gl_cmd, GLL_FATAL, str);
}

int main(int argc, char *argv[]) {
    char *in, *out, *outFinal;

    expect(argc == 2, "Invalid # of arguments passed!");

    gl_cmd = goatee_logger_new(GLL_INFO);

    /* read in file */
    in = dumpFile(argv[1]);
    expect(in != NULL, "Could not read file!")

    /* generate the template string that will get run through lua */
    out = goatee_gen(in, gl_cmd);
    expect(out != NULL, "Failed to compile template into lua code!");

    /* that's all that's done so far... print out output */
    gl_cmd->log(gl_cmd, GLL_INFO, "goatee_gen finished");
    
    /* generate the output */
    outFinal = goatee_run(NULL, out, gl_cmd);
    expect(outFinal != NULL, "Failed to run compiled template!");
    
    gl_cmd->log(gl_cmd, GLL_INFO, "goatee_run finished");
    
    printf("----\n%s----\n", outFinal);
    
    /* be responsible... free memory */
    string_free(in);
    string_free(out);
    string_free(outFinal);
    
    printf("%s", gl_cmd->messages);
    
    goatee_logger_destroy(gl_cmd);
    
    return 0;
}
