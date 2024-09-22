#include <tree_sitter\api.h>
#include <tree_sitter\highlight.h>
#include <tree-sitter-javascript.h>

// The API is thread specific. TS has some thread issues,
// so we expect to get called on the same thread always.

#define MAX_NODE_RESULTS 1000
#define MAX_SCM_BUFFER_SIZE 10000

enum Language {
    NONE,
    JAVASCRIPT,
};

typedef struct Context {
    enum Language language;
    const TSLanguage* tsls[1];
    uint32_t tsls_length;
    const char* scmpath[1];
    uint32_t scmpath_length;
    const char* scm[1];
    uint32_t scm_length; // number of entries in scm array
    uint32_t scm_sizes[1]; // the lengths of each pointer in scm array
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

__declspec(dllexport) Context* initialize();
__declspec(dllexport) bool set_language(Context* ctx, enum Language language);
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
__declspec(dllexport) NodeList* get_syntax(Context* ctx, uint32_t start, uint32_t end);
__declspec(dllexport) bool get_syntax_cb(Context* ctx, uint32_t start_row, uint32_t start_column, uint32_t end_row, uint32_t end_column, void (*syntax_callback)(uint32_t, uint32_t, uint32_t, uint32_t, bool, const char*));
__declspec(dllexport) void free_syntax(NodeList* node_list);
__declspec(dllexport) void free_context(Context* ctx);
__declspec(dllexport) void print_syntax_tree(Context* ctx);
__declspec(dllexport) bool get_highlights(Context* ctx, char* source);

void get_syntax_loop(TSNode node, NodeList* node_list);
void get_syntax_loop_cb(TSNode node, void* syntax_callback);
bool read_file(const char* path, char* out, uint32_t* out_len);
