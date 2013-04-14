#ifndef LNGLAT_H
#define LNGLAT_H
#include <string>
using namespace std;

class Map;
struct LngLatPos;

struct LngLat {
    double lng;
    double lat;

    LngLat() {}
    LngLat(double _lng, double _lat) : lng(_lng), lat(_lat) {

    }

    string toString() {
        return "("+to_string(lng)+","+to_string(lat)+")";
    }

    LngLatPos toPos(Map*);
};

struct LngLatPos {
    int lngPos;
    int latPos;

    LngLatPos() {}
    LngLatPos(int _lngPos, int _latPos) : lngPos(_lngPos), latPos(_latPos) {

    }

    string toString() {
        return "("+to_string(lngPos)+","+to_string(latPos)+")";
    }

    bool operator==(const LngLatPos& rhs) const {
        return (lngPos==rhs.lngPos) && (latPos == rhs.latPos);
    }

    bool operator<(const LngLatPos& rhs) const {
        if(this->latPos < rhs.latPos) {
            return true;
        } else if(this->latPos > rhs.latPos) {
            return false;
        } else {
            return this->lngPos < rhs.lngPos;
        }
    }

    bool operator!=(const LngLatPos& rhs) const {
        return !((*this) == rhs);
    }

    LngLat toLngLat(Map*);
    LngLatPos offset(pair<int,int>);
};

#endif