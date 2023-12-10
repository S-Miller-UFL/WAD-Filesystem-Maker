



//wadfs.cpp
/*
* Author: Steven Miller
* Date created: November 29, 2023
* Purpose: makes our wad file accessible from the fuse library
* Used by: N/A
* Uses: N/A
*/


#include <iostream>
#include "../libWad/Wad.h"
#include <string.h>
#include <fuse.h>
#include <unistd.h>
#include <errno.h>


//taken from:/
/* 
https://www.cs.hmc.edu/~geoff/classes/hmc.cs137.201601/homework/fuse/fuse_doc.html
https://www.cs.nmsu.edu/~pfeiffer/fuse-tutorial/html/callbacks.html
https://www.youtube.com/watch?v=Fnfflm3_9kw
https://engineering.facile.it/blog/eng/write-filesystem-fuse/
https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/errno.h.html
*/

static int wadfs_get_attr(const char* wadfs_file_path, struct stat* wadfs_file_status)
{
    //the wad file we mounted
    Wad* wad_file =(Wad*)fuse_get_context()->private_data;
    //if its a file
    if(wad_file->isContent(std::string(wadfs_file_path)))
    {

        wadfs_file_status->st_mode = S_IFREG | 0777;
        wadfs_file_status->st_nlink = 1;
        wadfs_file_status->st_size = wad_file->getSize(std::string(wadfs_file_path));
        return 0;
    }
    if(wad_file->isDirectory(std::string(wadfs_file_path)))
    {
        wadfs_file_status->st_mode = S_IFDIR | 0777;
        wadfs_file_status->st_nlink = 2;
        wadfs_file_status->st_size = 0;
        return 0;
    }
    return -ENOENT;
}

//make a new file
static int wadfs_mknod(const char* wadfs_file_path, mode_t mode, dev_t rdev)
{
    //the wad file we mounted
    Wad* wad_file =(Wad*)fuse_get_context()->private_data;
    
    wad_file->createFile(std::string(wadfs_file_path));

    //return if it was successful or not
    if(wad_file->isContent(std::string(wadfs_file_path)))
    {
        return 0;
    }
    return 0;

}

//make a new directory
static int wadfs_mkdir(const char* wadfs_file_path, mode_t mode)
{
    //the wad file we mounted
    Wad* wad_file =(Wad*)fuse_get_context()->private_data;

    wad_file->createDirectory(std::string(wadfs_file_path));

    //return if it was successful or not
    if(wad_file->isDirectory(std::string(wadfs_file_path)))
    {
        return 0;
    }
    return -1;
}

//read contents of file
static int wadfs_read(const char* wadfs_file_path, char *buffer, size_t length, off_t offset, struct fuse_file_info* wadfs_fuse_fileinfostruct)
{
    //the wad file we mounted
    Wad* wad_file =(Wad*)fuse_get_context()->private_data;
    return wad_file->getContents(std::string(wadfs_file_path),buffer, length, offset);
}

//write contents to EMPTY file
static int wadfs_write(const char* wadfs_file_path, const char *buffer, size_t length, off_t offset, struct fuse_file_info* wadfs_fuse_fileinfostruct)
{
    //the wad file we mounted
    Wad* wad_file =(Wad*)fuse_get_context()->private_data;
    return wad_file->writeToFile(std::string(wadfs_file_path),buffer,length,offset);
}
//get children of directory
static int wadfs_readdir(const char*wadfs_file_path, void* buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info* wadfs_fuse_fileinfostruct)
{
    //the wad file we mounted
    Wad* wad_file =(Wad*)fuse_get_context()->private_data;

    //vector of our file descriptors
    std::vector<std::string> file_vector;

    int status = wad_file->getDirectory(std::string(wadfs_file_path), &file_vector);
    if(status == -1)
    {
        return -1;
    }
    filler(buffer, ".", NULL, 0);
	filler(buffer, "..", NULL, 0);
    //put into buffer
    for(std::string name: file_vector)
    {
        filler(buffer, name.c_str(), NULL, 0);
    }
    //return our status code
    //NEEDS to be zero or it will fail
    return 0;
}
static int wadfs_open(const char* wadfs_file_path, struct fuse_file_info* wadfs_fuse_fileinfostruct)
{
    //since we dont "open" files in our file system, well just tell the calling function whether
    //this file exists or not

    //the wad file we mounted
    Wad* wad_file =(Wad*)fuse_get_context()->private_data;

    //if its a directory
    if(wad_file->isDirectory(std::string(wadfs_file_path)))
    {
        //tell the calling function that the directory exists
        return 0;
    }
    //if its a file
    if(wad_file->isContent(std::string(wadfs_file_path)))
    {
        //tell the calling function that the file exists
        return 0;
    }

    return -1;

}
struct fuse_operations wadfs_operations = 
{
    
    .getattr = wadfs_get_attr,
    .mknod = wadfs_mknod,
    .mkdir = wadfs_mkdir,
    .open = wadfs_open,
    .read = wadfs_read,
    .write = wadfs_write,
    .readdir = wadfs_readdir
};

int main(int argc, char* argcv[])
{
    //if not enough arguments
    if(argc < 3)
    {
        std::cout << "Not enough arguments" << std::endl;
        exit(EXIT_SUCCESS);
    }

    //get the path
    std::string wadpath = argcv[argc-2];
    //this is a relative path
    //convert it to an absolute one by adding a /
    if(wadpath.at(0) != '/')
    {
        wadpath = std::string(get_current_dir_name()) + "/" + wadpath;
    }

    //load the actual wad file specified
    Wad* mywad = Wad::loadWad(wadpath);
    argcv[argc-2] = argcv[argc-1];
    argc--;
    

    //pass the pointer to the wad file into the fuse main function
    return fuse_main(argc,argcv,&wadfs_operations,mywad);
}
