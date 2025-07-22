#ifndef CLIENTUSER_H
#define CLIENTUSER_H
#include <string>
#include <vector>
#include <WinSock2.h>
using namespace std;
class ClientUser {
public:
    ClientUser();
    //getters
    string getClientName ()const;
    vector<string> getDestinations() const;
    bool getInMessageMode()const;
    bool getIsActive()const;
	SOCKET getClientSocket() const;
    //setters
    void setClientName(const string &s);
    void setInMessageMode(bool b);
    void setIsActive(bool b);
    void setDestinations(const vector<string> &s);
	void setClientSocket(const SOCKET &s);
private:
    string clientName;
    vector<string> destinations;
    bool inMessageMode;
    bool isActive;
	SOCKET clientSocket;
};
#endif
