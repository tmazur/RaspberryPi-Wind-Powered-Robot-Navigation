#include "lnglat.h"
#include "map.h"

LngLat LngLatPos::toLngLat(Map *pMap) {
    return pMap->posToLngLat(this);
}

LngLatPos LngLat::toPos(Map *pMap) {
    return pMap->lngLatToPos(this);
}

LngLatPos LngLatPos::offset(int lngOff, int latOff, Map* map) {
	return LngLatPos(lngPos+(lngOff*((int)map->invertLng)), latPos+(latOff*((int)map->invertLat)));
}