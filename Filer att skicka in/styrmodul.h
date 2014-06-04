/*
* FILNAMN: styrmodul.h
* PROGRAMMERARE: Grupp 9 - WLRO
* DATUM: 2014-06-04
*
* .h-fil till styrmodulens ATmega1284P
*
* 
*/

typedef int bool;
enum{false, true};

bool remoteControl = false; // Change to Port connected to switch

/************BUSS**********************/
char dummy;

// Förenklingar för enklarekodning
const char front = 0b00000001;
const char rightfront = 0b00000010;
const char rightback = 0b00000011;
const char leftfront = 0b00000100;
const char leftback = 0b00000101;
const char traveldist = 0b00000110;
const char gyro = 0b00000111;
const char gyrostop = 0b10000000;
const char RFID = 0b00001000;
const char RFIDstop = 0b01010101;
const char direction = 0b00001001;
const char rightspeed = 0b00001010;
const char leftspeed = 0b00001011;
const char firstdone = 0b00001100;
const char alrdyDone = 0b00001101;

const char arraytransmit = 0b00001111;
const char updateroom = 0b00010000;
const char stop = 0x00; //Stop byte


//Control signals
const char turn = 3;
const char turnstop = 4;
const char trstraight = 5;
const char findzero = 6;
const char update = 7;

// Delayer för busskomm
const int time = 200;
char start_request = 0;

// Array för värden från buss
volatile unsigned char storedValues[13];


/******************REGLERING************************/
volatile float sensor1r, sensor2r, sensordiff, sensorfront, sensorleft, sensorright;
volatile float sensormeanr;
volatile float sensormeanr_old;
float K;
long Td;
volatile float rightpwm;
volatile float leftpwm;
volatile float distance = 0; // Avlagdsträcka

char speed = 60;

volatile char turnisDone = 0;


// Flaggor för regulateright
int firstRR=1;
char startregulate = 0;

/**************POSITION******************/
volatile char mydirection = 1; //1=X+ ; 2=Y+ ; 3=X- ; 4=Y-
volatile unsigned int myposX=15; //Robotens position i X-led
volatile unsigned int myposY=1; //Robotens position i Y-led
const unsigned int startX = 15;
const unsigned int startY = 1; //Startpositionen sätts till mitten på nedre långsidan

volatile unsigned int RFIDX = 42;
volatile unsigned int RFIDY = 42;

volatile float posdistance = 0;
volatile char foundRFID = 0;

/*************************LCD***********************/

const char lcd0 = 0b00110000;
const char lcd1 = 0b00110001;
const char lcd2 = 0b00110010;
const char lcd3 = 0b00110011;
const char lcd4 = 0b00110100;
const char lcd5 = 0b00110101;
const char lcd6 = 0b00110110;
const char lcd7 = 0b00110111;
const char lcd8 = 0b00111000;
const char lcd9 = 0b00111001;
const char lcda = 0b01000001;
const char lcdb = 0b01000010;
const char lcdc = 0b01000011;
const char lcdd = 0b01000100;
const char lcde = 0b01000101;
const char lcdf = 0b01000110;
const char lcdspace = 0b00100000;


/*******************FJÄRRSTYRT****************/
char button = 0x00;

/***************FLAGGOR FÖR MAIN******************/
volatile bool start = 1; //vi står i startpositionen

volatile bool finished=0; //1 då hela kartan utforskad
volatile bool onelap=0; //1 då yttervarvet körts
volatile bool home=0; //1 då robten återvänt till startposition

//secondlap
volatile bool searched = false;
volatile bool controlisland = false;

volatile int storeposX = 42;
volatile int storeposY = 42;
volatile char storedirection = 0;

//char room[29][15]; //=.... 0=outforskat, 1=vägg, 2=öppen yta




/************FUNKTIONER I ORDNING ENL .c*************/
void initiate_variables(); // Variabel initation vid reset av proc

void initiate_request_timer(); // Busstimer


void Initiation(); // styr och LCD

/****************LCD*******************/
void writechar(unsigned char data); // Skriv tecken på LCD
void shift(int steps); // Skifta pekaren höger på LCD ett antal steg
void shiftcursorleft(); // skifta pekaren till vänster
char what_lcd_number(char number); // Look up för Hex till LCD
void print_on_lcd(char number); // Skriv ett nummer på LCD
void setcursor(char place); // Placera pekaren på specifik plats


/*****************BUSS********************/
void MasterInit(void);
void MasterTransmit(char cData); // Skicka en byte på bussen

// Egen delay till bussen som skrivits för att delay bibloteket
// inte funkar när man programmerar utan optimering.
void bussdelay();

void transmit(); // Skicka sensorvärden från sensor till styr till komm

// Funktioner som används för att skicka sensorvärden/rapporterar händelser 
// mellan moduler.
void TransmitSensor(char invalue); //Skickar data mellan sensor och styr
void TransmitComm(char invalue); // Skickar data mellan komm och styr

/*******************FJÄRRSTYRNING***************/
void remotecontrol();

/******************POSITIONERING******************/
//Variabel för att variera antal sektorer som representerar 40 cm
volatile char n = 0;

// Se till att uppdaterapositionen efter 40cm
void traveledDist();

// Uppdatera positionen på karta samt gör åtgärder som krävs i mitten av en ruta
void updatepos(); 


/*****************STYRNING********************/
void stopp(); // stoppa motorer

void rotate90left();
void rotate90right();

void rotateleft();
void rotateright();

/****************KONVERTERING*************/
// konverterar digitala värden till cm
float shortsensor(unsigned char sensorvalue);
float longsensor(unsigned char sensorvalue);

/******************REGLERING & AVSÖKNING**************/
void straight(); // räta upp roboten mot vägg

void driveF();	// kör framåt
void drive(float dist); // kör en viss sträcka
void drivefromstill(float dist); // kör en viss sträcka från stillastående

void regulateright(); // reglerar efter högervägg
void firstlap();	// Process för första varvet

// Andra varvet
bool alreadyDone(); // Har vi varit vid väggen vänster sensor ser
void gotoIsland(); // Åk ut till ö
void storepos(); // skapar startpunkt för att åka runt ö
void throwpos(); // kastar den punkten
void secondlap(); // Process för andra varvet
void Island(); // kör runt ön
