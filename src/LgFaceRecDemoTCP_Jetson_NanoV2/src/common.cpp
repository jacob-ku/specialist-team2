//
// Created by zhou on 18-4-30.
//

#include "common.h"
#include "crypto_op.h"
#include "Logger.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


void* safeCudaMalloc(size_t memSize)
{
    void* deviceMem;
    CHECK(cudaMalloc(&deviceMem, memSize));
    if (deviceMem == nullptr)
    {
        std::cerr << "Out of memory" << std::endl;
        exit(1);
    }
    return deviceMem;
}


std::vector<std::pair<int64_t, nvinfer1::DataType>>
calculateBindingBufferSizes(const nvinfer1::ICudaEngine& engine, int nbBindings, int batchSize)
{
    std::vector<std::pair<int64_t, nvinfer1::DataType>> sizes;
    for (int i = 0; i < nbBindings; ++i)
    {
        nvinfer1::Dims dims = engine.getBindingDimensions(i);
        nvinfer1::DataType dtype = engine.getBindingDataType(i);

        int64_t eltCount = volume(dims) * batchSize;
        sizes.push_back(std::make_pair(eltCount, dtype));
    }

    return sizes;
}


inline int64_t volume(const nvinfer1::Dims& d)
{
    int64_t v = 1;
    for (int64_t i = 0; i < d.nbDims; i++)
        v *= d.d[i];
    return v;
}


void getFilePaths(std::string imagesPath, std::vector<struct Paths>& paths) {
    // std::cout << "Parsing Directory: " << imagesPath << std::endl;
    DIR *dir;
    struct dirent *entry;
    if ((dir = opendir (imagesPath.c_str())) != NULL) {
        while ((entry = readdir (dir)) != NULL) {
            std::string fileExtCheck(entry->d_name);
            if (entry->d_type != DT_DIR
                && ((fileExtCheck.find(".jpg") != std::string::npos)
                    || (fileExtCheck.find(".png") != std::string::npos)))
            {
                struct Paths tempPaths;
                tempPaths.fileName = std::string(entry->d_name);
                tempPaths.absPath = imagesPath + "/" + tempPaths.fileName;
                paths.push_back(tempPaths);
            }
        }
        closedir (dir);
    }
}


void getEncryptedFilePaths(std::string imagesPath, std::vector<struct Paths>& paths) {
    // std::cout << "Parsing Directory: " << imagesPath << std::endl;
    DIR *dir;
    struct dirent *entry;
    if ((dir = opendir (imagesPath.c_str())) != NULL) {
        while ((entry = readdir (dir)) != NULL) {
            std::string fileExtCheck(entry->d_name);
            if (entry->d_type != DT_DIR && (fileExtCheck.find(".") == std::string::npos)) {
                struct Paths tempPaths;
                tempPaths.fileName = std::string(entry->d_name);
                tempPaths.absPath = imagesPath + "/" + tempPaths.fileName;
                paths.push_back(tempPaths);
            }
        }
        closedir (dir);
    }
}

bool loadInputImage(std::string inputFilePath, cv::Mat& image, std::string& name, int videoFrameWidth, int videoFrameHeight) {
    unsigned int maximagesize = 1024 * 1024;
    unsigned char *buf;  /* decrypted buffer */
    char name_buf[128] = {0, };
    unsigned int imagesize = 0;

    buf = new (std::nothrow) unsigned char [maximagesize];
    if (buf == NULL) return false;
    // std::cout << "input file : " << inputFilePath << std::endl;

    if (!decrypt_file(inputFilePath.c_str(), buf, &imagesize, name_buf))
    {
        LOGGER->Log("File decryption is failed");
        delete [] buf;
        return false;
    }

    cv::imdecode(cv::Mat(imagesize, 1, CV_8UC1, buf), cv::IMREAD_COLOR, &image);
    delete [] buf;

    cv::resize(image, image, cv::Size(videoFrameWidth, videoFrameHeight));

    std::string filename = std::string(name_buf);
    std::size_t index = filename.find_last_of(".");
    name = filename.substr(0, index);

    // std::cout << "Finish loadInputImage(), name - " << name << std::endl;
    return true;
}


bool canFileOpen(const char *sFileName)
{
    bool   ret = false;

    try {
        int f;
        if((f = open(sFileName, O_RDONLY)) < 0) throw (sFileName);
        close(f);
        ret = true;
    }
    catch (char *fname) {
        printf("\"%s\" cannot be open.\n", fname);
    }

    return ret;
}

bool isLinkFile(const char *fileName)
{
    struct stat buf;
    int x;
    
    x = lstat(fileName, &buf);
    return(S_ISLNK(buf.st_mode));
}
