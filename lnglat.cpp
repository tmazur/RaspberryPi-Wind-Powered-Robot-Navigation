#include "lnglat.h"
#include "map.h"

LngLat LngLatPos::toLngLat(Map *pMap) {
    return pMap->posToLngLat(this);
}

LngLatPos LngLat::toPos(Map *pMap) {
    return pMap->lngLatToPos(this);
}

LngLatPos LngLatPos::offset(pair<int,int> offset) {
	return LngLatPos(lngPos+offset.first,latPos+offset.second);
}