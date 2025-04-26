#include <iostream>
#ifdef _WIN32
#include <winsock2.h>
#pragma comment(lib,"ws2_32.lib")
#else
#include <arpa/inet.h>
#endif

#include "ID3v2.h"

int ID3v2::_DefaultSetVersion = 3;

ID3v2::ID3v2(const std::string &songPath){
    this->songPath = songPath;
    
    // Load file
    ptrSongFile.open(songPath, std::ios::in | std::ios::binary);
    if(ptrSongFile.is_open()) fileExists = true;
    else fileExists = false;

    // Initialization
    padding = 0;

    e3TagHeader = nullptr;
    e4TagHeader = nullptr;
    t4Footer = nullptr;

    tagExists = false;
}

ID3v2::~ID3v2(){
    ptrSongFile.close();

    e3TagHeader = nullptr;
    e4TagHeader = nullptr;

    t4Footer = nullptr;

    frameHeader34Vector.clear();
}

// Utilization
bool ID3v2::processTags(){
    // Check if file exists
    if(!fileExists) return false;
    
    // Check tag header
    if(processTagHeader()){
        // Check for extended header's existance
        if((1 & (tagHeader.flags >> 6)) == 1 && (tagHeader.ver[0] == 0x03 || tagHeader.ver[0] == 0x04))  processExtendedTagHeader();

        // Find all frames
        processFrameHeader();

        // Check for footer's existance for v2.4
        if((1 & (tagHeader.flags >> 5)) == 1 && tagHeader.ver[0] == 0x04)  processFooter();

        tagExists = true;
        
        return true;
    }

    return false;
}

bool ID3v2::processTagHeader(){
    ptrSongFile.seekg(0, std::ios::beg);

    // Read tag header (Has to be ID3)
    ptrSongFile.read(reinterpret_cast<char *>(&tagHeader.tag), 3);
    if( !(tagHeader.tag[0] == 0x49 && tagHeader.tag[1] == 0x44 && tagHeader.tag[2] == 0x33) ) return false;
    // Read tag version
    ptrSongFile.read(reinterpret_cast<char *>(&tagHeader.ver), 2);    
    // Read tag flags
    ptrSongFile.read(reinterpret_cast<char *>(&tagHeader.flags), 1);
    
    // Check tag flags
    if(((tagHeader.flags >> 7) & 1) == 1) std::cout << "\nThe tag is unsynchronised";

    if(tagHeader.ver[0] == 0x02){
        totalFrames = 62; // Additionaly set total frame number

        if(((tagHeader.flags >> 6) & 1) == 1) return false; // Compression not supported
        if((tagHeader.flags << 2) == 1) std::cout << "\nThe tag may be corrupted!";
    }
    else if(tagHeader.ver[0] == 0x03){
        totalFrames = 74; // Additionaly set total frame number

        if((tagHeader.flags << 3) == 1) std::cout << "\nThe tag may be corrupted!";
    }
    else if(tagHeader.ver[0] == 0x04){
        totalFrames = 92; // Additionaly set total frame number

        if((tagHeader.flags << 4) == 1) std::cout << "\nThe tag may be corrupted!";
    }
    
    // Read whole tag's size and unsync it
    ptrSongFile.read(reinterpret_cast<char *>(&tagHeader.size), 4);
    tagHeader.size = to_unsync( ntohl(tagHeader.size) );

    return true;
}

void ID3v2::processExtendedTagHeader(){
    // Extended tag header for v2.3
    if(tagHeader.ver[0] == 0x03){
        e3TagHeader = std::make_unique<ID3v2_3_ExtendedTagHeader>();

        // Read extended header size
        ptrSongFile.read(reinterpret_cast<char *>(&e3TagHeader->eSize), 4);
        // Read extended header flags
        ptrSongFile.read(reinterpret_cast<char *>(e3TagHeader->eFlags), 2);
        // Read extended header flags
        ptrSongFile.read(reinterpret_cast<char *>(&e3TagHeader->padSize), 4);
        // Check if CRC data is present
        if((e3TagHeader->eFlags[0] >> 7) == 1){
            ptrSongFile.read(reinterpret_cast<char *>(&e3TagHeader->crc), 4);
        }
    }

    // Extended tag header for v2.4
    else if(tagHeader.ver[0] == 0x04){
        e4TagHeader = std::make_unique<ID3v2_4_ExtendedTagHeader>();

        // Read extended header size
        ptrSongFile.read(reinterpret_cast<char *>(&e4TagHeader->eSize), 4);
        // Read extended header number of flag bytes
        ptrSongFile.read(reinterpret_cast<char *>(&e4TagHeader->numFlags), 1);
        // Read extended header flags
        ptrSongFile.read(reinterpret_cast<char *>(&e4TagHeader->eFlags), 1);
        // Check if CRC data is present
        if((e3TagHeader->eFlags[0] >> 5) & 1 == 1){
            ptrSongFile.read(reinterpret_cast<char *>(e4TagHeader->crc), 5);
        }
        // Check if tag restrictions flag is present
        if((e3TagHeader->eFlags[0] >> 4) & 1 == 1){
            ptrSongFile.read(reinterpret_cast<char *>(&e4TagHeader->restrictFlag), 1);
        }
    }
}

void ID3v2::processFrameHeader(){
    // For v2.3 and v2.4
    if(tagHeader.ver[0] == 0x03 || tagHeader.ver[0] == 0x04){
        for(int frameNumber = 0; frameNumber < totalFrames; frameNumber++){
            auto frameHeader = std::make_unique<ID3v2_3_4_FrameHeader>();

            // Read frame TagID and convert it to a string
            ptrSongFile.read(reinterpret_cast<char *>(frameHeader->frID), 4);
            std::string id_tmp = ch2str(frameHeader->frID, 4);

            // Check frame's validity pt.1
            if(tagHeader.ver[0] == 0x03){
                if(ID3v2_3::str2FrameID.count(id_tmp) == 0){
                    ptrSongFile.seekg(-4, std::ios::cur);
                    break;
                }
            }
            else if(tagHeader.ver[0] == 0x04){
                if(ID3v2_4::str2FrameID.count(id_tmp) == 0){
                    ptrSongFile.seekg(-4, std::ios::cur);
                    break;
                }
            }

            // Read frame Size
            ptrSongFile.read(reinterpret_cast<char *>(&frameHeader->frSize), 4);
            frameHeader->frSize = ntohl(frameHeader->frSize);
            if(tagHeader.ver[0] == 0x04) frameHeader->frSize = to_unsync(frameHeader->frSize);

            // Check frame's validity pt.2
            if(frameHeader->frSize < 1) break;
            // Frame Flags
            ptrSongFile.read(reinterpret_cast<char *>(frameHeader->frFlags), 2);

            // Save content's location (after header)
            frameHeader->cPos = ptrSongFile.tellg();

            // Skip frame content       
            ptrSongFile.seekg(frameHeader->cPos + frameHeader->frSize, std::ios::beg);

            // Push pointer to vector
            frameHeader34Vector.push_back(std::move(frameHeader));
        }
    }

    // For v2.2
    else if(tagHeader.ver[0] == 0x02){
        for(int frameNumber = 0; frameNumber < totalFrames; frameNumber++){
            auto frameHeader = std::make_unique<ID3v2_2_FrameHeader>();

            // Read frame TagID and convert it to a string
            std::string id_tmp;
            ptrSongFile.read(reinterpret_cast<char *>(frameHeader->frID), 3);
            for(int i = 0; i < 3; i++){
                id_tmp.push_back(frameHeader->frID[i]);
            }

            // Check frame's validity pt.1
            if(ID3v2_2::str2FrameID.count(id_tmp) == 0){
                ptrSongFile.seekg(-3, std::ios::cur);
                break;
            }

            // Read frame size
            uint8_t tmpSize[3];
            ptrSongFile.read(reinterpret_cast<char *>(tmpSize), 3);
            frameHeader->frSize = 0 | (tmpSize[2] << 14) | (tmpSize[1] << 7) | tmpSize[0];

            // Check frame's validity pt.2
            if(frameHeader->frSize < 1) break;

            // Save content's location (after header)
            frameHeader->cPos = ptrSongFile.tellg();

            // Skip frame content       
            ptrSongFile.seekg(frameHeader->cPos + frameHeader->frSize, std::ios::beg);

            // Push pointer to vector
            frameHeader2Vector.push_back(std::move(frameHeader));
        }
    }
    
    padding = (tagHeader.size + 10) - ptrSongFile.tellg();
    
}

bool ID3v2::processFooter(){
    ptrSongFile.seekg(-10, std::ios::end);

    t4Footer = std::make_unique<ID3v2_4_Footer>();

    // Read footer identifier
    ptrSongFile.read(reinterpret_cast<char *>(&t4Footer->tag), 3);

    // Confirm that the footer actually exists
    if(t4Footer->tag[0] == 0x33 && t4Footer->tag[1] == 0x44 && t4Footer->tag[2] == 0x49){
        // Read tag version
        ptrSongFile.read(reinterpret_cast<char *>(&t4Footer->ver), 2);
        // Read footer / header tag flags
        ptrSongFile.read(reinterpret_cast<char *>(&t4Footer->flags), 1);
        // Read total tag size
        ptrSongFile.read(reinterpret_cast<char *>(&t4Footer->size), 4);
        // Reset position
        ptrSongFile.seekg(10, std::ios::beg);

        return true;
    }

    else return false;
}

// Check
std::string ID3v2::checkTagType(const ID3v2_2::FrameID &frameID){
    switch(frameID){  
//      Generic frames
        case ID3v2_2::COM: return "COM";
        case ID3v2_2::PIC: return "PIC";
        case ID3v2_2::BUF:
        case ID3v2_2::CNT:
        case ID3v2_2::CRA:
        case ID3v2_2::CRM:
        case ID3v2_2::ETC:
        case ID3v2_2::EQU:
        case ID3v2_2::GEO:
        case ID3v2_2::IPL:
        case ID3v2_2::LNK:
        case ID3v2_2::MCI:
        case ID3v2_2::MLL:
        case ID3v2_2::POP:
        case ID3v2_2::REV:
        case ID3v2_2::RVA:
        case ID3v2_2::SLT:
        case ID3v2_2::STC: return "DEF";

//      Text information frames
        case ID3v2_2::TAL:
        case ID3v2_2::TBP:
        case ID3v2_2::TCM:
        case ID3v2_2::TCO:
        case ID3v2_2::TCR:
        case ID3v2_2::TDA:
        case ID3v2_2::TDY:
        case ID3v2_2::TEN:
        case ID3v2_2::TFT:
        case ID3v2_2::TIM:
        case ID3v2_2::TKE:
        case ID3v2_2::TLA:
        case ID3v2_2::TLE:
        case ID3v2_2::TMT:
        case ID3v2_2::TOA:
        case ID3v2_2::TOF:
        case ID3v2_2::TOL:
        case ID3v2_2::TOR:
        case ID3v2_2::TOT:
        case ID3v2_2::TP1:
        case ID3v2_2::TP2:
        case ID3v2_2::TP3:
        case ID3v2_2::TP4:
        case ID3v2_2::TPA:
        case ID3v2_2::TPB:
        case ID3v2_2::TRC:
        case ID3v2_2::TRD:
        case ID3v2_2::TRK:
        case ID3v2_2::TSI:
        case ID3v2_2::TSS:
        case ID3v2_2::TT1:
        case ID3v2_2::TT2:
        case ID3v2_2::TT3:
        case ID3v2_2::TXT:
        case ID3v2_2::TYE: return "TIF";
        case ID3v2_2::TXX: return "UTIF";

//      Additional frames
        case ID3v2_2::UFI:
        case ID3v2_2::ULT: return "DEF";

//      URL link frames
        case ID3v2_2::WAF:
        case ID3v2_2::WAR:
        case ID3v2_2::WAS:
        case ID3v2_2::WCM:
        case ID3v2_2::WCP:
        case ID3v2_2::WPB: return "ULF";
        case ID3v2_2::WXX: return "UULF";

        default: return "UNKNOWN";
    }
}

std::string ID3v2::checkTagType(const ID3v2_3::FrameID &frameID){
    switch(frameID){
//      Generic frames
        case ID3v2_3::APIC: return "PIC";
        case ID3v2_3::COMM: return "COM";
        case ID3v2_3::AENC:
        case ID3v2_3::COMR:
        case ID3v2_3::ENCR:
        case ID3v2_3::EQUA:
        case ID3v2_3::ETCO:
        case ID3v2_3::GEOB:
        case ID3v2_3::GRID:
        case ID3v2_3::IPLS:
        case ID3v2_3::LINK:
        case ID3v2_3::MCDI:
        case ID3v2_3::MLLT:
        case ID3v2_3::OWNE:
        case ID3v2_3::PRIV:
        case ID3v2_3::PCNT:
        case ID3v2_3::POPM:
        case ID3v2_3::POSS:
        case ID3v2_3::RBUF:
        case ID3v2_3::RVAD:
        case ID3v2_3::RVRB:
        case ID3v2_3::SYLT:
        case ID3v2_3::SYTC: return "DEF";

//    Text information frames
        case ID3v2_3::TALB:
        case ID3v2_3::TBPM:
        case ID3v2_3::TCOM:
        case ID3v2_3::TCON:
        case ID3v2_3::TCOP:
        case ID3v2_3::TDAT:
        case ID3v2_3::TDLY:
        case ID3v2_3::TENC:
        case ID3v2_3::TEXT:
        case ID3v2_3::TFLT:
        case ID3v2_3::TIME:
        case ID3v2_3::TIT1:
        case ID3v2_3::TIT2:
        case ID3v2_3::TIT3:
        case ID3v2_3::TKEY:
        case ID3v2_3::TLAN:
        case ID3v2_3::TLEN:
        case ID3v2_3::TMED:
        case ID3v2_3::TOAL:
        case ID3v2_3::TOFN:
        case ID3v2_3::TOLY:
        case ID3v2_3::TOPE:
        case ID3v2_3::TORY:
        case ID3v2_3::TOWN:
        case ID3v2_3::TPE1:
        case ID3v2_3::TPE2:
        case ID3v2_3::TPE3:
        case ID3v2_3::TPE4:
        case ID3v2_3::TPOS:
        case ID3v2_3::TPUB:
        case ID3v2_3::TRCK:
        case ID3v2_3::TRDA:
        case ID3v2_3::TRSN:
        case ID3v2_3::TRSO:
        case ID3v2_3::TSIZ:
        case ID3v2_3::TSRC:
        case ID3v2_3::TSSE:
        case ID3v2_3::TYER: return "TIF";
        case ID3v2_3::TXXX: return "UTIF";

//      Additional frames
        case ID3v2_3::UFID:
        case ID3v2_3::USER:
        case ID3v2_3::USLT: return "DEF";

//      URL link frames
        case ID3v2_3::WCOM:
        case ID3v2_3::WCOP:
        case ID3v2_3::WOAF:
        case ID3v2_3::WOAR:
        case ID3v2_3::WOAS:
        case ID3v2_3::WORS:
        case ID3v2_3::WPAY:
        case ID3v2_3::WPUB: return "ULF";
        case ID3v2_3::WXXX: return "UULF";

        default: return "UNKNOWN";
    }
}

std::string ID3v2::checkTagType(const ID3v2_4::FrameID &frameID){
    switch(frameID){
//      Generic frames
        case ID3v2_4::COMM: return "COM";
        case ID3v2_4::APIC: return "PIC";
        case ID3v2_4::AENC:
        case ID3v2_4::ASPI:
        case ID3v2_4::COMR:
        case ID3v2_4::ENCR:
        case ID3v2_4::EQU2:
        case ID3v2_4::ETCO:
        case ID3v2_4::GEOB:
        case ID3v2_4::GRID:
        case ID3v2_4::LINK:
        case ID3v2_4::MCDI:
        case ID3v2_4::MLLT:
        case ID3v2_4::OWNE:
        case ID3v2_4::PRIV:
        case ID3v2_4::PCNT:
        case ID3v2_4::POPM:
        case ID3v2_4::POSS:
        case ID3v2_4::RBUF:
        case ID3v2_4::RVA2:
        case ID3v2_4::RVRB:
        case ID3v2_4::SYLT:
        case ID3v2_4::SYTC:
        case ID3v2_4::SEEK:
        case ID3v2_4::SIGN: return "DEF";
    
//    Text information frames
        case ID3v2_4::TALB:
        case ID3v2_4::TBPM:
        case ID3v2_4::TCOM:
        case ID3v2_4::TCON:
        case ID3v2_4::TCOP:
        case ID3v2_4::TDRC:
        case ID3v2_4::TDRL:
        case ID3v2_4::TDLY:
        case ID3v2_4::TDEN:
        case ID3v2_4::TDTG:
        case ID3v2_4::TENC:
        case ID3v2_4::TEXT:
        case ID3v2_4::TFLT:
        case ID3v2_4::TIME:
        case ID3v2_4::TIPL:
        case ID3v2_4::TIT1:
        case ID3v2_4::TIT2:
        case ID3v2_4::TIT3:
        case ID3v2_4::TKEY:
        case ID3v2_4::TLAN:
        case ID3v2_4::TLEN:
        case ID3v2_4::TMCL:
        case ID3v2_4::TMED:
        case ID3v2_4::TMOO:
        case ID3v2_4::TOAL:
        case ID3v2_4::TOFN:
        case ID3v2_4::TOLY:
        case ID3v2_4::TOPE:
        case ID3v2_4::TDOR:
        case ID3v2_4::TOWN:
        case ID3v2_4::TPE1:
        case ID3v2_4::TPE2:
        case ID3v2_4::TPE3:
        case ID3v2_4::TPE4:
        case ID3v2_4::TPOS:
        case ID3v2_4::TPRO:
        case ID3v2_4::TPUB:
        case ID3v2_4::TRCK:
        case ID3v2_4::TRSN:
        case ID3v2_4::TRSO:
        case ID3v2_4::TSOA:
        case ID3v2_4::TSOP:
        case ID3v2_4::TSOT:
        case ID3v2_4::TSRC:
        case ID3v2_4::TSSE:
        case ID3v2_4::TSST: return "TIF";
        case ID3v2_4::TXXX: return "UTIF";

//      Additional frames     
        case ID3v2_4::UFID:
        case ID3v2_4::USER:
        case ID3v2_4::USLT: return "DEF";

//      URL link frames
        case ID3v2_4::WCOM:
        case ID3v2_4::WCOP:
        case ID3v2_4::WOAF:
        case ID3v2_4::WOAR:
        case ID3v2_4::WOAS:
        case ID3v2_4::WORS:
        case ID3v2_4::WPAY:
        case ID3v2_4::WPUB: return "ULF";
        case ID3v2_4::WXXX: return "UULF";

        default: return "UNKNOWN";
    }
}

// Cleanup
void ID3v2::Reset(){
    frameHeader2Vector.clear();
    frameHeader34Vector.clear();

    e3TagHeader.reset();
    e4TagHeader.reset();
    t4Footer.reset();

    ptrSongFile.close();
    ptrSongFile.open(songPath, std::ios::in | std::ios::binary);
    if(ptrSongFile.is_open()) fileExists = true;
    else fileExists = false;

    tagExists = false;
    
    processTags();
}