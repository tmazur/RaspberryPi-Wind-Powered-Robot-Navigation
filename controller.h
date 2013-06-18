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
    bool debug;
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
    float rotationPenalty; // koszt zmiany orientacji robota (na 1 stopien obrotu)
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
public:
    void run();
    Controller();
    ~Controller() {
    };
};

#endif