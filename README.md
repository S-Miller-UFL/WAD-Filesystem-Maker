This is a library that allows you to mount DOOM WAD files as actual directories. Useful for sending entire filesystems in tiny little WAD files.

This library should be useable on any linux system, but just to be safe its highly recommended you run this under the reptitlian kernel.
https://cise.ufl.edu/research/reptilian/wiki/doku.php
functionality is not guaranteed on any other kernel

you will need fuse

to download fuse, run the following commands:
"sudo apt install libfuse-dev fuse"
"sudo chmod 666 /dev/fuse"
note that if you reboot your kernel, you will need to do
"sudo chmod 666 /dev/fuse"
as fuse permissions will be reset.

download the wad.tar.gz, and run the following commands:
"tar zxvf wad.tar.gz"
"cd libwad"
"make"
"cd .."
"cd wadfs"
"make"
"cd .."

Afterwards, youll need to run the fuse daemon by running the following command:
"./wadfs/wadfs -s [insert wad file name here].wad [insert directory here]"

Afterwards, your wad file is fully mounted!

You can retrieve file and directory attributes
read from existing files and write to new ones
read from existing directories and write to new ones.
