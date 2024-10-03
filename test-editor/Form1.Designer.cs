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
            openFileBtn = new ToolStripDropDownButton();
            syntaxBtn = new ToolStripDropDownButton();
            themeInfoBtn = new ToolStripDropDownButton();
            languageBtn = new ToolStripDropDownButton();
            toolStripMenuItem1 = new ToolStripMenuItem();
            toolStripMenuItem2 = new ToolStripMenuItem();
            cursorInfo = new ToolStripStatusLabel();
            splitContainer1 = new SplitContainer();
            textBox = new RichTextBox();
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
            statusStrip1.ImageScalingSize = new Size(24, 24);
            statusStrip1.Items.AddRange(new ToolStripItem[] { openFileBtn, syntaxBtn, themeInfoBtn, languageBtn, cursorInfo });
            statusStrip1.Location = new Point(0, 428);
            statusStrip1.Name = "statusStrip1";
            statusStrip1.Size = new Size(984, 22);
            statusStrip1.TabIndex = 1;
            statusStrip1.Text = "statusStrip1";
            // 
            // openFileBtn
            // 
            openFileBtn.DisplayStyle = ToolStripItemDisplayStyle.Text;
            openFileBtn.Image = (Image)resources.GetObject("openFileBtn.Image");
            openFileBtn.ImageTransparentColor = Color.Magenta;
            openFileBtn.Name = "openFileBtn";
            openFileBtn.ShowDropDownArrow = false;
            openFileBtn.Size = new Size(61, 20);
            openFileBtn.Text = "Open File";
            // 
            // syntaxBtn
            // 
            syntaxBtn.DisplayStyle = ToolStripItemDisplayStyle.Text;
            syntaxBtn.Image = (Image)resources.GetObject("syntaxBtn.Image");
            syntaxBtn.ImageTransparentColor = Color.Magenta;
            syntaxBtn.Name = "syntaxBtn";
            syntaxBtn.ShowDropDownArrow = false;
            syntaxBtn.Size = new Size(67, 20);
            syntaxBtn.Text = "SyntaxTree";
            // 
            // themeInfoBtn
            // 
            themeInfoBtn.DisplayStyle = ToolStripItemDisplayStyle.Text;
            themeInfoBtn.Image = (Image)resources.GetObject("themeInfoBtn.Image");
            themeInfoBtn.ImageTransparentColor = Color.Magenta;
            themeInfoBtn.Name = "themeInfoBtn";
            themeInfoBtn.ShowDropDownArrow = false;
            themeInfoBtn.Size = new Size(68, 20);
            themeInfoBtn.Text = "ThemeInfo";
            // 
            // languageBtn
            // 
            languageBtn.DisplayStyle = ToolStripItemDisplayStyle.Text;
            languageBtn.DropDownItems.AddRange(new ToolStripItem[] { toolStripMenuItem1, toolStripMenuItem2 });
            languageBtn.ImageTransparentColor = Color.Magenta;
            languageBtn.Name = "languageBtn";
            languageBtn.ShowDropDownArrow = false;
            languageBtn.Size = new Size(63, 20);
            languageBtn.Text = "Language";
            // 
            // toolStripMenuItem1
            // 
            toolStripMenuItem1.Name = "toolStripMenuItem1";
            toolStripMenuItem1.Size = new Size(136, 22);
            toolStripMenuItem1.Text = "JAVASCRIPT";
            // 
            // toolStripMenuItem2
            // 
            toolStripMenuItem2.Name = "toolStripMenuItem2";
            toolStripMenuItem2.Size = new Size(136, 22);
            toolStripMenuItem2.Text = "C";
            // 
            // cursorInfo
            // 
            cursorInfo.Name = "cursorInfo";
            cursorInfo.Size = new Size(710, 17);
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
            splitContainer1.Panel1.Controls.Add(textBox);
            // 
            // splitContainer1.Panel2
            // 
            splitContainer1.Panel2.Controls.Add(label1);
            splitContainer1.Panel2.Controls.Add(textBox1);
            splitContainer1.Size = new Size(984, 425);
            splitContainer1.SplitterDistance = 494;
            splitContainer1.TabIndex = 2;
            // 
            // textBox
            // 
            textBox.AcceptsTab = true;
            textBox.Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right;
            textBox.Font = new Font("Consolas", 11.25F, FontStyle.Regular, GraphicsUnit.Point, 0);
            textBox.Location = new Point(0, 0);
            textBox.Name = "textBox";
            textBox.Size = new Size(491, 425);
            textBox.TabIndex = 0;
            textBox.Text = "";
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
            textBox1.AcceptsReturn = true;
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
        private ToolStripDropDownButton languageBtn;
        private ToolStripStatusLabel cursorInfo;
        private SplitContainer splitContainer1;
        private TextBox textBox1;
        private ToolStripMenuItem toolStripMenuItem1;
        private ToolStripDropDownButton syntaxBtn;
        private ToolStripDropDownButton openFileBtn;
        private RichTextBox textBox;
        private Label label1;
        private ToolStripDropDownButton themeInfoBtn;
        private ToolStripMenuItem toolStripMenuItem2;
    }
}
