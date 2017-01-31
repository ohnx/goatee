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

void hexDump (char *desc, void *addr, int len) {
    int i;
    unsigned char buff[17];
    unsigned char *pc = (unsigned char*)addr;

    /* Output description if given. */
    if (desc != NULL)
        printf ("%s:\n", desc);

    if (len == 0) {
        printf("  ZERO LENGTH\n");
        return;
    }
    if (len < 0) {
        printf("  NEGATIVE LENGTH: %i\n",len);
        return;
    }

    /* Process every byte in the data. */
    for (i = 0; i < len; i++) {
        /* Multiple of 16 means new line (with line offset). */

        if ((i % 16) == 0) {
            /* Just don't print ASCII for the zeroth line. */
            if (i != 0)
                printf ("  %s\n", buff);

            /* Output the offset. */
            printf ("  %04x ", i);
        }

        /* Now the hex code for the specific character. */
        printf (" %02x", pc[i]);

        /* And store a printable ASCII character for later. */
        if ((pc[i] < 0x20) || (pc[i] > 0x7e))
            buff[i % 16] = '.';
        else
            buff[i % 16] = pc[i];
        buff[(i % 16) + 1] = '\0';
    }

    /* Pad out last line if not exactly 16 characters. */
    while ((i % 16) != 0) {
        printf ("   ");
        i++;
    }

    /* And print the final ASCII bit. */
    printf ("  %s\n", buff);
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
    
    hexDump("goatee_gen out", string_to_sr(out), sizeof(string_real) + string_to_sr(out)->tot);
    
    /* generate the output */
    outFinal = goatee_run(NULL, out);
    
    printf("goatee_run out:\n%s", outFinal);
    
    /* be responsible... free memory */
    string_free(in);
    string_free(out);
    string_free(outFinal);
    
    return 0;
}
