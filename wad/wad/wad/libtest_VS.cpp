








//libtest_VS.cpp
/*
* Author: Steven Miller
* Date created: November 30, 2023
* Purpose: Visual studio friendly version of libtest.cpp
* Used by: N/A
* Uses: Wad.h
*/


/*
* NOTES
* 
* These are some notes I wrote down while watching the discussion slides, I have left these in here so you can better understand
* this project.
*/

//wad format:
/*
* header->lumps->descriptors
* the way you write data in wad files:
* things need to be shifted forward first,
*/

//file header:
/*
* three things:
* 4 bytes of file magic: ASCII characters, going to be either
*   "IWAD" or "PWAD". dont edit this
* 4 bytes of number of descriptors: uint32_t,
*   how many descriptors you need to read when you get to offset
* 4 bytes of descriptor offset: uint32_t, relative to start of file.
*   describes how many bytes from start of file you need to go
*   before you hit the descriptors
*/

//once you read magic and number of descriptors,
//you wanna seek forward by the "descriptor offset" amount + 0

//the things you want to put into memory in your wad class is:
/*
* 1.magic
* 2.number of descriptors
* 3.descriptor offset
* 4.information about each descriptor
*/
//DONT PUT LUMP DATA INTO MEMORY, ITS VERY INEFFICIENT
//you dont want to read lump data until the user requests
//to read data inside the file

//the descriptor list is a set of information that describes files listed in the wad
/*
* each descriptor is 16 bytes in total
* 4 bytes of offset
* 4 bytes of length
* 4 bytes of name
*
* each wad file in the file system is described using a descriptor
* however, 16 bytes isnt enough to store anything useful.
* thats why the element offset exists
*/

//the element offset is the location relative to the start of the wad file where the file contents are
/*
* say the name is "file.txt", among the 16 bytes, theres nowhere to store the contents.
*
* thats because the contents of the files are stored in the lump data
*
* the element offset of "file.txt" relative to the start of the file (0) tells you where
* the actual contents of "file.txt" begin
*
* however, how much data are we supposed to read?
* thats where element length comes in
*/

//the element length tells you how much data describes the contents of the file
/*
* so if you read an offset of 32, with a length of 24, and a name of "meme.jpg"
* that means at byte 32 of your file, you will find the first byte of "meme.jpg"
* and the next 23 bytes will belong to "meme.jpg"
*/

//remember, there are two types of files:
/*
* directories: files that point to other files
* content files: actual files we care about
*
*/

//now, lets talk about lumps
/*
* elements in the WAD format are stored as "lumps"
* you cannot write to existing lumps, but you will create empty files and will
* write lumps to them.
* the contents of files is ITS LUMP DATA
*/

//directories are also stored in the "name" section of a descriptor
/*
* theres only going to be two types of directories in WAD files:
* E#M#: hashes will be numbers, also called map_marker directory
* ##_START:hashes will be characters, also called namespace marker directory
*
* Now, there are a couple rules with these two types:

*/

//the next ten descriptors after E#M# are going to be map elements, specifically, content files.
/*
* what that implies is that map markers will never have other directories inside of them
* E#M# means that the next ten descriptors will be content files, and those content files belong
* to that directory
*
* namespaces are different though
* namespaces will have 1-2 characters and a suffix
*/

//the beginning of a namespace directory will be ##_START and the end of a namespace directory will be #_END
/*
* a consequence of this is that there will only be two chars left to represetnt the name of the directory.
*
* any descriptors between the start and end descriptor belong to that directory
*
* so F1_START indicates its the start of directory F1, and F1_END indicates its the end of directory F1.
* any files inbetween those two descriptors are in the directory F1
*/

//you will be creating namespace directories
/*
* in the name for directory youre passed into "create directory" function, will have a max name of two chars.
*/

//you will not leave namespace directories until you encounter the "end" marker for that directory
/*
* for example, lets say you read in
* X_START
* then, you read in
* Y_START
* then you read in
* E1M1
* math.obj
* polygon.c
* then you read in
* Z_START
* then you read in
* Z_END
* then you read in
* Y_END
* then you read in
* X_END
*
* that means the directory will look like this:
*   X
*   |
*   -->Y
*     |
*     |-->E1M1
*     |  |
*     |  |-->math.obj
*     |  |
*     |  |-->polygon.c
*     |
*     |-->Z
*
* keep in mind, E1M1 can only have up to 10 content files
*
*/

//in addition, the root directory has its own root.
/*
* its your parent directory!
* what means is that you want to create an empty directory called "/"
* then, the first descriptor should be appended to "/"
*/

/*
* ACTUAL TESTS
*/
#include <iostream>
#include <vector>
#include <fstream>
#include <cctype>
#include <string>
#include <stack>
#include <map>
#include "libWad/Wad.h"

const std::string setupWorkspace() {

    const std::string wad_path = "./testfiles/sample1.wad";
    const std::string test_wad_path = "./testfiles/sample1_copy.wad";

    std::string command = "cp " + wad_path + " " + test_wad_path;
    int returnCode = system(command.c_str());

    if (returnCode == EXIT_FAILURE) {
        throw("Copy failure");
    }

    return test_wad_path;
}

void getmagic()
{
    std::string wad_path = setupWorkspace();
    Wad* testWad = Wad::loadWad(wad_path);
    if (testWad->getMagic() != "IWAD")
    {
        std::cout << "FAILED GETMAGIC" << std::endl;
        std::cout << "expected IWAD, got " << testWad->getMagic() << std::endl;
    }
    else
    {
        std::cout << "PASSED GETMAGIC" << std::endl;
    }

    delete testWad;
}

void iscontent()
{
    std::string wad_path = setupWorkspace();
    Wad* testWad = Wad::loadWad(wad_path);

    //Testing valid content
    if (testWad->isContent("/E1M0/01.txt") != true)
    {
        std::cout << "FAILED ISCONTENT FOR /E1M0/01.txt" << std::endl;
        std::cout << "shouldve been true but it came out false" << std::endl;
    }
    if (testWad->isContent("/E1M0/05.txt") != true)
    {
        std::cout << "FAILED ISCONTENT FOR /E1M0/05.txt" << std::endl;
        std::cout << "shouldve been true but it came out false" << std::endl;
    }
    if (testWad->isContent("/Gl/ad/os/cake.jpg") != true)
    {
        std::cout << "FAILED ISCONTENT FOR /Gl/ad/os/cake.jpg" << std::endl; 
        std::cout << "shouldve been true but it came out false" << std::endl;
    }
    if (testWad->isContent("/mp.txt") != true)
    {
        std::cout << "FAILED ISCONTENT FOR /mp.txt" << std::endl;
        std::cout << "shouldve been true but it came out false" << std::endl;
    }

    //Testing invalid content (directories)
    if (testWad->isContent("/E1M0") != false)
    {
        std::cout << "FAILED ISCONTENT FOR /E1M0" << std::endl;
        std::cout << "shouldve been false but it came out true" << std::endl;
    }
    if (testWad->isContent("/Gl/ad") != false)
    {
        std::cout << "FAILED ISCONTENT FOR /Gl/ad" << std::endl;
        std::cout << "shouldve been false but it came out true" << std::endl;
    }
    if (testWad->isContent("/Gl/ad/os") != false)
    {
        std::cout << "FAILED ISCONTENT FOR /Gl/ad/os" << std::endl;
        std::cout << "shouldve been false but it came out true" << std::endl;
    }
    if (testWad->isContent("/") != false)
    {
        std::cout << "FAILED ISCONTENT FOR /" << std::endl;
        std::cout << "shouldve been false but it came out true" << std::endl;
    }

    //Testing invalid content (nonexistent files/directories)
    if (testWad->isContent("/Fake/") != false)
    {
        std::cout << "FAILED ISCONTENT FOR /Fake/" << std::endl;
        std::cout << "shouldve been false but it came out true" << std::endl;
    }
    if (testWad->isContent("/notreal/") != false)
    {
        std::cout << "FAILED ISCONTENT FOR /notreal/" << std::endl;
        std::cout << "shouldve been false but it came out true" << std::endl;
    }
    if(testWad->isContent("/NotAFile")!= false)
    {
        std::cout << "FAILED ISCONTENT FOR /NotAFile" << std::endl;
        std::cout << "shouldve been false but it came out true" << std::endl;
    }
    if (testWad->isContent("/fake.jpg") != false)
    {
        std::cout << "FAILED ISCONTENT FOR /fake.jpg" << std::endl;
        std::cout << "shouldve been false but it came out true" << std::endl;
    }
    if (testWad->isContent("/not/a/real/file/text.txt") != false)
    {
        std::cout << "FAILED ISCONTENT FOR /not/a/real/file/text.txt" << std::endl;
        std::cout << "shouldve been false but it came out true" << std::endl;
    }
   if(testWad->isContent("/file.mp3")!=false)
   {
       std::cout << "FAILED ISCONTENT FOR /file.mp3" << std::endl;
       std::cout << "shouldve been false but it came out true" << std::endl;
   }
   //this needs to be fixed
    if(testWad->isContent("")==true);
    {
        std::cout << "FAILED ISCONTENT FOR NOTHING PASSED IN" << std::endl;
        std::cout << "shouldve been false but it came out true" << std::endl;
    }

    delete testWad;
}

void isdirectory()
{
    std::string wad_path = setupWorkspace();
    Wad* testWad = Wad::loadWad(wad_path);

    //Testing valid directories
    if (testWad->isDirectory("/E1M0") != true)
    {
        std::cout << "FAILED ISDIRECTORY FOR /E1M0" << std::endl;
        std::cout << "shouldve been true but it came out false" << std::endl;
    }
    if (testWad->isDirectory("/Gl/ad") != true)
    {
        std::cout << "FAILED ISDIRECTORY FOR /Gl/ad" << std::endl;
        std::cout << "shouldve been true but it came out false" << std::endl;
    }
    if (testWad->isDirectory("/Gl/ad/os") != true)
    {
        std::cout << "FAILED ISDIRECTORY FOR /Gl/ad/os" << std::endl;
        std::cout << "shouldve been true but it came out false" << std::endl;
    }
    if (testWad->isDirectory("/") != true)
    {
        std::cout << "FAILED ISDIRECTORY FOR /" << std::endl;
        std::cout << "shouldve been true but it came out false" << std::endl;
    }

    //Testing invalid directories (content))
    if (testWad->isDirectory("/E1M0/01.txt") != false)
    {
        std::cout << "FAILED ISDIRECTORY FOR /E1M0/01.txt" << std::endl;
        std::cout << "shouldve been false but it came out true" << std::endl;
    }
    if (testWad->isDirectory("/E1M0/05.txt") != false)
    {
        std::cout << "FAILED ISDIRECTORY FOR /E1M0/05.txt" << std::endl;
        std::cout << "shouldve been false but it came out true" << std::endl;
    }
    if (testWad->isDirectory("/Gl/ad/os/cake.jpg") != false)
    {
        std::cout << "FAILED ISDIRECTORY FOR /Gl/ad/os/cake.jpg" << std::endl;
        std::cout << "shouldve been false but it came out true" << std::endl;
    }
    if (testWad->isDirectory("/mp.txt") != false)
    {
        std::cout << "FAILED ISDIRECTORY FOR /mp.txt" << std::endl;
        std::cout << "shouldve been false but it came out true" << std::endl;
    }

    //Testing invalid content (nonexistent files/directories)
    if (testWad->isDirectory("/Fa/") != false)
    {
        std::cout << "FAILED ISDIRECTORY FOR /Fa/" << std::endl;
        std::cout << "shouldve been false but it came out true" << std::endl;
    }
    if (testWad->isDirectory("/Dr/") != false)
    {
        std::cout << "FAILED ISDIRECTORY FOR /Dr/" << std::endl;
        std::cout << "shouldve been false but it came out true" << std::endl;
    }
    if (testWad->isDirectory("/NotAFile") != false)
    {
        std::cout << "FAILED ISDIRECTORY FOR /NotAFile" << std::endl;
        std::cout << "shouldve been false but it came out true" << std::endl;
    }
    if (testWad->isDirectory("/fake.jpg") != false)
    {
        std::cout << "FAILED ISDIRECTORY FOR /fake.jpg" << std::endl;
        std::cout << "shouldve been false but it came out true" << std::endl;
    }
    if (testWad->isDirectory("/no/t/ar/ea/l/di/r/text.txt") != false)
    {
        std::cout << "FAILED ISDIRECTORY FOR /no/t/ar/ea/l/di/r/text.txt" << std::endl;
        std::cout << "shouldve been false but it came out true" << std::endl;
    }
    if (testWad->isDirectory("/file.mp3") != false)
    {
        std::cout << "FAILED ISDIRECTORY FOR /file.mp3" << std::endl;
        std::cout << "shouldve been false but it came out true" << std::endl;
    }
    if(testWad->isDirectory("")!=false)
    {
        std::cout << "FAILED ISDIRECTORY FOR nothing passed in" << std::endl;
        std::cout << "shouldve been false but it came out true" << std::endl;
    }

    delete testWad;
}
void getsize()
{
    std::string wad_path = setupWorkspace();
    Wad* testWad = Wad::loadWad(wad_path);

    //Testing valid content
    if (testWad->getSize("/E1M0/01.txt") != 17)
    {
        std::cout << "FAILED GETSIZE FOR /E1M0/01.txt" << std::endl;
        std::cout << "shouldve been 17 but it came out "<< testWad->getSize("/E1M0/01.txt") << std::endl;
    }
    if (testWad->getSize("/E1M0/05.txt")!= 12)
    {
        std::cout << "FAILED GETSIZE FOR /E1M0/05.txt" << std::endl;
        std::cout << "shouldve been 12 but it came out " << testWad->getSize("/E1M0/05.txt") << std::endl;
    }
    if (testWad->getSize("/E1M0/09.txt") != 10)
    {
        std::cout << "FAILED GETSIZE FOR /E1M0/05.txt" << std::endl;
        std::cout << "shouldve been 12 but it came out " << testWad->getSize("/E1M0/05.txt") << std::endl;
    }
    if (testWad->getSize("/E1M0/10.txt") != 13)
    {
        std::cout << "FAILED GETSIZE FOR /E1M0/05.txt" << std::endl;
        std::cout << "shouldve been 12 but it came out " << testWad->getSize("/E1M0/05.txt") << std::endl;
    }
    if (testWad->getSize("/Gl/ad/os/cake.jpg") != 29869)
    {
        std::cout << "FAILED GETSIZE FOR /E1M0/05.txt" << std::endl;
        std::cout << "shouldve been 12 but it came out " << testWad->getSize("/E1M0/05.txt") << std::endl;
    }
    if (testWad->getSize("/mp.txt") != 398)
    {
        std::cout << "FAILED GETSIZE FOR /E1M0/05.txt" << std::endl;
        std::cout << "shouldve been 12 but it came out " << testWad->getSize("/E1M0/05.txt") << std::endl;
    }

    //test invalid content (directories)
    if (testWad->getSize("/E1M0") != -1)
    {
        std::cout << "FAILED GETSIZE FOR /E1M0/05.txt" << std::endl;
        std::cout << "shouldve been 12 but it came out " << testWad->getSize("/E1M0/05.txt") << std::endl;
    }
    if (testWad->getSize("/Gl/ad/") != -1)
    {
        std::cout << "FAILED GETSIZE FOR /E1M0/05.txt" << std::endl;
        std::cout << "shouldve been 12 but it came out " << testWad->getSize("/E1M0/05.txt") << std::endl;
    }
    if (testWad->getSize("/Gl/ad") != -1)
    {
        std::cout << "FAILED GETSIZE FOR /E1M0/05.txt" << std::endl;
        std::cout << "shouldve been 12 but it came out " << testWad->getSize("/E1M0/05.txt") << std::endl;
    }
    if (testWad->getSize("/Gl/ad/os/") != -1)
    {
        std::cout << "FAILED GETSIZE FOR /E1M0/05.txt" << std::endl;
        std::cout << "shouldve been 12 but it came out " << testWad->getSize("/E1M0/05.txt") << std::endl;
    }

    //test invalid content (nonexistent files/directories)
    if (testWad->getSize("/E1M1") != -1)
    {
        std::cout << "FAILED GETSIZE FOR /E1M0/05.txt" << std::endl;
        std::cout << "shouldve been 12 but it came out " << testWad->getSize("/E1M0/05.txt") << std::endl;

    }
    if (testWad->getSize("/Ex/") != -1)
    {
        std::cout << "FAILED GETSIZE FOR /E1M0/05.txt" << std::endl;
        std::cout << "shouldve been 12 but it came out " << testWad->getSize("/E1M0/05.txt") << std::endl;
    }
    if (testWad->getSize("/Fi/le") != -1)
    {
        std::cout << "FAILED GETSIZE FOR /E1M0/05.txt" << std::endl;
        std::cout << "shouldve been 12 but it came out " << testWad->getSize("/E1M0/05.txt") << std::endl;
    }
    if (testWad->getSize("/file.jpg") != -1)
    {
        std::cout << "FAILED GETSIZE FOR /E1M0/05.txt" << std::endl;
        std::cout << "shouldve been 12 but it came out " << testWad->getSize("/E1M0/05.txt") << std::endl;
    }
    if (testWad->getSize("") != -1)
    {
        std::cout << "FAILED GETSIZE FOR /E1M0/05.txt" << std::endl;
        std::cout << "shouldve been 12 but it came out " << testWad->getSize("/E1M0/05.txt") << std::endl;
    }

    delete testWad;

}

void getcontentstest1()
{
    std::string wad_path = setupWorkspace();
    Wad* testWad = Wad::loadWad(wad_path);

    //getContents Test 1, reading full text file
    std::string testPath = "/E1M0/01.txt";
    char expectedString[] = "He loves to sing\n";
    char buffer[1024];
    memset(buffer, 0, 1024);

    //should return 17
    int sizeOfFile = testWad->getSize(testPath);

    //should return 17
    int retValue = testWad->getContents(testPath, buffer, sizeOfFile);

    if (retValue != sizeOfFile)
    {
        std::cout << "GETCONTENTSTEST3 FAILED " << "RETVALUE != SIZEOFFILE" << std::endl;
    }

    if (memcmp(expectedString, buffer, sizeOfFile) != 0)
    {
        std::cout << "GETCONTENTSTEST3 FAILED " << "memcmp(expectedString, buffer, sizeOfFile) != 0" << std::endl;
    }
    std::cout << "Expected string: " << expectedString << std::endl;
    std::cout << "Returned string: " << buffer << std::endl;

    delete testWad;
}
void getcontentstest2()
{
    std::string wad_path = setupWorkspace();
    Wad* testWad = Wad::loadWad(wad_path);

    //getContents Test 2, reading middle of text file using offset
    std::string testPath = "/mp.txt";
    char expectedString[] = "airspeed velocity";
    char buffer[1024];
    memset(buffer, 0, 1024);

    int expectedStringLen = 17;
    int offset = 117;

    //should return 17
    int retValue = testWad->getContents(testPath, buffer, expectedStringLen, offset);
    if (retValue != expectedStringLen)
    {
        std::cout << "GETCONTENTSTEST2 FAILED " << "RETVALUE != EXPECTEDSTRINGLEN" << std::endl;
    }

    if (memcmp(expectedString, buffer, expectedStringLen) != 0)
    {
        std::cout << "GETCONTENTSTEST2 FAILED " << "memcmp(expectedString, buffer, expectedStringLen) != 0" << std::endl;
    }

    std::cout << "Expected string: " << expectedString << std::endl;
    std::cout << "Returned string: " << buffer << std::endl;

    delete testWad;
}
void getcontentstest3()
{
    std::string wad_path = setupWorkspace();
    Wad* testWad = Wad::loadWad(wad_path);

    //getContents Test 3, trying to read past end of file (length is too large)
    std::string testPath = "/E1M0/03.txt";
    char expectedString[] = "song\n";
    char buffer[1024];
    memset(buffer, 0, 1024);

    int expectedStringLen = 5;
    int lengthTest = 1000;
    int offset = 7;

    //should return 5
    int retValue = testWad->getContents(testPath, buffer, lengthTest, offset);
    if (retValue != expectedStringLen)
    {
        std::cout << "GETCONTENTSTEST3 FAILED " << "RETVALUE != EXPECTEDSTRINGLEN" << std::endl;
    }

    if (memcmp(expectedString, buffer, expectedStringLen) != 0)
    {
        std::cout << "GETCONTENTSTEST2 FAILED " << "memcmp(expectedString, buffer, expectedStringLen) != 0" << std::endl;
    }

    delete testWad;
}
void getcontentstest4()
{
    std::string wad_path = setupWorkspace();
    Wad* testWad = Wad::loadWad(wad_path);

    //getContents Test 4, trying to read past end of file (offset is too large)
    std::string testPath = "/E1M0/03.txt";
    char buffer[1024];
    memset(buffer, 0, 1024);

    int expectedStringLen = 5;
    int lengthTest = 5;
    int offset = 1000;

    //should return 0
    int retValue = testWad->getContents(testPath, buffer, lengthTest, offset);
    if (retValue != 0)
    {
        std::cout << "GETCONTENTSTEST4 FAILED " << "RETVALUE != 0" << std::endl;
    }
    delete testWad;
}
void getcontentstest5()
{
    std::string wad_path = setupWorkspace();
    Wad* testWad = Wad::loadWad(wad_path);

    //getContents Test 5, passing getContents a bad path
    std::string testPath = "/ba/dfile";
    char buffer[1024];
    memset(buffer, 0, 1024);

    int retValue = testWad->getContents(testPath, buffer, 5);
    if (retValue != -1)
    {
        std::cout << "GETCONTENTSTEST5 FAILED " << "RETVALUE != -1" << std::endl;
    }

    delete testWad;
}
void getcontentstest6()
{
    std::string wad_path = setupWorkspace();
    Wad* testWad = Wad::loadWad(wad_path);

    //getContents Test 6, passing getContents a directory
    std::string testPath = "/Gl/";
    char buffer[1024];
    memset(buffer, 0, 1024);

    int retValue = testWad->getContents(testPath, buffer, 5);
    if (retValue != -1)
    {
        std::cout << "GETCONTENTSTEST6 FAILED " << "RETVALUE != -1" << std::endl;
    }

    delete testWad;
}
void getcontentstest7()
{
    std::string wad_path = setupWorkspace();
    Wad* testWad = Wad::loadWad(wad_path);

    //getContents Test 7, passing getContents an empty path
    std::string testPath = "";
    char buffer[1024];
    memset(buffer, 0, 1024);

    int retValue = testWad->getContents(testPath, buffer, 5);
    if (retValue != -1)
    {
        std::cout << "GETCONTENTSTEST7 FAILED " << "RETVALUE != -1" << std::endl;
    }

    delete testWad;
}
void getcontentstest8()
{
    /*
    std::string wad_path = setupWorkspace();
    Wad* testWad = Wad::loadWad(wad_path);

    //getContents Test 8, reading large image file
    std::string testPath = "/Gl/ad/os/cake.jpg";
    int size = testWad->getSize(testPath);
    if (size != 29869)
    {

    }

    char* expectedContents = new char[30000];
    char* buffer = new char[30000];

    int file_fd = open("./testfiles/cake.jpg", O_RDONLY, 0777);
    read(file_fd, expectedContents, size);

    int ret = testWad->getContents(testPath, buffer, 29869);
    ASSERT_EQ(ret, 29869);
    ASSERT_EQ(memcmp(expectedContents, buffer, 29869), 0);

    delete[] expectedContents;
    delete[] buffer;
    delete testWad;
    */
    std::cout << "DONT RUN 'getcontentstest8' IT REQUIRES LINUX" << std::endl;
}






void getdirectorytest1()
{
    std::string wad_path = setupWorkspace();
    Wad* testWad = Wad::loadWad(wad_path);

    //getDirectoryTest 1, E1M0
    std::string testPath = "/E1M0/";

    std::vector<std::string> expectedVector = {
            "01.txt",
            "02.txt",
            "03.txt",
            "04.txt",
            "05.txt",
            "06.txt",
            "07.txt",
            "08.txt",
            "09.txt",
            "10.txt"
    };

    std::vector<std::string> testVector;
    //should return 10
    int ret = testWad->getDirectory(testPath, &testVector);


    delete testWad;
}
void getdirectorytest2()
{
    std::string wad_path = setupWorkspace();
    Wad* testWad = Wad::loadWad(wad_path);

    //getDirectoryTest 2, '/Gl/ad'
    std::string testPath = "/Gl/ad";

    std::vector<std::string> expectedVector = {
            "os"
    };

    std::vector<std::string> testVector;
    //should return 1
    int ret = testWad->getDirectory(testPath, &testVector);

    delete testWad;
}

void createdirectorytest1()
{
    std::string wad_path = setupWorkspace();
    Wad* testWad = Wad::loadWad(wad_path);

    //createDirectory Test 1, create dir at root
    std::string testPath = "/ex/";

    testWad->createDirectory(testPath);



    std::vector<std::string> testVector;
    //should return 0
    int ret = testWad->getDirectory(testPath, &testVector);

    testVector.clear();
    //should return 4
    ret = testWad->getDirectory("/", &testVector);


    std::vector<std::string> expectedVector = {
            "E1M0",
            "Gl",
            "mp.txt",
            "ex",
    };



    //Deleting and reinitiating object
    delete testWad;
    testWad = Wad::loadWad(wad_path);


    testVector.clear();
    //should return 0
    ret = testWad->getDirectory(testPath, &testVector);


    testVector.clear();
    //should return 4
    ret = testWad->getDirectory("/", &testVector);


    delete testWad;
}
void createdirectorytest2()
{
    std::string wad_path = setupWorkspace();
    Wad* testWad = Wad::loadWad(wad_path);

    //createDirectory Test 2, create dir at existing directory
    std::string testPath = "/Gl/ex";

    testWad->createDirectory(testPath);
    //should return true
    testWad->isDirectory(testPath);
    //should return false
    testWad->isContent(testPath);

    std::vector<std::string> testVector;
    //should return 0
    int ret = testWad->getDirectory(testPath, &testVector);

    testVector.clear();
    //should return 2
    ret = testWad->getDirectory("/Gl", &testVector);
    std::vector<std::string> expectedVector = {
            "ad",
            "ex",
    };

    //Deleting and reinitiating object
    delete testWad;
    testWad = Wad::loadWad(wad_path);

    //should return true
    testWad->isDirectory(testPath);
    //should return false
    testWad->isContent(testPath);

    testVector.clear();
    //should return 0
    ret = testWad->getDirectory(testPath, &testVector);

    testVector.clear();
    //should return 2
    ret = testWad->getDirectory("/Gl", &testVector);


    delete testWad;
}

void createfiletest1()
{
    std::string wad_path = setupWorkspace();
    Wad* testWad = Wad::loadWad(wad_path);

    //createFile Test 1, creating file in root
    std::string testPath = "/file.txt";

    testWad->createFile(testPath);

    //should return true
    testWad->isContent(testPath);
    //should return false
    testWad->isDirectory(testPath);

    std::vector<std::string> testVector;
    //should return 4
    int ret = testWad->getDirectory("/", &testVector);

    std::vector<std::string> expectedVector = {
            "E1M0",
            "Gl",
            "mp.txt",
            "file.txt"
    };


    //Reinstantiating object and rerunning tests
    delete testWad;
    testWad = Wad::loadWad(wad_path);

    //should return true
    testWad->isContent(testPath);
    //should return false
    testWad->isDirectory(testPath);

    testVector.clear();
    //should return 4
    ret = testWad->getDirectory("/", &testVector);

    delete testWad;
}
void createfiletest3()
{
    std::string wad_path = setupWorkspace();
    Wad* testWad = Wad::loadWad(wad_path);

    //createFile Test 3, passing invalid name, too long
    std::string testPath = "/Gl/ad/example.txt";

    testWad->createFile(testPath);

    std::vector<std::string> testVector;
    int ret = testWad->getDirectory("/Gl/ad/", &testVector);

    // ASSERT_FALSE(testWad->isContent(testPath));
     //ASSERT_FALSE(testWad->isDirectory(testPath));

     //ASSERT_EQ(ret, 1);
     //ASSERT_EQ(testVector.size(), 1);

    std::vector<std::string> expectedVector = {
            "os"
    };

    // ASSERT_EQ(expectedVector, testVector);

    delete testWad;
}
void createfiletest6()
{
    std::string wad_path = setupWorkspace();
    Wad* testWad = Wad::loadWad(wad_path);

    //createFile Test 6, passing invalid name in E#M# format
    std::string testPath = "/E1M1";

    testWad->createFile(testPath);

    //should return false
    testWad->isContent(testPath);
    //should return false
    testWad->isDirectory(testPath);

    std::vector<std::string> testVector;
    //should return 3
    int ret = testWad->getDirectory("/", &testVector);

    std::vector<std::string> expectedVector = {
            "E1M0",
            "Gl",
            "mp.txt",
    };

    delete testWad;
}

void writetofiletest1()
{
    std::string wad_path = setupWorkspace();
    Wad* testWad = Wad::loadWad(wad_path);

    //writeToFile Test 1, creating and writing to a text file
    std::string testPath = "/file.txt";

    //testing file creation

    testWad->createFile(testPath);

    //should return true
    testWad->isContent(testPath);
    //should return false
    testWad->isDirectory(testPath);

    std::vector<std::string> testVector;
    //should return 4
    int ret = testWad->getDirectory("/", &testVector);

    std::vector<std::string> expectedVector = {
            "E1M0",
            "Gl",
            "mp.txt",
            "file.txt"
    };

    //testing file writing
    const char expectedFileContents[] = "Hello! This is a test to make sure I can write to files.\n";
    int expectedSizeOfFile = 57;

    //should return 57
    ret = testWad->writeToFile(testPath, expectedFileContents, expectedSizeOfFile);

    char buffer[100];
    memset(buffer, 0, 100);
    //should return 57
    ret = testWad->getContents(testPath, buffer, expectedSizeOfFile);
    std::cout << "Expected string: " << buffer << std::endl;
    std::cout << "Returned string: " << expectedFileContents << std::endl;

    //Reinstantiating object and rerunning tests
    delete testWad;
    testWad = Wad::loadWad(wad_path);

    memset(buffer, 0, 100);
    //should return 57
    ret = testWad->getContents(testPath, buffer, expectedSizeOfFile);
    std::cout << "Expected string: " << buffer << std::endl;
    std::cout << "Returned string: " << expectedFileContents << std::endl;

    //Attemping to write to the file again

    //should return 0
    ret = testWad->writeToFile(testPath, expectedFileContents, expectedSizeOfFile);
    //path is technically valid, but the file already exists.
    //function should return 0, to signify that the path is valid
    //but the the write should still fail.

    memset(buffer, 0, 100);
    //should return 57
    ret = testWad->getContents(testPath, buffer, expectedSizeOfFile);
    std::cout << "Expected string: " << expectedFileContents << std::endl;
    std::cout << "Returned string: " << buffer << std::endl;


    delete testWad;
}
void writetofiletest3()
{
    std::string wad_path = setupWorkspace();
    Wad* testWad = Wad::loadWad(wad_path);

    //writeToFile Test 3, writing to an existing directory
    std::string testPath = "/E1M0/";
    const char testWriteContent[] = "Example text";
    int sizeOfWrite = sizeof(testWriteContent);
    //should return -1
    int ret = testWad->writeToFile(testPath, testWriteContent, sizeOfWrite);

    delete testWad;
}


void bigtest()
{
    std::string wad_path = setupWorkspace();
    Wad* testWad = Wad::loadWad(wad_path);

    //Creating dir '/Ex'
    testWad->createDirectory("/Ex");
    std::vector<std::string> expectedVector = { "E1M0","Gl","mp.txt","Ex" };
    //should return true
    if (!testWad->isDirectory("/Ex"))
    {
        std::cout << "FAILED BIGTEST " << "/Ex" << "DID NOT RETURN TRUE" << std::endl;
    }
    std::vector<std::string> testVector;
    int ret = testWad->getDirectory("/", &testVector);
    if (ret != 4)
    {
        std::cout << "FAILED BIGTEST " << "RET" << "DID NOT RETURN 4" << std::endl;
    }
    if(testVector.size() != 4)
    {
        std::cout << "FAILED BIGTEST " << "testVector.size()" << "DID NOT RETURN 4" << std::endl;
    }
    if (testVector != expectedVector)
    {
        std::cout << "FAILED BIGTEST " << "testVector != expectedVector" << "" << std::endl;
    }

    //Creating dir '/Ex/ad/'
    testWad->createDirectory("/Ex/ad/");
    if (!testWad->isDirectory("/Ex/ad/"))
    {
        std::cout << "FAILED BIGTEST " << "/Ex/ad/" << "DID NOT RETURN TRUE" << std::endl;
    }
    testVector.clear();
    expectedVector.clear();
    expectedVector = { "ad" };
    ret = testWad->getDirectory("/Ex", &testVector);
    if(ret!= 1)
    {
        std::cout << "FAILED BIGTEST " << "RET" << "DID NOT RETURN 1" << std::endl;
    }
    if (testVector.size() != 1)
    {
        std::cout << "FAILED BIGTEST " << "testVector.size() != 1" << "" << std::endl;
    }
    if (testVector != expectedVector)
    {
        std::cout << "FAILED BIGTEST " << "testVector != expectedVector" << "" << std::endl;
    }

    //Creating and writing to '/Ex/ad/test.txt'
    testWad->createFile("/Ex/ad/test.txt");
    if(!testWad->isContent("/Ex/ad/test.txt"))
    {
        std::cout << "FAILED BIGTEST " << "/Ex/ad/test.txt" << "DID NOT RETURN TRUE" << std::endl;
    }
    testVector.clear();
    expectedVector.clear();
    expectedVector = { "test.txt" };
    ret = testWad->getDirectory("/Ex/ad/", &testVector);
    if (ret != 1)
    {
        std::cout << "FAILED BIGTEST " << "RET" << "DID NOT RETURN 1" << std::endl;
    }
    if (testVector.size() != 1)
    {
        std::cout << "FAILED BIGTEST " << "testVector.size() != 1" << "" << std::endl;
    }
    if (testVector != expectedVector)
    {
        std::cout << "FAILED BIGTEST " << "testVector != expectedVector" << "" << std::endl;
    }

    const char inputText[] =
        "This text is pretty long, but will be "
        "concatenated into just a single string. "
        "The disadvantage is that you have to quote "
        "each part, and newlines must be literal as "
        "usual.";

    //Removing one from size to exclude null term
    int inputSize = 170;

    ret = testWad->writeToFile("/Ex/ad/test.txt", inputText, inputSize);
    if (ret != inputSize)
    {
        std::cout << "FAILED BIGTEST " << "RET" << "DID NOT RETURN inputSize" << std::endl;
    }

    char buffer[200];
    memset(buffer, 0, 200);
    ret = testWad->getContents("/Ex/ad/test.txt", buffer, inputSize);
    if (ret != inputSize)
    {
        std::cout << "FAILED BIGTEST " << "RET" << "DID NOT RETURN inputSize" << std::endl;
    }
    if (testWad->getSize("/Ex/ad/test.txt") != inputSize)
    {
        std::cout << "FAILED BIGTEST " << "RET" << "DID NOT RETURN inputSize" << std::endl;
    }
    if (memcmp(buffer, inputText, inputSize)!= 0)
    {
        std::cout << "FAILED BIGTEST " << "memcmp" << "DID NOT RETURN 0" << std::endl;
    }

    //Creating dir '/Ex/ad/os/'
    testWad->createDirectory("/Ex/ad/os");
    if (testWad->isDirectory("/Ex/ad/os") != true)
    {
        std::cout << "FAILED BIGTEST " << "/Ex/ad/os" << "DID NOT RETURN TRUE" << std::endl;
    }
    testVector.clear();
    expectedVector.clear();
    expectedVector = { "test.txt", "os" };
    ret = testWad->getDirectory("/Ex/ad", &testVector);
    if (ret != 2)
    {
        std::cout << "FAILED BIGTEST " << "RET" << "DID NOT RETURN 2" << std::endl;
    }
    if (testVector.size() != 2)
    {
        std::cout << "FAILED BIGTEST " << "testVector.size() != 2" << "" << std::endl;
    }
    if (testVector != expectedVector)
    {
        std::cout << "FAILED BIGTEST " << "testVector != expectedVector" << "" << std::endl;
    }

    //Creating and writing to '/Ex/ad/os/test.txt'
    testWad->createFile("/Ex/ad/os/test.txt");
    if (testWad->isContent("/Ex/ad/os/test.txt") != true)
    {
        std::cout << "FAILED BIGTEST " << "/Ex/ad/os/test.txt" << "DID NOT RETURN TRUE" << std::endl;
    }
    testVector.clear();
    expectedVector.clear();
    expectedVector = { "test.txt" };
    ret = testWad->getDirectory("/Ex/ad/os", &testVector);
    if (ret != 1)
    {
        std::cout << "FAILED BIGTEST " << "RET" << "DID NOT RETURN 2" << std::endl;
    }
    if (testVector.size() != 1)
    {
        std::cout << "FAILED BIGTEST " << "testVector.size() != 1" << "" << std::endl;
    }
    if (testVector!= expectedVector)
    {
        std::cout << "FAILED BIGTEST " << "testVector != expectedVector" << "" << std::endl;
    }

    ret = testWad->writeToFile("/Ex/ad/os/test.txt", inputText, inputSize);
    if (ret!= inputSize)
    {
        std::cout << "FAILED BIGTEST " << "RET" << "DID NOT RETURN inputSize" << std::endl;
    }

    memset(buffer, 0, 200);
    ret = testWad->getContents("/Ex/ad/os/test.txt", buffer, inputSize);
    if (ret != inputSize)
    {
        std::cout << "FAILED BIGTEST " << "RET" << "DID NOT RETURN inputSize" << std::endl;
    }
    if (testWad->getSize("/Ex/ad/os/test.txt") != inputSize)
    {
        std::cout << "FAILED BIGTEST " << "RET" << "DID NOT RETURN inputSize" << std::endl;
    }
    if (memcmp(buffer, inputText, inputSize) != 0)
    {
        std::cout << "FAILED BIGTEST " << "memcmp" << "DID NOT RETURN 0" << std::endl;
    }


    //Deleting and reinstantiating object
    delete testWad;
    testWad = Wad::loadWad(wad_path);

    testVector.clear();
    expectedVector.clear();
    expectedVector = { "ad" };
    ret = testWad->getDirectory("/Ex/", &testVector);
    if (!testWad->isDirectory("/Ex/"))
    {
        std::cout << "FAILED BIGTEST " << "/Ex/" << "DID NOT RETURN TRUE" << std::endl;
    }
    if (!testWad->isDirectory("/Ex/ad"))
    {
        std::cout << "FAILED BIGTEST " << "/Ex/ad" << "DID NOT RETURN TRUE" << std::endl;
    }
    if (testWad->isContent("/Ex/test.txt"))
    {
        std::cout << "FAILED BIGTEST " << "/Ex/ad" << "DID NOT RETURN FALSE" << std::endl;
    }
    if (ret!= 1)
    {
        std::cout << "FAILED BIGTEST " << "RET" << "DID NOT RETURN 1" << std::endl;
    }
    if(testVector.size()!= 1)
    {
        std::cout << "FAILED BIGTEST " << "testVector.size()" << "DID NOT RETURN 1" << std::endl;
    }
    if (testVector != expectedVector)
    {
        std::cout << "FAILED BIGTEST " << "testVector != expectedVector" << "" << std::endl;
    }

    testVector.clear();
    expectedVector.clear();
    expectedVector = { "test.txt", "os" };
    ret = testWad->getDirectory("/Ex/ad", &testVector);
    if (ret != 2)
    {
        std::cout << "FAILED BIGTEST " << "RET" << "DID NOT RETURN 2" << std::endl;
    }
    if (testVector.size() != 2)
    {
        std::cout << "FAILED BIGTEST " << "testVector.size()" << "DID NOT RETURN 2" << std::endl;
    }
    if (testVector != expectedVector)
    {
        std::cout << "FAILED BIGTEST " << "testVector != expectedVector" << "" << std::endl;
    }

    testVector.clear();
    expectedVector.clear();
    expectedVector = { "test.txt" };
    ret = testWad->getDirectory("/Ex/ad/os", &testVector);
    if (ret != 1)
    {
        std::cout << "FAILED BIGTEST " << "RET" << "DID NOT RETURN 2" << std::endl;
    }
    if (testVector.size() != 1)
    {
        std::cout << "FAILED BIGTEST " << "testVector.size()" << "DID NOT RETURN 1" << std::endl;
    }
    if (testVector != expectedVector)
    {
        std::cout << "FAILED BIGTEST " << "testVector != expectedVector" << "" << std::endl;
    }

    if (!testWad->isContent("/Ex/ad/test.txt"))
    {
        std::cout << "FAILED BIGTEST " << "/Ex/ad/test.txt" << "DID NOT RETURN TRUE" << std::endl;
    }
    if (testWad->getSize("/Ex/ad/test.txt")!= inputSize)
    {
        std::cout << "FAILED BIGTEST " << "/Ex/ad/test.txt" << "DID NOT RETURN inputSize" << std::endl;
    }
    memset(buffer, 0, 200);
    ret = testWad->getContents("/Ex/ad/test.txt", buffer, inputSize);
    if (ret != inputSize)
    {
        std::cout << "FAILED BIGTEST " << "RET" << "DID NOT RETURN INPUTSIZE" << std::endl;
    }
    if (memcmp(buffer, inputText, inputSize)!= 0)
    {
        std::cout << "FAILED BIGTEST " << "memcmp" << "DID NOT RETURN 0" << std::endl;
    }

    if (!testWad->isContent("/Ex/ad/os/test.txt"))
    {
        std::cout << "FAILED BIGTEST " << "/Ex/ad/os/test.txt" << "DID NOT RETURN TRUE" << std::endl;
    }
    if (testWad->getSize("/Ex/ad/os/test.txt")!= inputSize)
    {
        std::cout << "FAILED BIGTEST " << "/Ex/ad/os/test.txt" << "DID NOT RETURN INPUTSIZE" << std::endl;
    }
    memset(buffer, 0, 200);
    ret = testWad->getContents("/Ex/ad/os/test.txt", buffer, inputSize);
    if (ret != inputSize)
    {
        std::cout << "FAILED BIGTEST " << "RET" << "DID NOT RETURN INPUTSIZE" << std::endl;
    }
    if (memcmp(buffer, inputText, inputSize) != 0)
    {
        std::cout << "FAILED BIGTEST " << "memcmp" << "DID NOT RETURN 0" << std::endl;
    }

    delete testWad;
}


int main()
{
    getmagic();
    iscontent();
    isdirectory();
    getsize();

    getcontentstest1();
    getcontentstest2();
    getcontentstest3();
    getcontentstest4();
    getcontentstest5();
    getcontentstest6();
    getcontentstest7();

    //getcontentstest8

    createdirectorytest1();
    createdirectorytest2();

    createfiletest1();
    createfiletest3();
    createfiletest6();

    writetofiletest1();
    getdirectorytest1();
    getdirectorytest2();

    bigtest();
	return 0;
}





