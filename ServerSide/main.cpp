#include <iostream>
#include <winsock2.h>
#include <windows.h>
#pragma comment(lib, "ws2_32.lib")
#include <unistd.h>
#include <cstdlib>
#include <vector>
#include "user.h"
#include <map>
#define NO_ERROR 0
#include <gdiplus.h>
#include <fstream>



void saveFile(const std::string &filename, SOCKET clientSocket, size_t fileSize) {
    std::ofstream outputFile(filename, std::ios::binary);
    if (!outputFile) {
        std::cerr << "Failed to open file for writing." << std::endl;
        return;
    }

    char buffer[1024];
    size_t totalReceived = 0;
    int bytesRead;

    while (totalReceived < fileSize) {
        bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesRead <= 0) {
            std::cerr << "Connection lost or error occurred." << std::endl;
            break;
        }
        outputFile.write(buffer, bytesRead);
        totalReceived += bytesRead;
    }

    outputFile.close();
    std::cout << "File saved successfully as " << filename << std::endl;
}

void SetConnection() {

    WSADATA wsaData;
    SOCKET sock;
    sockaddr_in server;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed." << std::endl;
        return;
    }


    SOCKET sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == INVALID_SOCKET) {
        std::cerr << "Could not create socket." << std::endl;
        std::cerr << WSAGetLastError() << std::endl;
        WSACleanup();
        return;
    }
    std::cout << "Socket is created" << std::endl;
    sockaddr_in sockaddr;
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_addr.s_addr = INADDR_ANY;
    sockaddr.sin_port = htons(8080);

    std::cout << "Trying to bind" << std::endl;
    if (bind(sockfd, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) < 0) {
        std::cout << "Failed to bind to port 9999. errno: " << errno << std::endl;
        exit(EXIT_FAILURE);
    } else {
        std::cout << "Sucessfully bind" << std::endl;

    }

    std::cout << "Listening..." << std::endl;
    if (listen(sockfd, 10) < 0) {
        std::cout << "Failed to listen on socket. errno: " << errno << std::endl;
        exit(EXIT_FAILURE);
    }

    SOCKET connection = accept(sockfd, (struct sockaddr*)&sockaddr, NULL);
    if (connection != 0) {
        std::cout << "Failed to grab connection. errno: " << errno << std::endl;
        exit(EXIT_FAILURE);
    }

    sockaddr_in clientAddr;
    int addrLen = sizeof(clientAddr);
    getpeername(connection, (struct sockaddr*)&clientAddr, &addrLen);
    std::string clientIP = inet_ntoa(clientAddr.sin_addr);

    // Получаем текущее время
    std::time_t now = std::time(nullptr);

    std::cout << "Client connected: " << clientIP << std::endl;
    std::cout << "Last active time: " << std::ctime(&now) << std::endl;

    const unsigned int MAX_BUF_LENGTH = 4096;
    std::vector<char> buffer(MAX_BUF_LENGTH);
    std::string msg;

    int bytesRead = recv(connection, &buffer[0], buffer.size(), 0);
    msg.append(buffer.begin(),buffer.end());
    std::cout << "The message was: " << msg << std::endl;
    std::cout << "Message size "  << bytesRead << std::endl;

    std::string response = "Good talking to you\n";
    send(connection, response.c_str(), response.size(), 0);

    close(connection);
    close(sockfd);

}

void RecvFile(){
    WSADATA wsaData;
    SOCKET serverSocket, clientSocket;
    sockaddr_in serverAddr, clientAddr;
    int addrLen = sizeof(clientAddr);

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed." << std::endl;
        exit(1);
    }


    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Could not create socket." << std::endl;
        WSACleanup();
        exit(1);
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(8080);

    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Bind failed." << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        exit(1);
    }

    listen(serverSocket, 3);
    std::cout << "Waiting for connections..." << std::endl;

    clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &addrLen);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Failed to accept connection." << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        exit(1);
    }

    size_t fileSize;
    recv(clientSocket, reinterpret_cast<char*>(&fileSize), sizeof(fileSize), 0);
    std::cout << "Receiving file of size: " << fileSize << " bytes" << std::endl;

    // Приём и сохранение файла
    saveFile("received_screenshot.png", clientSocket, fileSize);

    closesocket(clientSocket);
    closesocket(serverSocket);
    WSACleanup();
}


class ClientDataBase{
public:
    int AddCLient() {
        return 0;
    };

    int GetScreen(){

        return 0;
    };

    int ShowAllUser(){};

    int ShowLastActivity();

private:
    std::map<std::string, ClientInfo> clientdata;


};



int main() {
    WSADATA wsaData;
    SOCKET serverSock, clientSock;
    sockaddr_in serverAddr, clientAddr;
    int clientAddrSize = sizeof(clientAddr);

    // Инициализация WinSock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed." << std::endl;
        return 1;
    }

    // Создаем серверный сокет
    serverSock = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSock == INVALID_SOCKET) {
        std::cerr << "Could not create socket." << std::endl;
        WSACleanup();
        return 1;
    }

    // Настраиваем серверный адрес
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(8080);

    // Привязываем сокет к адресу
    if (bind(serverSock, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Bind failed." << std::endl;
        closesocket(serverSock);
        WSACleanup();
        return 1;
    }

    pid_t pid = fork();

    // Запускаем прослушивание порта
    listen(serverSock, 5);
    std::cout << "Server listening on port " << 8080 << std::endl;



}