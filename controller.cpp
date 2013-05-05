#include "controller.h"

Controller::Controller() : endThreads(false) {
	this->lngLatCurrent = LngLat(19.916178,50.064160); // todo: get current positon from ATmega
	this->lngLatGoal = this->lngLatCurrent;
	
	this->run();
}

void Controller::runMysql() {
	this->db = new Db("localhost","robot","root","krim.agh");
	
	//reset data
	this->db->updateLngLat(LngLat(0., 0.));

	//init
	this->map = new Map(this->db->getMapName());

	while(!endThreads) {
		this->db->updateLngLat(this->lngLatCurrent);
		LngLat newGoal = this->db->getLngLatGoal();
		// dlog << "old goal: " << lngLatGoal.toString() << "; new goal: " << newGoal.toString();
		if(newGoal != this->lngLatGoal) {
			this->lngLatGoal = newGoal;
			dlog << "nowy cel: " << newGoal.toString();
			this->astar();
		}

		std::chrono::milliseconds sleepDuration(10000);
		std::this_thread::sleep_for(sleepDuration);
	}
	dlog << "koniec threadMysql";
}

void Controller::run() {
	std::thread thr(&Controller::runMysql, this);
	std::swap(thr, threadMysql);

	string action;
	bool run=true;
	this->printMenu();
	while(run) {
		cout << endl << "Wybierz działanie: ";
		cin >> action;
		if(action=="wm") {
			string map;
			cout << "Ścieżka mapy: ";
			cin >> map;
			this->map = new Map(map);
		} else if(action=="sp") {
			double lng,lat;
			cout << endl << "lng: ";
			cin >> lng;
			cout << endl << "lat: ";
			cin >> lat;
			cout << endl << "position check: " << this->map->checkPosition(LngLatPos(lng,lat));
		// } else if(action=="scp") {
		// 	double lng,lat;
		// 	cout << endl << "lng: ";
		// 	cin >> lng;
		// 	cout << endl << "lat: ";
		// 	cin >> lat;
		// 	this->lngLatCurrent = LngLat(lng, lat);
		} else if(action=="uc") {
			double lng,lat;
			cout << endl << "lng: ";
			cin >> lng;
			cout << endl << "lat: ";
			cin >> lat;
			this->lngLatGoal = LngLat(lng, lat);
		} else if(action=="fp") {
			this->astar();
		} else if(action=="exit" || action=="e") {
			run=false;
		} else {
			if(action=="test1") {
				this->map = new Map("map_test1.txt");
				this->lngLatCurrent=LngLat(1.,1.);
				this->lngLatGoal=LngLat(4.,4.);
				this->astar();
			} else {
				cout << "Nieznane działanie: " << action << endl;
				this->printMenu();
			}
		}
	}
}

void Controller::printMenu() {
	cout << endl;
	cout << "Sprawdź pozycję (sp)" << endl;
	cout << "Wczytaj mapę (wm)" << endl;
	// cout << "Find path (fp)" << endl;
	cout << "Ustaw cel (uc)" << endl;
	cout << "Koniec (exit)" << endl;
}

double Controller::heuristic(LngLatPos p1, LngLatPos p2) {
	return sqrt(pow(p1.lngPos-p2.lngPos,2) + pow(p1.latPos-p2.latPos,2));
}

double Controller::heuristic(LngLatPos p1) {
	return this->heuristic(p1, this->lngLatGoal.toPos(this->map));
}

bool Controller::astar() {
	if(!sizeof(this->map)>0) {
		elog << "Brak wczytanej mapy!";
		return false;
	}
	bool found = false;
	LngLatPos startPos = this->lngLatCurrent.toPos(this->map);
	LngLatPos goalPos = this->lngLatGoal.toPos(this->map);
	dlog << "Szukam ścieżki z " << startPos.toString() << " do " << goalPos.toString();

	vector <OpenCell> openCells;
	OpenCell tempCell = OpenCell (0, this->heuristic(startPos, goalPos), startPos , LngLatPos(0,0)); //create open cell with current position
	openCells.push_back(tempCell);

	ClosedCellMap closedCells;
	Move moves[8] = {Move(1,0,1), Move(0,-1,1), Move(0,1,1), Move(-1,0,1), Move(1,1,1.4142), Move(1,-1,1.4142), Move(-1,1,1.4142), Move(-1,-1,1.4142)};

	while(!found && openCells.size()>0) {
		sort(openCells.begin(),openCells.end());
		tempCell = openCells.back();
		openCells.pop_back();
		// dlog << "opened cell " << tempCell.lngLatPos.toString();

		if(tempCell.lngLatPos==goalPos) {
			found=true;
			dlog << "Ścieżka znaleziona!";
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
		dlog << "Nie udało się odnaleźć ścieżki!";
		return false;
	} else {
		vPath path = this->getPath(closedCells);
		this->db->savePath(path, this->map);
		return true;
	}
}

vPath Controller::getPath(ClosedCellMap closedCells) {
	LngLatPos startPos = this->lngLatCurrent.toPos(this->map);
	LngLatPos goalPos = this->lngLatGoal.toPos(this->map);

	vPath path;
	LngLatPos tmpPos = closedCells[goalPos];
	while(tmpPos!=startPos) {
		path.push_back(tmpPos);
		tmpPos = closedCells[tmpPos];
	}
	return path;
}

string Controller::getSPath(vPath path) {
	string spath = "";
	while(path.size()>0) {
		spath += (path.back().toString()) + "; ";
		path.pop_back();
	}

	// dlog << "string path: " << spath;
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