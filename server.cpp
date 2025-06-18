#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <vector>
#include <mutex>
#include <algorithm>
#include <string>
#include "ClientUser.cpp"
#pragma comment(lib, "ws2_32.lib")
using namespace std;

vector<SOCKET> clients;
mutex clients_mutex;
// Client storage
vector<ClientUser> AllClients;
int client_count = 0;

/**
 * this method checks if the received name is cuplicated
 * @param s received name string
 * @return if it is duplicated or not
 */
bool isDuplicated(const string &s) {
    bool duplicated = false;
    for (int i = 0; i < AllClients.size(); i++) {
        if (AllClients[i].getClientName() == s) {
            duplicated = true;
        }
    }
    return duplicated;
}

void handle_client(SOCKET client_socket) {
    char buffer[1024];
    int bytes;
    while ((bytes = recv(client_socket, buffer, sizeof(buffer), 0)) > 0) {
        buffer[bytes] = '\0';

        lock_guard<mutex> lock(clients_mutex);
        for (SOCKET s : clients) {
            if (s != client_socket) {
                send(s, buffer, bytes, 0);
            }
        }
    }

    lock_guard<mutex> lock(clients_mutex);
    clients.erase(remove(clients.begin(), clients.end(), client_socket), clients.end());
    closesocket(client_socket);
}

int main() {
    char buffer[1024];

    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr));
    listen(server_socket, SOMAXCONN);

    cout << "Server started!"<< endl;


    while (true) {

        sockaddr_in client_addr;
        int client_size = sizeof(client_addr);
        SOCKET client_socket = accept(server_socket, (sockaddr*)&client_addr, &client_size);

        lock_guard<mutex> lock(clients_mutex);


        //first it will send clients the number of clients to decide whether they can continue
        send(client_socket, to_string(AllClients.size()).c_str(), to_string(AllClients.size()).length(), 0);

        //receiving the username
        string receivedName;
        recv(client_socket, buffer, sizeof(buffer), 0);
        receivedName = buffer;

        //checking if it is duplicated
        bool duplicated = false;
        duplicated = isDuplicated(receivedName);
        string duplicatedAnswer;
        if (duplicated)
            duplicatedAnswer ="duplicated";
        else if (!duplicated)
            duplicatedAnswer = "okey";

        cout << "received name : " << receivedName<<endl;
        cout <<"duplicated:" << duplicatedAnswer <<endl;

        //if it is duplicated, the server will send client the output
        while (duplicated){
            duplicatedAnswer ="duplicated";
            //sending the duplicated answer
            send(client_socket, duplicatedAnswer.c_str(), duplicatedAnswer.length(), 0);
            //receiving a new name
            recv(client_socket, buffer, sizeof(buffer), 0);
            receivedName = buffer;
            duplicated = isDuplicated(receivedName);
        }
        duplicatedAnswer = "okey";
        send(client_socket, duplicatedAnswer.c_str(), duplicatedAnswer.length(), 0);
        ClientUser newClient;
        newClient.setClientName(receivedName);
        newClient.setIsActive(true);
        AllClients.push_back(newClient);

        clients.push_back(client_socket);
        thread(handle_client, client_socket).detach();
    }

    WSACleanup();
    return 0;
}
