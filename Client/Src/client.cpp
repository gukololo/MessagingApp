#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <string>
#include <sstream>
#include <vector>
#include "MessageFormat.h"


#pragma comment(lib, "ws2_32.lib")
using namespace std;

/**
 * Enum to represent the different states of the client
 */
enum class State {
	REGISTER,
	MENU,
	MESSAGING_MODE,
	CHOOSE_DESTINATION,
	UNSEEN_MESSAGES,
	ACTIVE_USERS,
	MESSAGE_HISTORY,
	DISCONNECT,
	TERMINATE
};
// Global variable to keep track of the current state
State currentState;

/**
 * Sends a message to the server
 * @param message The message to send
 * @param clientSocket The socket to send the message through
 * @return true if the message was sent successfully, false otherwise
 */
bool static sendMessageToServer(string message, SOCKET clientSocket) {
	uint16_t length = static_cast<uint16_t>(message.size());
	//creating packet to 
	MessageFormat mf{};
	mf.h1 = 'c';
	mf.length = length;
	mf.checksum = calculateChecksum(message);
	memcpy(mf.data, message.c_str(), length);
	if (send(clientSocket, (const char*)&mf, length + 7, 0) <= 0) {
		currentState = State::TERMINATE;
		return false;
	}
	return true;
}

/**
 * Receives a message from the server
 * @param s The string to store the received message
 * @param client_socket The socket to receive the message from
 * @return true if the message was received successfully, false otherwise
 */
bool static receiveMessageFromServer(string& s, SOCKET client_socket) {
	MessageFormat mf{};

	//receiving first byte which is header that indicates the sender
	if (recv(client_socket, (char*)&mf.h1, 1, 0) <= 0) {
		currentState = State::TERMINATE;
		return false;
	}
	if (mf.h1 != 's') {
		return false;
	}
	//receiving the next two bytes which is length header
	if(recv(client_socket, (char*)&mf.length, 2, 0) <= 0)
	{
		currentState = State::TERMINATE;
		return false;
	}

	//receiving checksum header
	if (recv(client_socket, (char*)&mf.checksum, 4, 0) <= 0)
	{
		currentState = State::TERMINATE;
		return false;

	}	
	
	if (recv(client_socket, (char*)&mf.data, mf.length, 0) <= 0) {
		currentState = State::TERMINATE;
		return false;
	}

	//checking checksum
	if(mf.checksum == calculateChecksum(mf.data))
	{ 
		s = string(mf.data, mf.length); 
	}
	else { 
		return false; 
	}
	
	return true;
}

/**
 * Returns the current hour and minute in the format [HH:MM]
 * @return string in the form [hour:minute]
 */
static string getHourAndMinute() {

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
 * function to print options to user
 */
static void printMenu() {
	cout << "1.Open messaging mode\n2.Choose users to send message\n3.Check for unseen messages\n4.See all online users\n5.See my message history \n6.Disconnect from server\nWhat do you want to do? " << endl;
}

/**
 * This function handles the offline mode, it allows the user to disconnect and return later
 * @param client client socket to handle
 */
static void openOfflineMode(SOCKET client) {

	cout << "Disconnected from the server. Type /return to reconnect." << endl;

	while (true) {

		//taking input from the user
		string input;
		getline(cin, input);

		//if the input is /return, we try to reconnect
		if (input == "/return") {

			//send a signal to the server to check if it is possible to reconnect
			char sendSignal = '1';
			send(client, &sendSignal, 1, 0);
			char receiveSignal;
			recv(client, &receiveSignal, sizeof(receiveSignal), 0);

			//if the server accepts the reconnection
			if (receiveSignal == '1') {
				cout << "Reconnected to the server!" << endl;
				currentState = State::MENU; //change state to MENU
				return;
			}
			else {
				cout << "Failed to reconnect. Server is full try again." << endl;
			}

		}
		//if the user does not input /return, we display an error message
		else {
			cout << "Invalid input. Type /return to reconnect." << endl;
		}
	}
}

/**
 * This function displays all active clients and allows the user to choose one
 * @param clientSocket client socket to handle
 */
static void displayActiveClients(SOCKET clientSocket) {

	int count = 0;
	bool terminate = false;
	//receiving all active clients from the server
	while (!terminate) {

		string receivedUsername;
		receiveMessageFromServer(receivedUsername, clientSocket);
		count++;

		//printing the received username
		cout << count << "." << receivedUsername.substr(0, receivedUsername.size() - 1) << endl;

		if (receivedUsername.back() == '0') {
			terminate = true;
		}
	}
	cout << endl;
	//after displaying active users, the user is sent back to the menu
	currentState = State::MENU; //change state to MENU
}
/**
 * This function handles the process of choosing destinations, it sends all available usernames to the user and checks if the input is valid
 * @param clientSocket client socket to handle
 */
static void handleChoosingDestinations(SOCKET clientSocket) {

	//first we display all the clients
	displayActiveClients(clientSocket);

	//taking the input from the user
	string destinations;
	cout << "Who do you want to message? Enter as numbers with spaces: " << endl;
	getline(cin, destinations);

	while (destinations.empty()) {
		cout << "Invalid input try again: " << endl;
		getline(cin, destinations);
	}

	sendMessageToServer(destinations, clientSocket);
	string validationAnswer;
	receiveMessageFromServer(validationAnswer, clientSocket);

	//if the input is not valid, taking input until it is valid
	while (validationAnswer == "no") {

		cout << "Invalid input try again: " << endl;
		getline(cin, destinations);

		if (!destinations.empty())
		{
			sendMessageToServer(destinations, clientSocket);
			receiveMessageFromServer(validationAnswer, clientSocket);
		}
	}
	cout << "Your message destinations are sets successfully !" << endl;
	cout << endl;
	currentState = State::MENU; //change state to MENU
}

/**
 * if the input action is not valid this method is used to take input again
 * @return valid action input
 */
static string retakeAction() {
	string action;
	cout << "Your choice is invalid. Please enter a valid action (1-2-3-4-5-6)!" << endl;
	printMenu();
	getline(cin, action);
	cout << endl;
	return action;
}

/**
 * This function displays the messages received from the server in messaging mode
 * @param client client socket to handle
 */
static void displayMessages(SOCKET client) {
	while (true) {
		string msg;
		if (!receiveMessageFromServer(msg, client))
		{
			return;
		}
		if (msg == "/*/exit/*/")
		{
			return;
		}
		//display
		cout << msg << endl;
	}
}
/**
 * This function handles the messaging mode, it allows the user to send messages to chosen destinations
 * @param clientSocket client socket to handle
 */
static void handleMessagingMode(SOCKET clientSocket) {
	cout << "Opening message mode. Type /exit to exit." << endl;
	char isDestinationEmpty;
	int bytes = recv(clientSocket, &isDestinationEmpty, 1, 0);
	if (bytes <= 0) {
		currentState = State::TERMINATE;
		return;
	}
	string msg;
	thread(displayMessages, clientSocket).detach();
	while (msg != "/exit") {

		getline(cin, msg);

		if (msg.empty()) {
			cout << "Cannot send an empty message!" << endl;
		}
		else if (isDestinationEmpty == '1' && msg != "/exit") {
			cout << "Cannot send message, you have no destinations! " << endl;
		}
		else {
			sendMessageToServer(msg, clientSocket);
		}
	}
	currentState = State::MENU; //change state to MENU
}
/**
 * this method is for receiving and displaying unseen messages in the third option
 * @param client client socket to display
 */
static void receiveAndDisplayUnseenMessages(SOCKET client) {

	bool terminate = false; //boolean that depends on if the received message is the last one

	while (!terminate)
	{
		string msg;
		receiveMessageFromServer(msg, client);
		//determine if it is the last one
		if (msg.back() == '0')
		{
			terminate = true;
		}
		//display the message
		cout << msg.substr(0, msg.size() - 1) << endl;
	}
	cout << endl;
	//after displaying unseen messages, the user is sent back to the menu
	currentState = State::MENU; //change state to MENU
}

/**
 * this method applies in the 5th option where the user receives the message history
 * @param client client to handle
 */
static void receiveAndDisplayHistory(SOCKET client) {


	bool terminate = false; //boolean that depends on if the received message is the last one
	cout << endl;
	while (!terminate)
	{
		string msg;
		receiveMessageFromServer(msg, client);
		//determine if it is the last one
		if (msg.back() == '0')
		{
			terminate = true;
		}
		//display the message
		cout << msg.substr(0, msg.size() - 1) << endl;
	}
	cout << endl;
	currentState = State::MENU; //change state to MENU  
}


/**
 * This function handles the menu mode, it displays the menu and takes the action from the user
 * @param clientSocket client socket to handle
 */
static void handleMenuMode(SOCKET clientSocket) {
	string action;
	printMenu();
	getline(cin, action);
	cout << endl;

	//if the input is invalid
	while (action != "1" && action != "2" && action != "3" && action != "4" && action != "5" && action != "6") {
		action = retakeAction();
	}

	//sending the action to the server

	if (!sendMessageToServer(action, clientSocket)) {
		currentState = State::TERMINATE; //change state to TERMINATE
		return; //if sending fails
	}
	if (action == "1") {
		currentState = State::MESSAGING_MODE; //change state to MESSAGING_MODE
	}
	else if (action == "2") {
		currentState = State::CHOOSE_DESTINATION; //change state to CHOOSE_DESTINATION
	}
	else if (action == "3") {
		currentState = State::UNSEEN_MESSAGES; //change state to UNSEEN_MESSAGES
	}
	else if (action == "4") {
		currentState = State::ACTIVE_USERS; //change state to ACTIVE_USERS
	}
	else if (action == "5") {
		currentState = State::MESSAGE_HISTORY; //change state to MESSAGE_HISTORY
	}
	else if (action == "6") {
		currentState = State::DISCONNECT; //change state to DISCONNECT
	}
}

/**
 * This function handles the registration mode, it takes the username from the user and sends it to the server
 * @param clientSocket client socket to handle
 */
static void handleRegisterMode(SOCKET clientSocket) {
	while (true)
	{
		//taking username from the user until it is valid
		char isDuplicated{};
		while (isDuplicated != '0')
		{
			cout << "Please enter your username: ";
			string username;
			getline(cin, username);
			cout << endl;
			while (username.empty()) {
				cout << "Username cannot be empty. Please enter a valid username: ";
				getline(cin, username);
				cout << endl;
			}
			if (!sendMessageToServer(username, clientSocket)) {
				return;
			}
			recv(clientSocket, &isDuplicated, 1, 0);

			if (isDuplicated == '1') {
				cout << "Username is already taken. Please try again! " << endl;
				cout << endl;
			}
		}
		cout << "Username is chosen successfully!" << endl;

		//after the username is chosen, we wait for the user to press enter to join the server
		while (true) {

			cout << "Press enter to join to server! " << endl;
			string enterKey;
			getline(cin, enterKey);

			if (enterKey.empty()) {
				//send a signal to the server that the user is ready to join
				char readySignal = '1';
				if (send(clientSocket, &readySignal, 1, 0) <= 0) {
					currentState = State::TERMINATE;
					return;
				}
				char isServerReady;
				recv(clientSocket, &isServerReady, 1, 0);
				if (isServerReady == '1') {
					cout << "You have joined the server successfully!" << endl;
					currentState = State::MENU; //change state to MENU
					return; //exit the registration mode
				}
				//if the server is not ready, the user tries until it successfully connects 
				else if (isServerReady == '0') {
					cout << "Server is full. Please try again!" << endl;
				}
			}
		}

	}

}
int main() {

	//creating client socket
	WSADATA wsa;
	int result = WSAStartup(MAKEWORD(2, 2), &wsa);
	SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);

	//server address
	sockaddr_in server_addr{};
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(8080);
	inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

	//connecting to the server
	connect(clientSocket, (struct sockaddr*)&server_addr, sizeof(server_addr));

	currentState = State::REGISTER; //set initial state to REGISTER
	while (result == 0) {

		switch (currentState)
		{
		case State::REGISTER:
			handleRegisterMode(clientSocket);
			break;

		case State::MENU:
			handleMenuMode(clientSocket);
			break;

		case State::MESSAGING_MODE:
			handleMessagingMode(clientSocket);
			break;

		case State::CHOOSE_DESTINATION:
			handleChoosingDestinations(clientSocket);
			break;

		case State::UNSEEN_MESSAGES:
			receiveAndDisplayUnseenMessages(clientSocket);
			break;

		case State::ACTIVE_USERS:
			displayActiveClients(clientSocket);
			break;

		case State::MESSAGE_HISTORY:
			receiveAndDisplayHistory(clientSocket);
			break;

		case State::DISCONNECT:
			openOfflineMode(clientSocket);
			break;

		case State::TERMINATE:
			cout << "Lost connection with the server. " << endl;
			closesocket(clientSocket);
			WSACleanup();
			return 0;
		default:
			break;
		}
	}
}
