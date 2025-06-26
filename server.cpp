#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <vector>
#include <string>
#include <sstream>
#include "ClientUser.cpp"
#include "Message.cpp"
#pragma comment(lib, "ws2_32.lib")
using namespace std;


// Client storage
vector<SOCKET> allClientSockets;
vector<ClientUser> allClientObjects;
vector<Message> AllMessages;
vector<Message>allUnseenMessages;
int client_count = 0;

/**
 * a helper method to determine the client index in the storage from the given client socket
 * @param client given client socket
 * @return index
 */
int getClientIndex(SOCKET client) {
    for (int i = 0; i < allClientSockets.size(); i++) {
        if (allClientSockets[i] == client) {
            return i;
        }
    }
    return -1;
}

/**
 * this method sends  all unseen messages to given user
 * @param client_socket client to send
 * @return if there are no unseen messages it returns false else it returns true
 */
void sendUnseenMessagesToUser(SOCKET client_socket) {

    //name of the client socket
    string destinationName = allClientObjects[getClientIndex(client_socket)].getClientName();
    int count = 0;

    //determining how many messages to send
    for (int i = 0; i < allUnseenMessages.size(); i++) {
        if (allUnseenMessages[i].getDestination() == destinationName) {
            count++;
        }
    }
    char buffer[1024];
    for (int k = 0 , c = 0 ; k < allUnseenMessages.size(); k++) {

        //if this is not the last message to send
        if (allUnseenMessages[k].getDestination() == destinationName && c < count - 1) {

            //if it is not the last message, the server puts '1' to the end
            string msg = allUnseenMessages[k].getSender() +": " +allUnseenMessages[k].getMessage() + '1';
            send(client_socket, msg.c_str(), msg.length(), 0);
            c++;

            //taking feedback for stoping TCP to break the messages
            memset(buffer, 0, sizeof(buffer));
            recv(client_socket, buffer, sizeof(buffer), 0);


        }
        //last message to send
        else if (allUnseenMessages[k].getDestination() == destinationName && c == count - 1) {
            //if last it puts '0' to end of the message
            string msg = allUnseenMessages[k].getSender() +": " +allUnseenMessages[k].getMessage() + '0';
            send(client_socket, msg.c_str(), msg.length(), 0);

            //taking feedback for stoping TCP to break the messages
            memset(buffer, 0, sizeof(buffer));
            recv(client_socket, buffer, sizeof(buffer), 0);
        }

    }
}

/**
 * this method checks if the input of the user for choosing destinations is correct, if it is correct it add these destinations to storage and output true, if not it outputs false
 * @param destinations string input
 * @param client_socket client socket to apply this process
 * @return valid or not
 */
bool isDestinationsValid(const string &destinations,const SOCKET client_socket) {

    //dividing the received names string and displaying
    vector<string> allDestinations;
    stringstream ss(destinations);
    string singleDestination;

    while (getline(ss, singleDestination, ' ')) {
        //checks if the index already exists
        if (find(allDestinations.begin(), allDestinations.end(), singleDestination) == allDestinations.end() )
        allDestinations.push_back(singleDestination);
    }

    //checking if the inputs are valid
    for (int i = 0; i < allDestinations.size(); i++) {
        if ((stoi(allDestinations[i]) > client_count) || (stoi(allDestinations[i]) <= 0)) {
            return false;
        }
    }

    //creating the true int vector and adding it to the true ClientUser object in the storage
    vector <int> indexesOfDestinations;
    for (int i = 0; i < allDestinations.size(); i++) {
        indexesOfDestinations.push_back(stoi(allDestinations[i]) - 1);
    }
    //determining the index from the storage
    int indexOfTheUser = 0;
    for (int i = 0; i < allClientSockets.size(); i++) {
        if (allClientSockets[i] == client_socket) {
            indexOfTheUser = i;
        }
    }
    //setting the ClientUser object's destinations
    allClientObjects[indexOfTheUser].setDestinations(indexesOfDestinations);
    return true;



}
/**
 * method to check whether a name exists in the clients storage
 * @param name given name
 * @return if exists or not
 */
bool isDuplicated(const string &name) {
    for (int i = 0; i < allClientObjects.size(); i++) {
        if (allClientObjects[i].getClientName() == name) {
            return true;
        }
    }
    return false;

}


/**
 *this method sends all available users to given client
 * @param client which client to send
 */
void sendClientAllUserNames(SOCKET client) {
    string usernames;
    for (int i = 0; i < allClientObjects.size(); i++) {
        usernames += allClientObjects[i].getClientName() + " ";
    }
    send(client,usernames.c_str(), usernames.length(), 0);
}

/**
 * method for all the handling process of a client
 * @param server_socket
 */
void handle_client_all(SOCKET client_socket) {
    //a char array for receiving messages
    char buffer [1024];

    //first it will send feedback to clients to decide whether they can continue
    char readyToStart;
    if (client_count >= 3) {
        readyToStart = '0';
    }
    else {
        readyToStart = '1';
    }
    send(client_socket, &readyToStart, sizeof(readyToStart), 0);

    //receiving the username
    string receivedName;
    memset(buffer, 0, sizeof(buffer));
    recv(client_socket, buffer, sizeof(buffer), 0);
    receivedName = buffer;

    //checking if it is duplicated
    bool duplicated = isDuplicated(receivedName);
    char duplicatedAnswer;

    //if it is duplicated, the server will send client the output
    while (duplicated){

        duplicatedAnswer = '1';
        //sending the duplicated answer
        send(client_socket, &duplicatedAnswer, sizeof(duplicatedAnswer), 0);

        //receiving a new name
        memset(buffer, 0, sizeof(buffer));
        recv(client_socket, buffer, sizeof(buffer), 0);
        receivedName = buffer;
        duplicated = isDuplicated(receivedName);

    }

    //if the username is not a duplicate we send this feedback to user
    duplicatedAnswer = '0';
    send(client_socket, &duplicatedAnswer, sizeof(duplicatedAnswer), 0);

    //registering the new client
    ClientUser newClient;
    newClient.setClientName(receivedName);
    newClient.setIsActive(true);

    client_count++;
    allClientObjects.push_back(newClient);
    allClientSockets.push_back(client_socket);

    cout << "Client " <<receivedName << " connected."  << endl;



    //now the user is in the menu
    memset(buffer, 0, sizeof(buffer));
    recv(client_socket, buffer, sizeof(buffer), 0);
    string action = buffer;
    while (action != "6"){

        //messaging mode
        if (action == "1") {

            //determining client index
            int clientIndex = getClientIndex(client_socket) ;

            //setting the messaging mode
            allClientObjects[clientIndex].setInMessageMode(true);

            //receiving the message
            string msg;
            memset(buffer, 0, sizeof(buffer));
            recv(client_socket, buffer, sizeof(buffer), 0);
            msg = buffer;
            while (msg != "/exit") {

                for (int i = 0 ; i < allClientObjects[clientIndex].getDestinations().size(); i++ ) {

                    //storing message
                    int destinationIndex = allClientObjects[clientIndex].getDestinations()[i];
                    Message newMessage;
                    newMessage.setDestination(allClientObjects[destinationIndex].getClientName());
                    newMessage.setSender(allClientObjects[clientIndex].getClientName());
                    newMessage.setMessage(msg);
                    AllMessages.push_back(newMessage);
                    //if the destination is not in the messaging mode, the message is stored in unseen messages vector
                    if (!allClientObjects[destinationIndex].getInMessageMode()) {
                        allUnseenMessages.push_back(newMessage);
                        cout<<"Unseen message: " << newMessage.getSender() << "->" << newMessage.getDestination() << ": "<<newMessage.getMessage() << endl;
                    }
                    else {
                        string messageToSend = newMessage.getSender() + ": "+ newMessage.getMessage();
                        cout<< newMessage.getSender() << "->" << newMessage.getDestination() << ": "<<newMessage.getMessage() << endl;
                        send(allClientSockets[destinationIndex], messageToSend.c_str(), messageToSend.length(), 0);
                    }

                }
                memset(buffer, 0, sizeof(buffer));
                recv(client_socket, buffer, sizeof(buffer), 0);
                msg = buffer;
            }
            allClientObjects[clientIndex].setInMessageMode(false);

        }
        //choosing destinations
        else if (action == "2") {

            //sending user the names
            sendClientAllUserNames(client_socket);

            memset(buffer, 0, sizeof(buffer));
            recv(client_socket, buffer, sizeof(buffer), 0);
            string destinations = buffer;

            //checking if it is valid, this method adds destinations to storage if it is valid
            bool isValid = isDestinationsValid(destinations,client_socket);
            string destinationsValid;

            //if the input is not valid, server repeats the process until the input is valid
            while (!isValid) {
                destinationsValid = "no";
                send(client_socket, destinationsValid.c_str(), destinationsValid.length(), 0);
                memset(buffer, 0, sizeof(buffer));
                recv(client_socket, buffer, sizeof(buffer), 0);
                destinations = buffer;
                isValid = isDestinationsValid(destinations,client_socket);
            }
            //the input is valid, the server gives feedback
            destinationsValid = "yes";
            send(client_socket, destinationsValid.c_str(), destinationsValid.length(), 0);

        }
        //checking unseen messages
        else if (action == "3") {
            sendUnseenMessagesToUser(client_socket);
        }
        //see all available users
        else if (action == "4") {
            sendClientAllUserNames(client_socket);
        }
        //looking messaging history
        else if (action == "5") {

        }
        //disconnect
        else if (action == "6") {

        }

        memset(buffer, 0, sizeof(buffer));
        recv(client_socket, buffer, sizeof(buffer), 0);
        action = buffer;

    }
}

int main() {
    char buffer[1024];

    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    //defining server address
    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    //binding
    bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));

    //listening
    //second parameter is the number of maximum waiting clients, for ex if it is 5 it means that server can accept 5 clients and the sixth one will be declined
    listen(serverSocket, SOMAXCONN);

    cout << "Server started!"<< endl;


        while (true) {

            //creating client socket
            sockaddr_in client_addr{};
            int client_size = sizeof(client_addr);
            SOCKET client_socket = accept(serverSocket, (sockaddr*)&client_addr, &client_size);
            thread(handle_client_all, client_socket).detach();

    }

    WSACleanup();
    return 0;
}
