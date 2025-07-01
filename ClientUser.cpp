#include <iostream>
#include "ClientUser.h"
using namespace std;

ClientUser::ClientUser() {
    clientName = "";
    inMessageMode = false;
    isActive = false;
    destinations = vector<int>();
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

//setters
void ClientUser::setClientName(const string &s) {clientName = s;}
void ClientUser::setIsActive(const bool b) {isActive = b;}
void ClientUser::setDestinations(const vector<int> &d) {destinations = d;}
void ClientUser::setInMessageMode(const bool b) {inMessageMode = b;}




