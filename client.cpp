#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <string>
#include <sstream>
#include <vector>
#pragma comment(lib, "ws2_32.lib")
using namespace std;

/**
 * function to print options to user
 */
void printMenu() {
    cout << "1.Open messaging mode\n2.Choose users to send message\n3.Check for unseen messages\n4.See all online users\n5.See my message history \n6.Disconnect \nWhat do you want to do? " << endl;
}

/**
 * This function opens offline mode, sends user to waiting, if user wants to reconnect, it receives signal from the server and according to this signal it reconnects
 * @param client client that will go offline
 */
void openOfflineMode(SOCKET client) {
    //taking done signal from the server
    char start;
    recv(client, &start, sizeof(start), 0);
    cout << "Disconnected. Type /return to return." << endl;
    string back;
    while (back != "/return") {
        getline(cin, back);
        if (back != "/return") {
            cout << "Invalid comment." << endl;
        }
    }
    cout << "\nReconnecting to the server!" << endl;
    char end = '1';
    send(client, &end, 1, 0);
    char finish;
    recv(client, &finish, sizeof(finish), 0);
}

/**
* method for displaying active clients
*/
void displayActiveClients(SOCKET clientSocket) {

    char buffer[1024];
    int count = 0;
	bool terminate = false;
    while (!terminate) {
		memset(buffer, 0, sizeof(buffer));
        recv(clientSocket, buffer, sizeof(buffer), 0);
        string receivedUsername = buffer;
        count++;
        cout << count << "." << receivedUsername.substr(0, receivedUsername.size() - 1) << endl;
        char read = '1';
        send(clientSocket, &read, 1, 0);
        if(receivedUsername.back() == '0') {
            terminate = true;
		}
        
    }
    char done;
	recv(clientSocket, &done, sizeof(done), 0);
}
/**
 * if the input action is not valid this method is used to take input again
 * @return valid action input
 */
string retakeAction() {
    string action;
    cout << "Your choice is invalid. Please enter a valid action (1-2-3-4-5-6)!" << endl;
    printMenu();
    getline(cin, action);
	cout << endl;
    return action;
}
/**
 * this method is for receiving and displaying unseen messages in the third option
 * @param client client socket to display
 */
void receiveAndDisplayUnseenMessages(SOCKET client) {

    char buffer[1024]; //char array to receive messages
    bool terminate = false; //boolean that depends on if the received message is the last one

    while (!terminate) {

        //receiving message
        memset(buffer, 0, sizeof(buffer));
        recv(client, buffer, sizeof(buffer), 0);
        string msg = buffer;

        //determine if it is the last one
        if (msg.back() == '0')
        {
            terminate = true;
        }

        //display the message
        cout << msg.substr(0, msg.size() - 1) << endl;

        //giving feedback
        string feedback = "read";
        send(client, feedback.c_str(), feedback.size(), 0);


    }

}

/**
 * this method applies in the 5th option where the user receives the message history
 * @param client client to handle
 */
void receiveAndDisplayHistory(SOCKET client) {

    char buffer[1024]; //char array to receive messages
    bool terminate = false; //boolean that depends on if the received message is the last one
    cout << endl;
    while (!terminate) {

        //receiving message
        memset(buffer, 0, sizeof(buffer));
        recv(client, buffer, sizeof(buffer), 0);
        string msg = buffer;

        //determine if it is the last one
        if (msg.back() == '0')
        {
            terminate = true;
        }

        //display the message
        cout << msg.substr(0, msg.size() - 1) << endl;

        //giving feedback
        string feedback = "read";
        send(client, feedback.c_str(), feedback.size(), 0);


    }

}

/**
 * method to display coming messages in the message mode
 * @param client client to display
 */
void displayMessages(SOCKET client) {
    char buffer[1024];
    while (true) {
        //receiving messages
        memset(buffer, 0, sizeof(buffer));
        recv(client, buffer, sizeof(buffer), 0);
        string msg = buffer;
        if (msg == "/exit/")
            return;
        //display
        cout << msg << endl;
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


        cout << endl;
        cout << "Welcome to server." << endl;

        //getting the username from the user
        string name;
        cout << "Enter username: ";
        getline(cin, name);

        //sending the name we choose and sending it to the server
        send(clientSocket, name.c_str(), name.length(), 0);

        //receiving answer that shows if duplicated
        char isDuplicated;
        recv(clientSocket, &isDuplicated, sizeof(isDuplicated), 0);

        //if duplicated taking username input again
        while (isDuplicated == '1') {
            cout << endl;
            cout << "This username already exists. Try again!" << endl;
            cout << "Enter username: ";
            getline(cin, name);

            //sending the name we choose and sending it to the server
            send(clientSocket, name.c_str(), name.length(), 0);

            //taking duplicated answer again
            recv(clientSocket, &isDuplicated, sizeof(isDuplicated), 0);
        }

        //then checking if it is available to connect
        char readyToStart;
        recv(clientSocket, &readyToStart, sizeof(readyToStart), 0);

        //if server is full
        if (readyToStart == '0') {
            cout << "Cannot connect to the server, it is full. Try another time!" << endl;
            closesocket(clientSocket);
            WSACleanup();
            return 0;
        }

        //registration is successful
        cout << endl;
        cout << "You are registered successfully!" << endl;
        cout << "Connected to server!" << endl;
        cout << endl;
        //after registration, the new port number is received and the communication is changing according to the new port number
        //menu opens up and the application starts

        string action;
        printMenu();
        getline(cin, action);
		cout << endl;

        //if the input is invalid
        while (action != "1" && action != "2" && action != "3" && action != "4" && action != "5" && action != "6") {
            action = retakeAction();
        }


        send(clientSocket, action.c_str(), action.length(), 0);

        while (action == "1" || action == "2" || action == "3" || action == "4" || action == "5" || action == "6") {

            //1: Messaging Mode
            if (action == "1") {
                cout << "Opening message mode. Type /exit to exit." << endl;
                char isDestinationEmpty;
                recv(clientSocket, &isDestinationEmpty, sizeof(isDestinationEmpty), 0);
                string msg;
                thread(displayMessages, clientSocket).detach();
                while (action == "1" && msg != "/exit") {
                    getline(cin, msg);
                    if (isDestinationEmpty == '1' && msg != "/exit") {
                        cout << "Cannot send message, you have no destinations! " << endl;
                    }
                    else {
                        send(clientSocket, msg.c_str(), msg.length(), 0);
                    }
                }
            }

            //2: choosing users to send message
            if (action == "2") {

                displayActiveClients(clientSocket);
                //taking the input from the user
                string destinations;
                cout << "Who do you want to message? Enter as numbers with spaces: " << endl;
                getline(cin, destinations);
                send(clientSocket, destinations.c_str(), destinations.length(), 0);

                //receiving answer for validation
                memset(buffer, 0, sizeof(buffer));
                recv(clientSocket, buffer, sizeof(buffer), 0);
                string validationAnswer = buffer;

                //if the input is not valid, taking input until it is valid
                while (validationAnswer == "no") {
                    cout << "Invalid input try again: " << endl;
                    getline(cin, destinations);
                    send(clientSocket, destinations.c_str(), destinations.length(), 0);
                    memset(buffer, 0, sizeof(buffer));
                    recv(clientSocket, buffer, sizeof(buffer), 0);
                    validationAnswer = buffer;
                }
                cout << "Your message destinations are sets successfully !" << endl;
            }

            //3: displaying unseen messages
            if (action == "3") {
                receiveAndDisplayUnseenMessages(clientSocket);
                cout << endl;
            }
            //4: displaying all the active clients
            if (action == "4") {
                displayActiveClients(clientSocket);
                cout << endl;

            }
			//5: displaying message history
            if (action == "5") {
                receiveAndDisplayHistory(clientSocket);
                cout << endl;
            }
			//6: disconnect mode
            if (action == "6") {
                openOfflineMode(clientSocket);
                cout << endl;
            }
            printMenu();
            getline(cin, action);
			cout << endl;
            while (action != "1" && action != "2" && action != "3" && action != "4" && action != "5" && action != "6") {
                action = retakeAction();
            }
            send(clientSocket, action.c_str(), action.length(), 0);

        }

    
    closesocket(clientSocket);
    WSACleanup();
    return 0;
}
