#include <iostream>
#include "ClientUser.h"
using namespace std;

ClientUser::ClientUser() {
    clientName = "";
    portNumber = -1;
    isActive = false;
    destinations = vector<int>();
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
vector<int> ClientUser::getDestinations() const {
    return destinations;
}
//setters
void ClientUser::setClientName(const string &s) {clientName = s;}
void ClientUser::setPortNumber(const int i) {portNumber = i;}
void ClientUser::setIsActive(const bool b) {isActive = b;}
void ClientUser::setDestinations(const vector<int> &d) {destinations = d;}




