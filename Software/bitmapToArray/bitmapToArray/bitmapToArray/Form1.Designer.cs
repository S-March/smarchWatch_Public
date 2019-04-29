namespace bitmapToArray
{
    partial class bitmapToArray
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
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
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.previewPicture = new System.Windows.Forms.PictureBox();
            ((System.ComponentModel.ISupportInitialize)(this.previewPicture)).BeginInit();
            this.SuspendLayout();
            // 
            // previewPicture
            // 
            this.previewPicture.BackgroundImage = global::bitmapToArray.Properties.Resources.selectFolder;
            this.previewPicture.BackgroundImageLayout = System.Windows.Forms.ImageLayout.Center;
            this.previewPicture.Dock = System.Windows.Forms.DockStyle.Fill;
            this.previewPicture.ErrorImage = null;
            this.previewPicture.ImageLocation = "0,0";
            this.previewPicture.InitialImage = null;
            this.previewPicture.Location = new System.Drawing.Point(0, 0);
            this.previewPicture.Margin = new System.Windows.Forms.Padding(0);
            this.previewPicture.Name = "previewPicture";
            this.previewPicture.Size = new System.Drawing.Size(284, 286);
            this.previewPicture.TabIndex = 2;
            this.previewPicture.TabStop = false;
            this.previewPicture.Click += new System.EventHandler(this.previewPicture_Click);
            // 
            // bitmapToArray
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(54)))), ((int)(((byte)(54)))), ((int)(((byte)(59)))));
            this.ClientSize = new System.Drawing.Size(284, 286);
            this.Controls.Add(this.previewPicture);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "bitmapToArray";
            this.ShowIcon = false;
            this.Text = "bitmapToArray";
            ((System.ComponentModel.ISupportInitialize)(this.previewPicture)).EndInit();
            this.ResumeLayout(false);

        }

        #endregion
        private System.Windows.Forms.PictureBox previewPicture;
    }
}

