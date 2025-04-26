#ifndef TOOLS_H
#define TOOLS_H
#include <iostream>


// Convert a sync-safe int to a normal int
inline uint32_t to_unsync(const uint32_t &sync_safe){
    unsigned int a, b, c, d, result = 0x0;
    a = sync_safe & 0xFF;
    b = (sync_safe >> 8) & 0xFF;
    c = (sync_safe >> 16) & 0xFF;
    d = (sync_safe >> 24) & 0xFF;

    result = result | a;
    result = result | (b << 7);
    result = result | (c << 14);
    result = result | (d << 21);

    return result;
}

// Convert a normal int to a sync-safe int
inline uint32_t to_sync(const uint32_t &not_sync_safe)
{
    uint32_t a, b, c, d, result = 0x0;

    a = not_sync_safe & 0x7f;
    b = (not_sync_safe >> 7) & 0x7f;
    c = (not_sync_safe >> 14) & 0x7f;
    d = (not_sync_safe >> 21) & 0x7f;

    result = result | a;
    result = result | (b << 8);
    result = result | (c << 16);
    result = result | (d << 24);

    return result;
}

// Convert a string to an unsigned char (8 bit)
inline char *str2ch(const std::string str, const unsigned int max, const int offset){
    char *ch = new char[max];

    unsigned int count = 0;
    for(; count + offset < str.length(); count++){
        ch[count + offset] = str[count];
    }
    for(; count + offset < max; count++){
        ch[count + offset] = '\0';
    }

    return ch;
}

inline std::string ch2str(const char ch[], const unsigned int max){
    std::string str;

    for(unsigned int i = 0; i < max; i++){
        str.push_back(ch[i]);
    }

    return str;
}

inline std::wstring UTF16_Decoder(const wchar_t wch[], const unsigned int &max){    
    if(max < 2) return L"Invalid encoding";
    
    wchar_t BOM = wch[0];
    if(BOM != 0xFFFE && BOM != 0xFEFF) return L"Invalid encoding";
    
    std::wstring wstr;
    for(unsigned int i = 1; i < max; i++){
        if(wch[i] == L'\0') break;

        if(BOM == 0xFFFE){
            wchar_t LE_wch = L'\0';
            LE_wch |= wch[i] >> 8;
            LE_wch |= (wch[i] & 0x00FF) << 8;
            wstr.push_back(LE_wch);
        }
        else{
            wstr.push_back(wch[i]);
        }
    }
    
    return wstr;
}

#endif