#ifndef TWI_H
#define TWI_H
#include <string>
#include <wiringPiI2C.h>
#include "lnglat.h"
#include "mlog.h"
using namespace std;

class TWI {
private:
    int fdAtmega;
    float getFloat(int);
    int writeFloat(int, float);
public:
    TWI();
    /**
     * zwraca aktualną pozycję robota we współrzędnych LngLat
     * @return LngLat
     */
    LngLat getPosition();
    /**
     * zwraca punkt do którego podąża aktualnie robot
     * @return LngLat
     */
    LngLat getCurrentGoal(); //small goal
    /**
     * wpisuje punkt do którego robot ma jechać
     * wpisanie tego punktu spowoduje anulowanie poprzedniej trasy robota
     * @param  LngLat punkt
     * @return        powodzenie?
     */
    int writeFirstGoal(LngLat);
    /**
     * wpisuje kolejny punkt do którego powinien podążać robot
     * @param  LngLat punkt
     * @return        powodzenie?
     */
    int writeNextGoal(LngLat);
};

#endif