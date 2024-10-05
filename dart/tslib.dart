import 'dart:ffi' as ffi;
import 'dart:io' show Directory;
import 'package:path/path.dart' as path;
import 'package:ffi/ffi.dart';

enum TreeSitterLanguage {
  none(0),
  javascript(1),
  c(2);

  const TreeSitterLanguage(this.value);
  final int value;
}

// For now, we only support UTF8, since it seems that darts usage of code units
// is the same meaning as bytes, and this eases lots of interopt with tree-sitter.
// See https://api.dart.dev/stable/3.3.4/dart-convert/Utf8Codec-class.html for support
enum TreeSitterEncoding {
  Utf8(0);

  const TreeSitterEncoding(this.value);
  final int value;
}

// From tree-sitter
final class TSPoint extends ffi.Struct {
  @ffi.Uint32()
  external final int row;
  @ffi.Uint32()
  external final int column;
}

final class SyntaxNode extends ffi.Struct {
  external final TSPoint start;
  external final TSPoint end;
  @ffi.Bool()
  external final bool named;
  external final ffi.Pointer<Utf8> nodetype;
}

class SyntaxNode2 {
  late int startRow;
  late int startColumn;
  late int endRow;
  late int endColumn;
  late bool named;
  late String nodeType;
}

final class NodeList extends ffi.Struct {
  external final ffi.Pointer<SyntaxNode> list;
  @ffi.Uint32()
  external final int length;
}

// 'initialize'
typedef InitializeLib = ffi.Pointer Function(ffi.Bool);
typedef InitializeDart = ffi.Pointer Function(bool);
// 'set_language'
typedef SetLanguageLib = ffi.Bool Function(
    ffi.Pointer, ffi.Uint32, ffi.Pointer<Utf8>);
typedef SetLanguageDart = bool Function(ffi.Pointer, int, ffi.Pointer<Utf8>);
// 'parse_string'
typedef ParseStringUtf8Lib = ffi.Bool Function(
    ffi.Pointer, ffi.Pointer<Utf8>, ffi.Uint32, ffi.Uint8);
typedef ParseStringUtf8Dart = bool Function(
    ffi.Pointer, ffi.Pointer<Utf8>, int, int);
// 'edit_string'
typedef EditStringUtf8Callback = ffi.Pointer<Utf8> Function(
    ffi.Pointer<ffi.Void>, ffi.Uint32, TSPoint, ffi.Pointer<ffi.Uint32>);
typedef EditStringPayloadCallback = ffi.Void Function();
typedef EditStringUtf8Lib = ffi.Bool Function(
    ffi.Pointer,
    ffi.Uint32,
    ffi.Uint32,
    ffi.Uint32,
    ffi.Uint32,
    ffi.Uint32,
    ffi.Uint32,
    ffi.Uint32,
    ffi.Uint32,
    ffi.Uint32,
    ffi.Pointer<ffi.NativeFunction<EditStringUtf8Callback>>,
    ffi.Uint32);
typedef EditStringUtf8Dart = bool Function(
    ffi.Pointer,
    int,
    int,
    int,
    int,
    int,
    int,
    int,
    int,
    int,
    ffi.Pointer<ffi.NativeFunction<EditStringUtf8Callback>>,
    int);
typedef GetHighlightsCallback = ffi.Void Function(
    ffi.Uint32, ffi.Uint32, ffi.Pointer<Utf8>);
typedef GetHighlightsLib = ffi.Bool Function(ffi.Pointer, ffi.Uint32,
    ffi.Uint32, ffi.Pointer<ffi.NativeFunction<GetHighlightsCallback>>);
typedef GetHighlightsDart = bool Function(ffi.Pointer, int, int,
    ffi.Pointer<ffi.NativeFunction<GetHighlightsCallback>>);

bool loadedLibrary = false;

class TreeSitterLib {
  static late ffi.DynamicLibrary library;

  late int encodingEnum;
  late InitializeDart _initialize;
  late SetLanguageDart _setLanguage;
  late ParseStringUtf8Dart _parseStringUtf8;
  late EditStringUtf8Dart _editStringUtf8;
  late GetHighlightsDart _getHighlights;

  ffi.Pointer ctx = ffi.nullptr;

  TreeSitterLib(String tslibPath, TreeSitterEncoding encoding) {
    encodingEnum = encoding == TreeSitterEncoding.Utf8 ? 0 : 1;
    if (!loadedLibrary) {
      var libraryPath = path.join(Directory.current.path, tslibPath);
      library = ffi.DynamicLibrary.open(libraryPath);
      loadedLibrary = true;
    }
    _initialize =
        library.lookupFunction<InitializeLib, InitializeDart>('initialize');
    _setLanguage =
        library.lookupFunction<SetLanguageLib, SetLanguageDart>('set_language');
    _parseStringUtf8 =
        library.lookupFunction<ParseStringUtf8Lib, ParseStringUtf8Dart>(
            'parse_string');
    _editStringUtf8 = library
        .lookupFunction<EditStringUtf8Lib, EditStringUtf8Dart>('edit_string');
    _getHighlights = library
        .lookupFunction<GetHighlightsLib, GetHighlightsDart>('get_highlights');
  }

  void initialize(bool logToStdout) {
    ctx = _initialize(logToStdout);
    if (ctx == ffi.nullptr) {
      throw Exception('Failed to initialize tree-sitter');
    }
  }

  void setLanguage(TreeSitterLanguage language, String scmPath) {
    if (!_setLanguage(ctx, language.value, scmPath.toNativeUtf8())) {
      throw Exception('Failed to set tree-sitter parser languager: $language');
    }
  }

  bool parseString(String source) {
    var sourceCodePointer = source.toNativeUtf8();
    var res = _parseStringUtf8(
        ctx, sourceCodePointer, sourceCodePointer.length, encodingEnum);
    print("parseString: $res");
    return res;
  }

  bool editString(
      int startByte,
      int oldEndByte,
      int newEndByte,
      int startPointRow,
      int startPointColumn,
      int oldEndPointRow,
      int oldEndPointColumn,
      int newEndPointRow,
      int newEndPointColumn,
      ffi.Pointer<ffi.NativeFunction<EditStringUtf8Callback>> bufferCallback) {
    return _editStringUtf8(
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
        bufferCallback,
        encodingEnum);
  }

  bool getHighlights(int startByte, int byteLength,
      ffi.Pointer<ffi.NativeFunction<GetHighlightsCallback>> bufferCallback) {
    return _getHighlights(ctx, startByte, byteLength, bufferCallback);
  }
}
