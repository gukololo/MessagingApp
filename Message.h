//
// Created by gurka on 17.06.2025.
//
#ifndef MESSAGE_H
#define MESSAGE_H

#include <string>
using namespace  std;
class Message {

public:
    //constructor
    Message();
    //getters
    string getMessage() const;
    string getTime() const;
    string getSender() const;
    string getDestination() const;
    //setters
    void setMessage(const string &s);
    void setTime(const string &s);
    void setSender(const string &s);
    void setDestination(const string &s);

private:
    string msg;
    string time;
    string sender;
    string destination;


};
#endif //MESSAGE_H
