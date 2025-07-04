#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <vector>
#include <string>
#include <sstream>
#include <ctime>
#include "ClientUser.h"
#include "Message.h"
#pragma comment(lib, "ws2_32.lib")
using namespace std;

//storages
vector<ClientUser> allClientObjects; //stores all ClientUser objects
vector<Message> AllMessages; //stores all the message objects
vector<Message>allUnseenMessages; //stores all unseen messages
vector <string> allUserNames; //stores all usernames
int state = 0; //state of the server, it is used to determine which process is being applied

/**
 * a helper method to determine the client index in the storage from the given client socket
 * @param client given client socket
 * @return index
 */
int getClientIndex(SOCKET client) {
    for (int i = 0; i < allClientObjects.size(); i++) {
        if (allClientObjects[i].getClientSocket() == client) {
            return i;
        }
    }
    return -1;
}
/**
 * a method for deleting a client from the storage, it finds the client by its socket
 * @param client_socket client socket to delete
 */
void deleteClient(SOCKET client_socket) {
    //find the index of the client in the storage
    int index = getClientIndex(client_socket);

	//deleting the client from all destinations
    for(int i = 0 ; i < allClientObjects.size(); i++) {
		vector<int> destinations = allClientObjects[i].getDestinations();
        if (find(destinations.begin(), destinations.end(), index) != destinations.end()) {
			destinations.erase(remove(destinations.begin(), destinations.end(), index), destinations.end());
        }
	}

    if (index != -1) {
        //if it is found, we delete it from the storage
        vector<int> emptyDestinations;
        cout << "Client " << allClientObjects[index].getClientName() << " quit." << endl;
        allClientObjects[index].setDestinations(emptyDestinations);
        allClientObjects.erase(allClientObjects.begin() + index);
    }
    else {
        return;
    }
}
/**
 * a method for receiving strings from the client, it handles the socket errors
 * @param s string to store the received string
 * @param client_socket client socket to handle
 * @return if successful or not
 */
bool enhancedRecvStr(string& s, SOCKET client_socket) {

	char buffer[1024];
	int bytesReceived = 0;
	memset(buffer, 0, sizeof(buffer));
	bytesReceived = recv(client_socket, buffer, sizeof(buffer), 0);
    if (bytesReceived <= 0) {
		deleteClient(client_socket);
        closesocket(client_socket);
        return false;
    }
    else {
        s = buffer;
		return true;
    }
}
/**
 * a method for receiving characters from the client, it handles the socket errors
 * @param c character to store the received character
 * @param client_socket client socket to handle
 * @return if successful or not
 */
bool enhancedRecvChar(char& c, SOCKET client_socket) {
    int bytesReceived = recv(client_socket, &c, sizeof(c), 0);
    if (bytesReceived <= 0) {
        deleteClient(client_socket);
        closesocket(client_socket);
        return false;

    }
    return true;
}
/**
 * a method for sending strings to the client, it handles the socket errors
 * @param s string to send
 * @param client_socket client socket to handle
 * @return if successful or not
 */
bool enhancedSendStr(const string& s, SOCKET client_socket) {
    int bytesSent = send(client_socket, s.c_str(), s.length(), 0);
    if (bytesSent <= 0) {
        deleteClient(client_socket);
        closesocket(client_socket);
        return false;

    }
    else {
        return true;
    }
}
/**
 * a method for sending characters to the client, it handles the socket errors
 * @param c character to send
 * @param client_socket client socket to handle
 * @return if successful or not
 */
    bool enhancedSendChar(const char& c, SOCKET client_socket) {
        int bytesSent = send(client_socket, &c, sizeof(c), 0);
        if (bytesSent <= 0) {
            deleteClient(client_socket);
            closesocket(client_socket);
            return false;
        }
        else {
            return true;
        }
    }

/**
 * method for getting to amount of online users
 * @return count of active users
 */
int getActiveClientAmount() {
    int count = 0;
    for (const auto& allClientObject : allClientObjects) {
        if (allClientObject.getIsActive()) {
            count++;
        }
    }
    return count;
}


/**
 * Returns the current hour and minute in the format [HH:MM]
 * @return string in the form [hour:minute]
 */
string getHourAndMinute() {
  
    time_t now = time(0);
    tm timeInfo;
    localtime_s(&timeInfo, &now); 

    int hour = timeInfo.tm_hour;
    int minute = timeInfo.tm_min;

    string hourStr = to_string(hour);
    string minStr = to_string(minute);

    if (hour < 10) hourStr = "0" + hourStr;
    if (minute < 10) minStr = "0" + minStr;

    string result = "[" + hourStr + ":" + minStr + "] ";
    return result;

}



/**
 * a helper method which determines if a string is capable to convert to an integer
 * @param str given string
 * @return if successful or not
 */
bool isInteger(const string& str) {
    try {
        size_t pos;
        stoi(str, &pos);
        return pos == str.length();
    }
    catch (...) {
        return false;
    }
}

/**
 * a method for handling the offline mode, it sets the client object to inactive and waits for rejoining
 * @param client_socket client socket to handle
 * @return if successful or not
 */
bool handleOfflineMode(SOCKET client_socket) {
    cout << "Client " << allClientObjects[getClientIndex(client_socket)].getClientName() << " disconnected." << endl;
    int index = getClientIndex(client_socket);
    if(index == -1) {
        return false;
	}
    //setting the client object to inactive
    allClientObjects[index].setIsActive(false);
    char start = '1';
	if(!enhancedSendChar(start, client_socket)) { return false; }
    char endDisconnectMode;
    if (!enhancedRecvChar(endDisconnectMode, client_socket)) {return false;}
      
    char finish = '1';

	if(!enhancedSendChar(finish, client_socket)) { return false; }

  
    //after taking and sending rejoining feedback, the server sets the client object to active again
    allClientObjects[index].setIsActive(true);
    cout << "Client " << allClientObjects[getClientIndex(client_socket)].getClientName() << " reconnected." << endl;
    return true;
}

/**
 * a method for handling the message mode for the given user
 * @param client_socket client socket to handle
 */
bool handleMessagingMode(SOCKET client_socket) {

    char buffer[1024];

    //determining client index
    int clientIndex = getClientIndex(client_socket);
    if(clientIndex == -1) {
        return false;
	}
    //setting the messaging mode
    allClientObjects[clientIndex].setInMessageMode(true);
    
	//determining if the user has no destinations
    char isDestinationEmpty;
    if (allClientObjects[clientIndex].getDestinations().size() == 0) {
        isDestinationEmpty = '1';
    }
    else { isDestinationEmpty = '0'; }
	//sending the information to the user
    if (!enhancedSendChar(isDestinationEmpty, client_socket)) {
		return false;
    }
	//messaging mode starts
    //receiving the message
    string msg;
    if (!enhancedRecvStr(msg, client_socket)) {
        return false;
    }
    while (msg != "/exit") {
        
        for (int i = 0; i < allClientObjects[clientIndex].getDestinations().size(); i++) {

            //storing message
            int destinationIndex = allClientObjects[clientIndex].getDestinations()[i];
            Message newMessage;
			//getting the hour and minute
			string time = getHourAndMinute();
			newMessage.setTime(time);
            newMessage.setDestination(allClientObjects[destinationIndex].getClientName());
            newMessage.setSender(allClientObjects[clientIndex].getClientName());
            newMessage.setMessage(msg);
            AllMessages.push_back(newMessage);
            //if the destination is not in the messaging mode, the message is stored in unseen messages vector
            if (!allClientObjects[destinationIndex].getInMessageMode()) {
                allUnseenMessages.push_back(newMessage);
                //displaying in server
                cout << "Unseen message: " << newMessage.getTime() + newMessage.getSender() << "->" << newMessage.getDestination() << ": " << newMessage.getMessage() << endl;
            }
            else {
                string messageToSend = newMessage.getTime() + newMessage.getSender() + ": " + newMessage.getMessage();
                cout << newMessage.getTime() + newMessage.getSender() << "->" << newMessage.getDestination() << ": " << newMessage.getMessage() << endl;
                //sending the message
                
				if(!enhancedSendStr(messageToSend, allClientObjects[destinationIndex].getClientSocket()))
                {
                    return false;
                }
            }

        }
        //receiving the message
        if (!enhancedRecvStr(msg, client_socket)) {
            return false;
        }
    }
	//if the user exits the messaging mode, we send a signal to stop the messaging mode
    string finish = "/*/exit/*/";
    if (!enhancedSendStr(finish, client_socket)) { return false; }
    //setting the messaging mode to false
    allClientObjects[clientIndex].setInMessageMode(false);
    return true;
}

/**
 * this method sends message all message history to the user, if the user has no message history it sends a warning
 * @param client user to send
 */
void sendMessageHistoryToUser(const SOCKET& client) {

    char buffer[1024];
    int index = getClientIndex(client);
    if (index == -1) {
        return;
    }
    string clientName = allClientObjects[index].getClientName();
    int count = 0;

    //counting the related messages
    for (int i = 0; i < AllMessages.size(); i++) {
        if (AllMessages[i].getDestination() == clientName || AllMessages[i].getSender() == clientName) {
            count++;
        }
    }
    //if there are no related message we send end signal
    if (count == 0) {
        string endMessage = "You have no message history.0";
		enhancedSendStr(endMessage, client);
        return;
    }
    //scanning all messages and sending the related messages to client to print
    int c = 0;
    for (int i = 0; i < AllMessages.size(); i++) {
        if (AllMessages[i].getSender() == clientName || AllMessages[i].getDestination() == clientName) {
            string messageToSend = AllMessages[i].getTime() + AllMessages[i].getSender() + "->" + AllMessages[i].getDestination() + ": " + AllMessages[i].getMessage();
            if (c != count - 1) {
                messageToSend += "1";
            }
            else  {
                messageToSend += "0";
            }
			enhancedSendStr(messageToSend, client);
            string feedback;
			enhancedRecvStr(feedback, client); // receiving feedback to stop TCP from breaking the messages
            c++;
        }
    }


}
/**
 * this method sends  all unseen messages to given user
 * @param client_socket client to send
 * @return if there are no unseen messages it returns false else it returns true
 */
bool sendUnseenMessagesToUser(SOCKET client_socket) {

    //name of the client socket
    string destinationName = allClientObjects[getClientIndex(client_socket)].getClientName();
    int count = 0;

    //determining how many messages to send
    for (int i = 0; i < allUnseenMessages.size(); i++) {
        if (allUnseenMessages[i].getDestination() == destinationName) {
            count++;
        }
    }
    if (count == 0) {
        return false;
    }
    char buffer[1024];

    //tracing the vector and sending suitable messages
    for (int k = 0, c = 0; k < allUnseenMessages.size(); ) {

        //if this is not the last message to send
        if (allUnseenMessages[k].getDestination() == destinationName && c < count - 1) {

            //if it is not the last message, the server puts '1' to the end
            string msg = allUnseenMessages[k].getTime() + allUnseenMessages[k].getSender() + ": " + allUnseenMessages[k].getMessage() + '1';
			enhancedSendStr(msg, client_socket);
            c++;

            //after sending the message, the server deletes it from unseen messages
            allUnseenMessages.erase(allUnseenMessages.begin() + k);

            //taking feedback for stoping TCP to break the messages
			string feedback;
            enhancedRecvStr(feedback, client_socket);


        }
        //last message to send
        else if (allUnseenMessages[k].getDestination() == destinationName && c == count - 1) {
            //if last it puts '0' to end of the message
            string msg = allUnseenMessages[k].getTime() + allUnseenMessages[k].getSender() + ": " + allUnseenMessages[k].getMessage() + '0';
			enhancedSendStr(msg, client_socket);    
            //after sending the message, the server deletes it from unseen messages
            allUnseenMessages.erase(allUnseenMessages.begin() + k);

            //taking feedback for stoping TCP to break the messages
			string feedback;
            enhancedRecvStr(feedback, client_socket);
           
        }
        else
        {
            k++;
        }
    }
    return true;
}

/**
 * this method checks if the input of the user for choosing destinations is correct, if it is correct it add these destinations to storage and output true, if not it outputs false
 * @param destinations string input
 * @param client_socket client socket to apply this process
 * @return valid or not
 */
bool isDestinationsValid(const string& destinations, const SOCKET client_socket) {

    //dividing the received names string and displaying
    vector<string> allDestinations;
    stringstream ss(destinations);
    string singleDestination;

    while (getline(ss, singleDestination, ' ')) {
        //checks if the index already exists
        if (!isInteger(singleDestination)) {
            return false;
        }
		//checking if the index is already in the vector, if not it adds it
        if (find(allDestinations.begin(), allDestinations.end(), singleDestination) == allDestinations.end())
            allDestinations.push_back(singleDestination);
    }

    //checking if the inputs are valid
    for (int i = 0; i < allDestinations.size(); i++) {
		//if the input is not a number or if it is not in the range of the client objects, it returns false
        if ((stoi(allDestinations[i]) > allClientObjects.size()) || (stoi(allDestinations[i]) <= 0)) {
            return false;
        }
    }

    //creating the true int vector and adding it to the true ClientUser object in the storage
    vector <int> indexesOfDestinations;
    for (int i = 0; i < allDestinations.size(); i++) {
        indexesOfDestinations.push_back(stoi(allDestinations[i]) - 1);
    }

    //determining the index from the storage
	int indexOfTheUser = getClientIndex(client_socket); 

    //setting the ClientUser object's destinations
    allClientObjects[indexOfTheUser].setDestinations(indexesOfDestinations);
    return true;



}
/**
 * method to check whether a name exists in the clients storage
 * @param name given name
 * @return if exists or not
 */
bool isDuplicated(const string& name) {
    for (int i = 0; i < allUserNames.size(); i++) {
        if (allUserNames[i] == name) {
            return true;
        }
    }
    return false;

}


/**
 *this method sends all available users to given client
 * @param client which client to send
 */
void sendClientAllUserNames(SOCKET client) {
	
    int count = getActiveClientAmount();
    
	//scanning all clients and sending their names
    for (int i = 0, c = 0; i < allClientObjects.size(); i++) {
        string nameToSend = allClientObjects[i].getClientName();
		//if the client is the last one, it sends '0' to the end of the name
        if (c == count - 1) {
            nameToSend += "0";
			enhancedSendStr(nameToSend, client);
            char read;
			enhancedRecvChar(read, client);
        }
		// if it is not the last one, it sends '1' to the end of the name
        else {
            nameToSend += "1";
            c++;
			enhancedSendStr(nameToSend, client);
            char read;
			enhancedRecvChar(read, client);
        }	
    }
	//sending the end signal to the client
	char done = '1';
	enhancedSendChar(done, client);
}

/**
 * this method handles the process of choosing destinations, it sends all available usernames to the user and checks if the input is valid
 * @param client_socket client socket to handle
 * @return if successful or not
 */
bool handleChoosingDestinations(SOCKET client_socket) {
    char buffer[1024];
    //sending user the names
    sendClientAllUserNames(client_socket);

    string destinations;
    if (!enhancedRecvStr(destinations, client_socket)) {
		return false;   
    }
    //checking if it is valid, this method adds destinations to storage if it is valid
    bool isValid = isDestinationsValid(destinations, client_socket);
    string destinationsValid;

    //if the input is not valid, server repeats the process until the input is valid
    while (!isValid) {
        destinationsValid = "no";
        if (!enhancedSendStr(destinationsValid, client_socket)) { return false; }
       if(!enhancedRecvStr(destinations, client_socket)) { return false; }
        isValid = isDestinationsValid(destinations, client_socket);
    }
    //the input is valid, the server gives feedback
    destinationsValid = "yes";
	enhancedSendStr(destinationsValid, client_socket);
    return true;
}
/**
 * method for all the handling process of a client
 * @param client_socket client socket to handle
 */
void handle_client_all(SOCKET client_socket) {
    //a char array for receiving messages
    char buffer[1024];
    int bytes;
    
    //receiving the username
    string receivedName;
	memset(buffer, 0, sizeof(buffer));
	 bytes = recv(client_socket, buffer, sizeof(buffer), 0);
    if (bytes <= 0) {

		cout << "Client disconnected before sending a name." << endl;
        return;
    }
    receivedName = buffer;

    //checking if it is duplicated
    bool duplicated = isDuplicated(receivedName);
    char duplicatedAnswer;

    //if it is duplicated, the server will send client the output
    while (duplicated) {

        duplicatedAnswer = '1';
        //sending the duplicated answer
        if (!enhancedSendChar(duplicatedAnswer, client_socket)) { return; }

        //receiving a new name
		if(!enhancedRecvStr(receivedName, client_socket)) { return; }
        duplicated = isDuplicated(receivedName);

    }

    //if the username is not a duplicate we send this feedback to user
    duplicatedAnswer = '0';
	enhancedSendChar(duplicatedAnswer, client_socket);  

    //send feedback to clients to decide whether they can continue
    char readyToStart;
    if (getActiveClientAmount() >= 3) {
        readyToStart = '0';
    }
    else {
        readyToStart = '1';
    }
	enhancedSendChar(readyToStart, client_socket);

    //registering the new client
    ClientUser newClient;
	allUserNames.push_back(receivedName);
    newClient.setClientName(receivedName);
    newClient.setIsActive(true);
	newClient.setInMessageMode(false);
	newClient.setClientSocket(client_socket);
    
    //storing the new client
    allClientObjects.push_back(newClient);

    cout << "Client " << receivedName << " connected." << endl;

    //now the user is in the menu
    string action;
    if (!enhancedRecvStr(action, client_socket))
    {return;}
    while (true) {

        //messaging mode
        if (action == "1") {
            if (!handleMessagingMode(client_socket)) { return; }
        }
        //choosing destinations
        else if (action == "2") {

            if (!handleChoosingDestinations(client_socket)) {
                return;
            }

        }
        //checking unseen messages
        else if (action == "3") {
            bool unseenMsgFound = sendUnseenMessagesToUser(client_socket);
            if (!unseenMsgFound) {
                string msg = "No unseen message found!0";
				enhancedSendStr(msg, client_socket);    
            }
        }
        //see all available users
        else if (action == "4") {
            sendClientAllUserNames(client_socket);
        }
        //looking messaging history
        else if (action == "5") {
            sendMessageHistoryToUser(client_socket);
        }
        //disconnect
        else if (action == "6") {
            if (!handleOfflineMode(client_socket)) {return;
        }

        }

        if (!enhancedRecvStr(action, client_socket)) { return; }

    }
}

int main() {

    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    //defining server address
    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    //binding
    bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));

    //listening
    //second parameter is the number of maximum waiting clients, for ex: if it is 5 it means that server can accept 5 clients and the sixth one will be declined
    listen(serverSocket, SOMAXCONN);

    cout << "Server started!" << endl;
    while (true) {
        //creating client socket
        sockaddr_in client_addr{};
        int client_size = sizeof(client_addr);
        SOCKET client_socket = accept(serverSocket, (sockaddr*)&client_addr, &client_size);
        thread(handle_client_all, client_socket).detach();
    }
    WSACleanup();
    return 0;
}
