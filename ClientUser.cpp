#include "ClientUser.h"
#include <iostream>
using namespace std;

ClientUser::ClientUser() {
    clientName = "";
    portNumber = -1;
    isActive = false;
    destinations = vector<string>();
}

//getters
string ClientUser::getClientName() const {
    return clientName;
}
int ClientUser::getPortNumber() const {
    return portNumber;
}
bool ClientUser::getIsActive() const {
    return isActive;
}
vector<string> ClientUser::getDestinations() const {
    return destinations;
}
//setters
void ClientUser::setClientName(const string &s) {clientName = s;}
void ClientUser::setPortNumber(const int i) {portNumber = i;}
void ClientUser::setIsActive(const bool b) {isActive = b;}
void ClientUser::addDestination(const string &s) {destinations.push_back(s);}





