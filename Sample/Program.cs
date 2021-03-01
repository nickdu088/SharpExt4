using SharpExt4;
using System;
using System.IO;
using System.Text;

namespace Sample
{
    class Program
    {
        static void Main(string[] args)
        {
            //Open Ext4 disk image
            var ext4 = ExtDisk.Open(@".\org.img");
            var fs = ExtFileSystem.Open(ext4.Parititions[0]);
            //Open a file for read
            var file = fs.OpenFile("/etc/shells", FileMode.Open, FileAccess.Read);
            //Check the file length
            var filelen = file.Length;
            var buf = new byte[filelen];
            //Read the file content
            var count = file.Read(buf, 0, (int)filelen);
            var content = Encoding.Default.GetString(buf);
            Console.WriteLine(content);
        }
    }
}
