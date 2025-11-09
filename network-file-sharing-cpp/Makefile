CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall

all: server client

server: server/server.cpp server/file_ops.cpp
	mkdir -p bin
	$(CXX) $(CXXFLAGS) server/server.cpp server/file_ops.cpp -o bin/server

client: client/client.cpp
	mkdir -p bin
	$(CXX) $(CXXFLAGS) client/client.cpp -o bin/client

clean:
	rm -rf bin
