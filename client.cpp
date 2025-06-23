#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <string>
#include <sstream>
#include <vector>
#pragma comment(lib, "ws2_32.lib")
using namespace std;


int main() {

    //creating client socket
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    //server address
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    //connect(clientSocket, (sockaddr*)&server_addr, sizeof(server_addr));
    connect(clientSocket, (struct sockaddr*)&server_addr, sizeof(server_addr));
    cout << "connect" << endl;

    //then checking if it is available to connect
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    recv(clientSocket, buffer, sizeof(buffer), 0);

    string readyToStart = buffer;
    if (readyToStart == "no" ) {

        cout << "Cannot connect to the server, it is full. Try another time!" << endl;
        closesocket(clientSocket);
        WSACleanup();
        return 0;
    }

    else{

        cout<< "Welcome to server." << endl;

        //getting the username from the user
        string name;
    cout << endl;
    cout<<"Enter username: ";
    cin >> name;

    //sending the name we choose and sending it to the server
    send(clientSocket, name.c_str(), name.length(), 0);

        //receiving answer that shows if duplicated
    memset(buffer, 0, sizeof(buffer));
    recv(clientSocket, buffer, sizeof(buffer), 0);
    string duplicatedAnswer = string(buffer);

        //if duplicated taking user name input again
    while (duplicatedAnswer == "duplicated") {
        cout << endl;
        cout<<"This username already exists. Try again!" << endl;
        cout<<"Enter username: ";
        cin >> name;
        cout<<name<<endl;

        //sending the name we choose and sending it to the server
        send(clientSocket, name.c_str(), name.length(), 0);

        //taking duplicated answer again
        memset(buffer, 0, sizeof(buffer));
        recv(clientSocket, buffer, sizeof(buffer), 0);
        duplicatedAnswer = string(buffer);
        cout <<"received duplicated answer: " << duplicatedAnswer << endl;
    }
        if (duplicatedAnswer == "okey") {
            cout <<"You are registered successfully!"<< endl;
        }

        //registration is successful
        cout << "Connected to server!" << endl;

        //after registration, the new port number is received and the communication is changing according to the new port number

        //menu opens up and the application starts
        string action;
        cout<< "1.Open messaging mode\n2.Choose users to send message\n3.Check for messages\n4.See all available users\n5.See my message history\n6.Disconnect\nWhat do you want to do? "<<endl;
        cin >> action;
        //if the input is invalid
        while (action != "1" && action != "2" && action != "3" && action != "4" && action != "5" && action != "6" ) {
            cout << "Your choice is invalid. Please enter a valid action (1-2-3-4)!" << endl;
            cout<< "1.Open messaging mode\n2.Choose users to send message\n3.Check for messages\n4.See all available users\n5.See my message history\n6.Disconnect\nWhat do you want to do? "<<endl;
            cin >> action;
        }

        while (action == "1" || action == "2" || action == "3" || action == "4" || action == "5" || action == "6" ) {

            send(clientSocket, action.c_str(), action.length(), 0);

            //displaying all the active clients
            if (action == "4") {

                //receiving all the names of clients
                memset(buffer, 0, sizeof(buffer));
                recv(clientSocket, buffer, sizeof(buffer), 0);
                string activeClientNames = buffer;

                //dividing the received names string and displaying
                vector<string> clientNames;
                stringstream ss(activeClientNames);
                string singleName;

                while (getline(ss, singleName, ' ')) {
                    clientNames.push_back(singleName);
                }
                for (int i = 0 ; i < clientNames.size(); i++) {
                    cout << i+1 <<"."<< clientNames[i] << endl;
                }

            }

        }





}
    closesocket(clientSocket);
    WSACleanup();
    return 0;
}
