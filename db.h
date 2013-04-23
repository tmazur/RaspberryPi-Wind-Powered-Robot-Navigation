#ifndef DB_H
#define DB_H
#include <string>
#include <mysql++.h>
#include "lnglat.h"
#include "log.cpp"
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
	// Db() {};
    Db(const char*,const char*,const char*,const char*);
    // Db(const Db&);
    bool updateLngLat(LngLat);
};

#endif