#ifndef TSLIB_H
#define TSLIB_H

#include "tree_sitter\api.h"
#include "tree_sitter\highlight.h"
#include "tree-sitter-javascript.h"
#include "tree-sitter-c.h"

// The API is thread specific. TS has some thread issues,
// so we expect to get called on the same thread always.

#define MAX_NODE_RESULTS 1000
#define MAX_SCM_BUFFER_SIZE 10000

static bool tslib_log_to_stdout = true;

// https://stackoverflow.com/a/26305665/2491
#define LOG(fmt, ...) \
    if (!tslib_log_to_stdout) \
        do { \
            FILE* f = fopen("tslib.log", "a"); \
            if(!f) \
                break ; \
            fprintf(f, fmt, __VA_ARGS__); \
            fclose(f); \
        } while(0); \
    else \
        printf(fmt, __VA_ARGS__);

enum Language {
    NONE,
    JAVASCRIPT,
    C,
};

typedef struct Context {
    enum Language language;
    const TSLanguage* tsls[2];
    uint32_t tsls_length;
    const char* scm[2];
    const TSQuery* query[2];
    uint32_t query_len;
    // only single instance required?
    TSQueryCursor* cursor;
    uint32_t scm_length; // number of entries in scm array
    uint32_t scm_sizes[2]; // the lengths of each pointer in scm array
    TSParser* parser;
    TSTree* tree;
} Context;

typedef struct SyntaxNode {
    TSPoint start;
    TSPoint end;
    bool named;
    const char* nodetype;
} SyntaxNode;

typedef struct NodeList {
    struct SyntaxNode* list;
    uint32_t length;
} NodeList;

__declspec(dllexport) Context* initialize(bool log_to_stdout);
__declspec(dllexport) bool set_language(Context* ctx, enum Language language, char* scm, uint32_t scm_length);
__declspec(dllexport) bool parse_string(Context* ctx, char* string, uint32_t string_length, TSInputEncoding encoding);
__declspec(dllexport) bool edit_string(
    Context* ctx,
    // Dart has multiple issues with structs, so instead of trying to figure out how it works,
    // we'll just transfer all the fields.
    uint32_t start_byte,
    uint32_t old_end_byte,
    uint32_t new_end_byte,
    uint32_t start_point_row,
    uint32_t start_point_column,
    uint32_t old_end_point_row,
    uint32_t old_end_point_column,
    uint32_t new_end_point_row,
    uint32_t new_end_point_column,
    void* buffer_callback,
    TSInputEncoding encoding
);
__declspec(dllexport) char* syntax_tree(Context* ctx);
__declspec(dllexport) bool get_highlights(Context* ctx, uint32_t byte_offset, uint32_t byte_length, void (*hl_callback)(uint32_t, uint32_t, uint32_t, const char*));

const char* copy_string(char* scm, uint32_t scm_length);
#endif
