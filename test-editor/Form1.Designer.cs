namespace test_editor
{
    partial class Form1
    {
        /// <summary>
        ///  Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        ///  Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        ///  Required method for Designer support - do not modify
        ///  the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(Form1));
            statusStrip1 = new StatusStrip();
            toolStripDropDownButton3 = new ToolStripDropDownButton();
            toolStripDropDownButton2 = new ToolStripDropDownButton();
            toolStripDropDownButton1 = new ToolStripDropDownButton();
            toolStripMenuItem1 = new ToolStripMenuItem();
            cursorInfo = new ToolStripStatusLabel();
            splitContainer1 = new SplitContainer();
            richTextBox1 = new RichTextBox();
            label1 = new Label();
            textBox1 = new TextBox();
            statusStrip1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)splitContainer1).BeginInit();
            splitContainer1.Panel1.SuspendLayout();
            splitContainer1.Panel2.SuspendLayout();
            splitContainer1.SuspendLayout();
            SuspendLayout();
            // 
            // statusStrip1
            // 
            statusStrip1.Items.AddRange(new ToolStripItem[] { toolStripDropDownButton3, toolStripDropDownButton2, toolStripDropDownButton1, cursorInfo });
            statusStrip1.Location = new Point(0, 428);
            statusStrip1.Name = "statusStrip1";
            statusStrip1.Size = new Size(984, 22);
            statusStrip1.TabIndex = 1;
            statusStrip1.Text = "statusStrip1";
            // 
            // toolStripDropDownButton3
            // 
            toolStripDropDownButton3.DisplayStyle = ToolStripItemDisplayStyle.Text;
            toolStripDropDownButton3.Image = (Image)resources.GetObject("toolStripDropDownButton3.Image");
            toolStripDropDownButton3.ImageTransparentColor = Color.Magenta;
            toolStripDropDownButton3.Name = "toolStripDropDownButton3";
            toolStripDropDownButton3.ShowDropDownArrow = false;
            toolStripDropDownButton3.Size = new Size(61, 20);
            toolStripDropDownButton3.Text = "Open File";
            toolStripDropDownButton3.Click += toolStripDropDownButton3_Click;
            // 
            // toolStripDropDownButton2
            // 
            toolStripDropDownButton2.DisplayStyle = ToolStripItemDisplayStyle.Text;
            toolStripDropDownButton2.Image = (Image)resources.GetObject("toolStripDropDownButton2.Image");
            toolStripDropDownButton2.ImageTransparentColor = Color.Magenta;
            toolStripDropDownButton2.Name = "toolStripDropDownButton2";
            toolStripDropDownButton2.ShowDropDownArrow = false;
            toolStripDropDownButton2.Size = new Size(67, 20);
            toolStripDropDownButton2.Text = "SyntaxTree";
            // 
            // toolStripDropDownButton1
            // 
            toolStripDropDownButton1.DisplayStyle = ToolStripItemDisplayStyle.Text;
            toolStripDropDownButton1.DropDownItems.AddRange(new ToolStripItem[] { toolStripMenuItem1 });
            toolStripDropDownButton1.ImageTransparentColor = Color.Magenta;
            toolStripDropDownButton1.Name = "toolStripDropDownButton1";
            toolStripDropDownButton1.ShowDropDownArrow = false;
            toolStripDropDownButton1.Size = new Size(63, 20);
            toolStripDropDownButton1.Text = "Language";
            // 
            // toolStripMenuItem1
            // 
            toolStripMenuItem1.Name = "toolStripMenuItem1";
            toolStripMenuItem1.Size = new Size(136, 22);
            toolStripMenuItem1.Text = "JAVASCRIPT";
            // 
            // cursorInfo
            // 
            cursorInfo.Name = "cursorInfo";
            cursorInfo.Size = new Size(778, 17);
            cursorInfo.Spring = true;
            cursorInfo.Text = "cursorInfo";
            cursorInfo.TextAlign = ContentAlignment.MiddleRight;
            // 
            // splitContainer1
            // 
            splitContainer1.Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right;
            splitContainer1.Location = new Point(0, 0);
            splitContainer1.Name = "splitContainer1";
            // 
            // splitContainer1.Panel1
            // 
            splitContainer1.Panel1.Controls.Add(richTextBox1);
            // 
            // splitContainer1.Panel2
            // 
            splitContainer1.Panel2.Controls.Add(label1);
            splitContainer1.Panel2.Controls.Add(textBox1);
            splitContainer1.Size = new Size(984, 425);
            splitContainer1.SplitterDistance = 495;
            splitContainer1.TabIndex = 2;
            // 
            // richTextBox1
            // 
            richTextBox1.Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right;
            richTextBox1.Font = new Font("Consolas", 11.25F, FontStyle.Regular, GraphicsUnit.Point, 0);
            richTextBox1.Location = new Point(0, 0);
            richTextBox1.Name = "richTextBox1";
            richTextBox1.Size = new Size(493, 425);
            richTextBox1.TabIndex = 0;
            richTextBox1.Text = "";
            // 
            // label1
            // 
            label1.AutoSize = true;
            label1.Location = new Point(-100, 303);
            label1.Name = "label1";
            label1.Size = new Size(38, 15);
            label1.TabIndex = 1;
            label1.Text = "label1";
            // 
            // textBox1
            // 
            textBox1.Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right;
            textBox1.Location = new Point(0, 0);
            textBox1.MaxLength = 932767;
            textBox1.Multiline = true;
            textBox1.Name = "textBox1";
            textBox1.ReadOnly = true;
            textBox1.ScrollBars = ScrollBars.Both;
            textBox1.Size = new Size(485, 425);
            textBox1.TabIndex = 0;
            // 
            // richTextBox1
            // 
            richTextBox1.AcceptsTab = true;
            richTextBox1.Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right;
            richTextBox1.Font = new Font("Consolas", 11.25F, FontStyle.Regular, GraphicsUnit.Point, 0);
            richTextBox1.Location = new Point(0, 0);
            richTextBox1.Name = "richTextBox1";
            richTextBox1.Size = new Size(493, 425);
            richTextBox1.TabIndex = 0;
            richTextBox1.Text = "";
            // 
            // Form1
            // 
            AutoScaleDimensions = new SizeF(7F, 15F);
            AutoScaleMode = AutoScaleMode.Font;
            ClientSize = new Size(984, 450);
            Controls.Add(splitContainer1);
            Controls.Add(statusStrip1);
            Name = "Form1";
            Text = "test-editor";
            statusStrip1.ResumeLayout(false);
            statusStrip1.PerformLayout();
            splitContainer1.Panel1.ResumeLayout(false);
            splitContainer1.Panel2.ResumeLayout(false);
            splitContainer1.Panel2.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)splitContainer1).EndInit();
            splitContainer1.ResumeLayout(false);
            ResumeLayout(false);
            PerformLayout();
        }

        #endregion
        private StatusStrip statusStrip1;
        private ToolStripDropDownButton toolStripDropDownButton1;
        private ToolStripStatusLabel cursorInfo;
        private SplitContainer splitContainer1;
        private TextBox textBox1;
        private ToolStripMenuItem toolStripMenuItem1;
        private ToolStripDropDownButton toolStripDropDownButton2;
        private ToolStripDropDownButton toolStripDropDownButton3;
        private RichTextBox richTextBox1;
        private Label label1;
    }
}
