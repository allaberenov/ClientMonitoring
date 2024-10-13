#include <ctime>

#ifndef SERVERSIDE_USER_H
#define SERVERSIDE_USER_H

#endif //SERVERSIDE_USER_H


struct ClientInfo {
    std::string ip;
    std::string user;
    std::time_t lastActiveTime;  // Время последней активности
};