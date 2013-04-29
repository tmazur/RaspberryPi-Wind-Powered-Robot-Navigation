#ifndef DB_H
#define DB_H
#include <string>
#include <mysql++.h>
#include <ctime>
#include "lnglat.h"
#include "mlog.h"
using namespace std;
using namespace mysqlpp;

class Db {
private:
    const char *dbName;
    const char *dbUser;
    const char *dbPassword;
    const char *dbHost;
    Connection* conn;
    time_t lastUpdateTime;
    bool connect();
    bool updateDataParam(string,string);
    string getDataParam(string);
public:
    Db(const char*,const char*,const char*,const char*);
    bool updateLngLat(LngLat);
    LngLat getLngLatGoal();
};

#endif