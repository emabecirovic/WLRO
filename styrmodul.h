

typedef int bool;
enum{false, true};




bool remoteControl = false;  // Change to Port connected to switch

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
const char direction = 0b00001001;
const char rightspeed = 0b00001010;
const char leftspeed = 0b00001011;
const char stop = 0x00; //Stop bit

//Control signals
const char right = 1;
const char left = 2;
const char turn = 3;
const char turnstop = 4;
const char trstraight = 5;

// Delayer för busskomm
const int time = 200;
char start_request = 0;

// Array för värden från buss
volatile unsigned char storedValues[11];


/******************REGLERING************************/
float sensor1r, sensor2r, sensordiff, sensorfront, sensorleft, sensorright;
volatile float sensormeanr;
volatile float sensormeanr_old;
float K;
long Td;
volatile float rightpwm;
volatile float leftpwm;
volatile float distance = 0; // Avlagdsträcka

long overflow = 0; // Räknat för långt
long dt = 0;
int timer = 0;

char speed = 50;

volatile char turnisDone = 0;


// Flaggor för regulateright
int firstRR=1;
char startregulate = 0;

/**************POSITION******************/
char mydirection = 2; //1=X+ ; 2=Y+ ; 3=X- ; 4=Y-
unsigned int myposX=15; //Robotens position i X-led
unsigned int myposY=0; //Robotens position i Y-led
unsigned int startpos[2]={15,0}; //Startpositionen sätts till mitten på nedre långsidan
volatile float posdistance = 0;

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

bool finished=0; //1 då hela kartan utforskad
bool onelap=0; //1 då yttervarvet körts
bool home=0; //1 då robten återvänt till startposition
bool awaydone, zigzagdone, findemptydone, getinpos;

bool zzleftturn = true; // Till första toppsvängen i sicksacksak
bool zzfirst = true; // Till första bottensväng i sicksacksak

bool drivetoY = true; // Y-led är prioriterad riktining om sant i driveto

int firstzero; //Första nollan om man läser matrisen uppifrån och ned

char room[29][15]; //=.... 0=outforskat, 1=vägg, 2=öppen yta




/************FUNKTIONER I ORDNING ENL .c*************/
void initiate_variables();

void initiate_request_timer(); //D-regulation
void initiate_timer(); // Busstimer


void Initiation(); // styr och LCD
void writechar(unsigned char data);

/*****************BUSS********************/
void MasterInit(void);
void MasterTransmit(char cData);

void bussdelay();

void transmit();
void TransmitSensor(char invalue);
void TransmitComm();

/*******************FJÄRRSTYRNING***************/
void remotecontrol();

/******************POSITIONERING******************/
void updatepos();

/***********************KARTA************************/
void setwall(int x,int y);
void updatemap();
void extended_wall();

/*****************STYRNING********************/
void stopp();
void driveF();
void driveDist(float dist);
void drivefromstill(float dist);
void straight();

void temporary90right();
void temporary90left();

void rotate90left();
void rotate90right();

void leftturn();
void rotateleft();
void rotateright();

/****************KONVERTERING*************/
float sidesensor(unsigned char sensorvalue);
float frontsensor(unsigned char sensorvalue);

/******************REGLERING & AVSÖKNING**************/
void firstlap();
void regulateright();

void away();
void zigzag();
int * findfirstzero();
void driveto(int pos[2]);
void findempty();

void returntostart(); // Kolla om vi ska ha den
