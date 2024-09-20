#include <stdio.h>
#include "tslib.h"

Context* initialize() {
    Context* ctx = malloc(sizeof(Context));
    ctx->current = NONE;
    ctx->tsls[0] = tree_sitter_javascript();
	ctx->scmpath[0] = "/path/to/tree-sitter-rust/queries/highlights.scm";
    ctx->tsls_length = 1;
    ctx->parser = ts_parser_new();
    ctx->tree = NULL;
    return ctx;
}

bool set_language(Context* ctx, enum Language language) {
    switch (language) {
        case JAVASCRIPT:
			ctx->current = language;
			
            return ts_parser_set_language(ctx->parser, ctx->tsls[0]);
        case NONE:
            return false;
    }
    return false;
}

bool parse_string(Context* ctx, char* string, uint32_t string_length, TSInputEncoding encoding) {
    ctx->tree = ts_parser_parse_string_encoding(
        ctx->parser,
        NULL,
        string,
        string_length,
        encoding
    );
    return ctx->tree != NULL;
}



void print_syntax_tree(Context* ctx) {
    TSNode root_node = ts_tree_root_node(ctx->tree);
    printf("%s\n\n\n", ts_node_string(root_node));	
}

bool edit_string(
        Context* ctx, 
        // 9x uint32 params goes here
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
    ) {
    TSPoint start;
    start.row = start_point_row;
    start.column = start_point_column;

    TSPoint old_end_point;
    old_end_point.row = old_end_point_row;
    old_end_point.column = old_end_point_column;

    TSPoint new_end_point;
    new_end_point.row = new_end_point_row;
    new_end_point.column = new_end_point_column;

    TSInputEdit edit;
    edit.start_byte = start_byte;
    edit.old_end_byte = old_end_byte;
    edit.new_end_byte = new_end_byte;
    edit.start_point = start;
    edit.old_end_point = old_end_point;
    edit.new_end_point = new_end_point;
    ts_tree_edit(ctx->tree, &edit);
    TSInput input;
    input.encoding = encoding;
    input.read = buffer_callback;
    TSTree* newtree = ts_parser_parse(ctx->parser, ctx->tree, input);
    bool success = newtree != NULL;
    TSTree* oldtree = ctx->tree;
    if (success) {
        ctx->tree = newtree;
    }
    return success;
}

bool get_syntax_cb(Context* ctx, uint32_t start_row, uint32_t start_column, uint32_t end_row, uint32_t end_column, void (*syntax_callback)(uint32_t, uint32_t, uint32_t, uint32_t, bool, const char*)) {
    TSNode root_node = ts_tree_root_node(ctx->tree);
    get_syntax_loop_cb(root_node, syntax_callback);
    return true;
}

NodeList* get_syntax(Context* ctx, uint32_t start_byte, uint32_t end_byte) {
    printf("GET_SYNTAX: %d - %d\n", start_byte, end_byte);
    SyntaxNode* syntax_nodes = malloc(MAX_NODE_RESULTS * sizeof(SyntaxNode));
    NodeList* node_list = malloc(sizeof(NodeList));
    node_list->list = syntax_nodes;
    node_list->length = 0;
    TSNode root_node = ts_tree_root_node(ctx->tree);
    TSNode range_node = ts_node_descendant_for_byte_range(root_node, start_byte, end_byte);
    get_syntax_loop(range_node, node_list);
    return node_list;
}

void free_syntax(NodeList* node_list) {
    free(node_list);
}

void free_context(Context* ctx) {
    free(ctx);
}

void get_syntax_loop(TSNode node, NodeList* node_list) {
    // todo return NULL if we exceed the allocated memory
    const char *nodetype = ts_node_type(node);
    TSPoint nstart = ts_node_start_point(node);
    TSPoint nend = ts_node_end_point(node);
    bool is_named = ts_node_is_named(node);
    char *named_str = is_named ? "T" : "F";
    uint32_t idx = node_list->length;
    node_list->list[idx].start = nstart;
    node_list->list[idx].end = nend;
    node_list->list[idx].named = is_named;
    node_list->list[idx].nodetype = nodetype;
    node_list->length++;
    printf("[%d:%d]-[%d:%d] => %s\n", nstart.row, nstart.column, nend.row, nend.column, nodetype);
    uint32_t child_count = ts_node_child_count(node);
    for(int i = 0; i < child_count; i++) {
        get_syntax_loop(ts_node_child(node, i), node_list);
    }    
}

void get_syntax_loop_cb(TSNode node, void (*syntax_callback)(uint32_t, uint32_t, uint32_t, uint32_t, bool, const char*)) {
    const char *nodetype = ts_node_type(node);
    TSPoint nstart = ts_node_start_point(node);
    TSPoint nend = ts_node_end_point(node);
    bool is_named = ts_node_is_named(node);
    char *named_str = is_named ? "T" : "F";
    syntax_callback(nstart.row, nstart.column, nend.row, nend.column, is_named, nodetype);
    uint32_t child_count = ts_node_child_count(node);
    for(int i = 0; i < child_count; i++) {
        get_syntax_loop_cb(ts_node_child(node, i), syntax_callback);
    }
}
