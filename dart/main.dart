import 'dart:ffi';
import 'package:ffi/ffi.dart';
import 'tslib.dart';

const except = -1;

var sourceCode = "var x = 42;";
var sourceCode2 = 'var x = "42";';

Pointer<Utf8> editCallback(Pointer<Void> payload, int byteIndex,
    TSPoint position, Pointer<Uint32> bytes_read) {
  print('editCallback $byteIndex!');
  if (byteIndex < sourceCode2.length) {
    var snip = sourceCode2.substring(byteIndex);
    bytes_read.value = snip.length;
    return snip.toNativeUtf8();
  } else {
    bytes_read.value = 0;
    return nullptr;
  }
}

void getHighlights(int start, int length, Pointer<Utf8> captureName) {
  var name = captureName.toDartString();
  var src = sourceCode2.substring(start, start + length);
  print("HL: $name ($start, $length) => $src");
}

void main() {
  var tslib = new TreeSitterLib('../out/tslib.dll', TreeSitterEncoding.Utf8);
  tslib.initialize(true);
  tslib.setLanguage(TreeSitterLanguage.javascript,
      '../tree-sitter-javascript/queries/highlights.scm');
  tslib.parseString(sourceCode);
  var editCb =
      NativeCallable<EditStringUtf8Callback>.isolateLocal(editCallback);
  tslib.editString(8, 10, 12, 0, 8, 0, 10, 0, 12, editCb.nativeFunction);
  editCb.close();
  var hlCb = NativeCallable<GetHighlightsCallback>.isolateLocal(getHighlights);
  tslib.getHighlights(0, sourceCode2.length, hlCb.nativeFunction);
  hlCb.close();
}
