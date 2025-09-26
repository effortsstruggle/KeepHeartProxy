#pragma once


#define VER_MAJOR           0
#define VER_MINOR           0
#define VER_BUILD           0
#define VER_REVISION        

#define Stringizing(v)       #v

// generate version string
#define VER_STR(major,minor,build,revision)  \
    "V" Stringizing(major) "." Stringizing(minor) "." Stringizing(build) "." Stringizing(revision) " " __DATE__ " " __TIME__

// generate file version 
#define FILE_VERSION_STR(major,minor,build,revision) \
    "V" Stringizing(major) "." Stringizing(minor) "." Stringizing(build) "." Stringizing(revision) "-" "20250411" "_" "025343"

#define GitVersion FILE_VERSION_STR(VER_MAJOR,VER_MINOR,VER_BUILD,VER_REVISION)
#define BuildVersion VER_STR(VER_MAJOR,VER_MINOR,VER_BUILD,VER_REVISION)
#define CommitInfo "[bugfix]修复语音芯片无法静音问题"
