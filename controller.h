#ifndef CONTROLLER_H
#define CONTROLLER_H
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <map>
#include "log.cpp"
#include "lnglat.h"
#include "map.h"
using namespace std;

typedef map<LngLatPos,LngLatPos> ClosedCellMap; // (currentPos,parentPos)

class Controller {
private:
    LngLat lngLatGoal;
	LngLat lngLatCurrent;
	Map map;
    double heuristic(LngLatPos);
    double heuristic(LngLatPos, LngLatPos);
	void astar();
    string getPath(ClosedCellMap);
    string getPathNextCoord(ClosedCellMap);
public:
	Controller();
};

struct OpenCell {
    double f;
    double g;
    double h;
    LngLatPos lngLatPos;
    LngLatPos parentLngLatPos;

    OpenCell(double _g, double _h, LngLatPos _lngLatPos, LngLatPos _parentLngLatPos)
     : f(_g+_h), g(_g), h(_h), lngLatPos(_lngLatPos), parentLngLatPos(_parentLngLatPos) {
     	// cout << "openCell h: " << h << "g: " << g << " f: " << f << endl;
    }

    bool operator < (const OpenCell& str) const {
        return (f > str.f);
    }
};

// struct ClosedCell {
//     LngLatPos lngLatPos;
//     LngLatPos parentLngLatPos;
//     int expandCount;

//     ClosedCell() {}

//     ClosedCell(LngLatPos _lngLatPos, LngLatPos _parentLngLatPos)
//      : lngLatPos(_lngLatPos), parentLngLatPos(_parentLngLatPos) {

//     }
// };

#endif