#include <iostream>
#include <winsock2.h>
#include <windows.h>
#include <fstream>
#include <thread>
#include <chrono>
#pragma comment(lib, "ws2_32.lib")

const int PING_INTERVAL_MINUTES = 1;

bool SaveBitmapToFile(HBITMAP hBitmap, const std::string& filename) {
    BITMAP bmp;
    BITMAPFILEHEADER fileHeader;
    BITMAPINFOHEADER infoHeader;

    HDC hDC = CreateCompatibleDC(0);
    SelectObject(hDC, hBitmap);

    GetObject(hBitmap, sizeof(BITMAP), &bmp);

    fileHeader.bfType = 0x4D42;  // "BM"
    fileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    fileHeader.bfSize = fileHeader.bfOffBits + bmp.bmWidthBytes * bmp.bmHeight;
    fileHeader.bfReserved1 = 0;
    fileHeader.bfReserved2 = 0;

    infoHeader.biSize = sizeof(BITMAPINFOHEADER);
    infoHeader.biWidth = bmp.bmWidth;
    infoHeader.biHeight = bmp.bmHeight;
    infoHeader.biPlanes = 1;
    infoHeader.biBitCount = 32;
    infoHeader.biCompression = BI_RGB;
    infoHeader.biSizeImage = 0;
    infoHeader.biXPelsPerMeter = 0;
    infoHeader.biYPelsPerMeter = 0;
    infoHeader.biClrUsed = 0;
    infoHeader.biClrImportant = 0;

    FILE* file = fopen(filename.c_str(), "wb");
    if (!file) {
        DeleteDC(hDC);
        return false;
    }
    fwrite(&fileHeader, sizeof(BITMAPFILEHEADER), 1, file);
    fwrite(&infoHeader, sizeof(BITMAPINFOHEADER), 1, file);

    BYTE* pixels = new BYTE[bmp.bmWidthBytes * bmp.bmHeight];
    GetDIBits(hDC, hBitmap, 0, bmp.bmHeight, pixels, (BITMAPINFO*)&infoHeader, DIB_RGB_COLORS);

    fwrite(pixels, bmp.bmWidthBytes * bmp.bmHeight, 1, file);

    delete[] pixels;
    fclose(file);
    DeleteDC(hDC);

    return true;
}

void TakeScreenshot(const std::string& filename) {
    int screenX = GetSystemMetrics(SM_CXSCREEN);
    int screenY = GetSystemMetrics(SM_CYSCREEN);

    HDC hScreenDC = GetDC(NULL);
    HDC hMemoryDC = CreateCompatibleDC(hScreenDC);

    HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, screenX, screenY);
    SelectObject(hMemoryDC, hBitmap);

    BitBlt(hMemoryDC, 0, 0, screenX, screenY, hScreenDC, 0, 0, SRCCOPY);

    if (SaveBitmapToFile(hBitmap, filename)) {
        std::cout << "Screenshot saved " << filename << std::endl;
    } else {
        std::cerr << "Screenshot wasn't saved!" << std::endl;
    }

    DeleteObject(hBitmap);
    DeleteDC(hMemoryDC);
    ReleaseDC(NULL, hScreenDC);
}

void sendFileToServer(const std::string &filename) {
    WSADATA wsaData;
    SOCKET sock;
    sockaddr_in server;


    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed." << std::endl;
        return;
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Could not create socket." << std::endl;
        WSACleanup();
        return;
    }

    // Настройка сервера
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons(8080);

    std::cout << "Connecting to server..." << std::endl;
    if (connect(sock, (struct sockaddr *) &server, sizeof(server)) < 0) {
        std::cerr << "Connection failed: " << WSAGetLastError() << std::endl;
        closesocket(sock);
        WSACleanup();
        return;
    }

    std::cout << "Connected to server." << std::endl;

    // Открытие файла для чтения
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file) {
        std::cerr << "Could not open file." << std::endl;
        closesocket(sock);
        WSACleanup();
        return;
    }

    // Получение размера файла
    std::streamsize fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    // Отправка размера файла
    send(sock, reinterpret_cast<const char *>(&fileSize), sizeof(fileSize), 0);

    // Чтение и отправка файла по частям
    const int bufferSize = 1024;
    char buffer[bufferSize];
    while (file.read(buffer, bufferSize)) {
        send(sock, buffer, bufferSize, 0);
    }
    // Отправляем оставшиеся данные, если размер не кратен 1024
    send(sock, buffer, file.gcount(), 0);

    std::cout << "File sent: " << filename << std::endl;

    // Закрытие файла и сокета
    file.close();
    closesocket(sock);
    WSACleanup();
}

class Client {
    std::string username;
    std::string password;

    int CreateAccount() {
        UserInit();


    }

    int UserInit() {
        while (username.empty() && password.empty()) {
            std::cout << "Username ";
            std::cin >> username;
            std::cout << "\nPassword ";
            std::cin >> password;
        }
        return 0;
    }
};


void sendDataToServer(const std::string &data) {
    WSADATA wsaData;
    SOCKET sock;
    sockaddr_in server;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed." << std::endl;
        return;
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Could not create socket." << std::endl;
        WSACleanup();
        return;
    } else {
        std::cout << "Socket sucessfully created" << std::endl;
    }


    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons(8080);  // Порт сервера

    std::cout << "Try to connect" << std::endl;
    if (connect(sock, (struct sockaddr *) &server, sizeof(server)) < 0) {
        std::cerr << "Connection failed." << std::endl;
        std::cerr << WSAGetLastError() << std::endl;
        closesocket(sock);
        WSACleanup();
        return;
    } else {
        std::cout << "Sucessfully connect" << std::endl;
    }


    send(sock, data.c_str(), data.size(), 0);
    std::cout << "Data sent: " << data << std::endl;

    closesocket(sock);
    WSACleanup();
}

void sendPing(SOCKET sock) {
    auto lastPingTime = std::chrono::steady_clock::now();

    while (true) {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::minutes>(now - lastPingTime);

        // Если прошел нужный интервал, отправляем пинг
        if (elapsed.count() >= PING_INTERVAL_MINUTES) {
            std::string pingMessage = "PING";
            if (send(sock, pingMessage.c_str(), pingMessage.size(), 0) == SOCKET_ERROR) {
                std::cerr << "Ping failed. Connection might be lost." << std::endl;
                break;
            }
            std::cout << "Ping sent." << std::endl;
            lastPingTime = now;  // Обновляем время последнего пинга
        }

        // Немного ждем, чтобы не нагружать процессор
        Sleep(100);  // 100 миллисекунд
    }
}


int main() {
    std::string userName = "User";  // Пример данных
    //sendDataToServer(userName);
    // Инициализация сокетов (пример подключения к серверу)
    WSADATA wsaData;
    SOCKET sock;
    sockaddr_in server;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed." << std::endl;
        return 1;
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Could not create socket." << std::endl;
        WSACleanup();
        return 1;
    }

    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons(8080);

    if (connect(sock, (struct sockaddr *) &server, sizeof(server)) < 0) {
        std::cerr << "Connection failed." << std::endl;
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    std::cout << "Connected to server." << std::endl;

    // Запускаем функцию пинга в основном цикле
    sendPing(sock);

    // Закрытие сокета и очистка ресурсов
    closesocket(sock);
    WSACleanup();
    std::cout << "All done!\n";
}