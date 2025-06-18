#ifndef CLIENT_H
#define CLIENT_H
#include <string>
#include <vector>
using namespace std;
class ClientUser {
public:
    ClientUser();
    //getters
    string getClientName ()const;
    vector<string> getDestinations() const;
    int getPortNumber() const;
    bool getIsActive()const;
    //setters
    void setClientName(const string &s);
    void setIsActive(bool b);
    void setPortNumber(int i);
    void addDestination(const string &s);
private:
    string clientName;
    vector<string> destinations;
    int portNumber;
    bool isActive;
};
#endif //CLIENT_H
