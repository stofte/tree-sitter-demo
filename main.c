#include <windows.h>
#include <stdio.h>
#include <time.h>
#include <strsafe.h>
#include "tslib.h"

char* getbuffer(void *payload, uint32_t byte_index, TSPoint position, uint32_t *bytes_read);
void hl_callback(uint32_t byte_start, uint32_t byte_length, const char* capture_name);
bool read_file(const char*, char*, uint32_t*, uint32_t);
void perf_testing();

void main() {
    char* source_code = "var x = 42;";
    Context* ctx = initialize(true);
    char* scm_str = malloc(5000 * sizeof(char));
    uint32_t scm_length = 0;
    if (!read_file("tree-sitter-javascript/queries/highlights.scm", scm_str, &scm_length, 5000)) {
        printf("Failed to read scm\n");
    }
    set_language(ctx, 1, scm_str, scm_length);
    if (!parse_string(ctx, source_code, strlen(source_code), TSInputEncodingUTF8)) {
        printf("Failed parse_string.\n");
        return;
    }
	
    uint32_t startByte = 8;
    uint32_t oldEndByte = 10;
    uint32_t newEndByte = 12;
    uint32_t startPointRow = 0;
    uint32_t startPointColumn = 8;
    uint32_t oldEndPointRow = 0;
    uint32_t oldEndPointColumn = 10;
    uint32_t newEndPointRow = 0;
    uint32_t newEndPointColumn = 12;
    if (!edit_string(
        ctx,
        startByte,
        oldEndByte,
        newEndByte,
        startPointRow,
        startPointColumn,
        oldEndPointRow,
        oldEndPointColumn,
        newEndPointRow,
        newEndPointColumn,
        &getbuffer,
        TSInputEncodingUTF8
    )) {
        printf("Failed edit_string.\n");
        return;
    }

    printf("query matches (for just quote):\n\n");
    get_highlights(ctx, 8, 1, hl_callback);
    printf("query matches (for whole string):\n\n");
    get_highlights(ctx, 8, 4, hl_callback);
    printf("query matches (for whole buffer):\n\n");
    get_highlights(ctx, 0, 13, hl_callback);
    get_highlights(ctx, 0, 13, hl_callback);

    if (!read_file("tree-sitter-javascript/queries/highlights.scm", scm_str, &scm_length, 5000)) {
        printf("Failed to read scm\n");
    }
    
    // perf_testing();
    return;
}

void perf_testing() {
    srand(time(NULL));
    Context* ctx = initialize(true);
    char* scm_str = malloc(5000 * sizeof(char));
    uint32_t scm_length = 0;
    if (!read_file("tree-sitter-javascript/queries/highlights.scm", scm_str, &scm_length, 5000)) {
        printf("Failed to read scm\n");
    }
    printf("scm_length after read_file: %d\n", scm_length);
    set_language(ctx, 1, scm_str, scm_length);

    char* grammar_js_str = malloc(50000 * sizeof(char));
    uint32_t grammar_js_length = 0;
    if (!read_file("tree-sitter-javascript/grammar.js", grammar_js_str, &grammar_js_length, 50000)) {
        printf("Failed to read grammar.js\n");
    }
    if (!parse_string(ctx, grammar_js_str, grammar_js_length, TSInputEncodingUTF8)) {
        printf("Failed parse_string.\n");
        return;
    }
    // count newlines
    int newlines = 1;
    int newlines_idx[5000];
    for(int i = 0; i < grammar_js_length; i++) {
        if (grammar_js_str[i] == '\n') {
            newlines_idx[newlines] = i;
            newlines++;
        }
    }

    printf("Testing perf for grammar.js\n");
    printf("Line count: %d\n", newlines);
    for(int rc = 0; rc < 100; rc++) {
        int ridx = (rand() % (newlines - 55));
        int byte_start = newlines_idx[ridx];
        int byte_end = newlines_idx[ridx + 50];
        printf("Test: byte_start=%d, byte_end=%d\n", byte_start, byte_end);
    }
}

char* global_source = "var x = \"42\";";

void hl_callback(uint32_t byte_start, uint32_t byte_length, const char* capture_name) {
    // printf("%s:", capture_name);
    // printf("%.*s", byte_length, global_source + byte_start);
    // printf(" (capture_id: %p)\n", capture_name);

}

char foo[6] = " \"42\"";

char* getbuffer(void *payload, uint32_t byte_index, TSPoint position, uint32_t *bytes_read) {
    printf("getbuffer called => %d (point: %d/%d)\n", byte_index, position.row, position.column);
    if (byte_index != 7) {
        *bytes_read = 0;
        return NULL;
    } else {
        *bytes_read = 5;
        return foo;
    }
}

TSPoint getpoint(int row, int column) {
    struct TSPoint p;
    p.column = column;
    p.row = row;
    return p;
}

void iterate_contents(TSNode node) {
    const char *nodetype = ts_node_type(node);
    TSPoint nstart = ts_node_start_point(node);
    TSPoint nend = ts_node_end_point(node);
    bool is_named = ts_node_is_named(node);
    char *named_str = is_named ? "T" : "F";
    printf("[%d:%d]-[%d:%d][%s] => %s\n", nstart.row, nstart.column, nend.row, nend.column, named_str, nodetype);
    uint32_t child_count = ts_node_child_count(node);
    for(int i = 0; i < child_count; i++) {
        iterate_contents(ts_node_child(node, i));
    }
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

bool read_file(const char* path, char* out, uint32_t* out_len, uint32_t buffer_size) {
    DWORD bytes_read = 0;
    printf("read_file strlen (before): %zd\n", strlen(out));
    HANDLE hFile = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) { 
        return false;
    }
    if (!ReadFile(hFile, out, buffer_size - 1, &bytes_read, NULL)){
        ErrorExit(TEXT("ReadFile"));
        return false;
    }
    CloseHandle(hFile);
    printf("read_file strlen: %zd\n", strlen(out));
    printf("read_file bytes_read: %d\n", bytes_read);
    *out_len = bytes_read;
    printf("read_file out_len: %d\n", *out_len);

    return true;
}
