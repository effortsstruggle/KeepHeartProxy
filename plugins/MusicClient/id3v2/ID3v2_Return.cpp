#include "ID3v2.h"
#include <iostream>

// ------------------------------------- Frames ------------------------------------- //

void ID3v2::readTIF(const unsigned int &cPos, const uint32_t &frSize){
    ptrSongFile.seekg(cPos, std::ios::beg);

    // Text encoding
    char encoding;
    ptrSongFile.read(reinterpret_cast<char *>(&encoding), 1);
    
    // Return the requested content acoording to encoding
    wchar_t* wch = 0;
    char* frameContent = 0;
    if(encoding == 0x01){
        wch = new wchar_t[(frSize - 1) / 2];
        for(int i = 0; i < (frSize-1) / 2; i++){
            ptrSongFile.read(reinterpret_cast<char *>(&wch[i]), 2);
        }
        std::wcout << UTF16_Decoder(wch, (frSize - 1) / 2);
        delete wch;
       
    }
    else{
       
        char * frameContent = new char[frSize - 1];
        ptrSongFile.read(reinterpret_cast<char *>(frameContent), frSize - 1);
        
        //std::cout
        strdata = ch2str(frameContent, frSize - 1);

       
        delete frameContent;
        
    }

}

void ID3v2::readULF(const unsigned int &cPos, const uint32_t &frSize){
    ptrSongFile.seekg(cPos, std::ios::beg);

    // The requested URL
    char* frameContent = new char[frSize];
    ptrSongFile.read(reinterpret_cast<char *>(frameContent), frSize);

    // Return data converted to string
    if(frameContent[frSize - 1] == '\0')
     ss << ch2str(frameContent, frSize - 1);
    else 
    strdata == ch2str(frameContent, frSize);

    delete frameContent;
}

void ID3v2::readCOM(const unsigned int &cPos, const uint32_t &frSize){
    ptrSongFile.seekg(cPos, std::ios::beg);

    unsigned int contentSize = 0;

    // Text encoding
    char encoding;
    ptrSongFile.read(reinterpret_cast<char *>(&encoding), 1);

    // Text language
    char language[3];
    ptrSongFile.read(reinterpret_cast<char *>(language), 3);

    // Encoding specific
    if(encoding == 0x01){
        // Short description
        int descCount = 0;
        wchar_t sDescription[30];
        for(; descCount < 30; descCount++){
            ptrSongFile.read(reinterpret_cast<char *>(&sDescription[descCount]), 2);
            if(sDescription[descCount] == L'\0') break;
        }
        descCount *= 2;

       
        contentSize = frSize - 1 - 3 - descCount;

        // Content
        wchar_t* wch = new wchar_t[contentSize / 2];
        for(int i = 0; i < contentSize / 2; i++){
            ptrSongFile.read(reinterpret_cast<char *>(&wch[i]), 2);
        }
        std::wcout << UTF16_Decoder(wch, contentSize / 2);
        delete wch;
    }
    else{
        // Short description
        int descCount = 0;
        char sDescription[30];
        for(; descCount < 30; descCount++){
            ptrSongFile.read(reinterpret_cast<char *>(&sDescription[descCount]), 1);
            if(sDescription[descCount] == '\0') break;
        }

      
        contentSize = frSize - 1 - 3 - descCount;

        // Content
        char *frameContent = new char[contentSize];
        ptrSongFile.read(reinterpret_cast<char *>(frameContent), contentSize);
        strdata == ch2str(frameContent, contentSize);
        delete frameContent;
    }
}

char *ID3v2::readPIC(const unsigned int &cPos, const uint32_t &frSize, int &imageSize){
    ptrSongFile.seekg(cPos, std::ios::beg);

    // Text encoding
    char encoding;
    ptrSongFile.read(reinterpret_cast<char *>(&encoding), 1);

    // MIME type
    int mimeCount = 0;
    char mimeType[15];
    for(; mimeCount < 15; mimeCount++){
        ptrSongFile.read(reinterpret_cast<char *>(&mimeType[mimeCount]), 1);
        if(mimeType[mimeCount] == '\0') break;
    }

    // Picture type
    char picType;
    ptrSongFile.read(reinterpret_cast<char *>(&picType), 1);

    // Description
    int descCount = 0;
    if(encoding == 0x01){
        wchar_t description[64];
        for(; descCount < 64; descCount++){
            ptrSongFile.read(reinterpret_cast<char *>(&description[descCount]), 2);
            if(description[descCount] == L'\0') break;
        }
        descCount *= 2;
    }
    else{
        char description[64];
        for(; descCount < 64; descCount++){
            ptrSongFile.read(reinterpret_cast<char *>(&description[descCount]), 1);
            if(description[descCount] == '\0') break;
        }
    }


 
    unsigned int contentSize = frSize - 1 - mimeCount - 1 - descCount;
    imageSize = contentSize;

    // The requested image
    char *frameContent = new char[contentSize];
    ptrSongFile.read(frameContent, contentSize);

    // Return requested image data
    return frameContent;
}


// ------------------------------------- Return ------------------------------------- //

bool ID3v2::get(const std::string &whatID){
    // Check if file exists
    if(!fileExists) return false;

    // Check if tag exists
    if(!tagExists)  return false;

    // Search for requested tag
    if(tagHeader.ver[0] == 0x02){
        for(auto it = frameHeader2Vector.begin(); it != frameHeader2Vector.end(); it++){
            std::string id_tmp = ch2str((*it)->frID, 3);

            if(id_tmp == whatID){
                if(checkTagType(ID3v2_2::str2FrameID[whatID]) == "TIF"){
                    readTIF((*it)->cPos, (*it)->frSize);
                    return true;
                }       
                else if(checkTagType(ID3v2_2::str2FrameID[whatID]) == "ULF"){
                    readULF((*it)->cPos, (*it)->frSize);
                    return true;
                }  
                else if(checkTagType(ID3v2_2::str2FrameID[whatID]) == "COM"){
                    readCOM((*it)->cPos, (*it)->frSize);
                    return true;
                }
                break;
            }
        }
    }
    else if(tagHeader.ver[0] == 0x03 || tagHeader.ver[0] == 0x04){
        for(auto it = frameHeader34Vector.begin(); it != frameHeader34Vector.end(); it++){
            std::string id_tmp = ch2str((*it)->frID, 4);

            if(id_tmp == whatID){
                switch(tagHeader.ver[0]){
                    case 0x03:
                        if(checkTagType(ID3v2_3::str2FrameID[whatID]) == "TIF"){
                            readTIF((*it)->cPos, (*it)->frSize);
                            return true;
                        }       
                        else if(checkTagType(ID3v2_3::str2FrameID[whatID]) == "ULF"){
                            readULF((*it)->cPos, (*it)->frSize);
                            return true;
                        }  
                        else if(checkTagType(ID3v2_3::str2FrameID[whatID]) == "COM"){
                            readCOM((*it)->cPos, (*it)->frSize);
                            return true;
                        }
                        break;

                    case 0x04:
                        if(checkTagType(ID3v2_4::str2FrameID[whatID]) == "TIF"){
                            readTIF((*it)->cPos, (*it)->frSize);
                            return true;
                        }       
                        else if(checkTagType(ID3v2_4::str2FrameID[whatID]) == "ULF"){
                            readULF((*it)->cPos, (*it)->frSize);
                            return true;
                        }  
                        else if(checkTagType(ID3v2_4::str2FrameID[whatID]) == "COM"){
                            readCOM((*it)->cPos, (*it)->frSize);
                            return true;
                        }
                        break;    
                }
            }
        }
    }

    return false;
}

char *ID3v2::getImage(int &imageSize){
    // Check if file exists
    if(!fileExists) return nullptr;
    
    // Check if tag exists
    if(!tagExists)  return nullptr;

    // Search for requested tag
    if(tagHeader.ver[0] == 0x02){
        for(auto it = frameHeader2Vector.begin(); it != frameHeader2Vector.end(); it++){
            std::string id_tmp = ch2str((*it)->frID, 3);

            if(id_tmp == "PIC") return readPIC((*it)->cPos, (*it)->frSize, imageSize);
        }
    }
    else if(tagHeader.ver[0] == 0x03 || tagHeader.ver[0] == 0x04){
        for(auto it = frameHeader34Vector.begin(); it != frameHeader34Vector.end(); it++){
            std::string id_tmp = ch2str((*it)->frID, 4);

            if(id_tmp == "APIC") return readPIC((*it)->cPos, (*it)->frSize, imageSize);
        }
    }

    return nullptr;
}