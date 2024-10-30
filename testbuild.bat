CL ^
-I tree-sitter\lib\include ^
-I tree-sitter-c\bindings\c\ ^
/WX ^
/MD /O2 /DTREE_SITTER_HIDE_SYMBOLS ^
tree-sitter\lib\src\lib.c ^
tree-sitter-c\src\parser.c ^
ws2_32.lib ntdll.lib advapi32.lib userenv.lib bcrypt.lib kernel32.lib user32.lib ^
test.c /Foout\ /Feout\test.exe
