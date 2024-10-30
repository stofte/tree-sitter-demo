#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "tree_sitter\api.h"
#include "tree-sitter-c.h"

#define SHIFT_COUNT_SIZE 100

char* read_file(const char* filename) {
    char *source = NULL;
    FILE *fp = fopen(filename, "r");
    if (fp != NULL) {
        /* Go to the end of the file. */
        if (fseek(fp, 0L, SEEK_END) == 0) {
            /* Get the size of the file. */
            long bufsize = ftell(fp);
            if (bufsize == -1) { /* Error */ }

            /* Allocate our buffer to that size. */
            source = malloc(sizeof(char) * (bufsize + 1));

            /* Go back to the start of the file. */
            if (fseek(fp, 0L, SEEK_SET) != 0) { /* Error */ }

            /* Read the entire file into memory. */
            size_t newLen = fread(source, sizeof(char), bufsize, fp);
            if ( ferror( fp ) != 0 ) {
                fputs("Error reading file", stderr);
            } else {
                source[newLen++] = '\0'; /* Just to be safe. */
            }
        }
        fclose(fp);
    }
    return source;
}

void main() {
    char *source = read_file("sqlite3.txt");
    
    const TSLanguage* lang = tree_sitter_c();
    TSParser* parser = ts_parser_new();
    if (!ts_parser_set_language(parser, lang)) {
        printf("Failed ts_parser_set_language\n");
        return;
    }

    uint32_t sourcesize = strlen(source);
    TSTree* tree = ts_parser_parse_string_encoding(parser, NULL, source, sourcesize, TSInputEncodingUTF8);
    TSNode root = ts_tree_root_node(tree);

    uint32_t err_c = 0;
    uint32_t err_max_idx = 0;
    uint32_t err_max_c = 0;

    
    uint32_t SHIFT_COUNT[SHIFT_COUNT_SIZE] = {0};
    
    LARGE_INTEGER StartingTime, EndingTime, ElapsedMicroseconds;
    LARGE_INTEGER Frequency;
    QueryPerformanceFrequency(&Frequency);

    uint64_t perf_sum = 0;
    uint32_t i;
    uint64_t worst_perf = 0;
    uint32_t worst_shifts = 0;
    uint32_t worst_i = 0;

    // Walk through the full source code in byte increments.
    // For each point, check ts_node_[named]_descendant_for_byte_range,
    // if it's an ERROR node, then walk backwards until we get something else.
    for (i = 0; i < (sourcesize - 1000); i += 1) {
        QueryPerformanceCounter(&StartingTime);
        
        uint32_t i_tmp = i;
        TSNode n = {0};
        uint32_t best_fit_size_diff = 99999999999;
        uint32_t i_best_fit = i;

        while (true) {
            n = ts_node_descendant_for_byte_range(root, i_tmp, i_tmp + 1);
            if (ts_node_symbol(n) != 65535) { // err symbol?
                if (ts_node_is_error(n)) {
                    printf("Had non-err symbol, but was an error?\n");
                }
                uint32_t byte_diff = i - ts_node_start_byte(n);
                if (byte_diff < best_fit_size_diff) {
                    i_best_fit = i_tmp;
                    best_fit_size_diff = byte_diff;
                }
                if (ts_node_named_child_count(n) < 100) {
                    // If we get a node which contains a huge amount of children, 
                    // and if so we want to try and step further back.
                    break;
                }
                if (i - i_tmp  > 10000) {
                    // if we've backtraced too much, then settle for the best fit we had so far ...
                    i_tmp = i_best_fit;
                    printf("Overstepped\n");
                    break;
                }
            } else {
                if (!ts_node_is_error(n)) {
                    printf("Had err symbol, but was not an error?\n");
                }
            }
            i_tmp--;
        }

        uint32_t shift_c = i - i_tmp;
        if (shift_c < SHIFT_COUNT_SIZE) {
            SHIFT_COUNT[shift_c]++;
        } else {
            printf("bigger: %d\n", shift_c);
        }
        
        QueryPerformanceCounter(&EndingTime);
        
        ElapsedMicroseconds.QuadPart = EndingTime.QuadPart - StartingTime.QuadPart;
        ElapsedMicroseconds.QuadPart *= 1000000;
        ElapsedMicroseconds.QuadPart /= Frequency.QuadPart;

        if (worst_perf < ElapsedMicroseconds.QuadPart) {
            worst_perf = ElapsedMicroseconds.QuadPart;
            worst_shifts = shift_c;
            worst_i = i;
        }

        perf_sum += ElapsedMicroseconds.QuadPart;
    }

    for(uint32_t j = 0; j < SHIFT_COUNT_SIZE; j++) {
        printf("%3d => %d\n", j, SHIFT_COUNT[j]);
    }

    printf("Avg time: %lld us\n", perf_sum / i);
    printf("Worst time: %lld us (with %d shifts, index: %d)\n", worst_perf, worst_shifts, worst_i);
    free(source);
}
