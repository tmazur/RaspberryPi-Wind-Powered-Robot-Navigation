#include "controller.h"

Controller::Controller() {
	// this->map = Map ("park_jordana.txt");

	// this->lngLatCurrent = LngLat(this->map->getLongitudeStart(), this->map->getLatitudeStart());
	// this->lngLatGoal = LngLat(this->map->getLongitudeEnd(), this->map->getLatitudeEnd());
	
	// this->lngLatCurrent = LngLat(19.916178,50.064160);
	// this->lngLatGoal = LngLat(19.917730,50.061085);

	// astar();
	

	this->db = new Db("localhost","robot","root","krim.agh");
	this->db->updateLngLat(LngLat(20.0,10.0));
	this->run();
}

void Controller::run() {
	string action;
	bool run=true;
	while(run) {
		cout << endl;
		cout << "Check position (cp)" << endl;
		cout << "Load map (lm)" << endl;
		cout << "Find path (fp)" << endl;
		cout << "Set current position (scp)" << endl;
		cout << "Set goal (sg)" << endl;
		cout << "Exit (exit)" << endl;
		cout << "Choose action: ";
		cin >> action;
		if(action=="lm") {
			string map;
			cout << "Load map name: ";
			cin >> map;
			this->map = new Map(map);
		} else if(action=="cp") {
			double lng,lat;
			cout << endl << "lng: ";
			cin >> lng;
			cout << endl << "lat: ";
			cin >> lat;
			cout << endl << "position check: " << this->map->checkPosition(LngLatPos(lng,lat));
		} else if(action=="scp") {
			double lng,lat;
			cout << endl << "lng: ";
			cin >> lng;
			cout << endl << "lat: ";
			cin >> lat;
			this->lngLatCurrent = LngLat(lng, lat);
		} else if(action=="sg") {
			double lng,lat;
			cout << endl << "lng: ";
			cin >> lng;
			cout << endl << "lat: ";
			cin >> lat;
			this->lngLatGoal = LngLat(lng, lat);
		} else if(action=="fp") {
			this->astar();
		} else if(action=="exit") {
			run=false;
		} else {
			if(action=="test1") {
				this->map = new Map("map_test1.txt");
				this->lngLatCurrent=LngLat(1.,1.);
				this->lngLatGoal=LngLat(4.,4.);
				this->astar();
			} else {
				cout << "Unknown action: " << action << endl;
			}
		}
	}
}

double Controller::heuristic(LngLatPos p1, LngLatPos p2) {
	return sqrt(pow(p1.lngPos-p2.lngPos,2) + pow(p1.latPos-p2.latPos,2));
}

double Controller::heuristic(LngLatPos p1) {
	return this->heuristic(p1, this->lngLatGoal.toPos(this->map));
}

void Controller::astar() {
	if(!sizeof(this->map)>0) {
		elog << "Map not loaded!";
		return;
	}
	bool found = false;
	LngLatPos startPos = this->lngLatCurrent.toPos(this->map);
	LngLatPos goalPos = this->lngLatGoal.toPos(this->map);
	dlog << "Find path from " << startPos.toString() << " to " << goalPos.toString();

	vector <OpenCell> openCells;
	OpenCell tempCell = OpenCell (0, this->heuristic(startPos, goalPos), startPos , LngLatPos(0,0)); //create open cell with current position
	openCells.push_back(tempCell);

	ClosedCellMap closedCells;
	Move moves[8] = {Move(1,0,1), Move(0,-1,1), Move(0,1,1), Move(-1,0,1), Move(1,1,1.4142), Move(1,-1,1.4142), Move(-1,1,1.4142), Move(-1,-1,1.4142)};

	while(!found && openCells.size()>0) {
		sort(openCells.begin(),openCells.end());
		tempCell = openCells.back();
		openCells.pop_back();
		dlog << "opened cell " << tempCell.lngLatPos.toString();

		if(tempCell.lngLatPos==goalPos) {
			found=true;
			dlog << "found path!!!";
		} else {
			for(int i=0;i<8;i++) {
				Move curMove = moves[i];
				LngLatPos newPos = tempCell.lngLatPos.offset(make_pair(curMove.dx, curMove.dy));
				if(this->map->checkPosition(newPos)==false) { //no obstacle on new position
					if(closedCells.find(newPos)==closedCells.end()) { //first time visiting cell
						openCells.push_back(OpenCell(tempCell.g+curMove.cost, this->heuristic(newPos, goalPos), newPos, tempCell.lngLatPos));
						closedCells[newPos]=tempCell.lngLatPos;
					}
				}
			}
		}

	}

	if(!found) {
		dlog << "Path not found!";
	} else {
		this->getPath(closedCells);	
	}
}

string Controller::getPath(ClosedCellMap closedCells) {
	LngLatPos startPos = this->lngLatCurrent.toPos(this->map);
	LngLatPos goalPos = this->lngLatGoal.toPos(this->map);

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

	dlog << "found path: " << spath;
	return spath;
}

string Controller::getPathNextCoord(ClosedCellMap closedCells) {
	LngLatPos startPos = this->lngLatCurrent.toPos(this->map);
	LngLatPos goalPos = this->lngLatGoal.toPos(this->map);

	vector<LngLatPos> path;
	LngLatPos tmpPos = closedCells[goalPos];
	while(tmpPos!=startPos) {
		path.push_back(tmpPos);
		tmpPos = closedCells[tmpPos];
	}
	
	return tmpPos.toString();
}