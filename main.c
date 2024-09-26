#include <windows.h>
#include <stdio.h>
#include "tslib.h"

char* getbuffer(void *payload, uint32_t byte_index, TSPoint position, uint32_t *bytes_read);
void hl_callback(uint32_t byte_start, uint32_t byte_end, const char* capture_name);

void main() {
    char* source_code = "var x = 42;";
    Context* ctx = initialize(true);
    set_language(ctx, 1, "tree-sitter-javascript/queries/highlights.scm");
    if (!parse_string(ctx, source_code, strlen(source_code), TSInputEncodingUTF8)) {
        printf("Failed parse_string.\n");
        return;
    }
	
	print_syntax_tree(ctx);
	
    NodeList* nl = get_syntax(ctx, 8, 10);
    for (int i = 0; i < nl->length; i++) {
        SyntaxNode sn = nl->list[i];
        char *named_str = sn.named ? "T" : "F";
        printf("syntax before: [%d:%d]-[%d:%d][%s] => %s\n", sn.start.row, sn.start.column, sn.end.row, sn.end.column, named_str, sn.nodetype);
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

    NodeList* nl2 = get_syntax(ctx, 8, 12);
    for (int i = 0; i < nl2->length; i++) {
        SyntaxNode sn = nl2->list[i];
        char *named_str = sn.named ? "T" : "F";
        printf("syntax after: [%d:%d]-[%d:%d][%s] => %s\n", sn.start.row, sn.start.column, sn.end.row, sn.end.column, named_str, sn.nodetype);
    }
	
	print_syntax_tree(ctx);

    printf("query matches (for just quote):\n\n");
    get_highlights(ctx, 8, 1, hl_callback);
    printf("query matches (for whole string):\n\n");
    get_highlights(ctx, 8, 4, hl_callback);
    printf("query matches (for whole buffer):\n\n");
    get_highlights(ctx, 4, 2, hl_callback);
}

char* global_source = "var x = \"42\";";

void hl_callback(uint32_t byte_start, uint32_t byte_length, const char* capture_name) {
    printf("%s:", capture_name);
    printf("%.*s\n", byte_length, global_source + byte_start);
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
