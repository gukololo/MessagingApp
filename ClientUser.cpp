#include <iostream>
#include "ClientUser.h"
#include <WinSock2.h>
using namespace std;

/**
 * ClientUser class constructor initializes the client user with default values
 */
ClientUser::ClientUser() {
    clientName = "";
    inMessageMode = false;
    isActive = false;
    destinations = vector<int>();
	clientSocket = INVALID_SOCKET; // Initialize with an invalid socket
}

//getters
string ClientUser::getClientName() const {
    return clientName;
}
bool ClientUser::getIsActive() const {
    return isActive;
}
vector<int> ClientUser::getDestinations() const {
    return destinations;
}
bool ClientUser::getInMessageMode() const {
    return inMessageMode;
}
SOCKET ClientUser::getClientSocket() const {
    return clientSocket;
}
//setters
void ClientUser::setClientName(const string &s) {clientName = s;}
void ClientUser::setIsActive(const bool b) {isActive = b;}
void ClientUser::setDestinations(const vector<int> &d) {destinations = d;}
void ClientUser::setInMessageMode(const bool b) {inMessageMode = b;}
void ClientUser::setClientSocket(const SOCKET &s) {clientSocket = s;}



