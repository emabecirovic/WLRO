typedef int bool;
enum{false, true};




bool remoteControl = false;  // Change to Port connected to switch

/************BUSS**********************/
char start_request = 0;
char dummy;

// Förenklingar för enklarekodning
char front = 0b00000001;
char rightfront = 0b00000010;
char rightback = 0b00000011;
char leftfront = 0b00000100;
char leftback = 0b00000101;
char traveldist = 0b00000110;
char gyro = 0b00000111;
char gyrostop = 0b10000000;
char RFID = 0b00001000;
char direction = 0b00001001;
char rightspeed = 0b00001010;
char leftspeed = 0b00001011;
char stop = 0x00; //Stop bit
//Control signals
char right = 1;
char left = 2;
char turn = 3;
char turnstop = 4;

// Delayer för busskomm
int time = 200;
char start_request;

// Array för värden från buss
volatile unsigned char storedValues[11];


/******************REGLERING************************/
float sensor1r, sensor2r, sensordiff, sensorfront, sensorleft, sensorright;
volatile float sensormeanr;
volatile float sensormeanr_old;
int K;
long Td;
volatile float rightpwm;
volatile float leftpwm;
volatile float distance = 0; // Avlagdsträcka

long overflow = 0; // Räknat för långt
long dt = 0;
int timer = 0;

char speed = 110;

// Flaggor för regulateright
int firstRR=1;
char driven=0;

/**************POSITION******************/
char mydirection = 2; //1=X+ ; 2=Y+ ; 3=X- ; 4=Y-
unsigned int myposX=0; //Robotens position i X-led
unsigned int myposY=0; //Robotens position i Y-led
unsigned int startpos[2]={15,0}; //Startpositionen sätts till mitten på nedre långsidan
volatile float posdistance = 0;



/***************FLAGGOR FÖR MAIN******************/
bool start = 1; //vi står i startpositionen

bool finished=0; //1 då hela kartan utforskad
bool onelap=0; //1 då yttervarvet körts
bool home=0; //1 då robten återvänt till startposition
bool awaydone, zigzagdone, findemptydone, getinpos;

bool zzleftturn = true; // Till första toppsvängen i sicksacksak
bool first = true; // Till första bottensväng i sicksacksak

bool drivetoY = true; // Y-led är prioriterad riktining om sant i driveto

int firstzero; //Första nollan om man läser matrisen uppifrån och ned

char room[29][15]; //=.... 0=outforskat, 1=vägg, 2=öppen yta




/************FUNKTIONER I ORDNING ENL .c*************/
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















