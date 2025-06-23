#ifndef CLIENTUSER_H
#define CLIENTUSER_H
#include <string>
#include <vector>
using namespace std;
class ClientUser {
public:
    ClientUser();
    //getters
    string getClientName ()const;
    vector<int> getDestinations() const;
    int getPortNumber() const;
    bool getInMessageMode()const;
    bool getIsActive()const;
    //setters
    void setClientName(const string &s);
    void setInMessageMode(bool b);
    void setIsActive(bool b);
    void setPortNumber(int i);
    void setDestinations(const vector<int> &s);
private:
    string clientName;
    vector<int> destinations;
    int portNumber;
    bool inMessageMode;
    bool isActive;
};
#endif
