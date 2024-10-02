@echo off
mkdir out
cl tree-sitter-c\src\parser.c /Foout\ts_c_parser.obj /c /MD /O2 /nologo
cl tree-sitter-javascript\src\parser.c /Foout\ts_javascript_parser.obj /c /MD /O2 /nologo
cl tree-sitter-javascript\src\scanner.c /Foout\ts_javascript_scanner.obj /c /MD /O2 /nologo
set CLCMD=cl ^
-I tree-sitter\highlight\include\ ^
-I tree-sitter\lib\include ^
-I tree-sitter-c\bindings\c\ ^
-I tree-sitter-javascript\bindings\c\ ^
/nologo /Foout\ ^
/MD /O2 /DTREE_SITTER_HIDE_SYMBOLS ^
tslib.c ^
out\ts_c_parser.obj out\ts_javascript_parser.obj out\ts_javascript_scanner.obj ^
tree-sitter\lib\src\lib.c ^
ws2_32.lib ntdll.lib advapi32.lib userenv.lib bcrypt.lib kernel32.lib user32.lib
set CLCMDEXE=%CLCMD% main.c /Feout\main.exe
set CLCMDLIB=%CLCMD% /LD /Feout\tslib.dll
@echo on
%CLCMDLIB%
if %ErrorLevel%==0 (
	copy out\tslib.dll test-editor\tslib.dll
) 
%CLCMDEXE%
if %ErrorLevel%==0 (
	out\main.exe
)
