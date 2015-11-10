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
#include <wiringSerial.h>
#include "mlog.h"
#include "lnglat.h"
#include "moves.h"
#include "map.h"
#include "db.h"
#include "twi.h"
#define TASK_FINDPATH 1
#define TASK_LOADMAP 2
#define MAP_NOTLOADED 0
#define PATH_SEARCHING 0
#define PATH_FOUND 1
#define PATH_NOTFOUND 2
#define UART_DEVICE "/dev/ttyAMA0"
using namespace std;

class Controller {
private:
    bool debug; // włączyć informacje do debugowania
    LngLat lngLatGoal; // cel
	LngLat lngLatCurrent; // aktualna pozycja
    bool positionInBounds; // czy aktualna pozycja jest w granicach mapy?
	Map* map; // wskaźnik do mapy
    queue<int> workerTask; // kolejka zadan dla wątku 'worker'
    queue<string> workerTaskParams; // parametry do zadan wątku 'worker'
    thread threadMysql; // wątek komunikacji z bazą danych
    thread threadWorker; // wątek 'worker' - zadaniowy
    thread threadTWI; // wątek komunikacji TWI
    thread threadUart; // wątek komunikacji UART
    bool endThreads; // przerwać pracę wątków?
    vPath goalPath; // ścieżka do celu
    uint sentPoint; // nr wysłanych punktów ze ścieżki do celu
    int twiStatus; // status komunikacji TWI
    uint windSpeed; //prędkość wiatru pomnożona przez 10
    uint windDirection; //kierunek wiatru - odchyłka od połnocy
    float robotOrientation; //orientacja robota - odchyłka od północy
    bool mapLoading; // oczekiwanie na wczytanie mapy?
    int maxSailDeviantion; // maksymalna odchyłka żagla od wiatru, po której można się sprawnie poruszać
    float rotationPenalty; // koszt zmiany orientacji robota (na 1 stopien obrotu)
    /**
     * funkcja heurystyczna odległości podanego punktu do celu
     * @param  LngLatPos pozycja 1
     * @return           odległość
     */
    double heuristic(LngLatPos);
    /**
     * funkcja heurystyczna odległości pomiędzy dwoma punktami
     * @param  LngLatPos punkt 1
     * @param  LngLatPos punkt 2
     * @return           odległość
     */
    double heuristic(LngLatPos, LngLatPos);
    /**
     * algorytm wyznaczania trajektorii
     * @return znaleziono trajektorię?
     */
    bool astar();
    /**
     * zwraca wyznaczoną trajektorię w postaci wektora
     * na podstawie mapy zamkniętych komórek
     * @param  ClosedCellMap mapa zamkniętych komórek wraz z rodzicami
     * @return               wektor kolejnych komórek prowadzący do celu
     */
    vPath getPath(ClosedCellMap);
    /**
     * metody poszczególnych wątków
     */
    void runMysql();
    void runWorker();
    void runTWI();
    void runUart();
    void printMenu();
    void i2cComm();
public:
    void run();
    Controller();
    ~Controller() {
    };
};

#endif