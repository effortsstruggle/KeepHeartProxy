
#ifdef _WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif

#include "ID3v2.h"

#include <iostream>
#include <fstream>
 #include<stdio.h>
 #include<string.h>
std::streampos getFileSize(const std::string & imagePath) {

	std::fstream fs;
    fs.open(imagePath, std::ios::in | std::ios::binary);
	

	if (fs.is_open()) {
        fs.seekg(0, std::ios::end);
		std::streampos size = fs.tellg();
        fs.seekg(0, std::ios::end);
		return size;
	}
	else {
		// �ļ���ʧ��
		return -1;
	}
}

std::string getfileExtend(const std::string &path)
{
	int nLastSlashIndex = path.find_last_of('.');
	//��ȡ�ļ���׺
	std::string strSuffix = path.substr(nLastSlashIndex + 1, path.length());
    return strSuffix;
}
// -------------------------------------- Write -------------------------------------- //

void ID3v2::writeFile(const char *bufferTagHeader, const char *bufferFrames, const uint32_t &bufferFramesSize, const char *bufferNewFrame, const uint32_t &bufferNewFrameSize, const char *bufferSongData, const uint32_t &bufferSongDataSize, const int addPadding){
    ptrSongFile.close();
    ptrSongFile.open(songPath, std::ios::out | std::ios::binary | std::ios::trunc);

    ptrSongFile.seekp(std::ios::beg);
    if(bufferTagHeader != nullptr)  ptrSongFile.write(bufferTagHeader, 10);
    if(bufferFrames != nullptr)     ptrSongFile.write(bufferFrames, bufferFramesSize);
    if(bufferNewFrame != nullptr)   ptrSongFile.write(bufferNewFrame, bufferNewFrameSize);

    char zero = 0;
    for(int i = 0; i < addPadding; i++){
        ptrSongFile.write(reinterpret_cast<char *>(&zero), 1); 
    }

    if(bufferSongData != nullptr)  ptrSongFile.write(bufferSongData, bufferSongDataSize);

    ptrSongFile.close();
    ptrSongFile.open(songPath, std::ios::in | std::ios::binary);
}


// ------------------------------------- Buffers ------------------------------------- //

char *ID3v2::bufferSong(uint32_t &bufferSongSize){
    uint32_t fileSize = getFileSize(songPath);

    char *bufferSongData = nullptr;

    bufferSongSize = fileSize - (tagHeader.size + 10);

    // Remove footer if it exists
    if((1 & (tagHeader.flags >> 5)) == 1 && tagHeader.ver[0] == 0x04)  bufferSongSize -= 10;

    // Add song data to buffer
    ptrSongFile.seekg(tagHeader.size + 10, std::ios::beg);
    bufferSongData = new char[bufferSongSize];
    ptrSongFile.read(bufferSongData, bufferSongSize);

    return bufferSongData;
}

char *ID3v2::newTagHeader(const short int &version, const uint32_t &newTagSize){
    char *bufferNewTagHeader = new char[10];

    /* Indicator -> */  strcpy(bufferNewTagHeader, "ID3");

    /* Version -> */  bufferNewTagHeader[3] = version;
    /* Revision -> */  bufferNewTagHeader[4] = 0;

    /* Flags -> */  bufferNewTagHeader[5] = 0;

    /* Size -> */  uint32_t newTagSizeSynced = htonl( to_sync(newTagSize) );
    /* Size -> */  memcpy(bufferNewTagHeader + 6, &newTagSizeSynced, 4);

    return bufferNewTagHeader;
}

char *ID3v2::bufferFrame(const std::unique_ptr<ID3v2_3_4_FrameHeader> &frHeader){
    int bufferFrameSize = frHeader->frSize + 10;

    char *oneFrame = new char[bufferFrameSize];

    // Add the whole frame to the buffer
    ptrSongFile.seekg(frHeader->cPos - 10);
    ptrSongFile.read(oneFrame, bufferFrameSize);

    return oneFrame;
}


// ------------------------------------- Frames ------------------------------------- //

char *ID3v2::newFrameTIF(const std::string &newFrameID, const std::string &content, uint32_t &newFullFrameSize, const short int &tagVersion){
    uint32_t newFrameSize = content.length() + 1 + 1;   // +1: Encoding, +1: Content null terminator
    newFullFrameSize = newFrameSize + 10;

    char *newFrame = new char[newFullFrameSize];

    // -------- Frame header -------- //
    /* Frame ID -> */  char *ID = str2ch(newFrameID, 4, 0);
    /* Frame ID -> */  memcpy(newFrame, ID, 4);
    /* Frame ID -> */  delete[] ID;

    /* Frame Size -> */  uint32_t newFrameSizeSynced = htonl(newFrameSize);
    /* Frame Size -> */  if(tagVersion == 0x04) newFrameSizeSynced = htonl( to_sync(newFrameSize) ); 
    /* Frame Size -> */  memcpy(newFrame + 4, &newFrameSizeSynced, 4);

    /* Flags -> */  newFrame[8] = '\0';
    /* Flags -> */  newFrame[9] = '\0';

    // -------- Frame content -------- //
    /* Encoding -> */  newFrame[10] = 0x00;

    /* Content -> */  strcpy(newFrame + 11, content.c_str());

    return newFrame;
}

char *ID3v2::newFrameULF(const std::string &newFrameID, const std::string &content, uint32_t &newFullFrameSize, const short int &tagVersion){
    uint32_t newFrameSize = content.length() + 1;   // +1: Content null terminator
    newFullFrameSize = newFrameSize + 10;

    char *newFrame = new char[newFullFrameSize];

    // -------- Frame header -------- //
    /* Frame ID -> */  char *ID = str2ch(newFrameID, 4, 0);
    /* Frame ID -> */  memcpy(newFrame, ID, 4);
    /* Frame ID -> */  delete[] ID;

    /* Frame Size -> */  uint32_t newFrameSizeSynced = htonl(newFrameSize);
    /* Frame Size -> */  if(tagVersion == 0x04) newFrameSizeSynced = htonl( to_sync(newFrameSize) ); 
    /* Frame Size -> */  memcpy(newFrame + 4, &newFrameSizeSynced, 4);

    /* Flags -> */  newFrame[8] = '\0';
    /* Flags -> */  newFrame[9] = '\0';

    // -------- Frame content -------- //
    /* Content -> */  strcpy(newFrame + 10, content.c_str());

    return newFrame;
}

char *ID3v2::newFramePIC(const std::string &newFrameID, const std::string &imagePath, uint32_t &newFullFrameSize, const short int &tagVersion){
    // Image file
    std::fstream imageFile;
    imageFile.open(imagePath, std::ios::in | std::ios::binary);
	
    uint32_t fileSize = getFileSize(imagePath);

    if(!imageFile.is_open()){
        char *newFrame = new char[1];
        return newFrame;
    }

    // Image buffer
    char* bufferImage =new char[fileSize];
    memset(bufferImage, 0, fileSize);
    imageFile.read(bufferImage, fileSize);
    imageFile.close();

    // MIME precalculate
    int mimeSize = 0;
    std::string mimeType = "";
    if(getfileExtend(imagePath) == ".jpg"){
        mimeSize = 10;
        mimeType = "image/jpeg";
    }
    else if(getfileExtend(imagePath) == ".png"){
        mimeSize = 9;
        mimeType = "image/png";
    }
    else if(getfileExtend(imagePath) == ".bmp"){
        mimeSize = 12;
        mimeType = "image/bitmap";
    }
    else{
        mimeSize = 1;
        mimeType = "";
    }

    uint32_t newFrameSize = fileSize + 1 + (mimeSize + 1) + 1 + 1;   // +1: Encoding, +mimeSize with null terminator, +1: Desc null terminator, +1: Content null terminator 
    newFullFrameSize = newFrameSize + 10;

    char *newFrame = new char[newFullFrameSize];

    // -------- Frame header -------- //
    /* Frame ID -> */  char *ID = str2ch(newFrameID, 4, 0);
    /* Frame ID -> */  memcpy(newFrame, ID, 4);
    /* Frame ID -> */  delete[] ID;

    /* Frame Size -> */  uint32_t newFrameSizeSynced = htonl(newFrameSize);
    /* Frame Size -> */  if(tagVersion == 0x04) newFrameSizeSynced = htonl( to_sync(newFrameSize) ); 
    /* Frame Size -> */  memcpy(newFrame + 4, &newFrameSizeSynced, 4);

    /* Flags -> */  newFrame[8] = '\0';
    /* Flags -> */  newFrame[9] = '\0';

    // -------- Frame content -------- //
    /* Encoding -> */  newFrame[10] = 0x00;

    /* MIME type -> */  strcpy(newFrame+11, mimeType.c_str());

    /* Image type -> */  newFrame[12 + mimeSize] = '\3';

    /* Description -> */  newFrame[13 + mimeSize] = '\0';

    /* Content -> */  memcpy(newFrame + (14 + mimeSize), bufferImage, fileSize);

    return newFrame;
}

char *ID3v2::newFrameCOM(const std::string &newFrameID, const std::string &content, uint32_t &newFullFrameSize, const short int &tagVersion){
    uint32_t newFrameSize = content.length() + 1 + 3 + 1 + 1;   // +1: Encoding, +3: Language, +1: Desc null terminator, +1: Content null terminator 
    newFullFrameSize = newFrameSize + 10;

    char *newFrame = new char[newFullFrameSize];

    // -------- Frame header -------- //
    /* Frame ID -> */  char *ID = str2ch(newFrameID, 4, 0);
    /* Frame ID -> */  memcpy(newFrame, ID, 4);
    /* Frame ID -> */  delete[] ID;

    /* Frame Size -> */  uint32_t newFrameSizeSynced = htonl(newFrameSize);
    /* Frame Size -> */  if(tagVersion == 0x04) newFrameSizeSynced = htonl( to_sync(newFrameSize) ); 
    /* Frame Size -> */  memcpy(newFrame + 4, &newFrameSizeSynced, 4);

    /* Flags -> */  newFrame[8] = '\0';
    /* Flags -> */  newFrame[9] = '\0';

    // -------- Frame content -------- //
    /* Encoding -> */  newFrame[10] = 0x00;

    /* language -> */  const std::string indicator = "eng";
    /* Language -> */  strcpy(newFrame+11, indicator.c_str());

    /* Description -> */  newFrame[14] = '\0';

    /* Content -> */  strcpy(newFrame + 15, content.c_str());

    return newFrame;
}


// -------------------------------------- Set --------------------------------------- //

bool ID3v2::set(const std::string &whatID, const std::string &content){
    // Check if file exists
    if(!fileExists) return false;
    
    short int tagVersion = 0;  
    
    // Check if tag exists and search for the requested frame
    if(!tagExists){
        if(_DefaultSetVersion == 4){
            if(ID3v2_4::str2FrameID.count(whatID) != 0) tagVersion = 0x04;
            else if(ID3v2_3::str2FrameID.count(whatID) != 0) tagVersion = 0x03;
            else return false;
        }
        else {
            if(ID3v2_3::str2FrameID.count(whatID) != 0) tagVersion = 0x03;
            else if(ID3v2_4::str2FrameID.count(whatID) != 0) tagVersion = 0x04;
            else return false;
        }
    }
    else {
        tagVersion = tagHeader.ver[0];

        if(tagVersion == 0x02) return false;
        else if(tagVersion == 0x03 && ID3v2_3::str2FrameID.count(whatID) == 0) return false;
        else if(tagVersion == 0x04 && ID3v2_4::str2FrameID.count(whatID) == 0) return false;
    }

    int addPadding = 2044;

    // ---------------- Song Data ---------------- //
    uint32_t bufferSongDataSize = 0;
    char *bufferSongData = bufferSong(bufferSongDataSize);

    // ---------------- Save frames ---------------- //
    uint32_t bufferFramesSize = 0;
    char *bufferFrames = nullptr;
    
    if(tagExists){
        bufferFrames = new char[tagHeader.size - padding];
        for(auto it = frameHeader34Vector.begin(); it != frameHeader34Vector.end(); it++){
            std::string id_tmp = ch2str((*it)->frID, 4);

            if(id_tmp == whatID){
                continue;
            }

            char *oneFrame = bufferFrame(*it);
            memcpy(bufferFrames + bufferFramesSize, oneFrame, (*it)->frSize + 10);
            delete[] oneFrame;

            bufferFramesSize += (*it)->frSize + 10;
        }
    }
 
    // ---------------- New Frame ---------------- //
    uint32_t bufferNewFrameSize = 0;
    char *bufferNewFrame = nullptr;
    switch(tagVersion){
        case 0x03:
            if(checkTagType(ID3v2_3::str2FrameID[whatID]) == "TIF") bufferNewFrame = newFrameTIF(whatID, content, bufferNewFrameSize, tagVersion);
            else if(checkTagType(ID3v2_3::str2FrameID[whatID]) == "ULF") bufferNewFrame = newFrameULF(whatID, content, bufferNewFrameSize, tagVersion);
            else if(checkTagType(ID3v2_3::str2FrameID[whatID]) == "PIC") bufferNewFrame = newFramePIC(whatID, content, bufferNewFrameSize, tagVersion);
            else if(checkTagType(ID3v2_3::str2FrameID[whatID]) == "COM") bufferNewFrame = newFrameCOM(whatID, content, bufferNewFrameSize, tagVersion);

            else return false;
            break;
        
        case 0x04:
            if(checkTagType(ID3v2_4::str2FrameID[whatID]) == "TIF") bufferNewFrame = newFrameTIF(whatID, content, bufferNewFrameSize, tagVersion);
            else if(checkTagType(ID3v2_4::str2FrameID[whatID]) == "ULF") bufferNewFrame = newFrameULF(whatID, content, bufferNewFrameSize, tagVersion);
            else if(checkTagType(ID3v2_4::str2FrameID[whatID]) == "PIC") bufferNewFrame = newFramePIC(whatID, content, bufferNewFrameSize, tagVersion);            
            else if(checkTagType(ID3v2_4::str2FrameID[whatID]) == "COM") bufferNewFrame = newFrameCOM(whatID, content, bufferNewFrameSize, tagVersion);

            else return false;
            break;
    }

    // ---------------- New tag header ---------------- //
    char *bufferTagHeader = newTagHeader(tagVersion, bufferFramesSize + bufferNewFrameSize + addPadding);

    // ---------------- Apply changes ---------------- //
    writeFile(bufferTagHeader, bufferFrames, bufferFramesSize, bufferNewFrame, bufferNewFrameSize, bufferSongData, bufferSongDataSize, addPadding);

    // ---------------- Cleanup ---------------- //
    delete[] bufferNewFrame;
    delete[] bufferSongData;
    delete[] bufferTagHeader;
    delete[] bufferFrames;
    Reset();

    return true;
}

bool ID3v2::remove(const std::string &whatID){
    // Check if file exists
    if(!fileExists) return false;

    // Check if tag exists
    if(!tagExists) return false;

    // Check if the frame exists
    if(tagHeader.ver[0] == 0x02) return false;
    if(tagHeader.ver[0] == 0x03 && ID3v2_3::str2FrameID.count(whatID) == 0) return false;
    if(tagHeader.ver[0] == 0x04 && ID3v2_4::str2FrameID.count(whatID) == 0) return false;

    int addPadding = 2044;

    // ---------------- Song Data ---------------- //
    uint32_t bufferSongDataSize = 0;
    char *bufferSongData = bufferSong(bufferSongDataSize);

    // ---------------- Save frames ---------------- //
    uint32_t bufferFramesSize = 0;
    char *bufferFrames = new char[tagHeader.size - padding];
    for(auto it = frameHeader34Vector.begin(); it != frameHeader34Vector.end(); it++){
        std::string id_tmp = ch2str((*it)->frID, 4);

        if(id_tmp == whatID){
            continue;
        }

        char *oneFrame = bufferFrame(*it);
        memcpy(bufferFrames + bufferFramesSize, oneFrame, (*it)->frSize + 10);
        delete[] oneFrame;

        bufferFramesSize += (*it)->frSize + 10;
    }

    // ---------------- New tag header ---------------- //
    char *bufferTagHeader = newTagHeader(tagHeader.ver[0], bufferFramesSize + addPadding);
    
    // ---------------- Apply changes ---------------- //
    writeFile(bufferTagHeader, bufferFrames, bufferFramesSize, nullptr, 0, bufferSongData, bufferSongDataSize, addPadding);

    // ---------------- Cleanup ---------------- //
    delete[] bufferSongData;
    delete[] bufferTagHeader;
    delete[] bufferFrames;
    Reset();

    return true;
}

bool ID3v2::removeAll(){
    // Check if file exists
    if(!fileExists) return false;

    // Check if tag exists
    if(!tagExists) return false;

    uint32_t bufferSongDataSize = 0;
    char *bufferSongData = bufferSong(bufferSongDataSize);
    
    writeFile(nullptr, nullptr, 0, nullptr, 0, bufferSongData, bufferSongDataSize, 0);

    delete[] bufferSongData;
    Reset();

    return true;
}