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
public:
    Db(const char*,const char*,const char*,const char*);
    static Db getInstance() {
        return Db("localhost","robot","root","krim.agh");
    }
    string getDataParam(string);
    bool updateDataParam(string,string);
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
    /**
     * ustawia status wczytania mapy
     * @param  status status wczytania mapy:
     *                    0: niewczytana
     *                    1: wczytana
     *                    2: w trakcie wczytywania
     *                    3: mapa nie istnieje
     *                    4: błąd wczytywania
     * @return        powodzenie
     */
    bool setMapStatus(int status) {
        return this->updateDataParam("mapStatus", to_string(status));
    }
    LngLat getFakeTWI();
    bool setTwiStatus(int status) {
        return this->updateDataParam("twiStatus", to_string(status));
    }
    bool setGoalCost(float cost) {
        return this->updateDataParam("goalCost", to_string(cost));
    }
    string getConfig(string);
};

#endif