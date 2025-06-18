#include<string>
#include "Message.h"

using namespace std;

Message::Message() {
    msg = "";
    time = "";
    destination = "";
    sender = "";
}

//getters
string Message::getDestination() const {
    return destination;
}
string Message::getSender() const {
    return sender;
}
string Message::getMessage() const {
    return msg;
}
string Message::getTime() const {
    return time;
}

//setters
void Message::setMessage(const string &s) {
    msg = s;
}
void Message::setTime(const string &s) {
    time = s;
}
void Message::setSender(const string &s) {
    sender = s;
}
void Message::setDestination(const string &s) {
    destination = s;
}
