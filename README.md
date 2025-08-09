# SharpExt4 
[![Donate](https://img.shields.io/badge/Donate-PayPal-green.svg)](https://www.paypal.com/donate?business=C2RXG8SGHT366)

## About

The main purpose of this SharpExt4 project is to provide full access to Linux ext2/3/4 filesystem from Windows .Net application.

For a day-to-day Windows user, it is not easy to read/write ext2/3/4 filesystem directly from Windows environment. C# .Net programmers especially find it hard to search for a .Net library, which can provide full access to Linux ext2/3/4 filesystem.

These are the findings so far:
1. [DiscUtils](https://github.com/DiscUtils/DiscUtils), is a .NET library to read and write ISO files and Virtual Machine disk files (VHD, VDI, XVA, VMDK, etc). DiscUtils also provides limited access to ext2/3/4 filesystem.
2. Ext2Fsd is another Windows file system driver for the Ext2, Ext3, and Ext4 file systems. It allows Windows to read Linux file systems natively, providing access to the file system via a drive letter that any program can access.
3. DiskInternals Linux Reader is a freeware application from DiskInternals, developers of data recovery software. 
4. Ext2explore is an open-source application that works similarly to DiskInternals Linux Reader—but only for Ext4, Ext3, and Ext2 partitions.
5. The [lwext4](https://github.com/gkostka/lwext4) project provides the ext2/3/4 filesystem for microcontrollers.

## Overview

The lwext4 is a portable C project for microcontrollers and has many cool and unique features. Lwext4 is an excellent choice for SD/MMC cards, USB flash drives or any other wear leveled memory types. In Windows, the author recommended to use [MSYS-2](https://sourceforge.net/projects/msys2/)

I imported the lwext4 backbone over to MSVC compiler (Visual Studio 2019), and created the lwext4 as a static lib.
SharpExt4 is a clr wrapper of lwext4 to provide modern .Net application access. The SharpExt4 borrows the [DiscUtils](https://github.com/DiscUtils/DiscUtils) class concept and creates a friendly interface for .Net

## Compile
#### Visual Studio 2022 C/C++ (It can be simply modified to be compiled in Visual Studio 2013 and Visual Studio 2019)
#### .Net Framework 4.8
#### .Net Core 3.1 
#### .Net 6

## How to use the Library

[How to use SharpExt4 to access Raspberry Pi SD Card Linux partition](https://www.nickdu.com/?p=919).

Here's a few simple examples.

#### How to read a file from ext4 disk image
```
	...
	//Open a Linux ext4 disk image
	var disk = ExtDisk.Open(@".\ext4.img");
	//Get the Linux partition and open
	var fs = ExtFileSystem.Open(disk.Parititions[0]);
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
	...
```

#### How to list all files in a folder from ext4 disk image
```
	...
	//Open a Linux ext4 disk image
	var disk = ExtDisk.Open(@".\ext4.img");
	//Get the Linux partition, and open
	var fs = ExtFileSystem.Open(disk.Parititions[0]);
	//List all files in /etc folder
	foreach(var file in fs.GetFiles("/etc", "*", SearchOption.AllDirectories))
	{
		Console.WriteLine(file);
	}
	...
```

### How to create a file in ext4 disk image
```
	...
	//Open a Linux ext4 disk image
	var disk = ExtDisk.Open(@".\org.img");
	//Get the file system
	var fs = ExtFileSystem.Open(disk.Parititions[0]);
	//Open a file for write
	var file = fs.OpenFile("/etc/test", FileMode.Create, FileAccess.Write);
	var hello = "Hello World";
	var buf = Encoding.ASCII.GetBytes(hello);
	//Write to file
	var count = file.Read(buf, 0, buf.Length);
	file.Close();
	...
```

### How to change a file mode in ext4 disk
```
	...
	//Open a Linux ext4 disk assume your SD card/USB is physical disk 1
	var disk = ExtDisk.Open(1);
	//Get the file system
	var fs = ExtFileSystem.Open(disk.Parititions[0]);
	//Check file exists
	if(fs.FileExists("/etc/hosts"))
	{
		//0x1FF in HEX, 777 in OCT
		fs.SetMode("/etc/hosts", 0x1FF);
	}
	...
```



## Credits

The core library of SharpExt4 was taken from lwext4:
* https://github.com/gkostka/lwext4

The GPT partition was taken from DiskPartitionInfo:
* https://github.com/f1x3d/DiskPartitionInfo

## 📁 Related Project

#### 🔗 [**SharpExt4Explorer**](https://github.com/nickdu088/SharpExt4Explorer)

**SharpExt4Explorer** is a Windows GUI file browser built on top of the SharpExt4 library.  
It allows users to visually browse, read, and extract files from Linux ext2/3/4 disk images or physical drives, without writing a single line of code.

Ideal for:
- Inspecting Raspberry Pi SD cards
- Recovering files from Linux partitions
- Working with ext images directly from Windows

![Ext4Explorer Screenshot](https://github.com/nickdu088/SharpExt4Explorer/blob/main/images/file.png)

## Star History

[![Star History Chart](https://api.star-history.com/svg?repos=nickdu088/SharpExt4&type=Date)](https://star-history.com/#nickdu088/SharpExt4&Date)

