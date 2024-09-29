using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Text;
using System.Windows.Forms;
using static System.Windows.Forms.VisualStyles.VisualStyleElement;

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

        string prevText = string.Empty;

        Random random = new Random();

        IntPtr tsContext;
        TreeSitterLib.Language tsLanguage = TreeSitterLib.Language.JAVASCRIPT;
        IDictionary<string, Color> theme = new Dictionary<string, Color>();
        bool colorizing = false;
        bool openingText = false;
        
        OpenFileDialog openFileDialog = new OpenFileDialog();

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

        public Form1()
        {
            InitializeComponent();
            richTextBox1.SelectionChanged += RichTextBox1_SelectionChanged;
            richTextBox1.TextChanged += RichTextBox1_TextChanged;
            toolStripDropDownButton1.DropDownItemClicked += ToolStripDropDownButton1_DropDownItemClicked;
            toolStripDropDownButton2.Click += ToolStripDropDownButton2_Click;
            text = richTextBox1.Text;

            tsContext = TreeSitterLib.initialize(log_to_stdout: false);
            var status = TreeSitterLib.set_language(tsContext, tsLanguage, @"C:\Users\set\Desktop\tslib\tree-sitter-javascript\queries\highlights.scm");
            InsertLogLine($"tslib.set_language=\"{tsLanguage}\" \u2192 {status}");

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

        private void ToolStripDropDownButton2_Click(object? sender, EventArgs e)
        {
            var str = TreeSitterLib.syntax_tree(tsContext);
            InsertLogLine($"SYNTAX:\n{str}", replaceNewlines: false);
        }

        private void ToolStripDropDownButton1_DropDownItemClicked(object? sender, ToolStripItemClickedEventArgs e)
        {

        }

        private void RichTextBox1_TextChanged(object? sender, EventArgs e)
        {
            if (openingText)
            {
                openingText = false;
                return;
            }

            var newText = richTextBox1.Text;

            // We only care if the text is changed somehow
            if (text != newText)
            {
                var startIdx = Math.Min(oldSelectionStart, selectionStart);
                var rmTxt = text.Substring(startIdx, oldSelectionEnd - startIdx);
                var adTxt = newText.Substring(startIdx, selectionEnd - startIdx);

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
                        InsertLogLine($"CB: byte_index={byte_index}, bytes_read={bytes_read}, snip={snip}");
                        return snip;
                    },
                    TreeSitterLib.TSInputEncoding.TSInputEncodingUTF8
                );

                var evt = $"start={startIdx} \u2215 new_end={selectionEnd} \u2215 old_end={oldSelectionEnd} \u2215 removed=\"{rmTxt}\" \u2215 added=\"{adTxt}\" \u2215 edit_string={status}";
                InsertLogLine(evt);

                var sw = Stopwatch.StartNew();
                colorizing = true;
                SuspendDrawing(richTextBox1);
                label1.Focus();
                richTextBox1.Enabled = false;
                var prevSelectionStart = richTextBox1.SelectionStart;
                var prevSelectionLength = richTextBox1.SelectionLength;
                var hlCallbacks = 0;
                int firstVisibleChar = richTextBox1.GetCharIndexFromPosition(new Point(0, 0));

                TreeSitterLib.get_highlights(tsContext, 0, (uint)newText.Length, (byte_start, byte_length, captureName) =>
                {
                    if (!theme.ContainsKey(captureName))
                    {
                        theme.Add(captureName, GetRandomColor());
                    }
                    richTextBox1.SelectionStart = (int)byte_start;
                    richTextBox1.SelectionLength = (int)byte_length;
                    richTextBox1.SelectionColor = theme[captureName];
                    hlCallbacks++;
                });
                richTextBox1.SelectionLength = prevSelectionLength;
                richTextBox1.SelectionStart = prevSelectionStart;
                richTextBox1.Enabled = true;
                ResumeDrawing(richTextBox1);
                richTextBox1.Focus();
                colorizing = false;
                var elapsed = sw.Elapsed;
                InsertLogLine($"Paint info: callbacks={hlCallbacks}; elapsed={elapsed.TotalMilliseconds}ms");
            }

            text = newText;
        }

        private void RichTextBox1_SelectionChanged(object? sender, EventArgs e)
        {
            if (colorizing)
            {
                return;
            }

            // The ints returned seems to corrospond to bytes.
            var newStart = richTextBox1.SelectionStart;
            var newEnd = newStart + richTextBox1.SelectionLength;
            var newText = richTextBox1.Text;

            var newStartLine = richTextBox1.GetLineFromCharIndex(newStart);
            var newStartColumn = newStart - richTextBox1.GetFirstCharIndexFromLine(newStartLine);

            var newEndLine = richTextBox1.GetLineFromCharIndex(newEnd);
            var newEndColumn = newEnd - richTextBox1.GetFirstCharIndexFromLine(newEndLine);

            var oldEndLine = endLine;
            var oldEndColumn = endColumn;

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

        private void UpdateStatusLabel()
        {
            cursorInfo.Text = $"Line/Col: {startLine}/{startColumn} -- {endLine}/{endColumn}";
        }

        private void GenerateEvent(string text, int line, int column, int startByte, int oldStartByte, int byteLength)
        {

        }

        StringBuilder logContents = new StringBuilder();
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

        private void toolStripDropDownButton3_Click(object sender, EventArgs e)
        {
            if (openFileDialog.ShowDialog() == DialogResult.OK)
            {
                InsertLogLine($"Opening {openFileDialog.FileName}");
                LoadFile(openFileDialog.FileName);
            }
        }

        private void LoadFile(string filePath)
        {
            var code = string.IsNullOrWhiteSpace(filePath) ? "" : File.ReadAllText(openFileDialog.FileName);
            if (text != richTextBox1.Text)
            {
                // only if the text differs
                openingText = true;
            }
            text = code;
            richTextBox1.Text = code;
            if (!TreeSitterLib.parse_string(tsContext, code, (uint)code.Length, TreeSitterLib.TSInputEncoding.TSInputEncodingUTF8))
            {
                InsertLogLine("Failed to parse_string");
            }
        }

        private void toolStripDropDownButton4_Click(object sender, EventArgs e)
        {
            var s = string.Join("\n", theme.OrderBy(x => x.Key).Select(x => $"{x.Key}: {x.Value}"));
            InsertLogLine($"ThemeInfo\n{s}", replaceNewlines: false);
        }
    }
}
