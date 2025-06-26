#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <string>
#include <sstream>
#include <vector>
#pragma comment(lib, "ws2_32.lib")
using namespace std;

void printMenu() {
        cout<< "1.Open messaging mode\n2.Choose users to send message\n3.Check for messages\n4.See all available users\n5.See my message history\n6.Disconnect\nWhat do you want to do? "<<endl;
}

/**
 * this method is for receiving and displaying unseen messages in the third option
 * @param client client socket to display
 */
void receiveAndDisplayUnseenMessages(SOCKET client) {

    char buffer[1024];
    bool terminate = false;
    while (!terminate ) {
        memset(buffer, 0, sizeof(buffer));
        recv(client, buffer, sizeof(buffer), 0);
        string msg = buffer;
        if (msg.back() == '0')
        {terminate = true;}
        cout << msg.substr(0,msg.size() - 1 )<< endl;
        string feedback = "read";
        send(client, feedback.c_str(), feedback.size(), 0);


    }

}

/**
 * method to display coming messages in the message mode
 * @param client client to display
 */
void displayMessages(SOCKET client) {
    char buffer [1024];
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        recv(client, buffer, sizeof(buffer), 0);
        string msg = buffer;
        cout << msg << endl;
        memset(buffer, 0, sizeof(buffer));
    }
}

int main() {

     //for receiving strings from server
     char buffer[1024];

    //creating client socket
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    //server address
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    connect(clientSocket, (struct sockaddr*)&server_addr, sizeof(server_addr));

    //then checking if it is available to connect
    char readyToStart;
    recv(clientSocket, &readyToStart, sizeof(readyToStart), 0);

    if (readyToStart == '0'  ) {

        cout << "Cannot connect to the server, it is full. Try another time!" << endl;
        closesocket(clientSocket);
        WSACleanup();
        return 0;
    }

    else{
        cout  << endl;
        cout<< "Welcome to server." << endl;

        //getting the username from the user
        string name;
        cout<<"Enter username: ";
        getline(cin, name);

        //sending the name we choose and sending it to the server
        send(clientSocket, name.c_str(), name.length(), 0);

        //receiving answer that shows if duplicated
        char isDuplicated;
        recv(clientSocket, &isDuplicated, sizeof(isDuplicated), 0);

        //if duplicated taking username input again
        while (isDuplicated == '1') {
        cout << endl;
        cout<<"This username already exists. Try again!" << endl;
        cout<<"Enter username: ";
        getline(cin, name);

        //sending the name we choose and sending it to the server
        send(clientSocket, name.c_str(), name.length(), 0);

        //taking duplicated answer again
        recv(clientSocket, &isDuplicated, sizeof(isDuplicated), 0);
    }
        if (isDuplicated == '0') {
            cout <<"You are registered successfully!"<< endl;
        }

        //registration is successful
        cout << "Connected to server!" << endl;
        cout << endl;
        //after registration, the new port number is received and the communication is changing according to the new port number
        //menu opens up and the application starts

        string action;
        printMenu();
        getline(cin, action);

        //if the input is invalid
        while (action != "1" && action != "2" && action != "3" && action != "4" && action != "5" && action != "6" ) {
            cout << "Your choice is invalid. Please enter a valid action (1-2-3-4-5-6)!" << endl;
            printMenu();
            getline(cin, action);
        }


        send(clientSocket, action.c_str(), action.length(), 0);

        while (action == "1" || action == "2" || action == "3" || action == "4" || action == "5" || action == "6" ) {

            //1: Messaging Mode
            if (action == "1" ) {
                cout << "Opening message mode. Type /exit to exit." << endl;
                string msg;
                thread(displayMessages, clientSocket).detach();
                while (action == "1" && msg != "/exit") {
                    getline(cin, msg);
                    send(clientSocket, msg.c_str(), msg.length(), 0);
                }
            }

            //2: choosing users to send message
            if (action == "2") {
                string destinations;

                //receiving all the active client names and displaying them
                memset(buffer, 0, sizeof(buffer));
                recv(clientSocket, buffer, sizeof(buffer), 0);
                string activeClientNames = buffer;

                //dividing the received names string and displaying
                vector<string> clientNames;
                stringstream ss(activeClientNames);
                string singleName;
                int count = 0;

                while (getline(ss, singleName, '/')) {
                    cout << ++count << "." << singleName << endl;
                }


                //taking the input from the user
                cout << "Who do you want to message? Enter as numbers with spaces: " <<endl;
                getline(cin, destinations);
                send(clientSocket, destinations.c_str(), destinations.length(), 0);

                //receiving answer for validation
                memset(buffer, 0, sizeof(buffer));
                recv(clientSocket, buffer, sizeof(buffer), 0);
                string validationAnswer = buffer;

                //if the input is not valid, taking input until it is valid
                while (validationAnswer == "no") {
                    cout <<"Invalid input try again: "<<endl;
                    getline(cin, destinations);
                    send(clientSocket, destinations.c_str(), destinations.length(), 0);
                    memset(buffer, 0, sizeof(buffer));
                    recv(clientSocket, buffer, sizeof(buffer), 0);
                    validationAnswer = buffer;
                }
                cout << "Your message destinations are set successfully !" << endl;
            }

            //3: displaying unseen messages
            if (action == "3") {
                receiveAndDisplayUnseenMessages(clientSocket);
                cout << endl;
            }
            //4: displaying all the active clients
            if (action == "4") {

                //receiving all the names of clients
                memset(buffer, 0, sizeof(buffer));
                recv(clientSocket, buffer, sizeof(buffer), 0);
                string activeClientNames = buffer;

                //dividing the received names string and displaying
                stringstream ss(activeClientNames);
                string singleName;
                int count = 0;
                while (getline(ss, singleName, '/')) {
                    cout << ++count << "." << singleName << endl;
                }
                cout << endl;
            }

            printMenu();
            getline(cin, action);
            send(clientSocket, action.c_str(), action.length(), 0);

        }

}
    closesocket(clientSocket);
    WSACleanup();
    return 0;
}
