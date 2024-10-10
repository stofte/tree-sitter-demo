@ECHO OFF
MKDIR out
DEL /Q "out\*.*"
DEL "test-editor\tslib.*"
set BUILD_FLAGS=/O2
IF "%~1"=="debug" (SET BUILD_FLAGS=/DEBUG /Z7)
CL tree-sitter-c\src\parser.c /Foout\ts_c_parser.obj /c /MD %BUILD_FLAGS% /nologo
CL tree-sitter-javascript\src\parser.c /Foout\ts_javascript_parser.obj /c /MD %BUILD_FLAGS% /nologo
CL tree-sitter-javascript\src\scanner.c /Foout\ts_javascript_scanner.obj /c /MD %BUILD_FLAGS% /nologo
set CLCMD=CL /nologo ^
-I tree-sitter\highlight\include\ ^
-I tree-sitter\lib\include ^
-I tree-sitter-c\bindings\c\ ^
-I tree-sitter-javascript\bindings\c\ ^
/WX ^
/Foout\ ^
/MD %BUILD_FLAGS% /DTREE_SITTER_HIDE_SYMBOLS ^
tslib.c ^
tree-sitter\lib\src\lib.c ^
out\ts_c_parser.obj out\ts_javascript_parser.obj out\ts_javascript_scanner.obj ^
ws2_32.lib ntdll.lib advapi32.lib userenv.lib bcrypt.lib kernel32.lib user32.lib
set CLCMDEXE=%CLCMD% main.c /Feout\main.exe
set CLCMDLIB=%CLCMD% /LD /Feout\tslib.dll
@echo on
%CLCMDLIB%
@ECHO OFF
if %ErrorLevel%==0 (
    copy out\tslib.dll test-editor
    copy out\tslib.pdb test-editor
)
%CLCMDEXE%
if %ErrorLevel%==0 (
	out\main.exe
)
