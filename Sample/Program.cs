/*
 * Copyright (c) 2021 Nick Du (nick@nickdu.com)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * - The name of the author may not be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

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
            //Open a Linux ext4 disk image
            var disk = ExtDisk.Open(@".\ext4.img");
            //Get the file system
            var fs = ExtFileSystem.Open(disk, disk.Partitions[0]);
            ReadFileContent(fs);
            ListAllFiles(fs);
            CreateFile(fs);
            ChangeFileMode(fs);
        }
        static void ChangeFileMode(ExtFileSystem fs)
        {
            //777(OCT) = 511(DEC) = 0x1FF(HEX)
            fs.SetMode("/etc/hosts",511);
        }
        static void ReadFileContent(ExtFileSystem fs)
        {
            //Open a file for read
            var file = fs.OpenFile("/etc/shells", FileMode.Open, FileAccess.Read);
            //Check the file length
            var filelen = file.Length;
            var buf = new byte[filelen];
            //Read the file content
            var count = file.Read(buf, 0, (int)filelen);
            file.Close();
            var content = Encoding.Default.GetString(buf);
            Console.WriteLine(content);
        }
        static void ListAllFiles(ExtFileSystem fs)
        {
            //List all files in /etc folder
            foreach (var file in fs.GetFiles("/etc", "*", SearchOption.AllDirectories))
            {
                Console.WriteLine(file);
            }
        }
        static void CreateFile(ExtFileSystem fs)
        {
            //Open a file for write
            var file = fs.OpenFile("/etc/test", FileMode.Create, FileAccess.Write);
            var hello = "Hello World";
            var buf = Encoding.ASCII.GetBytes(hello);
            //Write to file
            file.Write(buf, 0, buf.Length);
            file.Close();
        }
    }
}
