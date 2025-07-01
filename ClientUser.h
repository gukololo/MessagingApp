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
    vector<int> getDestinations() const;
    bool getInMessageMode()const;
    bool getIsActive()const;
	SOCKET getClientSocket() const;
    //setters
    void setClientName(const string &s);
    void setInMessageMode(bool b);
    void setIsActive(bool b);
    void setDestinations(const vector<int> &s);
	void setClientSocket(const SOCKET &s);
private:
    string clientName;
    vector<int> destinations;
    bool inMessageMode;
    bool isActive;
	SOCKET clientSocket;
};
#endif
