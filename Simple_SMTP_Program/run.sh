#!/bin/bash

RESET="\033[0m"
RED="\033[31m"
GREEN="\033[32m"
YELLOW="\033[33m"
BLUE="\033[34m"

# Check if telnet is installed
if ! command -v telnet &> /dev/null
then
    echo -e "${RED}telnet could not be found. Please install telnet to proceed.${RESET}"
    exit 1
fi

# Compile the program
echo -e "${YELLOW}Compiling the program...${RESET}"
g++ *.cpp -o SMTPServer -pthread
if [ $? -ne 0 ]; then
    echo -e "${RED}Compilation failed.${RESET}"
    exit 1
fi
echo -e "${GREEN}Compilation successful.${RESET}"

# Run the executable
echo -e "${YELLOW}Starting the SMTP server...${RESET}"
./SMTPServer &
SERVER_PID=$!

# Wait for the server to start
sleep 2

# Connect to the server using telnet
echo -e "${BLUE}Connecting to the SMTP server using telnet...${RESET}"
telnet localhost 2525

# Kill the server process when done
echo -e "${YELLOW}Shutting down the SMTP server...${RESET}"
kill $SERVER_PID
sleep 1
rm -rf SMTPServer
echo -e "${GREEN}SMTP server shut down.${RESET}"