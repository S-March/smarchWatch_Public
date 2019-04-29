using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace bitmapToArray
{
    public partial class bitmapToArray : Form
    {
        public bitmapToArray()
        {
            InitializeComponent();
        }
        public static class DEFINES
        {
            public const int bmpImgSizeOffset = 0x02;
            public const int bmpImgDataOffset = 0x0A;
            public const int bmpImgWidthOffset = 0x12;
            public const int bmpImgHeightOffset = 0x16;
        }        

        private void previewPicture_Click(object sender, EventArgs e)
        {
            using (var fbd = new FolderBrowserDialog())
            {
                DialogResult result = fbd.ShowDialog();

                if (result == DialogResult.OK && !string.IsNullOrWhiteSpace(fbd.SelectedPath))
                {
                    string[] namesOfFiles = Directory.GetFiles(fbd.SelectedPath);
                    int numberOfFilesToConvert = (namesOfFiles.Length)-1;
                    string pictureFilesPath = fbd.SelectedPath + "\\pictureFiles.txt";
                    string pictureFilesHeaderPath = fbd.SelectedPath + "\\pictureFilesHeader.h";

                    int totalMemorySizeInBytes = 256*65536;//256 sectors of 64KB
                    int currentOffset = 0;
                    Byte tempDataPoint = 0;
                    using (FileStream fsHeader = File.Create(pictureFilesHeaderPath))
                    {
                        using (FileStream fs = File.Create(pictureFilesPath))
                        {
                            while (numberOfFilesToConvert >= 0)
                            {
                                string fileExtension = Path.GetExtension(namesOfFiles[numberOfFilesToConvert]);
                                if (fileExtension == ".bmp")
                                {
                                    previewPicture.BackgroundImage = new Bitmap(namesOfFiles[numberOfFilesToConvert]);
                                    previewPicture.BackgroundImage.RotateFlip(RotateFlipType.RotateNoneFlipY);
                                    previewPicture.Refresh();
                                    string nameOfFileForHeader = (namesOfFiles[numberOfFilesToConvert].Substring(fbd.SelectedPath.Length+1, (namesOfFiles[numberOfFilesToConvert].Length)- fbd.SelectedPath.Length - 5));
                                    Byte[] dataFromBitmap;
                                    using (var memoryStream = new System.IO.MemoryStream())
                                    {
                                        using (Bitmap oldPicture = new Bitmap(previewPicture.BackgroundImage))
                                        using (Bitmap newPicture = new Bitmap(oldPicture))
                                        using (Bitmap newBitmap = newPicture.Clone(new Rectangle(0, 0, newPicture.Width, newPicture.Height), System.Drawing.Imaging.PixelFormat.Format16bppRgb565))
                                            newBitmap.Save(memoryStream, System.Drawing.Imaging.ImageFormat.Bmp);
                                        dataFromBitmap = memoryStream.ToArray();
                                    }
                                    Byte dataStart = dataFromBitmap[bitmapToArray.DEFINES.bmpImgDataOffset];
                                    Byte[] dataSize = BitConverter.GetBytes((dataFromBitmap.Length - dataStart) + 2);
                                    for (int elementNumber = dataStart; elementNumber < dataFromBitmap.Length; elementNumber+=2)
                                    {
                                        tempDataPoint = dataFromBitmap[elementNumber];
                                        dataFromBitmap[elementNumber] = dataFromBitmap[elementNumber + 1];
                                        dataFromBitmap[elementNumber + 1] = tempDataPoint;
                                    }
                                    fs.Write(dataFromBitmap, bitmapToArray.DEFINES.bmpImgWidthOffset, 1);
                                    fs.Write(dataFromBitmap, bitmapToArray.DEFINES.bmpImgHeightOffset, 1);
                                    fs.Write(dataFromBitmap, dataStart, dataFromBitmap.Length - dataStart);
                                    byte[] headerLineToAppend = Encoding.ASCII.GetBytes("#define " + nameOfFileForHeader +"_OFFSET "+ currentOffset+"\n");
                                    currentOffset += (dataFromBitmap.Length - dataStart) + 2;
                                    fsHeader.Write(headerLineToAppend, 0, headerLineToAppend.Length);
                                    previewPicture.BackgroundImage.RotateFlip(RotateFlipType.RotateNoneFlipY);
                                    previewPicture.Refresh();
                                }
                                numberOfFilesToConvert--;
                            }
                            byte[] byteArrayOfTotalMemoryUsed = Encoding.ASCII.GetBytes("\n#define TOTAL_MEMORY_USED " + currentOffset + "\n");
                            fsHeader.Write(byteArrayOfTotalMemoryUsed, 0, byteArrayOfTotalMemoryUsed.Length);
                            byte[] byteArrayOfTotalMemoryAvailable = Encoding.ASCII.GetBytes("#define TOTAL_MEMORY_AVAILABLE " + (totalMemorySizeInBytes - currentOffset) + "\n");
                            fsHeader.Write(byteArrayOfTotalMemoryAvailable, 0, byteArrayOfTotalMemoryAvailable.Length);
                            byte[] byteArrayOfTotalMemoryUsedPercent = Encoding.ASCII.GetBytes("#define TOTAL_MEMORY_PERCENT USED " + (100*currentOffset / totalMemorySizeInBytes) + "\n");
                            fsHeader.Write(byteArrayOfTotalMemoryUsedPercent, 0, byteArrayOfTotalMemoryUsedPercent.Length);
                            byte[] byteArrayOfTimeToLoadData = Encoding.ASCII.GetBytes("//Time to load all data at 57600 baud: " + (8 * currentOffset / 57600) + " seconds");
                            fsHeader.Write(byteArrayOfTimeToLoadData, 0, byteArrayOfTimeToLoadData.Length);
                        }
                    }
                }
            }
        }
    }
}
