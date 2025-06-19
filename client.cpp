#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <string>
#include <sstream>
#pragma comment(lib, "ws2_32.lib")
using namespace std;

void receive_messages(SOCKET sock) {
    char buffer[1024];
    int bytes;
    while ((bytes = recv(sock, buffer, sizeof(buffer), 0)) > 0) {
        buffer[bytes] = '\0';
        cout << "\n[Message]: " << buffer << endl;
    }
}

int main() {
    //standards
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    //8080
    server_addr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);
    connect(sock, (sockaddr*)&server_addr, sizeof(server_addr));

    int server_size = sizeof(server_addr);
    SOCKET server_socket = accept(sock, (sockaddr*)&server_addr, &server_size);


    //then checking if it is available to connect
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    recv(sock, buffer, sizeof(buffer), 0);

    string numberOfClients = string(buffer);
    int numClients = stoi(numberOfClients);
    if (numClients >= 3 ) {

        cout << "Cannot connect to the server there are 3 clients. Try another time!" << endl;
        closesocket(sock);
        WSACleanup();
        return 0;
    }

    else{

        cout<< "There are " << numClients << " clients. Connecting to server." << endl;
    //getting the username from the user

    string name;
    cout << endl;
    cout<<"Enter username: ";
    cin >> name;

    //sending the name we choose and sending it to the server
    send(sock, name.c_str(), name.length(), 0);
    memset(buffer, 0, sizeof(buffer));
    recv(sock, buffer, sizeof(buffer), 0);
    string duplicatedAnswer = string(buffer);
    //cout <<"received duplicated answer: " << duplicatedAnswer << endl;
    while (duplicatedAnswer == "duplicated") {
        cout << endl;
        cout<<"This username already exists. Try again!" << endl;
        cout<<"Enter username: ";
        cin >> name;
        cout<<name<<endl;
        //sending the name we choose and sending it to the server
        send(sock, name.c_str(), name.length(), 0);
        memset(buffer, 0, sizeof(buffer));
        recv(sock, buffer, sizeof(buffer), 0);
        duplicatedAnswer = string(buffer);
        cout <<"received duplicated answer: " << duplicatedAnswer << endl;
    }
        if (duplicatedAnswer == "okey") {
            cout <<"You are registered successfully!"<< endl;
        }

        //registration is successful
        cout << "Connected to server!" << endl;



        //after registration, the new port number is received and the communication is changing according to the new port number
        int newPortNumber;
        recv(sock, (char*)&newPortNumber, sizeof(newPortNumber), 0);
        //cout << "NEW PORT NUM: "<<newPortNumber << endl;
        cout<< endl;
        closesocket(sock);
        closesocket(server_socket);

        sock = socket(AF_INET, SOCK_STREAM, 0);

        sockaddr_in newServer_addr{};
        newServer_addr.sin_family = AF_INET;
        newServer_addr.sin_port = htons(newPortNumber);
        inet_pton(AF_INET, "127.0.0.1", &newServer_addr.sin_addr);
        connect(sock, (sockaddr*)&newServer_addr, sizeof(newServer_addr));

        server_size = sizeof(newServer_addr);
        server_socket = accept(sock, (sockaddr*)&newServer_addr, &server_size);


        //trying to communciate with server with new port
        string tempMessage ="kanka port degisti haberin ola";
        send(sock, tempMessage.c_str(), tempMessage.length(), 0);
        //cout << tempMessage << endl;




















        //menu opens up and the application starts
        string action;
        cout<< "1.Open messaging mode\n2.Choose users to send message\n3.Check for messages\n4.See all available users\n5.See my message history\n6.Disconnect\nWhat do you want to do? "<<endl;
    cin >> action;
        //if the input is invalid
        while (action != "1" && action != "2" && action != "3" && action != "4" ) {
            cout << "Your choice is invalid. Please enter a valid action (1-2-3-4)!" << endl;
            cout<< "1.Open messaging mode\n2.Choose users to send message\n3.Check for messages\n4.See all available users\n5.See my message history\n6.Disconnect\nWhat do you want to do? "<<endl;
            cin >> action;
        }



    thread(receive_messages, sock).detach();

    string msg;
    while (true) {
        getline(cin, msg);
        send(sock, msg.c_str(), msg.length(), 0);
    }
}
    closesocket(sock);
    WSACleanup();
    return 0;
}
