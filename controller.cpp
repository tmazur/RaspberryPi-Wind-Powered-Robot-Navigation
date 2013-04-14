#include "controller.h"

Controller::Controller() {
	this->map = Map ("map1.txt");

	this->lngLatCurrent = LngLat(this->map.getLongitudeStart(), this->map.getLatitudeStart());
	this->lngLatGoal = LngLat(this->map.getLongitudeEnd(), this->map.getLatitudeEnd());
	

	// conversion - position check
	// double lng, lat;
	// int pos;
	// cout << "Enter position to convert:";
	// cin >> pos;

	// this->map.posToLngLat(pos,lng,lat);
	// cout << "resulting lng,lat: " << lng << ", " << lat << endl;
	// pos = this->map.lngLatToPos(lng,lat);
	// cout << "resulting pos: " << pos << endl;

	astar();
	// cout << this->map.checkPosition(-1,0) << endl;
	// cout << this->map.checkPosition(0,0) << endl;
	// cout << this->map.checkPosition(1,1) << endl;
	// cout << this->map.checkPosition(9,5) << endl;
	// cout << this->map.checkPosition(9,11) << endl;
	// cout << this->map.checkPosition(19.915021,50.071147) << endl; //first
	// cout << this->map.checkPosition(19.920337,50.071147) << endl; //last in first row
	// cout << this->map.checkPosition(19.915021,50.067001) << endl; //first in last row
	// cout << this->map.checkPosition(19.920337,50.067001) << endl; //last

	// cout << this->map.checkPosition(19.935021,50.071147) << endl; //longitude out of range
	// cout << this->map.checkPosition(19.915021,50.171147) << endl; //latitude out of range
}

double Controller::heuristic(LngLatPos p1, LngLatPos p2) {
	return sqrt(pow(p1.lngPos-p2.lngPos,2) + pow(p1.latPos-p2.latPos,2));
}

double Controller::heuristic(LngLatPos p1) {
	return this->heuristic(p1, this->lngLatGoal.toPos(&this->map));
}

void Controller::astar() {
	bool found = false;
	LngLatPos startPos = this->lngLatCurrent.toPos(&this->map);
	LngLatPos goalPos = this->lngLatGoal.toPos(&this->map);
	Log::getInstance().debug("Find path from "+startPos.toString()+" to "+goalPos.toString());

	vector <OpenCell> openCells;
	OpenCell tempCell = OpenCell (0, this->heuristic(startPos, goalPos), startPos , LngLatPos(0,0)); //create open cell with current position
	openCells.push_back(tempCell);
	// tempCell = OpenCell (0, this->heuristic(startLng,startLat), startLng, startLat,0,0); //create open cell with current position
	// openCells.push_back(tempCell);
	ClosedCellMap closedCells;
	pair<int,int> moves[4] = {make_pair(1,0), make_pair(0,-1), make_pair(0,1), make_pair(-1,0)};

	while(!found && openCells.size()>0) {
		sort(openCells.begin(),openCells.end());
		tempCell = openCells.back();
		openCells.pop_back();
		Log::getInstance().debug("opened cell "+tempCell.lngLatPos.toString());

		if(tempCell.lngLatPos==goalPos) {
			found=true;
			cout << "found path!!!" << endl;
		} else {
			for(int i=0;i<4;i++) {
				pair<int,int> tmpPair = moves[i];
				LngLatPos newPos = tempCell.lngLatPos.offset(tmpPair);
				// Log::getInstance().debug("new position: "+newPos.toString());
				if(this->map.checkPosition(newPos)==false) { //no obstacle on new position
					if(closedCells.find(newPos)==closedCells.end()) { //first time visiting cell
						openCells.push_back(OpenCell(tempCell.g+1, this->heuristic(newPos, goalPos), newPos, tempCell.lngLatPos));
						closedCells[newPos]=tempCell.lngLatPos;
					}
				}
			}
		}

	}

	if(!found) {
		Log::getInstance().debug("Path not found!");
	}

	this->getPath(closedCells);
}

string Controller::getPath(ClosedCellMap closedCells) {
	LngLatPos startPos = this->lngLatCurrent.toPos(&this->map);
	LngLatPos goalPos = this->lngLatGoal.toPos(&this->map);

	vector<LngLatPos> path;
	LngLatPos tmpPos = closedCells[goalPos];
	while(tmpPos!=startPos) {
		path.push_back(tmpPos);
		tmpPos = closedCells[tmpPos];
	}
	string spath = "";
	while(path.size()>0) {
		spath += (path.back().toString()) + "; ";
		path.pop_back();
	}

	Log::getInstance().debug(spath);
	return spath;
}

string Controller::getPathNextCoord(ClosedCellMap closedCells) {
	LngLatPos startPos = this->lngLatCurrent.toPos(&this->map);
	LngLatPos goalPos = this->lngLatGoal.toPos(&this->map);

	vector<LngLatPos> path;
	LngLatPos tmpPos = closedCells[goalPos];
	while(tmpPos!=startPos) {
		path.push_back(tmpPos);
		tmpPos = closedCells[tmpPos];
	}
	
	return tmpPos.toString();
}