echo "=== Compiling HTTP Server ==="

# Compile with all warnings and C++17
g++ -Wall -Wextra -Werror *.cpp -o http_server

if [ $? -eq 0 ]; then
    echo "=== Compilation successful ==="
    echo "Running: ./http_server 8080 ./www 127.0.0.1"
    
    # Run the server in the foreground with specified parameters
    ./http_server 8080 ./www 127.0.0.1
else
    echo "=== Compilation failed ==="
    exit 1
fi