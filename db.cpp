#include "db.h"

Db::Db(const char *host, const char *db, const char *user, const char *passwd) : dbHost(host), dbUser(user), dbName(db), dbPassword(passwd) {
	this->connect();
}

bool Db::connect() {
	try {
        this->conn = new Connection();
        this->conn->connect(dbName, dbHost, dbUser, dbPassword);
        Query query = this->conn->query();
    } catch (const BadConversion& er) {
        elog << "Db bad conversions error: " << er.what() << "; retrieved data size: " << er.retrieved << ", actual size: " << er.actual_size;
        return false;
    } catch (const Exception& er) {
        elog << "Db connection error: " << er.what();
        return false;
    }
    return true;
}

bool Db::updateDataParam(string name, string value) {
    try {
        Query query = this->conn->query();
        query << "UPDATE runData SET value=\"" << escape << value << "\" WHERE name=\"" << escape << name << "\" LIMIT 1;";
        query.execute();
        time_t t = std::time(0);
        if(difftime(t, this->lastUpdateTime) > 5) { //if lastUpdateTime outdated
            // dlog << "updateTime: " << t;
            this->lastUpdateTime = t;
            query.reset();
            query << "UPDATE runData SET value=\"" << t << "\" WHERE id=5 LIMIT 1;";
            query.execute();
        }
    } catch (const BadConversion& er) {
        elog << "Db bad conversions error: " << er.what() << "; retrieved data size: " << er.retrieved << ", actual size: " << er.actual_size;
        return false;
    } catch (const Exception& er) {
        elog << "Db connection error: " << er.what();
        return false;
    }
    return true;
}

string Db::getDataParam(string name) {
    try {
        Query query = this->conn->query();
        query << "SELECT value FROM runData WHERE name=\"" << escape << name << "\" LIMIT 1;";
        StoreQueryResult ares = query.store();

        if(ares.num_rows()==1) {
            string ret = "";
            ares[0]["value"].to_string(ret);
            return ret;
        } else {
            elog << "Pusty wynik zapytania: " << query;
        }
        return "";
    } catch (const BadConversion& er) {
        elog << "Db bad conversions error: " << er.what() << "; retrieved data size: " << er.retrieved << ", actual size: " << er.actual_size;
        return "";
    } catch (const Exception& er) {
        elog << "Db connection error: " << er.what();
        return "";
    }
    return "";
}

bool Db::updateLngLat(LngLat lnglat) {
    return this->updateDataParam("lng",to_string(lnglat.lng)) && this->updateDataParam("lat",to_string(lnglat.lat));
}

LngLat Db::getLngLatGoal() {
    string sLng = this->getDataParam("lng_goal");
    string sLat = this->getDataParam("lat_goal");
    // dlog << "goal: " << sLng << ", " << sLat;
    if(sLng=="" || sLat=="") {
        return LngLat(0.,0.);
    } else {
        // return LngLat(0.,0.);
        return LngLat(atof(sLng.c_str()), atof(sLat.c_str()));
    }
}

bool Db::savePath(vPath path, Map* map) {
    dlog << "Zapisuję ścieżkę do bazy danych...";
    try {
        Query query = this->conn->query();
        query << "TRUNCATE path;";
        query.execute();
        
        for(vPath::iterator i = path.begin(); i != path.end(); i++) {
            LngLat tmp = i->toLngLat(map);
            query.reset();
            query << "INSERT INTO path (lng,lat) VALUES(" << escape << tmp.lng << ", " << escape << tmp.lat << ");";
            query.execute();
            
        }
        dlog << "Zapisywanie ścieżki zakonczone.";
    } catch (const BadConversion& er) {
        elog << "Db bad conversions error: " << er.what() << "; retrieved data size: " << er.retrieved << ", actual size: " << er.actual_size;
        return false;
    } catch (const Exception& er) {
        elog << "Db connection error: " << er.what();
        return false;
    }
    return true;
}

LngLat Db::getFakeTWI() {
    try {
        string sLng, sLat;
        Query query = this->conn->query();
        query << "SELECT value FROM fakeTWI WHERE name=\"lng\" LIMIT 1;";
        StoreQueryResult ares = query.store();

        if(ares.num_rows()==1) {
            ares[0]["value"].to_string(sLng);
        } else {
            elog << "Pusty wynik zapytania: " << query;
        }

        query.reset();
        query << "SELECT value FROM fakeTWI WHERE name=\"lat\" LIMIT 1;";
        ares = query.store();

        if(ares.num_rows()==1) {
            ares[0]["value"].to_string(sLat);
        } else {
            elog << "Pusty wynik zapytania: " << query;
        }

        return LngLat(atof(sLng.c_str()), atof(sLat.c_str()));
    } catch (const BadConversion& er) {
        elog << "Db bad conversions error: " << er.what() << "; retrieved data size: " << er.retrieved << ", actual size: " << er.actual_size;
        return LngLat(0.,0.);
    } catch (const Exception& er) {
        elog << "Db connection error: " << er.what();
        return LngLat(0.,0.);
    }
    return LngLat(0.,0.);
}