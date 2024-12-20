#include <windows.h>
#include <stdio.h>
#include <fcntl.h>
#include <strsafe.h>
#include "tslib.h"

#define HIGHLIGHTS_START_NODE_MAX_CHILD_COUNT 100

void tslogger_log(void* payload, TSLogType log_type, const char *buffer) {
    if (log_type == TSLogTypeParse) {
        LOG("Parse: ");
    } else if (log_type == TSLogTypeLex) {
        LOG("Lex: ");
    }
    LOG("%s\n", buffer);
}

Context* initialize(bool enable_logging, bool log_to_stdout) {
    tslib_log_to_stdout = log_to_stdout;
    tslib_enable_logging = enable_logging;

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
    ctx->cursor = ts_query_cursor_new();
    ctx->tree = NULL;

    TSLogger logger;
    logger.payload = NULL;
    logger.log = &tslogger_log;

    // Enables the ts logger, which is very verbose...
    // ts_parser_set_logger(ctx->parser, logger);

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
            case JAVASCRIPT: ctx->tsls[idx] = tree_sitter_javascript(); break;
            case C: ctx->tsls[idx] = tree_sitter_c(); break;
        }
    }
    if (ctx->query[idx] == NULL) {
        ctx->query[idx] = ts_query_new(ctx->tsls[idx], copy_string(scm, scm_length), scm_length, &query_error_offset, &query_error);
    }
    LOG("set_language passed switch with idx %d\n", idx);
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

bool get_highlights(Context* ctx, uint32_t byte_offset, uint32_t byte_length, void (*hl_callback)(uint32_t, uint32_t, uint32_t, const char*)) {
    int idx = ctx->language - 1;

    // https://github.com/tree-sitter/tree-sitter/discussions/3423
    const TSQuery* query = ctx->query[idx];
    if (query == NULL) {
        LOG("No query found!\n");
        return false;
    }

    TSNode root_node = ts_tree_root_node(ctx->tree);
    TSNode query_node = {0};

    uint32_t byte_idx = byte_offset;

    while (true) {
        // Sometimes, we get back an error node for whatever reason. This node has start byte = 0, which
        // obviously causes havoc further down the line. To work around this, we step backwards in the code,
        // step by step, until we find a valid node.
        query_node = ts_node_descendant_for_byte_range(root_node, byte_idx, byte_idx + 1);
        // Depending on the grammar of the text, we can risk the node having a huge difference between
        // byte_offset and ts_node_start_byte(query_node), which leads to issues with returning
        // way too much information to the user as well. 
        // A good heuristic is that we check the found nodes named child count, and if this is larger
        // than some threshold, we step backwards some more.
        if (ts_node_is_error(query_node) || ts_node_named_child_count(query_node) > HIGHLIGHTS_START_NODE_MAX_CHILD_COUNT) {
            byte_idx--;
        } else {
            break;
        }
    }

    LOG("FOO %d, %d\n", byte_offset, byte_idx);
    if (byte_idx < byte_offset) {
        LOG("get_highlights:shifted byte_offset: %d times\n", (byte_offset - byte_idx));
    }

    TSQueryCursor *cursor = ctx->cursor;

    // assume that query_offset <= byte_offset
    uint32_t query_offset = ts_node_start_byte(query_node);
    uint32_t query_length = (byte_offset - query_offset) + byte_length;

    LOG("get_highlights:adjusted_offsets: %d => %d, %d => %d\n", byte_offset, query_offset, byte_length, query_length);
    
    ts_query_cursor_set_byte_range(cursor, query_offset, query_offset + query_length);
    ts_query_cursor_exec(cursor, query, root_node);
    
    TSQueryMatch match = {0};
    uint32_t capture_index = 0;
    while (ts_query_cursor_next_capture(cursor, &match, &capture_index)) {
        TSNode node = match.captures->node;
        uint32_t captures_index = match.captures->index;
        uint32_t capture_name_len = 0;
        const char *capture_name = ts_query_capture_name_for_id(query, captures_index, &capture_name_len);
        uint32_t start = ts_node_start_byte(node);
        uint32_t end = ts_node_end_byte(node);
        if (end - start < 0) {
            LOG("get_highlights:range_check: (%d - %d) < 0\n", end, start);
        }
        if (end - start > 0) {
            // todo nodes with width 0 have been observed?
            hl_callback(start, end - start, captures_index, capture_name);
        }
    }

    return true;
}
