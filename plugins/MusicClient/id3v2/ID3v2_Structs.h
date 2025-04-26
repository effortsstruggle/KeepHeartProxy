#ifndef ID3V2STR_H
#define ID3V2STR_H

// Tag header for v2
struct ID3v2_TagHeader{
    char tag[3];
    char ver[2];
    char flags;
    uint32_t size;
};

// Extended tag header for v2.3
struct ID3v2_3_ExtendedTagHeader{
    uint32_t eSize;
    char eFlags[2];
    uint32_t padSize;
    char crc[4];
};

// Extended tag header for v2.4
struct ID3v2_4_ExtendedTagHeader{
    uint32_t eSize;
    char numFlags;
    char eFlags;
    char crc[5];
    char restrictFlag;
};

// Frame header for v2.2
struct ID3v2_2_FrameHeader{
    char frID[3];
    uint32_t frSize; // It's 3 byte normally but I convert it to 4 bytes for function support

    unsigned int cPos; // Content Position
};

// Frame header for v2.3 and v2.4
struct ID3v2_3_4_FrameHeader{
    char frID[4];
    uint32_t frSize;
    char frFlags[2];

    unsigned int cPos; // Content Position
};

// Footer for v2.4 (It's basically a copy of the tag header but with a reversed identifier)
struct ID3v2_4_Footer{
    char tag[3];
    char ver[2];
    uint32_t size;
    char flags;
};

#endif