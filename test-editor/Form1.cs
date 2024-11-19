using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Text;

namespace test_editor
{
    public partial class Form1 : Form
    {
        string text;

        int selectionStart;
        int selectionEnd;
        int startLine;
        int startColumn;
        int endLine;
        int endColumn;

        int oldSelectionStart;
        int oldSelectionEnd;
        int oldStartLine;
        int oldStartColumn;
        int oldEndLine;
        int oldEndColumn;

        Dictionary<TreeSitterLib.Language, string> scmPaths = new()
        {
            { TreeSitterLib.Language.JAVASCRIPT, @"..\..\..\..\tree-sitter-javascript\queries\highlights.scm" },
            { TreeSitterLib.Language.C, @"..\..\..\..\tree-sitter-c\queries\highlights.scm" }
        };

        string prevText = string.Empty;
        StringBuilder logContents = new StringBuilder();
        Random random = new Random();

        IntPtr tsContext;
        TreeSitterLib.Language tsLanguage = TreeSitterLib.Language.C;
        IDictionary<string, Color> theme = new Dictionary<string, Color>();
        bool colorizing = false;
        bool openingText = false;
        
        OpenFileDialog openFileDialog = new OpenFileDialog();

        public Form1()
        {
            InitializeComponent();
            textBox.SelectionChanged += TextBoxSelectionChanged;
            textBox.TextChanged += TextBoxTextChanged;
            languageBtn.DropDownItemClicked += LanguageBtnClicked;
            openFileBtn.Click += OpenFileBtnClicked;
            syntaxBtn.Click += SyntaxBtnClicked;
            themeInfoBtn.Click += ThemeInfoBtnClicked;
            text = textBox.Text;
            LoadFile(null);
            UpdateStatusLabel();
        }

        private Color GetRandomColor()
        {
            Color c;
            do
            {
                c = Color.FromArgb(random.Next(0, 256), random.Next(0, 256), random.Next(0, 256));

            } while (IsLightColor(c));
            return c;
        }

        private bool IsLightColor(Color color)
        {
            var r = color.R & 0xFF;
            var g = color.G & 0xFF;
            var b = color.B & 0xFF;
            var luma = 0.2126 * r + 0.7152 * g + 0.0722 * b; // per ITU-R BT.709
            return luma > 180;
        }

        private void InsertLogLine(string line, bool replaceNewlines = true)
        {
            if (replaceNewlines)
            {
                line = line.Replace("\n", "\\n").Replace("\t", "\\t");
            }
            var ts = DateTime.Now.ToString("HH:mm:ss.fff");
            logContents.Insert(0, $"[{ts}] {line}{Environment.NewLine}");
            textBox1.Text = logContents.ToString();
        }

        private void UpdateStatusLabel()
        {
            cursorInfo.Text = $"Line/Col: {startLine}/{startColumn} -- {endLine}/{endColumn}";
        }

        private void LoadFile(string? filePath)
        {
            var code = string.IsNullOrWhiteSpace(filePath) ? textBox.Text : File.ReadAllText(openFileDialog.FileName);
            if (text != textBox.Text)
            {
                // only if the text differs
                openingText = true;
            }
            text = code;
            textBox.Text = code;
            tsContext = TreeSitterLib.initialize(enable_logging: false, log_to_stdout: false);
            var scmContents = File.ReadAllText(scmPaths[tsLanguage]);
            var status = TreeSitterLib.set_language(tsContext, tsLanguage, scmContents, (uint)scmContents.Length);
            InsertLogLine($"tslib.set_language=\"{tsLanguage}\" \u2192 {status}");
            if (!TreeSitterLib.parse_string(tsContext, code, (uint)code.Length, TreeSitterLib.TSInputEncoding.TSInputEncodingUTF8))
            {
                InsertLogLine("Failed to parse_string");
            }
            // First time, we colorize the whole thing
            HighlightRange(0, (uint)text.Length);
        }

        private void HighlightRange(uint from, uint count)
        {
            var sw = Stopwatch.StartNew();
            colorizing = true;
            SuspendDrawing(textBox);
            label1.Focus();
            textBox.Enabled = false;
            var prevSelectionStart = textBox.SelectionStart;
            var prevSelectionLength = textBox.SelectionLength;
            var hlCallbacks = 0;
            
            TreeSitterLib.get_highlights(tsContext, from, count, (byte_start, byte_length, captureId, captureName) =>
            {
                if (!theme.ContainsKey(captureName))
                {
                    theme.Add(captureName, GetRandomColor());
                }
                textBox.SelectionStart = (int)byte_start;
                textBox.SelectionLength = (int)byte_length;
                textBox.SelectionColor = theme[captureName];
                hlCallbacks++;
            });
            textBox.SelectionLength = prevSelectionLength;
            textBox.SelectionStart = prevSelectionStart;
            textBox.Enabled = true;
            ResumeDrawing(textBox);
            textBox.Focus();
            colorizing = false;
            var elapsed = sw.Elapsed;
            InsertLogLine($"Paint info: callbacks={hlCallbacks}; elapsed={elapsed.TotalMilliseconds}ms");
        }

        private void SyntaxBtnClicked(object? sender, EventArgs e)
        {
            var str = TreeSitterLib.syntax_tree(tsContext);
            InsertLogLine($"SYNTAX:\n{str}", replaceNewlines: false);
        }

        private void LanguageBtnClicked(object? sender, ToolStripItemClickedEventArgs e)
        {
            var ln = Enum.Parse<TreeSitterLib.Language>(e.ClickedItem.Text);
            tsLanguage = ln;
            LoadFile(null);
        }

        private void TextBoxTextChanged(object? sender, EventArgs e)
        {
            if (openingText)
            {
                openingText = false;
                return;
            }

            var newText = textBox.Text;

            // We only care if the text is changed somehow
            if (text != newText)
            {
                var startIdx = Math.Min(oldSelectionStart, selectionStart);
                var rmTxt = text.Substring(startIdx, oldSelectionEnd - startIdx);
                var adTxt = newText.Substring(startIdx, selectionEnd - startIdx);

                InsertLogLine($"edit_string: bytes=({startIdx}, {oldSelectionEnd}, {selectionEnd}), start=({startLine}, {startColumn}), oldEnd=({oldEndLine}, {oldEndColumn}), newEnd=({endLine}, {endColumn})");

                var status = TreeSitterLib.edit_string(
                    tsContext,
                    start_byte: (uint)startIdx,
                    old_end_byte: (uint)oldSelectionEnd,
                    new_end_byte: (uint)selectionEnd,
                    start_point_row: (uint)startLine,
                    start_point_column: (uint)startColumn,
                    old_end_point_row: (uint)oldEndLine,
                    old_end_point_column: (uint)oldEndColumn,
                    new_end_point_row: (uint)endLine,
                    new_end_point_column: (uint)endColumn,
                    bufferReader: (IntPtr payload, uint byte_index, TreeSitterLib.TSPoint position, ref uint bytes_read) =>
                    {
                        string? snip = null;
                        if (byte_index < newText.Length)
                        {
                            var c = Math.Min(newText.Length - byte_index, 2500);
                            snip = newText.Substring((int)byte_index, (int)c);
                            bytes_read = (uint)snip.Length;
                        }
                        else
                        {
                            bytes_read = 0;
                        }
                        var logsnip = snip;
                        if (logsnip != null && logsnip.Length > 100)
                        {
                            logsnip = $"{logsnip.Substring(0, 48)}...{logsnip.Substring(logsnip.Length - 48)}";
                        }
                        InsertLogLine($"CB: byte_index={byte_index}, bytes_read={bytes_read}, snip={logsnip}");
                        return snip;
                    },
                    TreeSitterLib.TSInputEncoding.TSInputEncodingUTF8
                );

                var evt = $"start={startIdx} \u2215 new_end={selectionEnd} \u2215 old_end={oldSelectionEnd} \u2215 removed=\"{rmTxt}\" \u2215 added=\"{adTxt}\" \u2215 edit_string={status}";
                InsertLogLine(evt);

                HighlightRange((uint)startIdx, (uint)(selectionEnd - startIdx));
            }

            text = newText;
        }

        private void TextBoxSelectionChanged(object? sender, EventArgs e)
        {
            if (colorizing)
            {
                return;
            }

            // The ints returned seems to corrospond to bytes.
            var newStart = textBox.SelectionStart;
            var newEnd = newStart + textBox.SelectionLength;
            var newText = textBox.Text;

            var newStartLine = textBox.GetLineFromCharIndex(newStart);
            var newStartColumn = newStart - textBox.GetFirstCharIndexFromLine(newStartLine);

            var newEndLine = textBox.GetLineFromCharIndex(newEnd);
            var newEndColumn = newEnd - textBox.GetFirstCharIndexFromLine(newEndLine);

            oldSelectionStart = selectionStart;
            oldSelectionEnd = selectionEnd;
            oldStartLine = startLine;
            oldStartColumn = startColumn;
            oldEndLine = endLine;
            oldEndColumn = endColumn;

            selectionStart = newStart;
            selectionEnd = newEnd;
            startLine = newStartLine;
            startColumn = newStartColumn;
            endLine = newEndLine;
            endColumn = newEndColumn;

            UpdateStatusLabel();
        }

        private void OpenFileBtnClicked(object? sender, EventArgs e)
        {
            if (openFileDialog.ShowDialog() == DialogResult.OK)
            {
                InsertLogLine($"Opening {openFileDialog.FileName}");
                LoadFile(openFileDialog.FileName);
            }
        }
        
        private void ThemeInfoBtnClicked(object? sender, EventArgs e)
        {
            var s = string.Join(",\r\n", theme.OrderBy(x => x.Key).Select(x => $"\"{x.Key}\": \"#{x.Value.R:X2}{x.Value.G:X2}{x.Value.B:X2}\""));
            InsertLogLine($"ThemeInfo\r\n{s}", replaceNewlines: false);
        }

        [DllImport("user32.dll")]
        public static extern int SendMessage(IntPtr hWnd, Int32 wMsg, bool wParam, Int32 lParam);
        private const int WM_SETREDRAW = 11;

        public static void SuspendDrawing(Control Target)
        {
            SendMessage(Target.Handle, WM_SETREDRAW, false, 0);
        }

        public static void ResumeDrawing(Control Target)
        {
            SendMessage(Target.Handle, WM_SETREDRAW, true, 0);
            Target.Invalidate(true);
            Target.Update();
        }
    }
}
