#pragma once
#include <iostream>
#include <sstream>
#include <string>

class MLog
{
public:
    MLog(const std::string &funcName, std::string type) {
        msg << type << ":" << funcName << ": ";
    }

    template <class T>
    MLog &operator<<(const T &v) {
        msg << v;
        return *this;
    }

    ~MLog() {
        std::cout << this->msg.str() << std::endl;
    }
private:
    std::stringstream msg;
};

#define elog MLog(__FUNCTION__, "error")
#define dlog MLog(__FUNCTION__, "debug")