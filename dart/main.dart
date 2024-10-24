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

void getHighlights(
    int start, int length, int captureId, Pointer<Utf8> captureName) {
  var name = captureName.toDartString();
  var src = sourceCode2.substring(start, start + length);
  print("HL: $name ($start, $length) => $src");
}

List<HLItem> hlitems = [];
List<int> hlitemsSyntax = [];

void getHighlights2(
    int start, int length, int captureId, Pointer<Utf8> captureName) {
  for (var i = 0; i < length; i++) {
    hlitemsSyntax[i + start] = captureId;
  }
  // hlitems.add(new HLItem(start, length, captureName.toDartString()));
}

void main() async {
  var tslib = new TreeSitterLib('../out/tslib.dll', TreeSitterEncoding.Utf8);
  // tslib.testingFfi();
  // return;
  tslib.initialize(true);
  var jsScm = await File('../tree-sitter-javascript/queries/highlights.scm')
      .readAsString();
  tslib.setLanguage(TreeSitterLanguage.javascript, jsScm);
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
  tslib.setLanguage(TreeSitterLanguage.javascript, jsScm);
  var largeSrcLines =
      await File('../tree-sitter-javascript/grammar.js').readAsLines();
  var fullText = largeSrcLines.join('\n');
  tslib.parseString(fullText);
  var rng = Random();
  var hl2Cb =
      NativeCallable<GetHighlightsCallback>.isolateLocal(getHighlights2);
  var stopwatch = Stopwatch();
  var totalDur = 0;
  var numOfPerfRuns = 10000;
  hlitemsSyntax = List.generate(fullText.length, (index) => 0);
  for (var i = 0; i < numOfPerfRuns; i++) {
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
    var elapsed = stopwatch.elapsedMicroseconds;
    totalDur += elapsed;
    hlitems.clear();
    stopwatch.reset();
  }
  print("Average over $numOfPerfRuns runs: ${totalDur / numOfPerfRuns}");
  hl2Cb.close();
}
