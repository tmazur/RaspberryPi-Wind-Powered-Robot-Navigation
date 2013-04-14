#include "map.h"

Map::Map(string mapName)  : latitudeStart(0), latitudeEnd(0), latitudeStep(0), latitudeCount(0), longitudeStart(0), longitudeEnd(0), longitudeStep(0), longitudeCount(0) {
	this->mapName = mapName;
	this->parseMap();
	this->debugInfo();
}

bool Map::setLongitudeCount(int count) {
	if(this->longitudeCount==0 || this->longitudeCount==count) {
		this->longitudeCount=count;
		return true;
	} else {
		Log::getInstance().error("Map::setLongitudeCount already set to: "+to_string(this->longitudeCount)+", trying to set to: "+to_string(count));
		return false;
	}
}

void Map::setLatitudeCount() {
	this->latitudeCount=(this->map.size()/this->longitudeCount);
}

void Map::parseMap() {
	ifstream mapFile;
	string line;
	bool mapStart = false;
	int i = 0;
	mapFile.open(this->mapName);
	if(mapFile.is_open()) {
		while(!mapFile.eof()) {
			getline(mapFile,line);
			if (line[line.size() - 1] == '\r')
			line.resize(line.size() - 1);
			if(mapStart) {
				this->parseMapLine(line,i);
				i++;
			} else {
				if(line=="StartMap") {
					mapStart=true;
				} else {
					this->parseMapParam(line);
				}
			}
		}
	} else {
		;
	}
	mapFile.close();
	this->verifyMapParams();
}

void Map::parseMapParam(string line) {
	if(line.substr(0,1) == "#") { //comment
		// cout << "comment: " << line <<endl;
	} else { //parse parameter
		string param;
		string value;
    	stringstream stream(line);
    	getline(stream, param, '=');
		getline(stream, value, '=');
		if (param=="LatitudeStart") {
			this->latitudeStart=atof(value.c_str());
		} else if (param=="LongitudeStart") {
			this->longitudeStart=atof(value.c_str());
		} else if (param=="LatitudeEnd") {
			this->latitudeEnd=atof(value.c_str());
		} else if (param=="LatitudeStep") {
			this->latitudeStep=atof(value.c_str());
		} else if (param=="LongitudeEnd") {
			this->longitudeEnd=atof(value.c_str());
		} else if (param=="LongitudeStep") {
			this->longitudeStep=atof(value.c_str());
		} else {
			cout << "unknown parameter: '" << param << "'" << endl;
		}
		// cout << "found param " << param << ": " << value << endl;
	}
}

bool Map::verifyMapParams() {
	bool noError=true;
	if(this->latitudeStart==0) {
		noError=false;
		Log::getInstance().error("latitudeStart not specified!");
	}
	if(this->longitudeStart==0) {
		noError=false;
		Log::getInstance().error("longitudeStart not specified!");
	}
	if(this->latitudeEnd==0) {
		if(this->latitudeStep==0) {
			noError=false;
			Log::getInstance().error("latitudeEnd and latitudeStep not specified!");
		} else {
			this->latitudeEnd=(this->latitudeStart+this->latitudeStep*(this->latitudeCount-1));
		}
	} else {
		if(this->latitudeStep==0) {
			this->latitudeStep=(latitudeEnd - latitudeStart)/(latitudeCount-1);
		} else {
			double temp = (latitudeEnd - latitudeStart)/latitudeCount;
			if(abs(this->latitudeStep-temp)>0.00005) {
				Log::getInstance().error("latitudeStep set to: "+to_string(this->latitudeStep)+", calculated to: "+to_string(temp));
				noError=false;
			}
		}
	}
	if(this->longitudeEnd==0) {
		if(this->longitudeStep==0) {
			noError=false;
			Log::getInstance().error("longitudeEnd and longitudeStep not specified!");
		} else {
			this->longitudeEnd=(this->longitudeStart+this->longitudeStep*(this->longitudeCount-1));
		}
	} else {
		if (this->longitudeStep==0) {
			this->longitudeStep=(longitudeEnd - longitudeStart)/(longitudeCount-1);
		} else {
			double temp = (longitudeEnd - longitudeStart)/longitudeCount;
			if(abs(this->longitudeStep-temp)>0.00005) {
				Log::getInstance().error("longitudeStep set to: "+to_string(this->longitudeStep)+", calculated to: "+to_string(temp));
				noError=false;
			}
		}
	}
	if(!noError) {
		Log::getInstance().error("Map::verifyMapParams; contains error!");
	}
	return noError;
}

void Map::parseMapLine(string line, int lineNum) {
	stringstream stream(line);
	string val;
	int length = (line.length()+1)/2;
	this->setLongitudeCount(length);
	int i=0;
	while(stream>>val) {
		this->map[LngLatPos(i,lineNum)] = stoi(val);
		i++;
	}
	this->setLatitudeCount();
}

void Map::debugInfo() {
	Log::getInstance().debug("=========Map::debugInfo===============")
	.debug("mapName: " + mapName)
	.debug("mapSize: " + to_string(map.size()))
	.debug("latitudeStart: " + to_string(latitudeStart))
	.debug("latitudeEnd: " + to_string(latitudeEnd))
	.debug("latitudeStep: " + to_string(latitudeStep))
	.debug("latitudeCount: " + to_string(latitudeCount))
	.debug("longitudeStart: " + to_string(longitudeStart))
	.debug("longitudeEnd: " + to_string(longitudeEnd))
	.debug("longitudeStep: " + to_string(longitudeStep))
	.debug("longitudeCount: " + to_string(longitudeCount))
	.debug("=========Map::debugInfo=====END=======");
}

bool Map::checkPosition(int lng, int lat) {
	return this->checkPosition(LngLatPos(lng,lat));
}

bool Map::checkPosition(LngLatPos pos) {
	if(pos.lngPos > this->longitudeCount || pos.latPos > this->latitudeCount || pos.lngPos < 0 || pos.latPos < 0) {
		return true; //if position out of range, obstacle=true
	}

	return this->map[pos];
}

LngLatPos Map::lngLatToPos(LngLat* lngLat) {
	int longitudePos = round((lngLat->lng-this->longitudeStart)/this->longitudeStep);
	int latitudePos = round((lngLat->lat-this->latitudeStart)/this->latitudeStep);
	LngLatPos temp = LngLatPos(longitudePos,latitudePos);
	return temp;
}

/**
 * Converts LngLatPos to int (1dim position)
 * @param  lpos LngLatPos to convert
 * @return      returns 1dim position as int
 */
// int Map::lngLatPosToPos(LngLatPos lpos) {
// 	return lpos.lng+lpos.lat*this->longitudeCount;
// }

// int Map::lngLatToPos(LngLat lngLat) {
// 	LngLatPos lpos = this->lngLatToLngLatPos(lngLat);
// 	int pos = this->lngLatPosToPos(lpos);

// 	// Log::getInstance().debug("pos for "+to_string(longitude)+", "+to_string(latitude)+": "+to_string(pos));
// 	if(lpos.lngPos > (this->longitudeCount) || lpos.latPos > this->latitudeCount || lpos.lngPos < 0 || lpos.latPos < 0) {
// 		Log::getInstance().debug("pos for "+lngLat.toString()+": "+to_string(pos)+" is out of range!");
// 		return -1;
// 	}
// 	return pos;
// }

// LngLat Map::posToLngLat(int pos) {
// 	int longitudePos = pos%(longitudeCount);
// 	int latitudePos = floor(pos/this->latitudeCount);
// 	return this->posToLngLat(LngLatPos (longitudePos,latitudePos,this*));
// }

LngLat Map::posToLngLat(LngLatPos* pos) {
	double longitude = this->longitudeStart+this->longitudeStep*pos->lngPos;
	double latitude = this->latitudeStart+this->latitudeStep*pos->latPos;
	return LngLat (longitude,latitude);
}