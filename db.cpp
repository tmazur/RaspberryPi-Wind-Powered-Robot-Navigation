#include "db.h"

Db::Db(const char *host, const char *db, const char *user, const char *passwd) : dbHost(host), dbUser(user), dbName(db), dbPassword(passwd) {
	this->connect();
}

bool Db::connect() {
	try {
        this->conn = new Connection(false);
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