#pragma once
#include <string>

static const std::string CMD_LIST = "LIST";
static const std::string CMD_GET  = "GET";
static const std::string CMD_PUT  = "PUT";
static const std::string CMD_OK   = "OK";
static const std::string CMD_ERR  = "ERR";

inline std::string make_cmd(const std::string &cmd, const std::string &arg=""){
    if(arg.empty()) return cmd + "\n";
    return cmd + " " + arg + "\n";
}
