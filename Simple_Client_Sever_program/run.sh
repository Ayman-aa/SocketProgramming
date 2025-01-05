#!/bin/bash

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

# Clean previous builds
echo -e "${GREEN}Cleaning previous builds...${NC}"
rm -f server client *.o

# Compile server
echo -e "${GREEN}Compiling server...${NC}"
g++ -c Server.cpp -o server.o
if [ $? -ne 0 ]; then
    echo -e "${RED}Server compilation failed${NC}"
    exit 1
fi

g++ server.o serverMain.cpp -o server
if [ $? -ne 0 ]; then
    echo -e "${RED}Server linking failed${NC}"
    exit 1
fi

# Compile client
echo -e "${GREEN}Compiling client...${NC}"
g++ -c Client.cpp -o client.o
if [ $? -ne 0 ]; then
    echo -e "${RED}Client compilation failed${NC}"
    exit 1
fi

g++ client.o clientMain.cpp -o client
if [ $? -ne 0 ]; then
    echo -e "${RED}Client linking failed${NC}"
    exit 1
fi

echo -e "${GREEN}Compilation successful!${NC}"

# Run server in background
echo -e "${GREEN}Starting server...${NC}"
./server &
SERVER_PID=$!

# Wait a moment for server to start
sleep 1

# Run client
echo -e "${GREEN}Starting client...${NC}"
./client

# Clean up server process when client exits
kill $SERVER_PID

# Clean up build files
echo -e "${GREEN}Cleaning up build files...${NC}"
rm -f server client *.o

echo -e "${GREEN}Done!${NC}"