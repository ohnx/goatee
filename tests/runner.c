#include <stdio.h>
#include <stdlib.h>
#include "goatee_gen.h"
#include "goatee_run.h"

/*
 * you expect() something to happen.
 * if it does not, error with the string given
 */
#define expect(x, y) if(!(x)) error(y);

void error(char *str) {
    fputs(str, stderr);
    fputs("\n", stderr);
    exit(-1337);
}

char *dumpFile(const char *filename) {
    char *buffer = 0;
    long length;
    FILE *f;
    
    f = fopen (filename, "rb");
    
    if (f) {
        fseek(f, 0, SEEK_END);
        length = ftell(f);
        fseek(f, 0, SEEK_SET);
        buffer = calloc(length+1, 1);
        if (buffer) {
            fread (buffer, 1, length, f);
            buffer[length] = '\0';
        }
        fclose (f);
    }
    
    if (buffer) return buffer;
    return NULL;
}

int main(int argc, char *argv[]) {
    char *in, *out, *outFinal;
    
    expect(argc == 2, "Invalid # of arguments passed!");

    /* read in file */
    in = dumpFile(argv[1]);
    expect(in != NULL, "Could not read file!")

    /* generate the template string that will get run through lua */
    out = goatee_gen(in);
    expect(out != NULL, "Failed to run goatee_gen!");

    /* that's all that's done so far... print out output */
    printf("goatee_gen out:\n%s", out);
    
    /* generate the output */
    outFinal = goatee_run(NULL, out);
    
    printf("goatee_run out:\n%s", outFinal);
    
    /* be responsible... free memory */
    string_free(in);
    string_free(out);
    string_free(outFinal);
    
    return 0;
}
