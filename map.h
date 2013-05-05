#ifndef MAP_H
#define MAP_H
#include <string>
#include <fstream>
#include <sstream>
#include <map>
#include <cmath>
#include "mlog.h"
#include "lnglat.h"
#include "db.h"
using namespace std;

typedef map <LngLatPos,bool> LngLatPosMap;
class Map {
private:
	string mapName;
	double latitudeStart;
	double longitudeStart;
	double latitudeStep;
	double longitudeStep;
	double latitudeEnd;
	double longitudeEnd;
	int latitudeCount;
	int longitudeCount;
	LngLatPosMap map;
	void setLatitudeCount();
	bool setLongitudeCount(int);
	bool parseMap();
	void parseMapParam(string);
	void parseMapLine(string,int);
	bool verifyMapParams();
public:
	Map(string mapName);
	string getMapName() {return mapName;};
	double getLongitudeStart() {return longitudeStart;};
	double getLongitudeEnd() {return longitudeEnd;};
	double getLatitudeStart() {return latitudeStart;};
	double getLatitudeEnd() {return latitudeEnd;};
	void debugInfo();
	bool checkPosition(int,int);
	bool checkPosition(LngLatPos);
	LngLatPos lngLatToPos(LngLat* const);
	LngLat posToLngLat(LngLatPos* const);
	// LngLatPos lngLatToLngLatPos(double, double);
	// LngLatPos lngLatToLngLatPos(LngLat);
	// int lngLatToPos(LngLat);
	// int lngLatToPos(double, double);
	// LngLat posToLngLat(int);
	// LngLat posToLngLat(LngLatPos);
};

#endif