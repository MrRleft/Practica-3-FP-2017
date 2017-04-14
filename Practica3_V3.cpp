//
#include <iostream>
#include <string>
#include <fstream>
#include <Windows.h>
#include <iomanip>
#include <conio.h>

using namespace std;

//Constantes.
const int MAX = 50;
const int MAXH = 10;
const int MAXE = 100;

//Enumerados.
typedef enum tCasilla{ Libre, Muro, DestinoL, DestinoC, DestinoJ, Jugador, Caja, Nada };// DestinoL= destino libre,destinoC = destinocaja,destinoJ = destinojugador
typedef enum tTecla { arriba, abajo, derecha, izquierda, salir, nada, back };

//Definicion del array del tablero
typedef tCasilla tTablero[MAX][MAX];


//Definicion de structs
typedef struct {

	tTablero tablero;
	 int nFila;
	 int nColumna;
	 int posFJ;
	 int posCJ;
	 int nCajasTotal;
	 int nCajasOK;

}tSokoban;

typedef tSokoban tHistorial[MAXH]; //Array para el historial

typedef struct {

	tHistorial historial;
	int cont;

} tHistoria;

typedef struct {

	tSokoban sokoban;
	unsigned int numMovimientos;
	string nFichero;
	int nivel;
	tHistoria historia;

} tJuego;

typedef struct {

	string nFichero;
	int nivel;
	int numMovimientos;

} tPartida;

typedef tPartida tExitos[MAXE]; //Definicion del tipo de array para los niveles completados

typedef struct {

	tExitos exitos;
	string nombre;
	int numPartidas;

} tInfo;


//Declaracion funciones V1
//Funciones visualizaci󮠴ablero
void colorFondo(int color);
void dibujaCasilla(tCasilla casilla);
void dibujar(const tJuego &juego);

//Funciones cargar nivel de juego
void inicializa(tJuego &juego);
bool cargarJuego(tJuego & juego);
bool cargarNivel(ifstream &fichero, tSokoban &sokoban, int nivel);

//Lectura de teclas especiales
tTecla leerTecla();

//Movimiento en el tablero
void hacerMovimiento(tJuego &juego, tTecla tecla);
void realizarMovimiento(tJuego &juego, const int i, const int j, const int futuroI, const int futuroI2, const int futuroJ, const int futuroJ2);
bool comprobarMovimiento(const tJuego &juego, const int futuroI, const int futuroI2, const int futuroJ, const int futuroJ2);

//Funciones necesarias para la version 2
bool bloqueado(const tJuego &juego);
bool deshacerMovimiento(tJuego &juego);
void actualizarHistorial(tJuego &juego);

//Funciones necesarias para la version 3
void guardaInfo(const tInfo &info);
bool cargarInfoJugador(tInfo &info);
void modificarInfo(tInfo &info, const tJuego &juego);
void darLaVuelta(tInfo &info);
void mostrarInfo(const tInfo &info);

//otros
tCasilla casilla(char valCasilla);
void opcion1(tJuego &juego, tInfo &info);

//Funciones V1

//Funciones visualizaci󮠤el tablero
void colorFondo(int color) {
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(handle, 15 | (color << 4));
}

void dibujaCasilla(tCasilla casilla){
	
	switch (casilla)
	{
	case Libre :
		colorFondo(1);
		cout << setw(2) << " ";
		break;
	case Jugador :
		colorFondo(7);
		cout << setw(2) << "00";
		break;
	case Caja :
		colorFondo(2);
		cout << setw(2) << "[]";
		break;
	case Muro:
		colorFondo(4);
		cout << setw(2) << " ";
		break;
	case DestinoC:
		colorFondo(12);
		cout << setw(2) << "[]";
		break;
	case DestinoJ:
		colorFondo(7);
		cout << setw(2) << "00";
		break;
	case DestinoL:
		colorFondo(3);
		cout << setw(2) << " ";
		break;
	case Nada:
		/*	La nada, este es un añadido mio para facilitarme el dibujado de la funcion. 
			Pues llenaremos el resto de espacios libres fuera de nuestra zona de juego como nada, para que a la hora de dibujar el tablero no aparezcan estos representados 
			como libres.*/
		colorFondo(0);
		cout << setw(2) << " ";
		break;

	}
	colorFondo(0);
}

void dibujar(const tJuego &juego){
	bool on = false; //Esto se encarga de empezar a dibujar cuando toca para que no dibuje espacios libres fuera del espacio de juego
	system("cls");
	cout << "Fichero : " << juego.nFichero << " Nivel: nivel " << juego.nivel << endl << endl;
	for (int j = 0; j <= juego.sokoban.nFila; j++){
		for (int i = 0; i < juego.sokoban.nColumna; i++){
			if (juego.sokoban.tablero[j][i] != Libre) on = true;
			if (on == true) dibujaCasilla(juego.sokoban.tablero[j][i]);
			else cout << setw(2) << " ";
		}
		cout << endl;
		on = false;
	}
	cout << endl;
	cout << "Numero de movimientos -> "<< juego.numMovimientos << endl;
}


//Funciones cargar nivel de juego
void inicializa(tJuego &juego){
	
	juego.numMovimientos = 0;
	for (int j = 0; j < MAX; j++) for (int i = 0; i < MAX; i++) juego.sokoban.tablero[j][i] = Libre;
	juego.historia.cont = 0;
	juego.sokoban.nFila = 0;

}

bool cargarJuego(tJuego & juego){


	bool resultado;
	ifstream file;
	string nomFile;
	cout << "Di el nombre del fichero.\n";
	cin >> nomFile;
	file.open(nomFile);
	if (file.is_open()){
		int nivelBuscar;
		cout << "Muy bien, ahora dime que nivel quieres.\n";
		cin >> nivelBuscar;
		if (cargarNivel(file, juego.sokoban, nivelBuscar)){
			juego.nFichero = nomFile;
			juego.nivel = nivelBuscar;
			resultado = true;
		}
		else resultado = false;
		file.close();
	}
	else {
		cout << "No se ha abierto el fichero.\n";
		resultado = false;
	}

	return resultado;
}

bool cargarNivel(ifstream &fichero, tSokoban &sokoban, int nivel){//Incompleta.
	
	bool resultado = true;
	string linea;
	string minivel = "Level " + to_string(nivel); //Esto se utiliza para buscar el nivel del archivo
	int i = 0, j = 0; //Estos son contadores que utilizaremos para guardar en el tablero
	tCasilla aux; // Aqui guardamos el caracter que saquemos del string con la linea
	getline(fichero, linea); //Cogemos la primera linea del archivo
	while (!fichero.eof() && linea != minivel){ //Aqui se van cogiendo lineas hasta que encontramos el nivel.
		getline(fichero, linea);
	}
	if (fichero.eof() || linea != minivel){ //Aqui comprobamos donde hemos parado de leer lineas para comprobar si se ha encontrado el nivel o no.
		resultado = false;
		cout << "El nivel no ha sido encontrado.\n";
	}
	else {//Aqui tomamos que se ha encontrado y empezamos a cargar el nivel.
	
		getline(fichero, linea); //Cogemos la primera linea del nivel.
		sokoban.nColumna = linea.size(); //Ponemos que el numero de columnas va a ser el tamaño de la linea que hemos cogido.
		sokoban.posFJ = 0; //Ponemos a 0 ciertos datos
		sokoban.posCJ = 0;
		sokoban.nCajasTotal = 0;
		sokoban.nCajasOK = 0;
		while (linea != "" && !fichero.eof()) { //Entramos en el bucle que se encarga de cargar el nivel
			while (i < linea.size()) { //Hacemos un bucle para que vaya cargando la linea
				aux = casilla(linea[i]); //Convertimos lo que tenemos en linea[i] en el valor compatible con tCasilla
				switch (aux) //Y en funcion de lo que haya hacemos las operaciones pertinentes
				{
				case Jugador:
					sokoban.posFJ = j;
					sokoban.posCJ = i;
					break;
				case Caja:
					sokoban.nCajasTotal++;
					break;
				}
				sokoban.tablero[j][i] = aux; //Aqui actualizamos el array
				i++;
			}
			while (i < MAX){
				sokoban.tablero[j][i] = Nada; //Aqui llenamos el resto de espacios con nada, que nos vendrá bien para luego cuando tengamos que dibujar el tablero
				i++;
			}
			if (sokoban.nColumna < linea.size()) sokoban.nColumna = linea.size(); //Con esto actualizamos el numero de columnas
			j++;
			getline(fichero, linea);
			i = 0;
		}
		sokoban.nFila = j - 1; //Aqui ponemos el numero de filas, que lo haremos con el contador j.
		resultado = true;
	}

	return resultado;
}

//Lectura de teclas especiales
tTecla leerTecla(){
	int dir;
    tTecla tecla;
    cin.sync();
    dir = _getch();
    if (dir == 0xe0) {
        dir = _getch();
        switch (dir){
            case 72:
                tecla = arriba;
                break;
            case 80:
                tecla = abajo;
                break;
            case 77:
                tecla = derecha;
                break;
            case 75:
                tecla = izquierda;
                break;
            default:
                tecla = nada;
                break;
        }
    }
	else if (dir == 100 || dir == 68) tecla = back;
    else if (dir == 27) tecla = salir;
    else tecla = nada;
    return tecla;
}

//Movimientos del tablero
void hacerMovimiento(tJuego &juego, tTecla tecla){
	/*	Esta funcion está dividida en otras 2 funciones para reducir la cantidad de codigo a utilizar.
		Aqui todo lo que hacemos es dar los valores a las funciones que realmente se encargan de hacer el movimiento.*/
	int i, j, futuroI, futuroI2, futuroJ, futuroJ2;
	j = juego.sokoban.posCJ;
	i = juego.sokoban.posFJ;
	if (tecla != back && tecla != nada) {
		switch (tecla) {
		case derecha:
			futuroJ = j + 1;
			futuroJ2 = futuroJ + 1;
			futuroI = i;
			futuroI2 = i;
			break;

		case izquierda:
			futuroJ = j - 1;
			futuroJ2 = futuroJ - 1;
			futuroI = i;
			futuroI2 = i;
			break;

		case arriba:
			futuroJ = j;;
			futuroJ2 = j;
			futuroI = i - 1;
			futuroI2 = futuroI - 1;
			break;

		case abajo:
			futuroJ = j;;
			futuroJ2 = j;
			futuroI = i + 1;
			futuroI2 = futuroI + 1;
			break;
	}

		if (comprobarMovimiento(juego, futuroI, futuroI2, futuroJ, futuroJ2) ) {
			actualizarHistorial(juego);
			realizarMovimiento(juego, i, j, futuroI, futuroI2, futuroJ, futuroJ2);
			juego.sokoban.posFJ = futuroI;
			juego.sokoban.posCJ = futuroJ;
			juego.numMovimientos++;
		}
	}
	else if(tecla == back){
		if (deshacerMovimiento(juego)) juego.numMovimientos--;
	}
}

void realizarMovimiento(tJuego &juego, const int i, const int j, const int futuroI, const int futuroI2, const int futuroJ,const int futuroJ2) {
	/*	Esta es una funcion extra que he creado para que el codigo que utilizo para hacer el movimiento sea reutilizable, en ella cogemos los valores
		de cada movimiento, que le he dado en la funcion principal de hacer el movimiento en funcion de la direccion, y ya con ellos se encarga de mover
		el objeto.*/
	switch (juego.sokoban.tablero[futuroI][futuroJ]) { //En funcion de lo que hay justo despues
	case Caja: //Si hay una caja se tiene que comprobar lo que hay despues de la caja
		switch (juego.sokoban.tablero[futuroI2][futuroJ2]) {
		case Libre: //Si despues de la caja tenemos un hueco libre
			juego.sokoban.tablero[futuroI2][futuroJ2] = Caja;
			juego.sokoban.tablero[futuroI][futuroJ] = Jugador;
			break;
		case DestinoL://Si despues de la caja hay un destino libre
			juego.sokoban.tablero[futuroI2][futuroJ2] = DestinoC; //Colocamos una caja en el destino, asi que sumamos uno al contador de cajasOK
			juego.sokoban.tablero[futuroI][futuroJ] = Jugador;
			juego.sokoban.nCajasOK++;
			break;
		}
		break;

	case DestinoC: //Si la caja que empujamos ya esta en el destino
		switch (juego.sokoban.tablero[futuroI2][futuroJ2]) {
		case Libre: //despues hay un hueco libre
			juego.sokoban.tablero[futuroI2][futuroJ2] = Caja;
			juego.sokoban.tablero[futuroI][futuroJ] = DestinoJ;
			juego.sokoban.nCajasOK--; //Quitamos una caja de un destino asi que bajamos el contador
			break;
		case DestinoL: //despues hay un destino libre
			juego.sokoban.tablero[futuroI2][futuroJ2] = DestinoC; //Aqui no añadimos un contador porque aunque metamos una en el destino, quitamos otra de un destino anterior.
			juego.sokoban.tablero[futuroI][futuroJ] = DestinoJ;
			break;
		}
		break;
	case Libre:
		juego.sokoban.tablero[futuroI][futuroJ] = Jugador; //Estos son los casos sencillos
		break;
	case DestinoL:
		juego.sokoban.tablero[futuroI][futuroJ] = DestinoJ;
		break;
	}
	switch (juego.sokoban.tablero[i][j]) { //Aqui se cambia la posicion original del jugador
	case Jugador:
		juego.sokoban.tablero[i][j] = Libre;
		break;
	case DestinoJ:
		juego.sokoban.tablero[i][j] = DestinoL;
		break;
	}
}

bool comprobarMovimiento(const tJuego &juego, const int futuroI, const int futuroI2, const int futuroJ, const int futuroJ2) {
	//	Esta funcion se encarga de comprobar si un movimiento es posible o no, con esta funcion he conseguido facilitarme la vida a la hora de hacer un movimiento.

	bool OK = false;
	
				switch (juego.sokoban.tablero[futuroI][futuroJ]) {
				case Muro:
					OK = false;
					break;
				case Caja:
				case DestinoC:
					switch (juego.sokoban.tablero[futuroI2][futuroJ2]) {
					case DestinoL:
					case Libre:
						OK = true;
						break;
					default:
						OK = false;
						break;
					}
					break;
				default:
					OK = false;
					break;
				case DestinoL:
				case Libre:
					OK = true;
					break;
				}
			

		
	return OK;
}

//Funciones version 2
bool bloqueado(const tJuego &juego){
	/*	He aqui la funcion bloqueado, como he decidido hacerla ha sido mediante un juego de variables booleanas que digan si hay algo
		en las proximidades de la caja, y que una vez estas tengan su valor se lse comprueba si la caja esta bloqueada
		o no.*/
	int i = 1, j = 1;
	bool blq = false, up = false, down = false, left = false, right = false;

	while ((juego.sokoban.nFila) > j && blq == false){
		while ((juego.sokoban.nColumna) > i && blq == false){
			if (juego.sokoban.tablero[j][i] == Caja){
				if (juego.sokoban.tablero[j][i - 1] == Muro)
					left = true;
				if (juego.sokoban.tablero[j][i + 1] == Muro)
					right = true;
				if (juego.sokoban.tablero[j - 1][i] == Muro)
					up = true;
				if (juego.sokoban.tablero[j + 1][i] == Muro)
					down = true;
				if ((up == true && left == true) || (up == true && right == true) || (down == true && right == true) || (down == true && left == true))
					blq = true;
				else blq = false;
			}
			right = false;
			left = false;
			down = false;
			up = false;
			i++;
		}
		j++;
		i = 1;
	}
	return !blq;
}

bool deshacerMovimiento(tJuego &juego) {
	bool resultado = true;
	if (juego.historia.cont > 0) {
		juego.historia.cont--;
		juego.sokoban = juego.historia.historial[juego.historia.cont];
		
		resultado = true;
	}
	else resultado = false;
	return resultado;
}

void actualizarHistorial(tJuego &juego) {
	/*	Esta funcion se encarga de actualizar el historial siempre que hacemos un movimiento.*/
	if (juego.historia.cont <= 9) {
		juego.historia.historial[juego.historia.cont] = juego.sokoban;
		juego.historia.cont++;
	}
	else {
		for (int i = 0; i < 9; i++) juego.historia.historial[i] = juego.historia.historial[i + 1];
		juego.historia.historial[9] = juego.sokoban;
	}
}

//Funciones version 3
void guardaInfo(const tInfo &info){
	
	
	ofstream file;
	file.open(info.nombre);
	if (file.is_open()){
		for (int i = 0; i < info.numPartidas; i++){
			file << info.exitos[i].nFichero << " "
				<< info.exitos[i].nivel << " "
				<< info.exitos[i].numMovimientos << endl;
		}
		file.close();
	}
}

bool cargarInfoJugador(tInfo &info){
	
	info.numPartidas = 0;
	bool resultado;
	ifstream file;
	file.open(info.nombre);
	if (file.is_open()) {
		while (!file.eof() && info.nombre != " "){
			file >> info.exitos[info.numPartidas].nFichero
				>> info.exitos[info.numPartidas].nivel 
				>> info.exitos[info.numPartidas].numMovimientos;
			info.numPartidas++;
		}
		info.numPartidas--;
		resultado = true;
		file.close();
	}
	else resultado = false;
	return resultado;

}

bool operator==(tPartida izquierda, tPartida derecha) {
	return (izquierda.nivel == derecha.nivel && izquierda.nFichero == derecha.nFichero);
}

bool operator<(tPartida izquierda, tPartida derecha) {
	return (izquierda.nFichero < derecha.nFichero || izquierda.nFichero == derecha.nFichero &&
		izquierda.nivel < derecha.nivel);
}

void modificarInfo(tInfo &info, const tJuego &juego){
	/*	Como el orden me lo pone a la inversa pero no consigo tocar los operadores sin que se me fastidie la compilacion
		voy a hacer que se invierta el array para que me lo coloque de forma incorrecta, para que luego lo vuelva a girar y quede colocado correctamente.
		Esto lo podría arreglar con un poco mas de tiempo, pero carezco de el.*/
	darLaVuelta(info);
	int i = 0, j = info.numPartidas;
	tPartida actual;
	actual.nFichero = juego.nFichero;
	actual.nivel = juego.nivel;
	actual.numMovimientos = juego.numMovimientos;
	while ( actual < info.exitos[i]) i++;
	if (info.exitos[i] == actual) {
		if (info.exitos[i].numMovimientos > actual.numMovimientos) info.exitos[i].numMovimientos = actual.numMovimientos;
	}
	else{
		if (j < 99){
		while (i <= j && j < 99) {
			info.exitos[j + 1] = info.exitos[j];
			j--;
		}
		info.exitos[i] = actual;
		info.numPartidas++;
		}
		else {
			j = 98;
			while (i <= j) {
				info.exitos[j + 1] = info.exitos[j];
				j--;
			}
			info.exitos[i] = actual;
		}
	}

	darLaVuelta(info);
}

void darLaVuelta(tInfo &info) {
	/*	Esta funcion la añado para cambiar el orden del array ya que se guarda de mayor a menor en lugar que de menos a mayor.*/
	tExitos Aux;
	int j = info.numPartidas - 1;
	for (int i = 0; i < info.numPartidas; i++) {
		Aux[i] = info.exitos[j];
		j--;
	}
	for (int i = 0; i < info.numPartidas; i++) info.exitos[i]= Aux[i];
}

void mostrarInfo(const tInfo &info){
	string nombre = info.nombre;
	int i = info.nombre.find(".txt");
	nombre.erase(i, 4);
	cout << "Usuario: " << nombre << endl
		<< "Tus partidas son:\n";
	for (int i = 0; i < info.numPartidas; i++){
		cout << "	" << info.exitos[i].nFichero
			<< " " << info.exitos[i].nivel
			<< " " << info.exitos[i].numMovimientos << endl;
	}
}

//Otros

tCasilla casilla(char valCasilla){
	tCasilla C;
	
	switch (valCasilla){
	case '@':
		C = Jugador;
		break;
	case '#':
		C = Muro;
		break;
	case ' ':
		C = Libre;
		break;
	case '.':
		C = DestinoL;
		break;
    case '+':
    	C = DestinoJ;
       	break;
    case '*':
       	C = DestinoC;
       	break;
	case '$':
		C = Caja;
		break;
	}
	
	return C;
}

void opcion1(tJuego &juego, tInfo &info){
	/* Para quitarle peso al main, he decidido poner las opciones como funciones a parte.*/
	tTecla dir;
	inicializa(juego);
	if (cargarJuego(juego)){
		dibujar(juego);
		dir = leerTecla();
		while (juego.sokoban.nCajasOK != juego.sokoban.nCajasTotal && dir != salir) {
			hacerMovimiento(juego, dir);
			dibujar(juego);
			if (!bloqueado(juego)) cout << "Ojo, estas bloqueado, vuelve atras ASAP \n";
			if (juego.sokoban.nCajasOK == juego.sokoban.nCajasTotal) {
				modificarInfo(info, juego);
				cout << "ENHORABUENA \nYa has colocado todas las cajas. \n";
			}
			dir = leerTecla();
		}
		
	}
	else cout << "No se ha podido cargar el juego.\n";

}

int main(){
	
	unsigned int opcion;
	tJuego juego;
	tInfo info;
	cout << "Bienvenido a SOKOBAN (Marca registrada).\nCual es su nombre?\n";
	cin >> info.nombre;
	info.nombre = info.nombre + ".txt";
	if (!cargarInfoJugador(info)) cout << "No hay ningun archivo con tu nombre. \nSupera un nivel para ir aumentando tu historia.(2 historia 1 jugar 0 salir) \n";	
	else cout << "Quieres jugar o quieres consultar tu informacion? (2 historia 1 jugar 0 salir).\n";
	cin >> opcion;
	while (opcion != 0){
		if ((opcion > 2) && (opcion < 0)) cout << "La opcion elegida no es valida, vuelve a probar\n";
		if (opcion == 1) opcion1(juego, info);
		else if (opcion == 2){
			system("cls");
			mostrarInfo(info);
		}
		cout << "Has vuelto al menu principal, que quieres hacer?\n(2 historia, 1 jugar, 0 salir)\n";
		cin >> opcion;
	}
	guardaInfo(info);
	cout << "\nSe han guardado tus datos.\nNOS VEMOS\n";
	system("pause");
}