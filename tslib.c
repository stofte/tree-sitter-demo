#include <windows.h>
#include <stdio.h>
#include <fcntl.h>
#include <strsafe.h>
#include "tslib.h"

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
    ctx->tsls[0] = tree_sitter_javascript();
    ctx->tsls_length = 1;
    ctx->scm[0] = NULL;
    ctx->scm_length = 1;
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

bool set_language(Context* ctx, enum Language language, char* scm_path) {
    LOG("set_language called with %d and scm_path: %s\n", language, scm_path);
    const TSLanguage *tsln = NULL;
    switch (language) {
        case C:
            tsln = tree_sitter_c();
            ctx->language = language;
            break;
        case JAVASCRIPT:
            tsln = tree_sitter_javascript();
            ctx->language = language; 
            break;
        case NONE: return false;
    }
    LOG("set_language passed switch with lang %d\n", ctx->language);
    // Unknown is 0, so everything is shifted once back
    int idx = ctx->language - 1;
    // Check if we have that language's SCM loaded

    if (ctx->scm[idx] == NULL) {
        char* highlights_query = malloc(sizeof(char) * MAX_SCM_BUFFER_SIZE + 1);
        uint32_t highlights_query_len = 0;
        if (!read_file(scm_path, highlights_query, &highlights_query_len)) {
            LOG("Failed read_file\n");
            return false;
        }
        ctx->scm[idx] = highlights_query;
        ctx->scm_sizes[idx] = highlights_query_len;
        LOG("SCM STRING:\n%s\n", highlights_query);
    }
    return ts_parser_set_language(ctx->parser, tsln);
}

bool parse_string(Context* ctx, char* string, uint32_t string_length, TSInputEncoding encoding) {
    LOG("INCOMING STR: \"%s\"\n", string);
    ctx->tree = ts_parser_parse_string_encoding(
        ctx->parser,
        NULL,
        string,
        string_length,
        encoding
    );
    return ctx->tree != NULL;
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
    TSQueryError query_error = {0};
    uint32_t query_error_offset = 0;
    TSQuery *query = ts_query_new(ctx->tsls[idx], ctx->scm[idx], ctx->scm_sizes[idx], &query_error_offset, &query_error);
    if (query == NULL) {
        LOG("ts_query_new failed: %d, %d", query_error, query_error_offset);
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
        int start = ts_node_start_byte(node);
        int end = ts_node_end_byte(node);
        hl_callback(start, end - start, capture_name);
    }

    return true;
}

void ErrorExit(LPCTSTR lpszFunction) 
{ 
    // Retrieve the system error message for the last-error code

    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError(); 

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

    // Display the error message and exit the process

    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, 
        (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR)); 
    StringCchPrintf((LPTSTR)lpDisplayBuf, 
        LocalSize(lpDisplayBuf) / sizeof(TCHAR),
        TEXT("%s failed with error %d: %s"), 
        lpszFunction, dw, lpMsgBuf); 
    printf("%s", (LPCTSTR)lpDisplayBuf);
    // MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK); 

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
    ExitProcess(dw); 
}

bool read_file(const char* path, char* out, uint32_t* out_len) {
    char ReadBuffer[MAX_SCM_BUFFER_SIZE] = {0};
    DWORD bytes_read = 0;
    HANDLE hFile = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) { 
        return false;
    }
    if (!ReadFile(hFile, out, MAX_SCM_BUFFER_SIZE - 1, &bytes_read, NULL)){
        ErrorExit(TEXT("ReadFile"));
        return false;
    }
    *out_len = bytes_read;

    return true;
}
