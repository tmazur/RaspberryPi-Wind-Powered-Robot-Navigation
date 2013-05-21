#include "controller.h"

Controller::Controller() : endThreads(false), positionInBounds(true) {
	this->lngLatCurrent = LngLat(19.916178,50.064160); // todo: get current positon from ATmega
	this->lngLatGoal = this->lngLatCurrent;
	
	
	this->run();
}

void Controller::runMysql() {
	Db db = Db::getInstance();
	
	//reset data
	db.updateLngLat(LngLat(0., 0.));
	db.setPathStatus(0);
	db.setMapStatus(0); //mapa niewczytana

	//add 'load map' to worker queue
	this->workerTaskParams.push(db.getMapName());
	this->workerTask.push(2);

	bool positionInBounds = this->positionInBounds;
	while(!endThreads) {
		db.updateLngLat(this->lngLatCurrent);
		LngLat newGoal = db.getLngLatGoal();
		// dlog << "old goal: " << lngLatGoal.toString() << "; new goal: " << newGoal.toString();
		if(newGoal != this->lngLatGoal && abs(newGoal.lat)>0. && abs(newGoal.lng)>0.) {
			this->lngLatGoal = newGoal;
			dlog << "nowy cel: " << newGoal.toString();
			db.setPathStatus(0);
			this->workerTask.push(1);
		}

		if(positionInBounds != this->positionInBounds) { //zapisz zmianę do db
			positionInBounds = this->positionInBounds;
			db.updateDataParam("positionInBounds", to_string(positionInBounds));
		}

		std::chrono::milliseconds sleepDuration(5000);
		std::this_thread::sleep_for(sleepDuration);
	}
	dlog << "koniec threadMysql";
}

void Controller::runWorker() {
	while(!endThreads) {
		if(!this->workerTask.empty()) {
			switch(this->workerTask.front()) {
				case 0:
					break;
				case 1:
					this->astar();
					break;
				case 2: {
					string mapName = this->workerTaskParams.front();
					this->workerTaskParams.pop();
					dlog << "wczytywanie mapy: " << mapName;
					this->map = new Map(mapName);
					break;
				} default:
					elog << "nieznana komenda dla wątku threadWorker";
					break;
			}
			this->workerTask.pop();
		}
		chrono::milliseconds sleepDuration(1000);
		this_thread::sleep_for(sleepDuration);
	}
	dlog << "koniec threadWorker";
}

void Controller::runTWI() {
	//tmp
	this->windSpeed = 20;
	this->windDirection=0;
	this->robotOrientation=0;
	

	chrono::milliseconds sleepDuration(2000); //tmp
	this_thread::sleep_for(sleepDuration);

	Db db = Db::getInstance();
	while(!endThreads) {
		this->lngLatCurrent = db.getFakeTWI(); // todo: get current positon from ATmega

		// sprawdź czy pozycja jest granicach mapy
		if(sizeof(this->map)>0 && !this->mapLoading)
			this->positionInBounds = this->map->inBounds(this->lngLatCurrent);

		chrono::milliseconds sleepDuration(2000);
		this_thread::sleep_for(sleepDuration);
	}
	dlog << "koniec threadTWI";
}

void Controller::i2cComm() {
	int fd = wiringPiI2CSetup(3);
	if(fd > -1) {
		int reg, data, op, resp;
		while(1) {
			cout << "read (0) / write(1): ";
			cin >> op;
			if(op == 1) {
				cout << "write register: ";
				cin >> reg;
				cout << "write data: ";
				cin >> data;
				resp = wiringPiI2CWriteReg8(fd, reg, data);
				dlog << "response status: " << resp;
			} else if (op==2) {
				int s = sizeof(float);
				dlog << "float size: " << s;
				// int *byteArray[s];
				union doubleOrByte {
					char b[sizeof(float)];
					float f;
				} u;

				int addrStart=1;
				for(int i = 0; i<s; i++) {
					resp = wiringPiI2CReadReg8(fd, addrStart+i);
					dlog << "resp: " << resp;
					u.b[i] = resp;	
				}

				// double final = *reinterpret_cast<double*>(byteArray);
				dlog << "gpsLat: " << u.f;
			} else {
				cout << "read register: ";
				cin >> reg;
				resp = wiringPiI2CReadReg8(fd, reg) ;
				dlog << "response data: " << resp;
			}
		}
	} else {
		elog << "nie można rozpocząć komunikacji i2c";
	}
}

void Controller::run() {
	std::thread thr(&Controller::runMysql, this);
	std::swap(thr, threadMysql);
	std::thread thr2(&Controller::runWorker, this);
	std::swap(thr2, threadWorker);
	std::thread thr3(&Controller::runTWI, this);
	std::swap(thr3, threadTWI);
	

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
		} else if (action=="i2c") {
			i2cComm();
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

	endThreads=true;
    dlog << "Czekanie na zakończenie threadWorker";
    threadWorker.join();
    dlog << "Czekanie na zakończenie threadMysql";
    threadMysql.join();
    dlog << "Czekanie na zakończenie threadTWI";
    threadTWI.join();
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
		Db::getInstance().setPathStatus(2);
		return false;
	} else {
		vPath path = this->getPath(closedCells);
		Db tmpDb = Db::getInstance();
		tmpDb.savePath(path, this->map);
		tmpDb.setPathStatus(1);
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