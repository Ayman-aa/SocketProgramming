#include "Client.hpp"

int main() {
    Client client("127.0.0.1", "8080");
    
    if (!client.connectToServer()) {
        return 1;
    }
    
    client.communicate();
    return 0;
}
