#include <windows.h>
#include <stdio.h>
#include <fcntl.h>
#include <strsafe.h>
#include "tslib.h"

void testing_ffi(uint32_t* data, uint32_t* data_len, char* str, uint32_t* str_len) {
    uint8_t* foo = malloc(sizeof(uint8_t) * 5000);
    data[0] = 42;
    data[1] = 41;
    data[2] = 40;
    data[3] = 39;
    *data_len = 4;
    str[0] = 'f';
    str[1] = 'o';
    str[2] = 'o';
    str[3] = 0;
    *str_len = 3;
}

void tslogger_log(void* payload, TSLogType log_type, const char *buffer) {
    if (log_type == TSLogTypeParse) {
        LOG("Parse: ");
    } else if (log_type == TSLogTypeLex) {
        LOG("Lex: ");
    }
    LOG("%s\n", buffer);
}

Context* initialize(bool log_to_stdout) {
    LOG("TSLIB INIT: log_to_stdout: %d \n", log_to_stdout);

    // TODO also set encoding here?

    Context* ctx = malloc(sizeof(Context));
    ctx->language = NONE;
    ctx->tsls[0] = NULL;
    ctx->tsls[1] = NULL;
    ctx->tsls_length = 2;
    ctx->query[0] = NULL;
    ctx->query[1] = NULL;
    ctx->query_len = 2;
    ctx->scm[0] = NULL;
    ctx->scm[1] = NULL;
    ctx->scm_length = 2;
    ctx->scm_sizes[0] = 0;
    ctx->parser = ts_parser_new();
    ctx->tree = NULL;

    TSLogger logger;
    logger.payload = NULL;
    logger.log = &tslogger_log;

    // Enables the ts logger, which is very verbose...
    // ts_parser_set_logger(ctx->parser, logger);

    tslib_log_to_stdout = log_to_stdout;
    
    return ctx;
}

bool set_language(Context* ctx, enum Language language, char* scm, uint32_t scm_length) {
    LOG("set_language called with %d and scm_length: %d\n", language, scm_length);
    // Unknown is 0, so everything is shifted once back
    ctx->language = language;
    int idx = ctx->language - 1;
    TSQueryError query_error = {0};
    uint32_t query_error_offset = 0;
    if (ctx->tsls[idx] == NULL) {
        switch (language) {
            case NONE: return false;
            case JAVASCRIPT: 
                ctx->tsls[idx] = tree_sitter_javascript();
                ctx->query[idx] = ts_query_new(ctx->tsls[idx], copy_string(scm, scm_length), scm_length, &query_error_offset, &query_error);
                break;
            case C: 
                ctx->tsls[idx] = tree_sitter_c();
                ctx->query[idx] = ts_query_new(ctx->tsls[idx], copy_string(scm, scm_length), scm_length, &query_error_offset, &query_error);
                break;
        }
    }
    LOG("set_language passed switch with idx %d\n", idx);

    // Check if we have that language's SCM loaded, otherwise copy the scm code to our own buffer
    // if (ctx->scm[idx] == NULL) {
    //     LOG("Trying to init scm properties on ctx\n");
    //     size_t scm_size_t = sizeof(char) * scm_length + 1;
    //     char* scm_copy = malloc(scm_size_t);
    //     LOG("Trying to copy scm string: %d\n", scm_length);
    //     // LOG("Orig SCM: %sXXX\n", scm);
    //     LOG("SCM len: %zd -> %d\n", strlen(scm), scm_length);
    //     // dont understand ...
    //     for(int i = 0; i < scm_length; i++) {
    //         scm_copy[i] = scm[i];
    //     }
    //     // errno_t err = strcpy_s(scm_copy, scm_size_t, scm);
    //     ctx->scm[idx] = scm_copy;
    //     ctx->scm_sizes[idx] = scm_length;
    //     // LOG("SCM COPY: %s\n", scm_copy);
    // }

    return ts_parser_set_language(ctx->parser, ctx->tsls[idx]);
}

const char* copy_string(char* scm, uint32_t scm_length) {
    size_t scm_size_t = sizeof(char) * scm_length + 1;
    char* scm_copy = malloc(scm_size_t);
    for(int i = 0; i < scm_length; i++) {
        scm_copy[i] = scm[i];
    }
    scm_copy[scm_length] = '\0';
    return scm_copy;
}

bool parse_string(Context* ctx, char* string, uint32_t string_length, TSInputEncoding encoding) {
    LOG("INCOMING STR: \"%s\"\nlength: %d\n", string, string_length);
    ctx->tree = ts_parser_parse_string_encoding(
        ctx->parser,
        NULL,
        string,
        string_length,
        encoding
    );
    return ctx->tree != NULL;
}

char* syntax_tree(Context* ctx) {
    TSNode root_node = ts_tree_root_node(ctx->tree);
    return ts_node_string(root_node);
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

bool get_highlights(Context* ctx, uint32_t byte_offset, uint32_t byte_length, void (*hl_callback)(uint32_t, uint32_t, const char*)) {
    int idx = ctx->language - 1;

    // https://github.com/tree-sitter/tree-sitter/discussions/3423
    const TSQuery* query = ctx->query[idx];
    if (query == NULL) {
        LOG("No query found!\n");
        return false;
    }

    TSNode root_node = ts_tree_root_node(ctx->tree);
    TSNode query_node = ts_node_descendant_for_byte_range(root_node, byte_offset, byte_offset + byte_length);
    LOG("query_node: from %d to %d\n", ts_node_start_byte(query_node), ts_node_end_byte(query_node));
    TSQueryCursor *cursor = ts_query_cursor_new();
    ts_query_cursor_set_byte_range(cursor, byte_offset, byte_offset + byte_length);
    ts_query_cursor_exec(cursor, query, root_node);
    
    TSQueryMatch match = {0};
    uint32_t capture_index = 0;
    while (ts_query_cursor_next_capture(cursor, &match, &capture_index)) {
        TSNode node = match.captures->node;
        uint32_t captures_index = match.captures->index;
        uint32_t capture_name_len = 0;
        const char *capture_name = ts_query_capture_name_for_id(query, captures_index, &capture_name_len);
        // LOG("capture_index %d => %p => %s\n", capture_index, capture_name, capture_name);
        int start = ts_node_start_byte(node);
        int end = ts_node_end_byte(node);
        hl_callback(start, end - start, capture_name);
    }

    ts_query_cursor_delete(cursor);

    return true;
}

bool get_highlights2(Context* ctx, uint32_t byte_offset, uint32_t byte_length, uint32_t* data, uint32_t data_buffer_size, uint32_t* data_len, char* str, uint32_t* str_len) {
    // int idx = ctx->language - 1;

    // // https://github.com/tree-sitter/tree-sitter/discussions/3423
    // TSQueryError query_error = {0};
    // uint32_t query_error_offset = 0;
    // TSQuery *query = ts_query_new(ctx->tsls[idx], ctx->scm[idx], ctx->scm_sizes[idx], &query_error_offset, &query_error);
    // if (query == NULL) {
    //     LOG("ts_query_new failed: %d, %d", query_error, query_error_offset);
    //     return false;
    // }

    // TSNode root_node = ts_tree_root_node(ctx->tree);
    // TSNode query_node = ts_node_descendant_for_byte_range(root_node, byte_offset, byte_offset + byte_length);
    // LOG("query_node: from %d to %d\n", ts_node_start_byte(query_node), ts_node_end_byte(query_node));
    // TSQueryCursor *cursor = ts_query_cursor_new();
    // ts_query_cursor_set_byte_range(cursor, byte_offset, byte_offset + byte_length);
    // ts_query_cursor_exec(cursor, query, root_node);
    
    // TSQueryMatch match = {0};
    // uint32_t capture_index = 0;
    // while (ts_query_cursor_next_capture(cursor, &match, &capture_index)) {
    //     TSNode node = match.captures->node;
    //     uint32_t captures_index = match.captures->index;
    //     uint32_t capture_name_len = 0;
    //     const char *capture_name = ts_query_capture_name_for_id(query, captures_index, &capture_name_len);
    //     LOG("capture_index %d => %p => %s\n", capture_index, capture_name, capture_name);
    //     int start = ts_node_start_byte(node);
    //     int end = ts_node_end_byte(node);

    //     hl_callback(start, end - start, capture_name);
    // }

    // ts_query_cursor_delete(cursor);
    // ts_query_delete(query);

    return true;
}
