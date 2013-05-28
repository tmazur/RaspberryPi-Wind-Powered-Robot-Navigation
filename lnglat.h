#ifndef LNGLAT_H
#define LNGLAT_H
#include <string>
#include <cmath>
#include <map>
#include <vector>
#define FLOAT_ACCURACY 0.00005
using namespace std;

class Map;
struct LngLatPos;
typedef map<LngLatPos,LngLatPos> ClosedCellMap; // (currentPos,parentPos)
typedef vector<LngLatPos> vPath;

struct LngLat {
    double lng;
    double lat;

    LngLat() {}
    LngLat(double _lng, double _lat) : lng(_lng), lat(_lat) {

    }

    string toString() const{
        return "("+to_string(lng)+","+to_string(lat)+")";
    }

    LngLatPos toPos(Map*);

    bool operator==(const LngLat& rhs) const {
        return (abs(lng-rhs.lng) < FLOAT_ACCURACY) && (abs(lat-rhs.lat) < FLOAT_ACCURACY);
    }

    bool operator!=(const LngLat& rhs) const {
        return !((*this) == rhs);
    }

    friend ostream& operator<<(ostream& os, const LngLat& lnglat) {
        os << lnglat.toString();
        return os;
    }
};

struct LngLatPos {
    int lngPos;
    int latPos;

    LngLatPos() {}
    LngLatPos(int _lngPos, int _latPos) : lngPos(_lngPos), latPos(_latPos) {

    }

    string toString() const {
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

    friend ostream& operator<<(ostream& os, const LngLatPos& lnglat) {
        os << lnglat.toString();
        return os;
    }

    LngLat toLngLat(Map*);
    LngLatPos offset(int,int,Map*);
};

#endif