#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "../common/protocol.h"
#include "file_ops.h"

#define PORT 54000
#define BACKLOG 5
static const std::string SHARED_DIR = "./shared";

ssize_t recv_line(int fd, std::string &out){
    out.clear();
    char c;
    while(true){
        ssize_t r = recv(fd, &c, 1, 0);
        if(r<=0) return r;
        if(c=='\n') break;
        out.push_back(c);
    }
    return out.size();
}

bool send_all(int fd, const void *buf, size_t len){
    const char *p=(const char*)buf;
    size_t sent=0;
    while(sent < len){
        ssize_t r = send(fd, p+sent, len-sent, 0);
        if(r<=0) return false;
        sent += r;
    }
    return true;
}

int handle_client(int client_fd){
    std::cout<<"Client connected\n";
    std::string line;
    while(true){
        ssize_t r = recv_line(client_fd, line);
        if(r<=0) break;
        if(line.rfind(CMD_LIST,0)==0){
            auto files = list_directory(SHARED_DIR);
            std::string resp = CMD_OK + " ";
            for(size_t i=0;i<files.size();++i){
                resp += files[i];
                if(i+1<files.size()) resp += ";";
            }
            resp += "\n";
            send_all(client_fd, resp.c_str(), resp.size());
        } else if(line.rfind(CMD_GET,0)==0){
            std::string name = line.substr(4);
            if(!name.empty() && name.back()=='\r') name.pop_back();
            std::string full = SHARED_DIR + "/" + name;
            ssize_t sz = get_file_size(full);
            if(sz<0){
                std::string e = CMD_ERR + " File not found\n";
                send_all(client_fd, e.c_str(), e.size());
            } else {
                std::string ok = CMD_OK + " " + std::to_string(sz) + "\n";
                send_all(client_fd, ok.c_str(), ok.size());
                std::ifstream ifs(full, std::ios::binary);
                char buf[4096];
                while(!ifs.eof()){
                    ifs.read(buf, sizeof(buf));
                    std::streamsize n = ifs.gcount();
                    if(n>0) send_all(client_fd, buf, n);
                }
            }
        } else if(line.rfind(CMD_PUT,0)==0){
            std::istringstream iss(line);
            std::string cmd, fname;
            long long sz;
            iss >> cmd >> fname >> sz;
            std::string full = SHARED_DIR + "/" + fname;
            std::ofstream ofs(full, std::ios::binary);
            long long received = 0;
            char buf[4096];
            while(received < sz){
                ssize_t toread = (sz - received) > (long long)sizeof(buf) ? sizeof(buf) : (sz - received);
                ssize_t rr = recv(client_fd, buf, toread, 0);
                if(rr<=0) break;
                ofs.write(buf, rr);
                received += rr;
            }
            if(received==sz) send_all(client_fd, (CMD_OK+"\n").c_str(), 4);
            else send_all(client_fd, (CMD_ERR+" Incomplete upload\n").c_str(), 20);
        } else {
            std::string e = CMD_ERR + " Unknown command\n";
            send_all(client_fd, e.c_str(), e.size());
        }
    }
    close(client_fd);
    std::cout<<"Client disconnected\n";
    return 0;
}

int main(){
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0){ perror("socket"); return 1; }
    int opt=1; setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;
    if(bind(sock, (struct sockaddr*)&addr, sizeof(addr))<0){ perror("bind"); return 1; }
    if(listen(sock, BACKLOG)<0){ perror("listen"); return 1; }
    std::cout<<"Server listening on port "<<PORT<<"\n";
    system((std::string("mkdir -p ")+SHARED_DIR).c_str());
    while(true){
        struct sockaddr_in cli; socklen_t len = sizeof(cli);
        int client_fd = accept(sock, (struct sockaddr*)&cli, &len);
        if(client_fd < 0) { perror("accept"); continue; }
        handle_client(client_fd);
    }
    close(sock);
    return 0;
}
