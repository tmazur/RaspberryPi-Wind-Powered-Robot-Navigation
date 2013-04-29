#ifndef CONTROLLER_H
#define CONTROLLER_H
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <map>
#include <thread>
#include "mlog.h"
#include "lnglat.h"
#include "map.h"
#include "db.h"
using namespace std;

typedef map<LngLatPos,LngLatPos> ClosedCellMap; // (currentPos,parentPos)

class Controller {
private:
    LngLat lngLatGoal;
	LngLat lngLatCurrent;
	Map* map;
    Db* db;
    thread threadMysql;
    bool endThreads;
    double heuristic(LngLatPos);
    double heuristic(LngLatPos, LngLatPos);
	bool astar();
    string getPath(ClosedCellMap);
    string getPathNextCoord(ClosedCellMap);
    void runMysql();
    void printMenu();
public:
    void run();
    Controller();
    ~Controller() {
        endThreads=true;
        dlog << "Czekanie na zakończenie threadMysql";
        threadMysql.join();
    };
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

struct Move {
    int dx;
    int dy;
    float cost;

    Move(int dx, int dy, float cost) : dx(dx), dy(dy), cost(cost) {
        
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