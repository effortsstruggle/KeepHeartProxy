#include "FHelper.h"


#include <dirent.h>
#include <iostream>
#include <vector>
#include <stdlib.h>
#include <string.h>

using namespace std;
// 读取目录下指定后缀的文件
int FHelper::getFiles(const std::string &path, vector<string> &files,const string  & filter) {
  
   
    DIR *dir = opendir(path.c_str());
    if (dir == nullptr) {
        std::cerr << "Failed to open directory " << path << std::endl;
        return -1;
    }

    struct dirent *entry;
   
    while ((entry = readdir(dir)) != nullptr) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
             char tmp[3] = {0};
             std::string fname = entry->d_name;
          
             if(fname.substr(fname.find_last_of('.')+1) == "mp3")
                files.push_back(fname.c_str());

            std::cout << "Founded : " << entry->d_name << std::endl;
            //LOGD("Founded plugins: %s", entry->d_name);
        }
    }
    closedir(dir);
    return 0;
}