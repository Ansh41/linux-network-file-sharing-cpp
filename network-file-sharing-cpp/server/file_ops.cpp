#include "file_ops.h"
#include <sys/stat.h>
#include <dirent.h>
#include <vector>
#include <string>

std::vector<std::string> list_directory(const std::string &path){
    std::vector<std::string> out;
    DIR *d = opendir(path.c_str());
    if(!d) return out;
    struct dirent *entry;
    while((entry = readdir(d)) != nullptr){
        std::string name(entry->d_name);
        if(name=="." || name=="..") continue;
        out.push_back(name);
    }
    closedir(d);
    return out;
}

ssize_t get_file_size(const std::string &path){
    struct stat st;
    if(stat(path.c_str(), &st) != 0) return -1;
    return st.st_size;
}
