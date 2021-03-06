#include "pfm.h"

PagedFileManager* PagedFileManager::_pf_manager = 0;

PagedFileManager* PagedFileManager::instance()
{
    if(!_pf_manager)
        _pf_manager = new PagedFileManager();

    return _pf_manager;
}

    
PagedFileManager::PagedFileManager()
{
}


PagedFileManager::~PagedFileManager()
{
    //delete _pf_manager;
}


RC PagedFileManager::createFile(const string &fileName)
{
    // we must first test to see if the file already exists
    ifstream test_file(fileName.c_str());
    if (test_file.good()) {
        return -1;
    } else {
        ofstream new_file(fileName.c_str(), ios::binary);
        if (new_file.is_open()) {
            new_file.close();
            return 0;
        } else {
            return -1;
        }
    }
}


RC PagedFileManager::destroyFile(const string &fileName)
{
    if (!std::remove(fileName.c_str())) {
        return 0;
    } else {
        return -1;
    }
}


RC PagedFileManager::openFile(const string &fileName, FileHandle &fileHandle)
{
    // check if the file exists
    struct stat buffer;
    if (stat (fileName.c_str(), &buffer) == 0) {
        if(fileHandle.infile != NULL) {
            // this means the handle is associated with another file
            return 0;
        }
        // link this new file handle to this opened file
        fileHandle.outfile = new ofstream(fileName.c_str(), ios::binary | ios::in | ios::out);
        fileHandle.infile = new ifstream(fileName.c_str(), ios::binary);

        fileHandle.infile->seekg(0, ios::end);
        int length = fileHandle.infile->tellg();
        fileHandle.numPages = length / PAGE_SIZE;
        return 0;
    } else {
        return -1;
    }
}



RC PagedFileManager::closeFile(FileHandle &fileHandle)
{
    if (fileHandle.infile == NULL) {
        // file is not associated with a file (error)
        return -1;
    }
    // check to see if the file is open and close it
    if (fileHandle.infile->is_open()) {
        // the file exists and its open
        if (fileHandle.currentPage != NULL) {
            fileHandle.writePage(fileHandle.currentPageNum, fileHandle.currentPage); 
            fileHandle.currentPage = NULL;
            fileHandle.currentPageNum = -1;
        }
        // clear the free space list
        fileHandle.freeSpace.clear();
        fileHandle.numPages = 0;

        fileHandle.infile->close();
        fileHandle.outfile->close();
        delete fileHandle.infile;
        delete fileHandle.outfile;
        fileHandle.infile = NULL;
        fileHandle.outfile = NULL;
        return 0;
    }
    return -1;
}



FileHandle::FileHandle()
{
    readPageCounter = 0;
    writePageCounter = 0;
    appendPageCounter = 0;
    numPages = 0;
    infile = NULL;
    outfile = NULL;
    currentPage = NULL;
    currentPageNum = -1;
}


FileHandle::~FileHandle()
{
}


RC FileHandle::readPage(PageNum pageNum, void *data)
{
    /*infile->seekg(0, ios::end);
    int length = infile->tellg();

    // Added length guard to protect reading from outside of bounds
    if ((pageNum * PAGE_SIZE) >= length) {
        return -1;
    }*/

    if (pageNum >= numPages) {
        return -1;
    }

    if (infile != NULL && infile->is_open()) {
        infile->seekg(pageNum * PAGE_SIZE, ios::beg);
        infile->read(((char *) data), PAGE_SIZE);
        readPageCounter++;
        return 0;
    } else {
        return -1;
    }
}


RC FileHandle::writePage(PageNum pageNum, const void *data)
{
    if (outfile != NULL && outfile->is_open()) {
        outfile->seekp(pageNum * PAGE_SIZE, ios::beg);
        outfile->write(((char *) data), PAGE_SIZE);
        writePageCounter++;
        return 0;
    } else {
        return -1;
    }
}


RC FileHandle::appendPage(const void *data)
{
    if (outfile != NULL && outfile->is_open()) {
        outfile->seekp(0, ios::end);
        outfile->write(((char *) data), PAGE_SIZE);
        appendPageCounter++;
        numPages++;
        return 0;
    } else {
        return -1;
    }
}


unsigned FileHandle::getNumberOfPages()
{
    return numPages;
}


RC FileHandle::collectCounterValues(unsigned &readPageCount, unsigned &writePageCount, unsigned &appendPageCount)
{
    readPageCount = readPageCounter;
    writePageCount = writePageCounter;
    appendPageCount = appendPageCounter;
    return 0;
}
