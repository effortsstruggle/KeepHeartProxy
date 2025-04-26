#ifndef ID3V2_ID_H
#define ID3V2_ID_H

// ----------------------------------2.2------------------------------------- //

namespace ID3v2_2{

  
    enum FrameID{
        BUF,        // Recommended buffer size
        CNT,        // Play counter
        COM,        // Comments
        CRA,        // Audio encryption
        CRM,        // Encrypted meta frame
        ETC,        // Event timing codes
        EQU,        // Equalization
        GEO,        // General encapsulated object
        IPL,        // Involved people list
        LNK,        // Linked information
        MCI,        // Music CD Identifier
        MLL,        // MPEG location lookup table
        PIC,        // Attached picture
        POP,        // Popularimeter
        REV,        // Reverb
        RVA,        // Relative volume adjustment
        SLT,        // Synchronized lyric/text
        STC,        // Synced tempo codes
        
        TAL,        // Album/Movie/Show title
        TBP,        // BPM (Beats Per Minute)
        TCM,        // Composer
        TCO,        // Content type
        TCR,        // Copyright message
        TDA,        // Date
        TDY,        // Playlist delay
        TEN,        // Encoded by
        TFT,        // File type
        TIM,        // Time
        TKE,        // Initial key
        TLA,        // Language(s)
        TLE,        // Length
        TMT,        // Media type
        TOA,        // Original artist(s)/performer(s)
        TOF,        // Original filename
        TOL,        // Original Lyricist(s)/text writer(s)
        TOR,        // Original release year
        TOT,        // Original album/Movie/Show title
        TP1,        // Lead artist(s)/Lead performer(s)/Soloist(s)/Performing group
        TP2,        // Band/Orchestra/Accompaniment
        TP3,        // Conductor/Performer refinement
        TP4,        // Interpreted, remixed, or otherwise modified by
        TPA,        // Part of a set
        TPB,        // Publisher
        TRC,        // ISRC (International Standard Recording Code)
        TRD,        // Recording dates
        TRK,        // Track number/Position in set
        TSI,        // Size
        TSS,        // Software/hardware and settings used for encoding
        TT1,        // Content group description
        TT2,        // Title/Songname/Content description
        TT3,        // Subtitle/Description refinement
        TXT,        // Lyricist/text writer
        TXX,        // User defined text information frame
        TYE,        // Year
        
        UFI,        // Unique file identifier
        ULT,        // Unsychronized lyric/text transcription
        
        WAF,        // Official audio file webpage
        WAR,        // Official artist/performer webpage
        WAS,        // Official audio source webpage
        WCM,        // Commercial information
        WCP,        // Copyright/Legal information
        WPB,        // Publishers official webpage
        WXX         // User defined URL link frame
    };

    // This map is used to convert string to enums for v2.2
    static std::unordered_map<std::string, ID3v2_2::FrameID> str2FrameID = {
        {"BUF", ID3v2_2::FrameID::BUF},
        {"CNT", ID3v2_2::FrameID::CNT},
        {"COM", ID3v2_2::FrameID::COM},
        {"CRA", ID3v2_2::FrameID::CRA},
        {"CRM", ID3v2_2::FrameID::CRM},
        {"ETC", ID3v2_2::FrameID::ETC},
        {"EQU", ID3v2_2::FrameID::EQU},
        {"GEO", ID3v2_2::FrameID::GEO},
        {"IPL", ID3v2_2::FrameID::IPL},
        {"LNK", ID3v2_2::FrameID::LNK},
        {"MCI", ID3v2_2::FrameID::MCI},
        {"MLL", ID3v2_2::FrameID::MLL},
        {"PIC", ID3v2_2::FrameID::PIC},
        {"POP", ID3v2_2::FrameID::POP},
        {"REV", ID3v2_2::FrameID::REV},
        {"RVA", ID3v2_2::FrameID::RVA},
        {"SLT", ID3v2_2::FrameID::SLT},
        {"STC", ID3v2_2::FrameID::STC},
        {"TAL", ID3v2_2::FrameID::TAL},
        {"TBP", ID3v2_2::FrameID::TBP},
        {"TCM", ID3v2_2::FrameID::TCM},
        {"TCO", ID3v2_2::FrameID::TCO},
        {"TCR", ID3v2_2::FrameID::TCR},
        {"TDA", ID3v2_2::FrameID::TDA},
        {"TDY", ID3v2_2::FrameID::TDY},
        {"TEN", ID3v2_2::FrameID::TEN},
        {"TFT", ID3v2_2::FrameID::TFT},
        {"TIM", ID3v2_2::FrameID::TIM},
        {"TKE", ID3v2_2::FrameID::TKE},
        {"TLA", ID3v2_2::FrameID::TLA},
        {"TLE", ID3v2_2::FrameID::TLE},
        {"TMT", ID3v2_2::FrameID::TMT},
        {"TOA", ID3v2_2::FrameID::TOA},
        {"TOF", ID3v2_2::FrameID::TOF},
        {"TOL", ID3v2_2::FrameID::TOL},
        {"TOR", ID3v2_2::FrameID::TOR},
        {"TOT", ID3v2_2::FrameID::TOT},
        {"TP1", ID3v2_2::FrameID::TP1},
        {"TP2", ID3v2_2::FrameID::TP2},
        {"TP3", ID3v2_2::FrameID::TP3},
        {"TP4", ID3v2_2::FrameID::TP4},
        {"TPA", ID3v2_2::FrameID::TPA},
        {"TPB", ID3v2_2::FrameID::TPB},
        {"TRC", ID3v2_2::FrameID::TRC},
        {"TRD", ID3v2_2::FrameID::TRD},
        {"TRK", ID3v2_2::FrameID::TRK},
        {"TSI", ID3v2_2::FrameID::TSI},
        {"TSS", ID3v2_2::FrameID::TSS},
        {"TT1", ID3v2_2::FrameID::TT1},
        {"TT2", ID3v2_2::FrameID::TT2},
        {"TT3", ID3v2_2::FrameID::TT3},
        {"TXT", ID3v2_2::FrameID::TXT},
        {"TXX", ID3v2_2::FrameID::TXX},
        {"TYE", ID3v2_2::FrameID::TYE},
        {"UFI", ID3v2_2::FrameID::UFI},
        {"ULT", ID3v2_2::FrameID::ULT},
        {"WAF", ID3v2_2::FrameID::WAF},
        {"WAR", ID3v2_2::FrameID::WAR},
        {"WAS", ID3v2_2::FrameID::WAS},
        {"WCM", ID3v2_2::FrameID::WCM},
        {"WCP", ID3v2_2::FrameID::WCP},
        {"WPB", ID3v2_2::FrameID::WPB},
        {"WXX", ID3v2_2::FrameID::WXX}
    };

}

// ----------------------------------2.3------------------------------------- //

namespace ID3v2_3{
    
    //  names for v2.3
    enum FrameID{
        //      Generic frames
        AENC,       // Audio encryption
        APIC,       // Attached picture
        COMM,       // Comments
        COMR,       // Commercial frame
        ENCR,       // Encryption method registration
        EQUA,       // Equalization
        ETCO,       // Event timing codes
        GEOB,       // General encapsulated object
        GRID,       // Group identification registration
        IPLS,       // Involved people list
        LINK,       // Linked information
        MCDI,       // Music CD identifier
        MLLT,       // MPEG location lookup table
        OWNE,       // Ownership frame
        PRIV,       // Private frame
        PCNT,       // Play counter
        POPM,       // Popularimeter
        POSS,       // Position synchronisation frame
        RBUF,       // Recommended buffer size
        RVAD,       // Relative volume adjustment
        RVRB,       // Reverb
        SYLT,       // Synchronized lyric/text
        SYTC,       // Synchronized tempo codes

        //    Text information frames
        TALB,       // Album/Movie/Show title
        TBPM,       // BPM (beats per minute)
        TCOM,       // Composer
        TCON,       // Content type
        TCOP,       // Copyright message
        TDAT,       // Date
        TDLY,       // Playlist delay
        TENC,       // Encoded by
        TEXT,       // Lyricist/Text writer
        TFLT,       // File type
        TIME,       // Time
        TIT1,       // Content group description
        TIT2,       // Title/songname/content description
        TIT3,       // Subtitle/Description refinement
        TKEY,       // Initial key
        TLAN,       // Language(s)
        TLEN,       // Length
        TMED,       // Media type
        TOAL,       // Original album/movie/show title
        TOFN,       // Original filename
        TOLY,       // Original lyricist(s)/text writer(s)
        TOPE,       // Original artist(s)/performer(s)
        TORY,       // Original release year
        TOWN,       // File owner/licensee
        TPE1,       // Lead performer(s)/Soloist(s)
        TPE2,       // Band/orchestra/accompaniment
        TPE3,       // Conductor/performer refinement
        TPE4,       // Interpreted, remixed, or otherwise modified by
        TPOS,       // Part of a set
        TPUB,       // Publisher
        TRCK,       // Track number/Position in set
        TRDA,       // Recording dates
        TRSN,       // Internet radio station name
        TRSO,       // Internet radio station owner
        TSIZ,       // Size
        TSRC,       // ISRC (international standard recording code)
        TSSE,       // Software/Hardware and settings used for encoding
        TYER,       // Year
        TXXX,       // -User defined text information frame-
        
        //      Additional frames
        UFID,       // Unique file identifier
        USER,       // Terms of use
        USLT,       // Unsychronized lyric/text transcription
        
        //      URL link frames
        WCOM,       // Commercial information
        WCOP,       // Copyright/Legal information
        WOAF,       // Official audio file webpage
        WOAR,       // Official artist/performer webpage
        WOAS,       // Official audio source webpage
        WORS,       // Official internet radio station homepage
        WPAY,       // Payment
        WPUB,       // Publishers official webpage
        WXXX        // -User defined URL link frame-
    };

    // This map is used to convert string to enums for v2.3
    static std::unordered_map<std::string, ID3v2_3::FrameID> str2FrameID = {
        {"AENC", ID3v2_3::FrameID::AENC},
        {"APIC", ID3v2_3::FrameID::APIC},
        {"COMM", ID3v2_3::FrameID::COMM},
        {"COMR", ID3v2_3::FrameID::COMR},
        {"ENCR", ID3v2_3::FrameID::ENCR},
        {"EQUA", ID3v2_3::FrameID::EQUA},
        {"ETCO", ID3v2_3::FrameID::ETCO},
        {"GEOB", ID3v2_3::FrameID::GEOB},
        {"GRID", ID3v2_3::FrameID::GRID},
        {"IPLS", ID3v2_3::FrameID::IPLS},
        {"LINK", ID3v2_3::FrameID::LINK},
        {"MCDI", ID3v2_3::FrameID::MCDI},
        {"MLLT", ID3v2_3::FrameID::MLLT},
        {"OWNE", ID3v2_3::FrameID::OWNE},
        {"PRIV", ID3v2_3::FrameID::PRIV},
        {"PCNT", ID3v2_3::FrameID::PCNT},
        {"POPM", ID3v2_3::FrameID::POPM},
        {"POSS", ID3v2_3::FrameID::POSS},
        {"RBUF", ID3v2_3::FrameID::RBUF},
        {"RVAD", ID3v2_3::FrameID::RVAD},
        {"RVRB", ID3v2_3::FrameID::RVRB},
        {"SYLT", ID3v2_3::FrameID::SYLT},
        {"SYTC", ID3v2_3::FrameID::SYTC},
        {"TALB", ID3v2_3::FrameID::TALB},
        {"TBPM", ID3v2_3::FrameID::TBPM},
        {"TCOM", ID3v2_3::FrameID::TCOM},
        {"TCON", ID3v2_3::FrameID::TCON},
        {"TCOP", ID3v2_3::FrameID::TCOP},
        {"TDAT", ID3v2_3::FrameID::TDAT},
        {"TDLY", ID3v2_3::FrameID::TDLY},
        {"TENC", ID3v2_3::FrameID::TENC},
        {"TEXT", ID3v2_3::FrameID::TEXT},
        {"TFLT", ID3v2_3::FrameID::TFLT},
        {"TIME", ID3v2_3::FrameID::TIME},
        {"TIT1", ID3v2_3::FrameID::TIT1},
        {"TIT2", ID3v2_3::FrameID::TIT2},
        {"TIT3", ID3v2_3::FrameID::TIT3},
        {"TKEY", ID3v2_3::FrameID::TKEY},
        {"TLAN", ID3v2_3::FrameID::TLAN},
        {"TLEN", ID3v2_3::FrameID::TLEN},
        {"TMED", ID3v2_3::FrameID::TMED},
        {"TOAL", ID3v2_3::FrameID::TOAL},
        {"TOFN", ID3v2_3::FrameID::TOFN},
        {"TOLY", ID3v2_3::FrameID::TOLY},
        {"TOPE", ID3v2_3::FrameID::TOPE},
        {"TORY", ID3v2_3::FrameID::TORY},
        {"TOWN", ID3v2_3::FrameID::TOWN},
        {"TPE1", ID3v2_3::FrameID::TPE1},
        {"TPE2", ID3v2_3::FrameID::TPE2},
        {"TPE3", ID3v2_3::FrameID::TPE3},
        {"TPE4", ID3v2_3::FrameID::TPE4},
        {"TPOS", ID3v2_3::FrameID::TPOS},
        {"TPUB", ID3v2_3::FrameID::TPUB},
        {"TRCK", ID3v2_3::FrameID::TRCK},
        {"TRDA", ID3v2_3::FrameID::TRDA},
        {"TRSN", ID3v2_3::FrameID::TRSN},
        {"TRSO", ID3v2_3::FrameID::TRSO},
        {"TSIZ", ID3v2_3::FrameID::TSIZ},
        {"TSRC", ID3v2_3::FrameID::TSRC},
        {"TSSE", ID3v2_3::FrameID::TSSE},
        {"TYER", ID3v2_3::FrameID::TYER},
        {"TXXX", ID3v2_3::FrameID::TXXX},
        {"UFID", ID3v2_3::FrameID::UFID},
        {"USER", ID3v2_3::FrameID::USER},
        {"USLT", ID3v2_3::FrameID::USLT},
        {"WCOM", ID3v2_3::FrameID::WCOM},
        {"WCOP", ID3v2_3::FrameID::WCOP},
        {"WOAF", ID3v2_3::FrameID::WOAF},
        {"WOAR", ID3v2_3::FrameID::WOAR},
        {"WOAS", ID3v2_3::FrameID::WOAS},
        {"WORS", ID3v2_3::FrameID::WORS},
        {"WPAY", ID3v2_3::FrameID::WPAY},
        {"WPUB", ID3v2_3::FrameID::WPUB},
        {"WXXX", ID3v2_3::FrameID::WXXX}
    };

}

// ----------------------------------2.4------------------------------------- //

namespace ID3v2_4{
    
    //  names for v2.4
    enum FrameID{
        //      Generic frames
        AENC,       // Audio encryption
        APIC,       // Attached picture
        ASPI,       // Audio seek point index
        COMM,       // Comments
        COMR,       // Commercial frame
        ENCR,       // Encryption method registration
        EQU2,       // Equalization
        ETCO,       // Event timing codes
        GEOB,       // General encapsulated object
        GRID,       // Group identification registration
        LINK,       // Linked information
        MCDI,       // Music CD identifier
        MLLT,       // MPEG location lookup table
        OWNE,       // Ownership frame
        PRIV,       // Private frame
        PCNT,       // Play counter
        POPM,       // Popularimeter
        POSS,       // Position synchronisation frame
        RBUF,       // Recommended buffer size
        RVA2,       // Relative volume adjustment
        RVRB,       // Reverb
        SYLT,       // Synchronized lyric/text
        SYTC,       // Synchronized tempo codes
        SEEK,       // Seek frame
        SIGN,       // Signature time

        //    Text information frames
        TALB,       // Album/Movie/Show title
        TBPM,       // BPM (beats per minute)
        TCOM,       // Composer
        TCON,       // Content type
        TCOP,       // Copyright message
        TDRC,       // Recording time
        TDRL,       // Release time
        TDLY,       // Playlist delay
        TDEN,       // Encoding time
        TDTG,       // Tagging time
        TENC,       // Encoded by
        TEXT,       // Lyricist/Text writer
        TFLT,       // File type
        TIME,       // Time
        TIPL,       // Involved people list
        TIT1,       // Content group description
        TIT2,       // Title/songname/content description
        TIT3,       // Subtitle/Description refinement
        TKEY,       // Initial key
        TLAN,       // Language(s)
        TLEN,       // Length
        TMCL,       // Musician credits list
        TMED,       // Media type
        TMOO,       // Mood
        TOAL,       // Original album/movie/show title
        TOFN,       // Original filename
        TOLY,       // Original lyricist(s)/text writer(s)
        TOPE,       // Original artist(s)/performer(s)
        TDOR,       // Original release time
        TOWN,       // File owner/licensee
        TPE1,       // Lead performer(s)/Soloist(s)
        TPE2,       // Band/orchestra/accompaniment
        TPE3,       // Conductor/performer refinement
        TPE4,       // Interpreted, remixed, or otherwise modified by
        TPOS,       // Part of a set
        TPRO,       // Produced notice
        TPUB,       // Publisher
        TRCK,       // Track number/Position in set
        TRSN,       // Internet radio station name
        TRSO,       // Internet radio station owner
        TSOA,       // Album sort order
        TSOP,       // Performer sort order
        TSOT,       // Title sort order
        TSRC,       // ISRC (international standard recording code)
        TSSE,       // Software/Hardware and settings used for encoding
        TSST,       // Set subtitle
        TXXX,       // -User defined text information frame-
        
        //      Additional frames
        UFID,       // Unique file identifier
        USER,       // Terms of use
        USLT,       // Unsychronized lyric/text transcription
        
        //      URL link frames
        WCOM,       // Commercial information
        WCOP,       // Copyright/Legal information
        WOAF,       // Official audio file webpage
        WOAR,       // Official artist/performer webpage
        WOAS,       // Official audio source webpage
        WORS,       // Official internet radio station homepage
        WPAY,       // Payment
        WPUB,       // Publishers official webpage
        WXXX        // -User defined URL link frame-
    };

    // This map is used to convert string to enums for v2.4
    static std::unordered_map<std::string, ID3v2_4::FrameID> str2FrameID = {
        {"AENC",    ID3v2_4::FrameID::AENC},
        {"APIC",    ID3v2_4::FrameID::APIC},
        {"ASPI",    ID3v2_4::FrameID::ASPI},
        {"COMM",    ID3v2_4::FrameID::COMM},
        {"COMR",    ID3v2_4::FrameID::COMR},
        {"ENCR",    ID3v2_4::FrameID::ENCR},
        {"EQU2",    ID3v2_4::FrameID::EQU2},
        {"ETCO",    ID3v2_4::FrameID::ETCO},
        {"GEOB",    ID3v2_4::FrameID::GEOB},
        {"GRID",    ID3v2_4::FrameID::GRID},
        {"LINK",    ID3v2_4::FrameID::LINK},
        {"MCDI",    ID3v2_4::FrameID::MCDI},
        {"MLLT",    ID3v2_4::FrameID::MLLT},
        {"OWNE",    ID3v2_4::FrameID::OWNE},
        {"PRIV",    ID3v2_4::FrameID::PRIV},
        {"PCNT",    ID3v2_4::FrameID::PCNT},
        {"POPM",    ID3v2_4::FrameID::POPM},
        {"POSS",    ID3v2_4::FrameID::POSS},
        {"RBUF",    ID3v2_4::FrameID::RBUF},
        {"RVA2",    ID3v2_4::FrameID::RVA2},
        {"RVRB",    ID3v2_4::FrameID::RVRB},
        {"SYLT",    ID3v2_4::FrameID::SYLT},
        {"SYTC",    ID3v2_4::FrameID::SYTC},
        {"SEEK",    ID3v2_4::FrameID::SEEK},
        {"SIGN",    ID3v2_4::FrameID::SIGN},
        {"TALB",    ID3v2_4::FrameID::TALB},
        {"TBPM",    ID3v2_4::FrameID::TBPM},
        {"TCOM",    ID3v2_4::FrameID::TCOM},
        {"TCON",    ID3v2_4::FrameID::TCON},
        {"TCOP",    ID3v2_4::FrameID::TCOP},
        {"TDRC",    ID3v2_4::FrameID::TDRC},
        {"TDRL",    ID3v2_4::FrameID::TDRL},
        {"TDLY",    ID3v2_4::FrameID::TDLY},
        {"TDEN",    ID3v2_4::FrameID::TDEN},
        {"TDTG",    ID3v2_4::FrameID::TDTG},
        {"TENC",    ID3v2_4::FrameID::TENC},
        {"TEXT",    ID3v2_4::FrameID::TEXT},
        {"TFLT",    ID3v2_4::FrameID::TFLT},
        {"TIME",    ID3v2_4::FrameID::TIME},
        {"TIPL",    ID3v2_4::FrameID::TIPL},
        {"TIT1",    ID3v2_4::FrameID::TIT1},
        {"TIT2",    ID3v2_4::FrameID::TIT2},
        {"TIT3",    ID3v2_4::FrameID::TIT3},
        {"TKEY",    ID3v2_4::FrameID::TKEY},
        {"TLAN",    ID3v2_4::FrameID::TLAN},
        {"TLEN",    ID3v2_4::FrameID::TLEN},
        {"TMCL",    ID3v2_4::FrameID::TMCL},
        {"TMED",    ID3v2_4::FrameID::TMED},
        {"TMOO",    ID3v2_4::FrameID::TMOO},
        {"TOAL",    ID3v2_4::FrameID::TOAL},
        {"TOFN",    ID3v2_4::FrameID::TOFN},
        {"TOLY",    ID3v2_4::FrameID::TOLY},
        {"TOPE",    ID3v2_4::FrameID::TOPE},
        {"TDOR",    ID3v2_4::FrameID::TDOR},
        {"TOWN",    ID3v2_4::FrameID::TOWN},
        {"TPE1",    ID3v2_4::FrameID::TPE1},
        {"TPE2",    ID3v2_4::FrameID::TPE2},
        {"TPE3",    ID3v2_4::FrameID::TPE3},
        {"TPE4",    ID3v2_4::FrameID::TPE4},
        {"TPOS",    ID3v2_4::FrameID::TPOS},
        {"TPRO",    ID3v2_4::FrameID::TPRO},
        {"TPUB",    ID3v2_4::FrameID::TPUB},
        {"TRCK",    ID3v2_4::FrameID::TRCK},
        {"TRSN",    ID3v2_4::FrameID::TRSN},
        {"TRSO",    ID3v2_4::FrameID::TRSO},
        {"TSOA",    ID3v2_4::FrameID::TSOA},
        {"TSOP",    ID3v2_4::FrameID::TSOP},
        {"TSOT",    ID3v2_4::FrameID::TSOT},
        {"TSRC",    ID3v2_4::FrameID::TSRC},
        {"TSSE",    ID3v2_4::FrameID::TSSE},
        {"TSST",    ID3v2_4::FrameID::TSST},
        {"TXXX",    ID3v2_4::FrameID::TXXX},
        {"UFID",    ID3v2_4::FrameID::UFID},
        {"USER",    ID3v2_4::FrameID::USER},
        {"USLT",    ID3v2_4::FrameID::USLT},
        {"WCOM",    ID3v2_4::FrameID::WCOM},
        {"WCOP",    ID3v2_4::FrameID::WCOP},
        {"WOAF",    ID3v2_4::FrameID::WOAF},
        {"WOAR",    ID3v2_4::FrameID::WOAR},
        {"WOAS",    ID3v2_4::FrameID::WOAS},
        {"WORS",    ID3v2_4::FrameID::WORS},
        {"WPAY",    ID3v2_4::FrameID::WPAY},
        {"WPUB",    ID3v2_4::FrameID::WPUB},
        {"WXXX",    ID3v2_4::FrameID::WXXX}
    };

}


#endif