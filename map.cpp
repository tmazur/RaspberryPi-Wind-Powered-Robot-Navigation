#include "map.h"

Map::Map(string mapName)  : latitudeStart(0), latitudeEnd(0), latitudeStep(0), latitudeCount(0),
 longitudeStart(0), longitudeEnd(0), longitudeStep(0), longitudeCount(0), mapName(mapName), invertLng(1), invertLat(1) {
	if(mapName=="") {
		Db::getInstance().setMapStatus(3); // mapa nie istnieje
		elog << "Nie podano pliku z mapą!";
	} else {
		this->mapName = mapName;
		Db db = Db::getInstance();
		db.setMapStatus(2); // wczytywanie mapy
		if(this->parseMap()) {
			db.setMapStatus(1); // mapa wczytana
			dlog << "Mapa wczytana poprawnie.";
		}
	}
}

bool Map::setLongitudeCount(int count) {
	if(this->longitudeCount==0 || this->longitudeCount==count) {
		this->longitudeCount=count;
		return true;
	} else {
		elog << "LongitudeCount już ustawiono na: " << this->longitudeCount << ", próba ustawienia na: " << count;
		return false;
	}
}

void Map::setLatitudeCount() {
	this->latitudeCount=(this->map.size()/this->longitudeCount);
}

bool Map::parseMap() {
	ifstream mapFile;
	string line;
	bool mapStart = false;
	int i = 0;
	mapFile.open(MAP_DIR + this->mapName);
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
		elog << "Brak pliku: " << mapName;
		Db::getInstance().setMapStatus(3); // mapa nie istnieje
		return false;
	}
	mapFile.close();
	if(!this->verifyMapParams()) {
		Db::getInstance().setMapStatus(4); // błąd wczytywania mapy
		return false;
	} else {
		if(this->longitudeStep<0)
			this->invertLng = -1;
		if(this->latitudeStep<0)
			this->invertLat = -1;
		return true;
	}
}

void Map::parseMapParam(string line) {
	if(line.substr(0,1) != "#") { //not a comment
		//dlog << "found param " << param << ": " << value;
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
			elog << "Nieznany parametr: '" << param << "'";
		}
	}
}

bool Map::verifyMapParams() {
	bool noError=true;
	if(this->latitudeStart==0) {
		noError=false;
		elog << "latitudeStart nieokreślony!";
	}
	if(this->longitudeStart==0) {
		noError=false;
		elog << "longitudeStart nieokreślony!";
	}
	if(this->latitudeEnd==0) {
		if(this->latitudeStep==0) {
			noError=false;
			elog << "latitudeEnd i latitudeStep nieokreślony!";
		} else {
			this->latitudeEnd=(this->latitudeStart+this->latitudeStep*(this->latitudeCount-1));
		}
	} else {
		if(this->latitudeStep==0) {
			this->latitudeStep=(latitudeEnd - latitudeStart)/(latitudeCount-1);
		} else {
			double temp = (latitudeEnd - latitudeStart)/latitudeCount;
			if(abs(this->latitudeStep-temp)>FLOAT_ACCURACY) {
				elog << "latitudeStep ustawiony na: " << this->latitudeStep << ", obliczono: " << temp;
				noError=false;
			}
		}
	}
	if(this->longitudeEnd==0) {
		if(this->longitudeStep==0) {
			noError=false;
			elog << "longitudeEnd i longitudeStep nieokreślony!";
		} else {
			this->longitudeEnd=(this->longitudeStart+this->longitudeStep*(this->longitudeCount-1));
		}
	} else {
		if (this->longitudeStep==0) {
			this->longitudeStep=(longitudeEnd - longitudeStart)/(longitudeCount-1);
		} else {
			double temp = (longitudeEnd - longitudeStart)/longitudeCount;
			if(abs(this->longitudeStep-temp)>FLOAT_ACCURACY) {
				elog << "longitudeStep ustawiony na: " << this->longitudeStep << ", obliczono: " << temp;
				noError=false;
			}
		}
	}
	if(!noError) {
		elog << "Mapa zawiera nieprawidłowe parametry!";
	}
	return noError;
}

void Map::parseMapLine(string line, int lineNum) {
	stringstream stream(line);
	string val;
	int length = (line.length()+1)/2;
	if(!this->setLongitudeCount(length)) { //longitudeCount inconsistent
		elog << "parseMapLine #" << lineNum;
	}
	int i=0;
	while(stream>>val) {
		this->map[LngLatPos(i,lineNum)] = stoi(val);
		i++;
	}
	this->setLatitudeCount();
}

void Map::debugInfo() {
	dlog << "=========Map::debugInfo===============" << "\n"
	<< "mapName: " << mapName << "\n"
	<< "mapSize: " << map.size() << "\n"
	<< "latitudeStart: " << latitudeStart << "\n"
	<< "latitudeEnd: " << latitudeEnd << "\n"
	<< "latitudeStep: " << latitudeStep << "\n"
	<< "latitudeCount: " << latitudeCount << "\n"
	<< "latitudeInvert: " << invertLat << "\n"
	<< "longitudeStart: " << longitudeStart << "\n"
	<< "longitudeEnd: " << longitudeEnd << "\n"
	<< "longitudeStep: " << longitudeStep << "\n"
	<< "longitudeCount: " << longitudeCount << "\n"
	<< "longitudeInvert: " << invertLng << "\n"
	<< "=========Map::debugInfo=====END=======";
}

bool Map::checkPosition(int lng, int lat) {
	return this->checkPosition(LngLatPos(lng,lat));
}

bool Map::inBounds(LngLat lngLat) {
	return this->inBounds(this->lngLatToPos(&lngLat));
}

bool Map::inBounds(LngLatPos pos) {
	return (pos.lngPos < this->longitudeCount && pos.latPos < this->latitudeCount && pos.lngPos >= 0 && pos.latPos > 0);
}

bool Map::checkPosition(LngLatPos pos) {
	if(!this->inBounds(pos)) {
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

LngLat Map::posToLngLat(LngLatPos* pos) {
	double longitude = this->longitudeStart+this->longitudeStep*pos->lngPos;
	double latitude = this->latitudeStart+this->latitudeStep*pos->latPos;
	return LngLat (longitude,latitude);
}