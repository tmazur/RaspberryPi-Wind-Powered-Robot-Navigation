#ifndef LOG_H
#define LOG_H
#include <string>
#include <iostream>
using namespace std;

class Log
{
    public:
        static void d(string msg) {
            Log::getInstance().debug(msg);
        }

        static void e(string msg) {
            Log::getInstance().error(msg);
        }

        static Log& getInstance() {
            static Log instance;
            return instance;
        }
        
        void error(string msg) {
            cout << "error: " << msg << endl;
        }

        Log& debug(string msg) {
            cout << "debug: " << msg << endl;
            return *this;
        }
    private:
        Log() {};
        Log(Log const&);              // Don't Implement
        void operator=(Log const&); // Don't implement
};

#endif