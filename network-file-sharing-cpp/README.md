# Linux Network File Sharing (C++)

Simple TCP-based file sharing application built in C++ using BSD sockets. 
This repository contains a **server** exposing a `shared/` directory and a **client** that can LIST, GET and PUT files.

## Build & Run (Ubuntu/Debian)
```bash
sudo apt update
sudo apt install -y build-essential g++
make
# start server (in one terminal)
./bin/server
# start client (in another terminal)
./bin/client
```

## Usage (client)
- `list` : List files on the server
- `get <filename>` : Download file from server
- `put <filename>` : Upload local file to server
- `quit` : Exit client
