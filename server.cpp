#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <vector>
#include <mutex>
#include <algorithm>
#include <string>
#include <sstream>
#include "ClientUser.cpp"
#pragma comment(lib, "ws2_32.lib")
using namespace std;


// Client storage
vector<SOCKET> clients;
mutex clients_mutex;
vector<ClientUser> AllClients;
int client_count = 0;
const int portNumbers[3] = {8081, 8082, 8083};

/**
 * method to check whether a name exists in the clients storage
 * @param name given name
 * @return if exists or not
 */
bool checkName(const string &name) {
    for (int i = 0; i < AllClients.size(); i++) {
        if (AllClients[i].getClientName() == name) {
            return true;
        }
    }
    return false;

}

/**
 * this method checks if the received name is duplicated
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

/**
 *this method sends all available users to given client
 * @param client which client to send
 */
void sendClientAllUserNames(SOCKET client) {
    string usernames;
    for (int i = 0; i < AllClients.size(); i++) {
        usernames += AllClients[i].getClientName() + " ";
    }
    send(client,usernames.c_str(), usernames.length(), 0);
}

/**
 * method for all the handling process of a client
 * @param server_socket
 */
void handle_client_all(SOCKET server_socket,SOCKET client_socket,sockaddr_in client_addr,int client_size) {

        //a char array for receiving messages
        char buffer [1024];

        //first it will send feedback to clients to decide whether they can continue
        string readyToStart;
        if (client_count >= 3) {
            readyToStart = "no";
        }
        else {
            readyToStart = "yes";
        }
        send(client_socket, readyToStart.c_str(), readyToStart.length(), 0);

        //receiving the username
        string receivedName;
        memset(buffer, 0, sizeof(buffer));
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

        //if it is duplicated, the server will send client the output
        while (duplicated){

            duplicatedAnswer ="duplicated";
            //sending the duplicated answer
            send(client_socket, duplicatedAnswer.c_str(), duplicatedAnswer.length(), 0);
            //receiving a new name
            memset(buffer, 0, sizeof(buffer));
            recv(client_socket, buffer, sizeof(buffer), 0);
            receivedName = buffer;
            duplicated = isDuplicated(receivedName);

        }
        duplicatedAnswer = "okey";
        send(client_socket, duplicatedAnswer.c_str(), duplicatedAnswer.length(), 0);

        //registering the new client
        ClientUser newClient;
        newClient.setClientName(receivedName);
        newClient.setIsActive(true);
        newClient.setPortNumber(portNumbers[client_count]);
        client_count++;
        AllClients.push_back(newClient);

        //register part ended, now the application starts
        //first the user should receive the special port number
        //changing the communication according to new port number
        int newPortNumber = portNumbers[client_count];
        send(client_socket,(char*)&newPortNumber,sizeof(newPortNumber),0);



    //determining the index of the user for later use
    int indexOfUser = -1;
    for (int i = 0; i < AllClients.size(); i++) {
        if (AllClients[i].getPortNumber() == newPortNumber) {
            indexOfUser = i;
        }
    }

    //creating new sockets according to new port number
    SOCKET server_socket_for_client = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in newServer_addr{};
    newServer_addr.sin_family = AF_INET;
    newServer_addr.sin_port = htons(newPortNumber);
    newServer_addr.sin_addr.s_addr = INADDR_ANY;

    //binding and listening
    bind(server_socket_for_client, (sockaddr*)&newServer_addr, sizeof(newServer_addr));
    listen(server_socket_for_client, SOMAXCONN);

    //new client socket to communicate on the new port
    SOCKET client_socket_new = accept(server_socket_for_client, (sockaddr*)&client_addr, &client_size);

    //now the user is in the menu
    memset(buffer, 0, sizeof(buffer));
    recv(client_socket_new, buffer, sizeof(buffer), 0);
    string action = buffer;

    //messaging mode
    if (action == "1") {

    }
    //choosing destinations
    else if (action == "2") {
        string destinations;
        string feedback;
        memset(buffer, 0, sizeof(buffer));
        recv(client_socket_new, buffer, sizeof(buffer), 0);
        destinations = buffer;

         stringstream ss(destinations);
         string singleDest;

         while (getline(ss, singleDest, ' ')) {

             if (!checkName(singleDest)) {
                feedback = "notOkey";
                 send(client_socket_new, feedback.c_str(), feedback.length(), 0);



             }
             (AllClients[indexOfUser].getDestinations()).push_back(singleDest);
        }

    }
    //checking unseen messages
    else if (action == "3") {
    }
    //see all available users
    else if (action == "4") {
        sendClientAllUserNames(client_socket_new);

    }
    //looking messaging history
    else if (action == "5") {

    }
    //disconnect
    else if (action == "6") {

    }


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

            //creating client socket
            sockaddr_in client_addr{};
            int client_size = sizeof(client_addr);
            SOCKET client_socket = accept(server_socket, (sockaddr*)&client_addr, &client_size);
            thread(handle_client_all, server_socket, client_socket,client_addr,client_size).detach();
        // int client_size = sizeof(client_addr);
        // SOCKET client_socket = accept(server_socket, (sockaddr*)&client_addr, &client_size);
        //
        // //first it will send clients the number of clients to decide whether they can continue
        // send(client_socket, to_string(AllClients.size()).c_str(), to_string(AllClients.size()).length(), 0);
        //
        // //receiving the username
        // string receivedName;
        // memset(buffer, 0, sizeof(buffer));
        // recv(client_socket, buffer, sizeof(buffer), 0);
        // receivedName = buffer;
        //
        // //checking if it is duplicated
        // bool duplicated = false;
        // duplicated = isDuplicated(receivedName);
        // string duplicatedAnswer;
        // if (duplicated)
        //     duplicatedAnswer ="duplicated";
        // else if (!duplicated)
        //     duplicatedAnswer = "okey";
        //
        // //if it is duplicated, the server will send client the output
        // while (duplicated){
        //
        //     duplicatedAnswer ="duplicated";
        //     //sending the duplicated answer
        //     send(client_socket, duplicatedAnswer.c_str(), duplicatedAnswer.length(), 0);
        //     //receiving a new name
        //     memset(buffer, 0, sizeof(buffer));
        //     recv(client_socket, buffer, sizeof(buffer), 0);
        //     receivedName = buffer;
        //     duplicated = isDuplicated(receivedName);
        //
        // }
        // duplicatedAnswer = "okey";
        // send(client_socket, duplicatedAnswer.c_str(), duplicatedAnswer.length(), 0);
        //
        // //registering the new client
        // ClientUser newClient;
        // newClient.setClientName(receivedName);
        // newClient.setIsActive(true);
        // client_count++;
        // newClient.setPortNumber(portNumbers[client_count]);
        // AllClients.push_back(newClient);
        //
        // //register part ended, now the application starts
        // //first the user should receive the special port number
        // //changing the communication according to new port number
        // int portNumberToSend = portNumbers[client_count];
        // send(client_socket,(char*)&portNumberToSend,sizeof(portNumberToSend),0);
        // closesocket(server_socket);
        //
        // server_socket = socket(AF_INET, SOCK_STREAM, 0);
        // sockaddr_in newServer_addr{};
        // newServer_addr.sin_family = AF_INET;
        // newServer_addr.sin_port = htons(portNumberToSend);
        // newServer_addr.sin_addr.s_addr = INADDR_ANY;
        //
        // bind(server_socket, (sockaddr*)&newServer_addr, sizeof(newServer_addr));
        // listen(server_socket, SOMAXCONN);
        //
        // closesocket(client_socket);
        // client_size = sizeof(client_addr);
        // client_socket = accept(server_socket, (sockaddr*)&client_addr, &client_size);
        //
        // //trying to receive a new message after changing port
        // string tempMessage;
        // memset(buffer, 0, sizeof(buffer));
        // recv(client_socket, buffer, sizeof(buffer), 0);
        // tempMessage = buffer;
        // cout << "tempMessage : " << tempMessage<<endl;

    }

    WSACleanup();
    return 0;
}
