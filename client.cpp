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
	//taking done signal from the server
	int bytes;

	//client receives a start signal to open offlline mode
	char start;
	bytes = recv(client, &start, sizeof(start), 0);
	if(bytes <= 0) {
		currentState = State::TERMINATE;
		return;
	}
	cout << "Disconnected. Type /return to return." << endl;
	string back;
	while (back != "/return") {
		getline(cin, back);
		if (back != "/return") {
			cout << "Invalid comment." << endl;
		}
	}

	//sending back signal to the server
	char endOfflineMode = '1';
	bytes = send(client, &endOfflineMode, 1, 0);
	if (bytes <= 0) {
		currentState = State::TERMINATE;
		return;
	}

	//receiving the finish signal from the server
	char finish;
	bytes = recv(client, &finish, sizeof(finish), 0);
	if (bytes <= 0) {
		currentState = State::TERMINATE;
		return;
	}
	//if the server is full, the client cannot reconnect
	if (finish == '0') {
		cout << "Cannot reconnect, the server is full! " << endl;
		currentState = State::TERMINATE;
		return;
	}
	cout << "\nReconnecting to the server!" << endl;

	currentState = State::MENU; 
}
/**
 * This function displays all active clients and allows the user to choose one
 * @param clientSocket client socket to handle
 */
static void displayActiveClients(SOCKET clientSocket) {

	char buffer[1024];
	int count = 0;
	bool terminate = false;
	//receiving all active clients from the server
	while (!terminate) {
		memset(buffer, 0, sizeof(buffer));
		recv(clientSocket, buffer, sizeof(buffer), 0);
		string receivedUsername = buffer;
		count++;

		//printing the received username
		cout << count << "." << receivedUsername.substr(0, receivedUsername.size() - 1) << endl;

		//feedback to the server that we received the username
		char read = '1';
		send(clientSocket, &read, 1, 0);
		//checking if it is the last username
		if (receivedUsername.back() == '0') {
			terminate = true;
		}

	}
	//sending end signal to the server
	char done;
	recv(clientSocket, &done, sizeof(done), 0);
	cout << endl;

	//after displaying active users, the user is sent back to the menu
	currentState = State::MENU; //change state to MENU
}
/**
 * This function handles the process of choosing destinations, it sends all available usernames to the user and checks if the input is valid
 * @param clientSocket client socket to handle
 */
static void handleChoosingDestinations(SOCKET clientSocket) {

	char buffer[1024];
	displayActiveClients(clientSocket);
	//taking the input from the user
	string destinations;
	cout << "Who do you want to message? Enter as numbers with spaces: " << endl;
	getline(cin, destinations);
	int bytes = send(clientSocket, destinations.c_str(), destinations.length(), 0);
	if (bytes <= 0) {
		currentState = State::TERMINATE;
		return;
	}

	//receiving answer for validation
	memset(buffer, 0, sizeof(buffer));
	recv(clientSocket, buffer, sizeof(buffer), 0);
	string validationAnswer = buffer;

	//if the input is not valid, taking input until it is valid
	while (validationAnswer == "no") {
		cout << "Invalid input try again: " << endl;
		getline(cin, destinations);
		int bytes = send(clientSocket, destinations.c_str(), destinations.length(), 0);
		if (bytes <= 0) {
			currentState = State::TERMINATE;
			return;

		}
		memset(buffer, 0, sizeof(buffer));
		recv(clientSocket, buffer, sizeof(buffer), 0);
		validationAnswer = buffer;
	}
	cout << "Your message destinations are sets successfully !" << endl;
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
 * method to display coming messages in the message mode
 * @param client client to display
 */
static void displayMessages(SOCKET client) {
	char buffer[1024];
	while (true) {
		//receiving messages
		memset(buffer, 0, sizeof(buffer));
		int bytes = recv(client, buffer, sizeof(buffer), 0);
		if (bytes <= 0) {
			break;
		}
		string msg = buffer;
		if (msg == "/*/exit/*/")
			return;
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
	recv(clientSocket, &isDestinationEmpty, sizeof(isDestinationEmpty), 0);
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
			int bytes = send(clientSocket, msg.c_str(), msg.length(), 0);
			if (bytes <= 0) {
				currentState = State::TERMINATE;
				return;
			}
		}
	}
	currentState = State::MENU; //change state to MENU
}
/**
 * this method is for receiving and displaying unseen messages in the third option
 * @param client client socket to display
 */
static void receiveAndDisplayUnseenMessages(SOCKET client) {

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
	cout << endl;
	//after displaying unseen messages, the user is sent back to the menu
	currentState = State::MENU; //change state to MENU
}

/**
 * this method applies in the 5th option where the user receives the message history
 * @param client client to handle
 */
static void receiveAndDisplayHistory(SOCKET client) {

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
	int bytes = send(clientSocket, action.c_str(), action.length(), 0);
	if (bytes <= 0) {
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
	int bytes;
	cout << endl;
	cout << "Welcome to server." << endl;

	//getting the username from the user
	string name;
	cout << "Enter username: ";
	getline(cin, name);

	//sending the name we choose and sending it to the server
	bytes = send(clientSocket, name.c_str(), name.length(), 0);
	if( bytes <= 0) {
		currentState = State::TERMINATE; //change state to TERMINATE
		return;
	}
	//receiving answer that shows if duplicated
	char isDuplicated;
	bytes = recv(clientSocket, &isDuplicated, sizeof(isDuplicated), 0);
	if (bytes <= 0) {
		currentState = State::TERMINATE; //change state to TERMINATE
		return;
	}

	//if duplicated taking username input again
	while (isDuplicated == '1') {
		cout << endl;
		cout << "This username already exists. Try again!" << endl;
		cout << "Enter username: ";
		getline(cin, name);

		//sending the name we choose and sending it to the server
		int bytes = send(clientSocket, name.c_str(), name.length(), 0);
		if (bytes <= 0) {
			currentState = State::TERMINATE; //change state to TERMINATE
			return;
		}
		//taking duplicated answer again
		bytes = recv(clientSocket, &isDuplicated, sizeof(isDuplicated), 0);
		if (bytes <= 0) {
			currentState = State::TERMINATE; //change state to TERMINATE
			return;
		}
	}

	//then checking if it is available to connect
	char readyToStart;
	bytes = recv(clientSocket, &readyToStart, sizeof(readyToStart), 0);
	if (bytes <= 0) {
		currentState = State::TERMINATE; //change state to TERMINATE
		return;
	}

	//if server is full
	if (readyToStart == '0') {
		cout << "Cannot connect to the server, it is full. Try another time!" << endl;
		currentState = State::TERMINATE; //change state to TERMINATE
		return;
	}

	//registration is successful
	cout << endl;
	cout << "You are registered successfully!" << endl;
	cout << "Connected to server!" << endl;
	cout << endl;
	currentState = State::MENU; //change state to MENU
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
	while (true && result == 0) {

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

		case State::MESSAGE_HISTORY:{
			receiveAndDisplayHistory(clientSocket);
			break;
		}

		case State::DISCONNECT: 
			openOfflineMode(clientSocket);
			break;
			
		case State::TERMINATE:
			closesocket(clientSocket);
			WSACleanup();
			return 0;
		default:
			break;
		}
	}
}
