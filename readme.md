# TreeSitter Demo Editor

This repository contains a simple [tree-sitter](https://tree-sitter.github.io/tree-sitter/) wrapper library written in C,
and a C# based WinForms based editor, which integrates to it using P/Invoke,
which demonstrates the basic ideas required for hosting and utilizing
tree-sitter to perform syntax-highlighting.

This has been tested on Windows 11, using VS2022.

![image](test-editor.png)

## Build

- Initialize the git submodules (`git submodule init` and `git submodule update`)
- Run `build.bat` in the Developer Command Prompt for VS 2022
- Open `test-editor.sln` and press F5

## Notes

- This library is intended to be used in Flutter/Dart.
- Editor performance is quite poor, since it "just" builds on the RichTextBox control.
