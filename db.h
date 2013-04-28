#ifndef DB_H
#define DB_H
#include <string>
#include <mysql++.h>
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
    bool connect();
    bool updateDataParam(string,string);
public:
    Db(const char*,const char*,const char*,const char*);
    bool updateLngLat(LngLat);
};

#endif