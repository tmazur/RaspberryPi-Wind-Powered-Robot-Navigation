#ifndef CONTROLLER_H
#define CONTROLLER_H
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
#include <map>
#include <thread>
#include <queue>
#include <wiringPiI2C.h>
#include "mlog.h"
#include "lnglat.h"
#include "moves.h"
#include "map.h"
#include "db.h"
#include "twi.h"
using namespace std;

class Controller {
private:
    LngLat lngLatGoal;
	LngLat lngLatCurrent;
    bool positionInBounds; // aktualna pozycja jest w granicach mapy
	Map* map;
    queue<int> workerTask;
    queue<string> workerTaskParams;
    thread threadMysql;
    thread threadWorker;
    thread threadTWI;
    bool endThreads;
    vPath goalPath; // ścieżka do celu
    uint sentPoint; // nr wysłanych punktów ze ścieżki do celu
    int twiStatus;
    uint windSpeed; //prędkość wiatru pomnożona przez 10
    float windDirection; //kierunek wiatru - odchyłka od połnocy
    float robotOrientation; //orientacja robota - odchyłka od północy
    bool mapLoading;
    int maxSailDeviantion; // maksymalna odchyłka żagla od wiatru, po której można się sprawnie poruszać
    //vector<float> robotOrientationHistory; //historia orientacji robota
    double heuristic(LngLatPos);
    double heuristic(LngLatPos, LngLatPos);
    bool astar();
    vPath getPath(ClosedCellMap);
    string getSPath(vPath);
    string getPathNextCoord(ClosedCellMap);
    void runMysql();
    void runWorker();
    void runTWI();
    void printMenu();
    void i2cComm();
    /**
     * Generuje wektor obiektów Move
     * określających możliwe ruchy po mapie, wraz z kosztem ruchu
     * @return vector<Move>
     */
    vMoves generateMoveVector();
    /**
     * zwraca koszt ruchu w danym kierunku, uwzględnia wiatr
     * 0: połnoc, 90: wschód, 180: południe, -90: zachód
     * @param  int kierunek ruchu (odchyłka od północy)
     * @return     koszt ruchu
     */
    float calculateMoveCost(int);
public:
    void run();
    Controller();
    ~Controller() {
    };
};

#endif