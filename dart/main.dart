import 'dart:ffi';
import 'dart:io';
import 'dart:math';
import 'package:ffi/ffi.dart';
import 'tslib.dart';

const except = -1;

var sourceCode = "var x = 42;";
var sourceCode2 = 'var x = "42";';

class HLItem {
  int start;
  int length;
  String name;
  HLItem(this.start, this.length, this.name);
}

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

List<HLItem> hlitems = [];

void getHighlights2(int start, int length, Pointer<Utf8> captureName) {
  hlitems.add(new HLItem(start, length, captureName.toDartString()));
}

void main() async {
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

  // testing perf ...
  tslib.initialize(false);
  tslib.setLanguage(TreeSitterLanguage.javascript,
      '../tree-sitter-javascript/queries/highlights.scm');
  var largeSrcLines =
      await File('../tree-sitter-javascript/grammar.js').readAsLines();
  tslib.parseString(largeSrcLines.join('\n'));
  var rng = Random();
  var hl2Cb =
      NativeCallable<GetHighlightsCallback>.isolateLocal(getHighlights2);
  var stopwatch = Stopwatch();
  for (var i = 0; i < 100; i++) {
    var lineStart = rng.nextInt(largeSrcLines.length - 50);
    var byteCount = 0;
    var lineCount = 0;
    var byteStart = largeSrcLines.sublist(0, lineStart).join('\n').length;
    while (lineCount < 50) {
      byteCount += largeSrcLines[lineStart + lineCount].length + 1;
      lineCount++;
    }
    stopwatch.start();
    tslib.getHighlights(byteStart, byteCount, hl2Cb.nativeFunction);
    stopwatch.stop();
    var elapsed = stopwatch.elapsedMilliseconds;
    var c = hlitems.length;
    print(
        "$i\tElapsed: $elapsed ms (found $c highlights for $byteCount bytes)");
    hlitems.clear();
    stopwatch.reset();
  }
  hl2Cb.close();
}
