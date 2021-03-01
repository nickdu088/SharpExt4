# SharpExt4

## About
=====

The main goal of the SharpExt4 project is to provide ext2/3/4 filesystem access from .Net application.

For a day-to-day Windows user, it is not easy to read/write ext2/3/4 filesystem directly from Windows environment. Especially for a .Net programmer, it is hardly to find a .Net library, which can provide full access to ext2/3/4 Linux filesystem.

These are the findings so far:
1. [DiscUtils](https://github.com/DiscUtils/DiscUtils), is a .NET library to read and write ISO files and Virtual Machine disk files (VHD, VDI, XVA, VMDK, etc). DiscUtils also provides limited access to ext2/3/4 filesystem.
2. Ext2Fsd is another Windows file system driver for the Ext2, Ext3, and Ext4 file systems. It allows Windows to read Linux file systems natively, providing access to the file system via a drive letter that any program can access.
3. DiskInternals Linux Reader is a freeware application from DiskInternals, developers of data recovery software. 
4. Ext2explore is an open-source application that works similarly to DiskInternals Linux Reader—but only for Ext4, Ext3, and Ext2 partitions.
5. The [lwext4](https://github.com/gkostka/lwext4) project is to provide ext2/3/4 filesystem for microcontrollers.

## Overview

The lwext4 is a portable C project for microcontrollers and the library has some cool and unique features. Lwext4 is an excellent choice for SD/MMC card, USB flash drive or any other wear leveled memory types. In Windows, the author recommended to use [MSYS-2](https://sourceforge.net/projects/msys2/)

I port the lwext4 backbone over to MSVC compiler (Visual Studio 2019), and create the lwext4 as a static lib.
SharpExt4 is a clr wrapper of lwext4 to provide .Net application access. The SharpExt4 borrows the [DiscUtils](https://github.com/DiscUtils/DiscUtils) class concept and creates a friendly interface for .Net

## Compile
#### Visual Studio 2019 C/C++
#### .Net Framework 4.5

## How to use the Library
Here's a few simple examples.
#### How to read a file from ext4 disk
```
  ...
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
  ...
```

## Credits
=====

The core library of SharpExt4 was taken from lwext4:
* https://github.com/gkostka/lwext4
