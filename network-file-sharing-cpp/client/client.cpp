#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include "../common/protocol.h"

#define PORT 54000

bool send_all(int fd, const void *buf, size_t len){
    const char *p=(const char*)buf; size_t sent=0;
    while(sent<len){ ssize_t r = send(fd, p+sent, len-sent, 0); if(r<=0) return false; sent+=r; }
    return true;
}

std::string recv_line(int fd){
    std::string out; char c;
    while(true){ ssize_t r = recv(fd, &c, 1, 0); if(r<=0) return std::string(); if(c=='\n') break; out.push_back(c); }
    return out;
}

int main(){
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock<0){ perror("socket"); return 1; }
    struct sockaddr_in serv{};
    serv.sin_family = AF_INET; serv.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &serv.sin_addr);
    if(connect(sock, (struct sockaddr*)&serv, sizeof(serv))<0){ perror("connect"); return 1; }
    std::cout<<"Connected to server\n";
    while(true){
        std::cout<<"Enter command (list/get/put/quit): ";
        std::string cmd; std::cin>>cmd;
        if(cmd=="list"){
            std::string msg = make_cmd(CMD_LIST);
            send_all(sock, msg.c_str(), msg.size());
            std::string resp = recv_line(sock);
            std::cout<<"Server: "<<resp<<"\n";
        } else if(cmd=="get"){
            std::string fname; std::cin>>fname;
            std::string msg = make_cmd(CMD_GET, fname);
            send_all(sock, msg.c_str(), msg.size());
            std::string resp = recv_line(sock);
            if(resp.rfind(CMD_OK,0)==0){
                size_t sp = resp.find(' ');
                long long sz = atoll(resp.substr(sp+1).c_str());
                std::ofstream ofs(fname, std::ios::binary);
                char buf[4096]; long long received = 0;
                while(received < sz){
                    ssize_t rr = recv(sock, buf, sizeof(buf), 0);
                    if(rr<=0) break;
                    ofs.write(buf, rr);
                    received += rr;
                }
                if(received==sz) std::cout<<"Download complete\n"; else std::cout<<"Download failed\n";
            } else std::cout<<"Error: "<<resp<<"\n";
        } else if(cmd=="put"){
            std::string fname; std::cin>>fname;
            std::ifstream ifs(fname, std::ios::binary);
            if(!ifs){ std::cout<<"File not found locally\n"; continue; }
            ifs.seekg(0, std::ios::end); long long sz = ifs.tellg(); ifs.seekg(0);
            std::string header = CMD_PUT + " " + fname + " " + std::to_string(sz) + "\n";
            send_all(sock, header.c_str(), header.size());
            char buf[4096];
            while(!ifs.eof()){ ifs.read(buf, sizeof(buf)); std::streamsize n = ifs.gcount(); if(n>0) send_all(sock, buf, n); }
            std::string resp = recv_line(sock);
            std::cout<<"Server: "<<resp<<"\n";
        } else if(cmd=="quit"){ close(sock); break; } else { std::cout<<"Unknown command\n"; }
    }
    return 0;
}
