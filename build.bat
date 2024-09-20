@echo off
mkdir out
set CLCMD=cl ^
-I tree-sitter\highlight\include\ ^
-I tree-sitter\lib\include ^
-I tree-sitter-javascript\bindings\c\ ^
/nologo /Foout\ ^
/MD /O2 /DTREE_SITTER_HIDE_SYMBOLS ^
tslib.c ^
tree-sitter-javascript\src\parser.c tree-sitter-javascript\src\scanner.c tree-sitter\lib\src\lib.c ^
ws2_32.lib ntdll.lib advapi32.lib userenv.lib bcrypt.lib kernel32.lib
set CLCMDEXE=%CLCMD% main.c /Feout\main.exe
set CLCMDLIB=%CLCMD% /LD /Feout\tslib.dll
@echo on
%CLCMDLIB%
%CLCMDEXE%
out\main.exe