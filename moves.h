#ifndef MOVES_H
#define MOVES_H
#include <vector>
#include "lnglat.h"

class Move;
typedef vector<Move> vMoves;
class Move {
public:
    int dLatPos;
    int dLngPos;
    float cost;
    int dir;

    Move(int dLngPos, int dLatPos, int dir, int windDir, int maxSailDeviantion) : dLatPos(dLatPos), dLngPos(dLngPos), dir(dir) {
        this->cost = this->calculateCost(dir, windDir, maxSailDeviantion);
    }

    friend ostream& operator<<(ostream& os, const Move& m) {
        os << "(" << m.dLngPos << ", " << m.dLatPos << "): " << m.cost;
        return os;
    }

    /**
     * zwraca koszt ruchu w danym kierunku, uwzględnia wiatr
     * 0: połnoc, 90: wschód, 180: południe, -90: zachód
     * @param  int kierunek ruchu (odchyłka od północy)
     * @return     koszt ruchu
     */
    float calculateCost(int dir, int windDir, int maxSailDeviantion) {
        float cost;
        if(dir%90==0) {
            cost = 1.;
        } else {
            cost = 1.4142;
        }
        float w = windDir - dir; // kąt pomiędzy kierunkiem wiatru, a kierunkiem ruchu

        cost = (abs(Move::reduceDegrees(w)) <= maxSailDeviantion ? cost : -1);
        //dlog << "w: " << w << " = " << cost;
        return cost;
    }

    /**
     * Generuje wektor obiektów Move
     * określających możliwe ruchy po mapie, wraz z kosztem ruchu
     * @return vector<Move>
     */
    static vMoves generateMoveVector(int windDirection, int maxSailDeviantion) {
        vMoves moves;
        moves.push_back(Move(0, 1, 0, windDirection, maxSailDeviantion)); // N
        moves.push_back(Move(1, 1, 45, windDirection, maxSailDeviantion)); // NE
        moves.push_back(Move(1, 0, 90, windDirection, maxSailDeviantion)); // E
        moves.push_back(Move(1, -1, 135, windDirection, maxSailDeviantion)); // SE
        moves.push_back(Move(0, -1, 180, windDirection, maxSailDeviantion)); // S
        moves.push_back(Move(-1, -1, -135, windDirection, maxSailDeviantion)); // SW
        moves.push_back(Move(-1, 0, -90, windDirection, maxSailDeviantion)); // W
        moves.push_back(Move(-1, 1, -45, windDirection, maxSailDeviantion)); // NW

        vMoves ret;
        for(int i = 0; i < moves.size(); i++) {
            if(moves[i].cost > 0) {
                ret.push_back(moves[i]);
                // dlog << moves[i];
            }
        }

        return ret;
    }

    /**
     * redukuje stopnie do zakresu (-180, 180> stopni
     * @param  w stopnie
     * @return   stopnie zredukowane
     */
    static float reduceDegrees(float w) {
        while(w<-180)
            w+=360;
        while(w>180)
            w-=360;
        return w;
    }
};

struct OpenCell {
    double f;
    double g;
    double h;
    LngLatPos lngLatPos;
    LngLatPos parentLngLatPos;
    int dir;

    OpenCell(double _g, double _h, LngLatPos _lngLatPos, LngLatPos _parentLngLatPos, int dir)
     : f(_g+_h), g(_g), h(_h), lngLatPos(_lngLatPos), parentLngLatPos(_parentLngLatPos), dir(dir) {
     	// cout << "openCell h: " << h << "g: " << g << " f: " << f << endl;
    }

    bool operator < (const OpenCell& str) const {
        return (f > str.f);
    }
};

#endif