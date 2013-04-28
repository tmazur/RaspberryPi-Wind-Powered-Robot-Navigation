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
        string t = to_string(std::time(0));
        dlog << "updateTime: " << t;
        query << "UPDATE runData SET value=\"" << escape << value << "\" WHERE name=\"" << escape << name << "\" LIMIT 1;";
        query.execute();
        query.reset();
        query << "UPDATE runData SET value=\"" << t << "\" WHERE id=5 LIMIT 1;";
        query.execute();
    } catch (const BadConversion& er) {
        elog << "Db bad conversions error: " << er.what() << "; retrieved data size: " << er.retrieved << ", actual size: " << er.actual_size;
        return false;
    } catch (const Exception& er) {
        elog << "Db connection error: " << er.what();
        return false;
    }
    return true;
}

bool Db::updateLngLat(LngLat lnglat) {
	return this->updateDataParam("lng",to_string(lnglat.lng)) && this->updateDataParam("lat",to_string(lnglat.lat));
}