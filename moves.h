#ifndef MOVES_H
#define MOVES_H
#include <vector>
#include "lnglat.h"

struct Move {
    int dLatPos;
    int dLngPos;
    float cost;

    Move(int dLngPos, int dLatPos, float cost) : dLatPos(dLatPos), dLngPos(dLngPos), cost(cost) {
        
    }

    friend ostream& operator<<(ostream& os, const Move& m) {
        os << "(" << m.dLngPos << ", " << m.dLatPos << "): " << m.cost;
        return os;
    }
};
typedef vector<Move> vMoves;

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

#endif