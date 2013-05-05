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
    static Db getInstance() {
        return Db("localhost","robot","root","krim.agh");
    }
    bool updateLngLat(LngLat);
    LngLat getLngLatGoal();
    string getMapName() {
        return this->getDataParam("map");
    };
    bool savePath(vPath, Map*);
    /**
     * ustawia status ścieżki
     * @param  status status ścieżki:
     *                    0: szukam
     *                    1: wyznaczona
     *                    2: nie udało się wyznaczyć ścieżki
     * @return        powodzenie
     */
    bool setPathStatus(int status) {
        return this->updateDataParam("pathStatus", to_string(status));
    }
};

#endif