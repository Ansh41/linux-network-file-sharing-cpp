#pragma once
#include <string>
#include <vector>
#include <sys/types.h>

std::vector<std::string> list_directory(const std::string &path);
ssize_t get_file_size(const std::string &path);
