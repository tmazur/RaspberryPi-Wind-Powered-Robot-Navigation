#include "db.h"

Db::Db(const char *host, const char *db, const char *user, const char *passwd) : dbHost(host), dbUser(user), dbName(db), dbPassword(passwd) {
	this->connect();
}

// Db::Db(const Db& a) {
// 	this->dbHost = a.dbHost;
// 	this->dbName = a.dbName;
// 	this->dbUser = a.dbUser;
// 	this->dbPassword = a.dbPassword;
// 	this->connect();
// }

bool Db::connect() {
	try {
        this->conn = new Connection(false);
        this->conn->connect(dbName, dbHost, dbUser, dbPassword);
        Query query = this->conn->query();
    } catch (const BadConversion& er) {
        // Handle bad conversions
        Log::d("Db bad conversions error: "+ *er.what());// + "; retrieved data size: " + er.retrieved +", actual size: " + er.actual_size);
        return false;
    } catch (const Exception& er) {
        // Catch-all for any other MySQL++ exceptions
        Log::d("Db connection error: "+ *er.what());
        return false;
    }
    return true;
}

bool Db::updateDataParam(string name, string value) {
	try {
        Query query = this->conn->query();
		query << "UPDATE runData SET value=\"" << escape << value << "\" WHERE name=\"" << escape << name << "\" LIMIT 1";
		query.execute();
    } catch (const BadConversion& er) {
        Log::d("Db bad conversions error: "+ *er.what());// + "; retrieved data size: " + er.retrieved +", actual size: " + er.actual_size);
        return false;
    } catch (const Exception& er) {
        Log::d("Db connection error: "+ *er.what());
        return false;
    }
    return true;
}

bool Db::updateLngLat(LngLat lnglat) {
	return this->updateDataParam("lng",to_string(lnglat.lng)) && this->updateDataParam("lat",to_string(lnglat.lat));
}