#include "controller.h"

Controller::Controller() : endThreads(false), positionInBounds(true), mapLoading(true), windDirection(0), robotOrientation(0) {
	//this->lngLatCurrent = LngLat(19.916178,50.064160); // todo: get current positon from ATmega
	//this->lngLatGoal = this->lngLatCurrent;
	
	this->run();
}

void Controller::runMysql() {
	Db db = Db::getInstance();
	
	//wczytanie konfiguracji
	this->debug = (db.getConfig("debug")=="1"?true:false);
	this->maxSailDeviantion = stoi(db.getConfig("maxSailDeviation"));
	this->rotationPenalty = stof(db.getConfig("rotationPenalty"));

	//reset data
	db.updateLngLat(LngLat(0., 0.));
	db.setPathStatus(PATH_SEARCHING);
	db.setMapStatus(MAP_NOTLOADED); //mapa niewczytana

	// dodaj zadanie wczytania mapy do kolejki wątku 'worker'
	this->workerTaskParams.push(db.getMapName());
	this->workerTask.push(TASK_LOADMAP);

	bool positionInBounds = this->positionInBounds; // czy pozycja znajduje się w obrębie mapy?
	string mapName; // nazwa wczytanej mapy pobierana z db
	LngLat newGoal; // cel pobierany z db
	while(!endThreads) {
		db.updateLngLat(this->lngLatCurrent);
		
		// zmiana celu
		newGoal = db.getLngLatGoal(); // pobierz cel z bazy danych
		if(newGoal != this->lngLatGoal && abs(newGoal.lat)>0. && abs(newGoal.lng)>0.) { // zmieniono cel
			this->lngLatGoal = newGoal;
			dlog << "nowy cel: " << newGoal.toString();
			db.setPathStatus(PATH_SEARCHING);
			this->workerTask.push(TASK_FINDPATH);
		}

		// uaktualnij pole positionInBounds w bazie danych
		if(positionInBounds != this->positionInBounds) { //zapisz zmianę do db
			positionInBounds = this->positionInBounds;
			db.updateDataParam("positionInBounds", to_string(positionInBounds));
		}

		// zmiana mapy
		if(this->map && !this->mapLoading) {
			mapName = db.getMapName();
			if(mapName != this->map->getMapName()) {
				// dodaj zadanie wczytania mapy do kolejki wątku 'worker'
				this->workerTaskParams.push(mapName);
				this->workerTask.push(TASK_LOADMAP);
				this->mapLoading = true;
				// dodaj zadanie wyznaczenia trajektori do kolejki wątku 'worker'
				db.setPathStatus(PATH_SEARCHING);
				this->workerTask.push(TASK_FINDPATH);
			}
		}

		db.setTwiStatus(this->twiStatus); // uaktualnij status TWI

		// todo: usunąć po podpięciu czujnika wiatru do uart
		this->windDirection = stoi(db.getDataParam("windDirection")); // pobierz informację o kierunku wiatru z bazy danych

		// opóźnienie pętli
		std::chrono::milliseconds sleepDuration(5000);
		std::this_thread::sleep_for(sleepDuration);
	}
	dlog << "koniec threadMysql";
}

/**
 * metoda wątku 'worker'
 */
void Controller::runWorker() {
	while(!endThreads) {
		if(!this->workerTask.empty()) {
			switch(this->workerTask.front()) {
				case 0:
					break;
				case TASK_FINDPATH: // wyznacz trajektorię
					this->astar();
					break;
				case TASK_LOADMAP: { // wczytaj mapę
					string mapName = this->workerTaskParams.front();
					this->workerTaskParams.pop();
					dlog << "wczytywanie mapy: " << mapName;
					this->map = new Map(mapName);
					this->mapLoading = false;
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

void Controller::runUart() {
	int fd = serialOpen(UART_DEVICE, 9600);
	int bSize = 0; // ilość znaków w buforze
	while(!endThreads) {
		bSize = serialDataAvail(fd);
		if(bSize==8) { // dwa inty czekają na odczyt
			union IntOrByte {
				char b[4];
				int i;
			} u;
			
			// odczyt kierunku wiatru
			for (int i =0; i < 4; i++) { //odczytaj 4 bajty
				u.b[i] = serialGetchar(fd);
			}
			this->windDirection = u.i;

			// odczyt prędkości wiatru
			u.i = 0; // resetujemy union
			for (int i =0; i < 4; i++) { //odczytaj 4 bajty
				u.b[i] = serialGetchar(fd);
			}
			this->windSpeed = u.i;
		} else if(bSize>8) { // zbyt dużo informacji w buforze
			serialFlush(fd);
		}

		chrono::milliseconds sleepDuration(500);
		this_thread::sleep_for(sleepDuration);
	}
	serialClose(fd);
	dlog << "koniec threadWorker";
}

void Controller::runTWI() {
	chrono::milliseconds sleepDuration(2000); //tmp
	this_thread::sleep_for(sleepDuration);

	TWI twi = TWI();
	LngLat currGoal = LngLat(0., 0.);
	Db db = Db::getInstance();
	while(!endThreads) {
		this->lngLatCurrent = db.getFakeTWI(); // todo: get current positon from ATmega
		if(sizeof(this->map)>0 && !this->mapLoading) // sprawdź czy pozycja jest granicach mapy
			this->positionInBounds = this->map->inBounds(this->lngLatCurrent);

		if(this->goalPath.size() > 0) { // ścieżka wyznaczona
			if(this->sentPoint == 0) { // nowa ścieżka
				this->twiStatus = twi.writeFirstGoal(this->goalPath[0].toLngLat(this->map));
				if(this->twiStatus < -1) { // twi ok
					currGoal = this->goalPath[1].toLngLat(this->map);
					this->twiStatus = twi.writeNextGoal(currGoal);
					if(this->twiStatus > -1) { // twi ok
						this->sentPoint=1;
					} else { // twi error
						// elog << "Problem z komunikacją TWI: " << this->twiStatus;
					}
				} else { // twi error
					// elog << "Problem z komunikacją TWI: " << this->twiStatus;
				}
			} else { // kontynuuj wysyłanie trajektorii
				LngLat tmp = twi.getCurrentGoal();
				// dlog << "current goal from robot: " << tmp;
				// dlog << "current set goal: " << currGoal;
				if(tmp == currGoal) { // robot podąża do 'następnego' celu
					// wyślij nowy 'następny' cel
					this->sentPoint++;
					if(this->sentPoint < this->goalPath.size()) { // pozostały punkty trajektorii do wysłania
						currGoal = this->goalPath[this->sentPoint].toLngLat(this->map);
						this->twiStatus = twi.writeNextGoal(currGoal);
						if(this->twiStatus <= -1) { // twi error
							// elog << "Problem z komunikacją TWI: " << this->twiStatus;
						}
					} else { // wysłano ostatni punkt
					}
				}
			}
		}

		// opóźnij wątek
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
	// uruchom poszczególne wątki
	std::thread thr(&Controller::runMysql, this);
	std::swap(thr, threadMysql);
	std::thread thr2(&Controller::runWorker, this);
	std::swap(thr2, threadWorker);
	std::thread thr3(&Controller::runTWI, this);
	std::swap(thr3, threadTWI);
	std::thread thr4(&Controller::runUart, this);
	std::swap(thr4, threadUart);
	

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
	// odległość Czebyszewa
	int dx = abs(p1.lngPos-p2.lngPos);
	int dy = abs(p1.latPos-p2.latPos);
	return dx + dy - 0.5858 * min(dx, dy); // -0.58578 = sqrt(2) - 2

	// odległość euklidesowa
	//return sqrt(pow(p1.lngPos-p2.lngPos,2) + pow(p1.latPos-p2.latPos,2));
}

double Controller::heuristic(LngLatPos p1) {
	return this->heuristic(p1, this->lngLatGoal.toPos(this->map));
}

bool Controller::astar() {
	if(!sizeof(this->map)>0 && !this->mapLoading) { // mapa niewczytana lub w trakcie wczytywania
		elog << "Brak wczytanej mapy!";
		Db::getInstance().setPathStatus(PATH_NOTFOUND);
		return false;
	}
	if(!this->map->inBounds(this->lngLatGoal)) { // cel poza mapą
		dlog << "Cel poza obszerem mapy!";
		Db::getInstance().setPathStatus(PATH_NOTFOUND);
		return false;
	}

	// inicjalizacja
	LngLatPos startPos = this->lngLatCurrent.toPos(this->map);
	LngLatPos goalPos = this->lngLatGoal.toPos(this->map);
	dlog << "Szukam ścieżki z " << startPos.toString() << " do " << goalPos.toString();

	// utworz listę otwartych komórek, dopisz aktualną pozycję
	vector <OpenCell> openCells;
	OpenCell tempCell = OpenCell (0, this->heuristic(startPos, goalPos), startPos , LngLatPos(0,0), (int) robotOrientation); //create open cell with current position
	openCells.push_back(tempCell);

	// utwórz listę komórek zamkniętych
	ClosedCellMap closedCells;

	// wygenerowanie wektorów dozwolonych ruchów
	vMoves moves = Move::generateMoveVector(this->windDirection, this->maxSailDeviantion);
	int movesSize = moves.size(); // liczba dozwolonych ruchów
	bool found = false; // znaleziono cel
	float goalCost = 0.; // koszt dotarcia do celu
	int i = 0, progress = 0, interval = round(this->map->size()/100); // zmienna do wyliczania postępu pracy algorytmu
	while(!found && openCells.size()>0) { // cel nie osiągnięty i lista komórek otwartych nie jest pusta
		if(i==interval) { // wypisz postęp procentowy (co 1%)
			i=0;
			progress++; // postęp procentowy (przy założeniu 0% przeszkód)
			dlog << progress << "%";
		}
		i++;

		sort(openCells.begin(),openCells.end()); // sortowanie listy otwartych komórek po koszcie całkowitym 'f'
		tempCell = openCells.back(); // pobranie komórki z najmniejszym kosztem
		openCells.pop_back();
		if (this->debug)
			dlog << "otwarto komórkę " << tempCell.lngLatPos.toString() << "koszt: " << tempCell.f;

		if(tempCell.lngLatPos==goalPos) { // osiągnięto cel
			found=true;
			goalCost = tempCell.f;
			dlog << "Ścieżka znaleziona! Koszt: " << goalCost;
		} else {
			for(int i=0;i<movesSize;i++) { // iteracja po dozwolonych ruchach
				Move curMove = moves[i]; // aktualny ruch
				LngLatPos newPos = tempCell.lngLatPos.offset(curMove.dLngPos, curMove.dLatPos, this->map); // pozycja po wykonaniu ruchu
				if(this->map->checkPosition(newPos)==false) { // brak przeszkód na nowej pozycji
					if(closedCells.find(newPos)==closedCells.end()) { // komórka nie występuje na liście komórek zamkniętych
						float dirChange = abs(Move::reduceDegrees(curMove.dir - tempCell.dir)); // zmiana orientacji
						float dirChangeCost = dirChange * this->rotationPenalty; // koszt zmiany orientacji
						if(this->debug)
							dlog << "dopisz komórkę " << newPos << "heurestyka: " << this->heuristic(newPos, goalPos) << " dirChange: " << dirChange << "rotationPenalty: " << dirChangeCost << "całkowity koszt: " << tempCell.g + curMove.cost + dirChangeCost + this->heuristic(newPos, goalPos);
						openCells.push_back(OpenCell(tempCell.g + curMove.cost + dirChangeCost, this->heuristic(newPos, goalPos), newPos, tempCell.lngLatPos, curMove.dir)); // dopisz komórkę do listy komórek otwartch
						closedCells[newPos]=tempCell.lngLatPos; // dopisz komórkę do listy komórek zamkniętych
					}
				}
			}
		}

	}

	Db tmpDb = Db::getInstance();
	tmpDb.setGoalCost(goalCost); // ustaw koszt osiągnięcia celu
	if(!found) { // nie udało się wyznaczyć ścieżki do celu
		dlog << "Nie udało się odnaleźć ścieżki!";
		tmpDb.setPathStatus(PATH_NOTFOUND);
		tmpDb.setGoalCost(goalCost);
		return false;
	} else { // znaleziono trajektorię do celu
		this->goalPath = this->getPath(closedCells); // ustaw trajektorię do celu
		this->sentPoint = 0; // resetuj liczbę wysłanych punktów trajektorii

		tmpDb.savePath(this->goalPath, this->map); // zapisz wyznaczoną trajektorię do bazy danych
		tmpDb.setPathStatus(PATH_FOUND);
		return true;
	}
}

vPath Controller::getPath(ClosedCellMap closedCells) {
	LngLatPos startPos = this->lngLatCurrent.toPos(this->map);
	LngLatPos goalPos = this->lngLatGoal.toPos(this->map);

	vPath path;
	LngLatPos tmpPos = closedCells[goalPos]; // ustaw cel jako aktualną komórkę
	while(tmpPos!=startPos) { // wróciliśmy do komórki startowej
		path.push_back(tmpPos); // dopisz aktualną komórkę do 
		tmpPos = closedCells[tmpPos]; // ustaw poprzednią komórkę (rodzica) jako aktualną komórkę
	}
	return path;
}