/*****************************************************************************
 * Project Title: Cira Center Tetris
 *          Team: Luke Gary, Ross Kessler
 *   Description: Port of Valentin Ivanov Tetris to Arduino Due and parallel WS2812 Strips
 *		Hardware: Arduino Due, x20 30 element WS2812 strips
 *****************************************************************************/

#include <Adafruit_NeoPixel.h>     // library for addressing the LED Strips 
#include <WS2812_Definitions.h>  // HTML Color definitions for WS2812 controllers, 32bit unsigned integers
#include <stdint.h>                // defenitions for fast integer types
#include <DueTimer.h> 			   // Hardware timer wrapper for polling Serial Buffer &&|| the requested inputs 
#include <Scheduler.h>
#include <KeyboardController.h>

#define PIN0 			44
#define PIN1 			45
#define PIN2 			46
#define PIN3 			47
#define PIN4 			48
#define PIN5 			49
#define PIN6 			50
#define PIN7 			51
#define PIN8 			52
#define PIN9 			53

#define LED_ROWS        30 
#define LED_COLUMNS     10
#define GAME_ROWS       15
#define GAME_COLUMNS    10
#define NUM_PIXELS		150

#define leftBound    9
#define rightBound   0
#define topBound     0
#define bottomBound  14

#define COLOR_O 41  // Yellow
#define COLOR_T 145 // Purple
#define COLOR_I 210  // Light Blue
#define COLOR_L 68  // Orange
#define COLOR_J 169 // Dark Blue
#define COLOR_S 251 // Green
#define COLOR_Z 100 // Red

// Main Colors for pieces
uint8_t colors[7] = {
	COLOR_O,
	COLOR_I,
	COLOR_Z,
	COLOR_S,
	COLOR_L,
	COLOR_J,
	COLOR_T
};

// Shapes for Game Pieces
int8_t blocks[7][4][4][2]= //[block type] [Rotation] [cell number] [cell cordinates]
	{
        { // SQUARE
			{  {0,-1}, {1,-1}, {0,0}, {1,0} },//Angle of Piece = 0*
			{  {0,-1}, {1,-1}, {0,0}, {1,0} },//Angle of Piece = 90*
			{  {0,-1}, {1,-1}, {0,0}, {1,0} },//Angle of Piece = 180*
			{  {0,-1}, {1,-1}, {0,0}, {1,0} } //Angle of Piece = 270*
        },
  
        { // BAR
			{  {-1,0}, {0,0}, {1,0}, {2,0} },//Angle of Piece = 0*
			{  {0,-1}, {0,0}, {0,1}, {0,2} },//Angle of Piece = 90*
			{  {-1,0}, {0,0}, {1,0}, {2,0} },//Angle of Piece = 180*
			{  {0,-1}, {0,0}, {0,1}, {0,2} } //Angle of Piece = 270*
        },
		
		{ // S
			{  {0,-1},  {1,-1}, {-1,0}, {0,0} },//Angle of Piece = 0*
			{  {-1,-1}, {-1,0}, {0,0 }, {0,1} },//Angle of Piece = 90*
			{  {0,-1},  {1,-1}, {-1,0}, {0,0} },//Angle of Piece = 180*
			{  {-1,-1}, {-1,0}, {0,0 }, {0,1} } //Angle of Piece = 270*
        },		
		
        { // Z
			{  {-1,-1}, {0,-1}, {0,0}, {1,0} },//Angle of Piece = 0*
			{  {1,-1},  {0,0},  {1,0}, {0,1} },//Angle of Piece = 90*
			{  {-1,-1}, {0,-1}, {0,0}, {1,0} },//Angle of Piece = 180*
			{  {1,-1},  {0,0},  {1,0}, {0,1} } //Angle of Piece = 270*
        },   
		
		{ // J
			{  {-1, 0}, {0, 0}, {1,0}, {1, 1} },//Angle of Piece = 0*
			{  {0, -1}, {1,-1}, {0,0}, {0, 1} },//Angle of Piece = 90*
			{  {-1,-1}, {-1,0}, {0,0}, {1, 0} },//Angle of Piece = 180*
			{  {0, -1}, {0, 0}, {0,1}, {-1,1} } //Angle of Piece = 270*                      
        },
        
        { // L
			{  {-1,0 }, {0, 0}, {1,0}, {-1,1} },//Angle of Piece = 0*
			{  {0,-1 }, {0, 0}, {0,1}, {1,1} },//Angle of Piece = 90*
			{  {1,-1 }, {-1,0}, {0,0}, {1,0} },//Angle of Piece = 180*
			{  {-1,-1}, {0,-1}, {0,0}, {0,1} } //Angle of Piece = 270*  
        },
		
        { // T
			{  {-1, 0}, {0, 0}, {1,0}, {0,1} },//Angle of Piece = 0*
			{  {0, -1}, {0, 0}, {1,0}, {0,1} },//Angle of Piece = 90*
			{  {0, -1}, {-1,0}, {0,0}, {1,0} },//Angle of Piece = 180*
			{  {0, -1}, {-1,0}, {0,0}, {0,1} } //Angle of Piece = 270*
        }         
    };

uint8_t frame[LED_COLUMNS][LED_ROWS] = { 
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0} ,
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0} ,
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0} ,
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0} ,
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0} ,
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0} ,
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0} ,
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0} ,
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0} ,
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}   
};

uint8_t gameField[GAME_COLUMNS][GAME_ROWS] = {
				{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
				{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
				{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
				{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
				{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
				{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
				{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
				{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
				{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
				{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
};

uint8_t TETRIS_LOGO[GAME_COLUMNS][GAME_ROWS] = {
				{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
				{0,1,0,0,0,0,0,2,0,2,2,2,0,0,0},
				{0,1,1,1,1,1,0,2,0,2,0,2,0,0,0},
				{0,1,0,0,0,0,0,2,2,2,0,2,0,0,0},
				{0,2,0,0,0,2,0,0,0,0,0,0,0,0,0},
				{0,2,0,2,0,2,0,1,1,1,1,1,0,0,0},
				{0,2,2,2,2,2,0,0,0,0,0,0,0,0,0},
				{0,1,0,0,0,0,0,2,2,0,2,2,0,0,0},
				{0,1,1,1,1,1,0,2,0,2,0,0,0,0,0},
				{0,1,0,0,0,0,0,2,2,2,2,2,0,0,0}
};
uint8_t NFS_LOGO[GAME_COLUMNS][GAME_ROWS] = {
				{0,3,0,3,3,3,0,0,0,0,0,0,0,0,0},
				{0,3,0,3,0,3,0,0,0,0,0,0,0,0,0},
				{0,3,3,3,0,3,0,0,0,0,0,0,0,0,0},
				{0,2,0,0,0,0,0,0,0,0,0,0,0,0,0},
				{0,2,0,2,0,0,0,0,0,0,0,0,0,0,0},
				{0,2,2,2,2,2,0,0,0,0,0,0,0,0,0},
				{0,1,1,1,1,1,0,0,0,0,0,0,0,0,0},
				{0,0,0,1,0,0,0,0,0,0,0,0,0,0,0},
				{0,0,1,0,0,0,0,0,0,0,0,0,0,0,0},
				{0,1,1,1,1,1,0,0,0,0,0,0,0,0,0}
};

uint8_t staticField[GAME_COLUMNS][GAME_ROWS] = {
				{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
				{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
				{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
				{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
				{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
				{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
				{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
				{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
				{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
				{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
};
  
Adafruit_NeoPixel column0 = Adafruit_NeoPixel(60, PIN0, NEO_GRB + NEO_KHZ800); // if we start to observe processor load, go to 400 KHz
Adafruit_NeoPixel column1 = Adafruit_NeoPixel(60, PIN1, NEO_GRB + NEO_KHZ800); // if we start to observe processor load, go to 400 KHz
Adafruit_NeoPixel column2 = Adafruit_NeoPixel(60, PIN2, NEO_GRB + NEO_KHZ800); // if we start to observe processor load, go to 400 KHz
Adafruit_NeoPixel column3 = Adafruit_NeoPixel(60, PIN3, NEO_GRB + NEO_KHZ800); // if we start to observe processor load, go to 400 KHz
Adafruit_NeoPixel column4 = Adafruit_NeoPixel(60, PIN4, NEO_GRB + NEO_KHZ800); // if we start to observe processor load, go to 400 KHz
Adafruit_NeoPixel column5 = Adafruit_NeoPixel(60, PIN5, NEO_GRB + NEO_KHZ800); // if we start to observe processor load, go to 400 KHz
Adafruit_NeoPixel column6 = Adafruit_NeoPixel(60, PIN6, NEO_GRB + NEO_KHZ800); // if we start to observe processor load, go to 400 KHz
Adafruit_NeoPixel column7 = Adafruit_NeoPixel(60, PIN7, NEO_GRB + NEO_KHZ800); // if we start to observe processor load, go to 400 KHz
Adafruit_NeoPixel column8 = Adafruit_NeoPixel(60, PIN9, NEO_GRB + NEO_KHZ800); // if we start to observe processor load, go to 400 KHz
Adafruit_NeoPixel column9 = Adafruit_NeoPixel(60, PIN8, NEO_GRB + NEO_KHZ800); // if we start to observe processor load, go to 400 KHz

//uint8_t currentColor = 0x01;

uint8_t requestedInputs = B00000000; // byte to hold user requests, each bit represents a request, when serviced, that bit is reset to 0

uint_fast8_t currentX				= 4; 
uint_fast8_t currentY				= 2; 
uint_fast8_t currentOrientation		= 0; 
uint_fast8_t currentBlock			= 0; 
uint_fast8_t lastBlock 				= 0;

int setX = 4;
int setY = 1;

uint32_t timeStamp0  = 0;
uint32_t timeStamp1  = 0;

bool playGame			= false;
bool beatGame	   		= false;
bool gameOver		    = false;
bool autoDrop			= false;
int level       		= 1;
int dropTime    		= 700;
const int autoDropTime  = 10;
const int levelsToBeat	= 15;
int lineCounter 		= 0;
int dropDelay     		= 10000;

// stuff for the demo
unsigned long time1 = 0;
unsigned long time0 = 0;
uint8_t clearColor = 0;

uint32_t colas[11] = { FORESTGREEN, SALMON, CADETBLUE, ROSYBROWN, LIGHTGOLDENRODYELLOW, PLUM, LEMONCHIFFON, IVORY, MAGENTA, PAPAYAWHIP, KHAKI};

uint8_t demoMoves[600] {
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

unsigned long timeout = 10000;
uint8_t thisMove = 0;
uint8_t DEMO_MOVES = 600;
boolean playDemo = false;
uint16_t mov = 0;

// USBHost usb;
// KeyboardController keyboard(usb);


// game thread
void loop()
{
	//showLogos(0);
	
	rainbowCycle(random(0,5), random(0,5), 1);
	
	resetGameSettings();
	
	if(playDemo) {
		startTrissin();
	}
	while(playDemo) {
		
		demoGame();
	}
	
	if(playGame){
		emptyField();
		startTrissin();
		
		// main gameplay
		while(!beatGame && !gameOver){
			// check requests
			for(uint_fast8_t ins = 0; ins < 8; ins++) {
				if(bitRead(requestedInputs, ins) == 1){
					switch(ins) {
						case 0:
							Serial.println("up");
							// rotate block
							if(isValidMove(currentX, currentY, currentOrientation + 1 )){
								currentOrientation++;
								if(currentOrientation > 3) {
									currentOrientation = 0;
								}
							}
							//recordMove();
							bitClear(requestedInputs, ins);
						break;
						
						case 1:
							Serial.println("down");
							// auto drop block
							
							if(isValidMove(currentX, currentY + 1, currentOrientation)){
								//currentY++;
								autoDrop = true;
								//bitClear(requestedInputs, ins);
							}
							recordMove(1);
							bitClear(requestedInputs, ins);
						break;
						
						case 2:
							Serial.println("left");
							// move block left
							if(isValidMove(currentX + 1, currentY, currentOrientation)){
								currentX++;
							}
							recordMove(2);
							bitClear(requestedInputs, ins);
						break;
						
						case 3:
							Serial.println("right");
							// move block right
							if(isValidMove(currentX - 1, currentY, currentOrientation)){
								currentX--;
							}
							recordMove(3);
							bitClear(requestedInputs, ins);
						break;
						
						case 4:
							Serial.println("rotate");
							// rotate block
							if(isValidMove(currentX, currentY, currentOrientation + 1)){
								currentOrientation++;
								if(currentOrientation > 3) {
									currentOrientation = 0;
								}
							}
							recordMove(4);
							bitClear(requestedInputs, ins);
						break;
						
						case 6:
							Serial.println("gimme a new one");
							//change block type
							lastBlock = currentBlock;
							currentBlock = random(0,7);
							if(!isValidMove(currentX, currentY, currentOrientation)){
								currentBlock = lastBlock;
								Serial.println("no good bro");
							}
							//recordMove();
							bitClear(requestedInputs, ins);
						break;
						
						case 7:
							Serial.println("set block");
							setPiece();
							//mergeField();
							newPiece();
							//recordMove();
							bitClear(requestedInputs, ins);
						break;
						
					}
				}
			}
	
			
		
			// put active block on the field
			placeBlock(currentBlock, currentOrientation, currentX, currentY);
			
			
			
			checkCompletedLines(1);
			
			updateFrame();
			refreshScreen();
			
			
			//if(autoDrop) { dropDelay = autoDropTime;} else { dropDelay = dropTime;}
			if(autoDrop == true){
				Serial.println("auto dropping");
				dropDelay = autoDropTime;
			} else {
				dropDelay = dropTime;
			}
			
			timeStamp0 = millis();
			
			if((timeStamp0 - timeStamp1) > dropDelay) {
				timeStamp1 = timeStamp0;
				
				// move block down
				if(isValidMove(currentX, currentY + 1, currentOrientation)){
					Serial.println("auto MoveDown");
					currentY++;
				} else {
					autoDrop = false;
					dropDelay = dropTime;
				}
				printCoordinates();
				
			}

			checkGame();
		}
		// if you beat the game, do dis stuff
		if(beatGame) {
			Serial.println("The Game Hath Been Beaten");
			
			playGame = false;
		}
		// if you have failed, do dis
		else if (gameOver) {
			Serial.println("Game Over, Sorry Charlie!");
			
			rainbowCycle(3,20,2);
			
			delay(2000);
				
			emptyGame();
		
			showLogos(1);
			
			gameOver = false;
			playGame = false;
			
		}
	}
	//usb.Task();
}

// Arduino Main thread
// void loop() {	
// 	// usb.Task();

// 	yield();
// }

// Arduino Initialization
void setup() {

    pinMode( 44 , OUTPUT);pinMode( 49 , OUTPUT);
    pinMode( 45 , OUTPUT);pinMode( 50 , OUTPUT);
    pinMode( 46 , OUTPUT);pinMode( 51 , OUTPUT);
    pinMode( 47 , OUTPUT);pinMode( 52 , OUTPUT);
    pinMode( 48 , OUTPUT);pinMode( 53 , OUTPUT);
    
	randomSeed(analogRead(A0));
	
	column0.begin();  column0.show();
	column1.begin();  column1.show();
	column2.begin();  column2.show();
	column3.begin();  column3.show();
	column4.begin();  column4.show();
	column5.begin();  column5.show();
	column6.begin();  column6.show();
	column7.begin();  column7.show();
	column8.begin();  column8.show();
	column9.begin();  column9.show();
   
    set_frame_brightness(0xDF);
	
	Serial.begin(115200);
	
	// init timers
	Timer4.attachInterrupt(scanSerialInput);
	

	Serial.println("Testing LED Columns...");
	
	rainbowCycle(3,20,1);
	
	delay(1000);
	
	//startTrissin();
	showLogos(1);

	Serial.println("Please Connect NFS Game Controller or USB Keyboard...");

	Serial.println("Starting Cira Center...");
	// Scheduler.startLoop(ServiceCiraCenter);
	// Timer4.attachInterrupt(PollUsb);
	Timer4.start(10000); // poll serial input buffer every 15 milliseconds
}

void recordMove(uint8_t bv) {
	mov++;
	
	bitSet(demoMoves[mov], bv);
}

void demoGame() {
	time1 = millis();
	if(time1 - time0 > 250) {
		time0 = time1;
		
		for(uint_fast8_t ins = 0; ins < 8; ins++) {
					if(bitRead(demoMoves[thisMove], ins) == 1){
						switch(ins) {
							case 0:
								Serial.println("up");
								// rotate block
								if(isValidMove(currentX, currentY, currentOrientation + 1 )){
									currentOrientation++;
									if(currentOrientation > 3) {
										currentOrientation = 0;
									}
								}
								bitClear(demoMoves[thisMove], ins);
							break;
							
							case 1:
								Serial.println("down");
								// auto drop block
								
								if(isValidMove(currentX, currentY + 1, currentOrientation)){
									//currentY++;
									autoDrop = true;
									//bitClear(demoMoves, ins);
								}
								
								bitClear(demoMoves[thisMove], ins);
							break;
							
							case 2:
								Serial.println("left");
								// move block left
								if(isValidMove(currentX + 1, currentY, currentOrientation)){
									currentX++;
								}
								bitClear(demoMoves[thisMove], ins);
							break;
							
							case 3:
								Serial.println("right");
								// move block right
								if(isValidMove(currentX - 1, currentY, currentOrientation)){
									currentX--;
								}
								bitClear(demoMoves[thisMove], ins);
							break;
							
							case 4:
								Serial.println("rotate");
								// rotate block
								if(isValidMove(currentX, currentY, currentOrientation + 1)){
									currentOrientation++;
									if(currentOrientation > 3) {
										currentOrientation = 0;
									}
								}
								bitClear(demoMoves[thisMove], ins);
							break;
							
							case 6:
								Serial.println("gimme a new one");
								//change block type
								lastBlock = currentBlock;
								currentBlock = random(0,7);
								if(!isValidMove(currentX, currentY, currentOrientation)){
									currentBlock = lastBlock;
									Serial.println("no good bro");
								}
								bitClear(demoMoves[thisMove], ins);
							break;
							
							case 7:
								Serial.println("move down one");
								if(isValidMove(currentX, currentY + 1, currentOrientation)){
									currentY++;
									bitClear(demoMoves[thisMove], ins);
								}
							break;
							
						}
					}
				}
		
		// put active block on the field
		placeBlock(currentBlock, currentOrientation, currentX, currentY);
		
		checkCompletedLines(1);
		
		updateFrame();
		refreshScreen();
		
		thisMove++;
		if(thisMove >= DEMO_MOVES) {
			thisMove = 0;
			//playDemo = false;
		}
		
		timeStamp0 = millis();
			
			if((timeStamp0 - timeStamp1) > dropDelay) {
				timeStamp1 = timeStamp0;
				
				// move block down
				if(isValidMove(currentX, currentY + 1, currentOrientation)){
					Serial.println("auto MoveDown");
					currentY++;
				} 
				printCoordinates();
				
			}
	}
	
	checkGame();
	
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Display Handling Funcitons
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**************************************************************/
/* 	   Name: updateFrame()
*  Funciton: translates all game playing field data onto the frame
*  	 inputs: none
*  	returns: none
*/
/**************************************************************/
void updateFrame() {

	//push playing field data to frame matrix
    for(int x = 0; x < 10; x++) {
        for(int y = 0; y < 30; y+=2) {

			frame[x][y]   = gameField[x][y>>1];
			frame[x][y+1] = frame[x][y];
			
        } 
    }	
}

/**************************************************************/
/*     Name: refreshScreen
*  Function: Pushes all current frame data onto the display
*  	 inputs: none
*  	returns: none
*/
/**************************************************************/
void refreshScreen() {
	for(int i = 0; i < 30; i++) {
		column0.setPixelColor(i,(frame[0][i] == 0)? 0 : (frame[0][i] == 1 ) ? AZURE : (frame[0][i] == 2) ? SEAGREEN : (frame[0][i] == 3) ? MEDIUMBLUE : wheelTranslate(frame[0][i]));
		column1.setPixelColor(i,(frame[1][i] == 0)? 0 : (frame[1][i] == 1 ) ? AZURE : (frame[1][i] == 2) ? SEAGREEN : (frame[1][i] == 3) ? MEDIUMBLUE : wheelTranslate(frame[1][i]));
		column2.setPixelColor(i,(frame[2][i] == 0)? 0 : (frame[2][i] == 1 ) ? AZURE : (frame[2][i] == 2) ? SEAGREEN : (frame[2][i] == 3) ? MEDIUMBLUE : wheelTranslate(frame[2][i]));
		column3.setPixelColor(i,(frame[3][i] == 0)? 0 : (frame[3][i] == 1 ) ? AZURE : (frame[3][i] == 2) ? SEAGREEN : (frame[3][i] == 3) ? MEDIUMBLUE : wheelTranslate(frame[3][i]));
		column4.setPixelColor(i,(frame[4][i] == 0)? 0 : (frame[4][i] == 1 ) ? AZURE : (frame[4][i] == 2) ? SEAGREEN : (frame[4][i] == 3) ? MEDIUMBLUE : wheelTranslate(frame[4][i]));
		column5.setPixelColor(i,(frame[5][i] == 0)? 0 : (frame[5][i] == 1 ) ? AZURE : (frame[5][i] == 2) ? SEAGREEN : (frame[5][i] == 3) ? MEDIUMBLUE : wheelTranslate(frame[5][i]));
		column6.setPixelColor(i,(frame[6][i] == 0)? 0 : (frame[6][i] == 1 ) ? AZURE : (frame[6][i] == 2) ? SEAGREEN : (frame[6][i] == 3) ? MEDIUMBLUE : wheelTranslate(frame[6][i]));
		column7.setPixelColor(i,(frame[7][i] == 0)? 0 : (frame[7][i] == 1 ) ? AZURE : (frame[7][i] == 2) ? SEAGREEN : (frame[7][i] == 3) ? MEDIUMBLUE : wheelTranslate(frame[7][i]));
		column8.setPixelColor(i,(frame[8][i] == 0)? 0 : (frame[8][i] == 1 ) ? AZURE : (frame[8][i] == 2) ? SEAGREEN : (frame[8][i] == 3) ? MEDIUMBLUE : wheelTranslate(frame[8][i]));
		column9.setPixelColor(i,(frame[9][i] == 0)? 0 : (frame[9][i] == 1 ) ? AZURE : (frame[9][i] == 2) ? SEAGREEN : (frame[9][i] == 3) ? MEDIUMBLUE : wheelTranslate(frame[9][i]));
	}
	showframe();
}

/**************************************************************/
/*     Name: showframe()
*  Function: Shows the current frame
*    inputs: none
*   returns: none
*/
/**************************************************************/
void showframe() {
	column0.show();
	column1.show();
	column2.show();
	column3.show();
	column4.show();
	column5.show();
	column6.show();
	column7.show();
	column8.show();
	column9.show();
}
/**************************************************************/
/* 	   Name: set_frame_brightness()
*  Function: sets the global LED brighness
*  	 inputs: 1 byte for brightness, 0x00 = off, 0xFF = full brightness
*  	returns: none
*/
/**************************************************************/
void set_frame_brightness(uint8_t brite) {
       column0.setBrightness(brite);
       column1.setBrightness(brite);
       column2.setBrightness(brite);
       column3.setBrightness(brite);
       column4.setBrightness(brite);
       column5.setBrightness(brite);
       column6.setBrightness(brite);
       column7.setBrightness(brite);
       column8.setBrightness(brite);
       column9.setBrightness(brite);
}

/**************************************************************/
/*     Name: wheelTranslate()
*  Function: adjusts an 8 bit value for color to 32 bit along a color wheelTranslate
*    inputs: 1 byte for position in the color wheelTranslate
*   returns: 4 byte value for WS2812 adjusted for the color wheelTranslate
*/
/**************************************************************/
uint32_t wheelTranslate(byte Pos) {
	if(Pos < 85) {
		return column0.Color(Pos * 3, 255 - Pos * 3, 0);
    } else if(Pos < 170) {
		Pos -= 85;
        return column0.Color(255 - Pos * 3, 0, Pos * 3);
    } else {
		Pos -= 170;
		return column0.Color(0, Pos * 3, 255 - Pos * 3);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Tetris Game Functions
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void resetGameSettings() {
	dropTime = 600;
	dropDelay = dropTime;
	beatGame = false;
	lineCounter = 0;
	level = 1;

}
/**************************************************************/
/*     Name: startTrissin()
*  Function: Initialized gameplay
*  	 inputs: none
*  	returns: none
*/
/**************************************************************/
void startTrissin() {
	Serial.println("start Game");
	currentBlock = random(0,7);
	currentX = setX;
    currentY = setY;
	currentOrientation = 0;
	dropTime    = 600;
	dropDelay	= dropTime;
	lineCounter = 0;
	level       = 1;
	
	beatGame	= false;
}

/**************************************************************/
/*     Name: emptyField()
*  Function: Clears current gameplay
*  	 inputs: none
*  	returns: none
*/
/**************************************************************/
void emptyField() {
	for(uint8_t x = 0; x < 10; x++){
		for(uint8_t y = 0; y < 15; y++){
			gameField[x][y] = 0;
		}
	}
}

void emptyGame() {
for(uint8_t x = 0; x < 10; x++){
		for(uint8_t y = 0; y < 15; y++){
			gameField[x][y]   = 0;
			staticField[x][y] = 0;
		}
	}

}

void resetField(){
	//emptyField;
	mergeField();
}

/**************************************************************/
/*     Name: mergeField
*  Function: Pushes all current frame data onto the display
*  	 inputs: none
*  	returns: none
*/
/**************************************************************/
void mergeField() {
	for(uint8_t x = 0; x < 10; x++){
		for(uint8_t y = 0; y < 15; y++){
			gameField[x][y] = staticField[x][y];
		}
	}
}

/**************************************************************/
/*     Name: setPiece
*  Function: copies all current data from the game field onto the static field
*			 deletes information from game field
*  	 inputs: none
*  	returns: none
*/
/**************************************************************/
void setPiece() {
	for(uint8_t x = 0; x < 10; x++){
		for(uint8_t y = 0; y < 15; y++){
			if(gameField[x][y] != 0){
				staticField[x][y] = gameField[x][y];
				gameField[x][y] = 0;
			}
		}
	}	
}

/**************************************************************/
/*     Name: checkLines()
*  Function: Pushes all current frame data onto the display
*  	 inputs: none
*  	returns: none
*/
/**************************************************************/
void checkCompletedLines(boolean animation) {

	for(uint8_t y = 0; y < 15; y++){
		// check for complete line
		if( staticField[0][y] != 0 && staticField[1][y] != 0 && staticField[2][y] != 0 &&
			staticField[3][y] != 0 && staticField[4][y] != 0 && staticField[5][y] != 0 &&
			staticField[6][y] != 0 && staticField[7][y] != 0 && staticField[8][y] != 0 &&
			staticField[9][y] != 0                                                        ) {
			
			// clear the line that has been completed
			for(uint8_t x = 0; x < 10; x++) {
				staticField[x][y] = 0;

			}
			
			if(animation){
				frameColor(colas[clearColor++]);
				if(clearColor >= 10) {
				clearColor = 0;
				}
				delay(25);
			}
			
			// move non cleared blocks down
			for(uint8_t x = 0; x < 10; x++) {
				for(uint8_t i = y; i > 0; i--){	
					staticField[x][i] = staticField[x][i-1];
					delay(1);
				}
			}
			
			lineCounter++;
		}
	}
	
	// new level every 10 lines that are cleared
	if(lineCounter >= 4) {
		lineCounter = 0;
		Serial.println("**********Next Level**********");
		// decrease the drop time for the new level
		dropTime -= 75;
		Serial.print("Current drop delay: ");
		Serial.println(dropTime);
		level++;
		
		// if levelsToBeat have been cleared, exit the main gameplay loop
		if(level > levelsToBeat) {
			beatGame == true;
		}
	}
}

void checkGame(){
	for(uint8_t x = 0; x < 10; x++) {
		if(staticField[x][0] != 0){
			gameOver = true;
		}
	}
}

/**************************************************************/
/*     Name: checkNextMove
*  Function: 
*  	 inputs: none
*  	returns: boolean, false if next move is not valid, true if is
*/
/**************************************************************/
boolean isValidMove(uint8_t nextXOffset ,uint8_t nextYOffset, uint8_t nextOrientation) {
	boolean valid = true;
	
	// wrap around if rotated too far
	nextOrientation = ((nextOrientation > 3) ? 0 : nextOrientation); // wrap rotation around if rotated too far
	
	
	// check each of the 4 cells of the tetriminoe
	for(uint8_t cellCheck = 0; cellCheck <= 3; cellCheck++) {
	
		// check if the cell is at the bottom of the display
		if((blocks[currentBlock][nextOrientation][cellCheck][1] + nextYOffset > 14)) {
			valid = false;
			Serial.println("Bottom of Screen");
			//if(blocks[currentBlock][nextOrientation][cellCheck][1] + nextYOffset == 14){
				setPiece();
				//mergeField();
				newPiece();
			//}
			break;
		}
		
		// check left and right of cell for collision
		if((blocks[currentBlock][nextOrientation][cellCheck][0] + nextXOffset  > leftBound) || (blocks[currentBlock][nextOrientation][cellCheck][0] + nextXOffset  < rightBound)) {
			
			valid = false;
			Serial.println("Edge of Screen");
			break;
			
		}
		
		// check for vertical collisions with any static cells
		if(staticField[blocks[currentBlock][nextOrientation][cellCheck][0] + currentX][blocks[currentBlock][nextOrientation][cellCheck][1] + nextYOffset] != 0) {
		
			valid = false;
			Serial.println("vertical static block collsion");
			
			if(autoDrop == true) {
				autoDrop = false;
			}
			
			if(blocks[currentBlock][nextOrientation][cellCheck][1] + nextYOffset <= 1){
				gameOver = true;
			}
			
			setPiece();
			newPiece();
			
			break;
		}
		
		// check for horizontal collisions with any static blocks
		if(staticField[blocks[currentBlock][nextOrientation][cellCheck][0] + nextXOffset][blocks[currentBlock][nextOrientation][cellCheck][1] + currentY] != 0) {
		
			valid = false;
			Serial.println("horizontal static block collsion");
			
			break;
		}
		
	}
	
	return valid;
}

void placeBlock(uint8_t type, uint8_t rotation, uint8_t seedX, uint8_t seedY) {
	mergeField();
	for(uint8_t cell = 0; cell < 4; cell++){
		gameField[(seedX ) + blocks[type][rotation][cell][0]][(seedY ) + blocks[type][rotation][cell][1]] = colors[type];
	}
}

void newPiece() {
	currentX = setX;
	currentY = setY;
	
	//randomSeed(analogRead(A0) + analogRead(A4));
	
	currentBlock = random(0,7);
	
}

/**************************************************************/
/*     Name: renderStaticBlocks
*  Function: Pushes all current frame data onto the display
*  	 inputs: none
*  	returns: none
*/
/**************************************************************/
void renderStaticBlocks() {
	for(uint8_t x = 0; x < 10; x++) {
		for(uint8_t y = 0; y < 15; y++) {
			
		}
	}
}

/**************************************************************/
/*     Name: renderActiveBlock
*  Function: Pushes all current frame data onto the display
*  	 inputs: none
*  	returns: none
*/
/**************************************************************/
void renderActiveBlock() {

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Communication and User Input Handlers
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**************************************************************/
/* 	   Name: scanSerialInput
*  Function: wipes color across frame
*    inputs: none
*   returns: none
*/
/**************************************************************/
void scanSerialInput() {
	if(Serial.available()){
		uint8_t dataIn = Serial.read();
		
		if(dataIn == 'i') {
			playDemo = true;
			playGame = false;
		} else {
			playGame = true;
			playDemo = false;
		}
		
		//Serial.print("  ");Serial.println(dataIn, HEX);
		
		switch(dataIn) {
			case 0x77: // move up
				bitSet(requestedInputs, 0);
				//Serial.println("up");
			break;
			
			case 0x61: // move left
				bitSet(requestedInputs, 2);
				//Serial.println("left");
			break;
			
			case 0x73: // move down
				bitSet(requestedInputs, 1);
				//Serial.println("down");
			break;
			
			case 0x64: // move right
				bitSet(requestedInputs, 3);
				//Serial.println("right");
			break;
			
			case 0x72: // rotate
				bitSet(requestedInputs, 4);
				//Serial.println("rotate");
			break;
			
			case 0x63: // change block color
				bitSet(requestedInputs, 5);
			break;
			
			case 0x7A: // change block
				bitSet(requestedInputs, 6);
			break;
			
			case 0x78: // set block
				bitSet(requestedInputs, 7);
			break;
		
		}
	}
}


void printCoordinates() {
	Serial.print("x: ");Serial.print(currentX    	   );
	Serial.print("y: ");Serial.print(currentY      	   );
	Serial.print("r: ");Serial.print(currentOrientation);
	Serial.print("b: ");Serial.println(currentBlock	   );
}

void PollUsb()
{
	// Serial.println("usb polled");
	// usb.Task();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Random Animations
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**************************************************************/
/* 	   Name: colorWipe()
*  Function: wipes color across frame
*    inputs: none
*   returns: none
*/
/**************************************************************/
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint8_t i=29; i>=0; i--) {
    column0.setPixelColor(i, c + random(0x00,0xFF));//((c == 0)? c : 0));
    column1.setPixelColor(i, c + random(0x00,0xFF));//((c == 0)? c : 0));
	column2.setPixelColor(i, c + random(0x00,0xFF));//((c == 0)? c : 0));
	column3.setPixelColor(i, c + random(0x00,0xFF));//((c == 0)? c : 0));
	column4.setPixelColor(i, c + random(0x00,0xFF));//((c == 0)? c : 0));
	column5.setPixelColor(i, c + random(0x00,0xFF));//((c == 0)? c : 0));
	column6.setPixelColor(i, c + random(0x00,0xFF));//((c == 0)? c : 0));
	column7.setPixelColor(i, c + random(0x00,0xFF));//((c == 0)? c : 0));
	column8.setPixelColor(i, c + random(0x00,0xFF));//((c == 0)? c : 0));
	column9.setPixelColor(i, c + random(0x00,0xFF));//((c == 0)? c : 0));
	
    showframe();
      
    if(wait > 0){delay(wait);}else{/*nowait*/}
	
  }
  //showframe();
}

/**************************************************************/
/* 	   Name: frameColor()
*  Function: Sets the color of the entire frame
*    inputs: none
*   returns: none
*/
/**************************************************************/
void frameColor(uint32_t color) {
	for(uint16_t i=0; i<30; i++) {
		column0.setPixelColor(i, color);//((c == 0)? c : 0));
		column1.setPixelColor(i, color);//((c == 0)? c : 0));
		column2.setPixelColor(i, color);//((c == 0)? c : 0));
		column3.setPixelColor(i, color);//((c == 0)? c : 0));
		column4.setPixelColor(i, color);//((c == 0)? c : 0));
		column5.setPixelColor(i, color);//((c == 0)? c : 0));
		column6.setPixelColor(i, color);//((c == 0)? c : 0));
		column7.setPixelColor(i, color);//((c == 0)? c : 0));
		column8.setPixelColor(i, color);//((c == 0)? c : 0));
		column9.setPixelColor(i, color);//((c == 0)? c : 0));
    }
    showframe();
}

/**************************************************************/
/* This function Displays a rainbow wave on the screen
*  inputs:  cycle delay
*  returns: none
*/
/**************************************************************/
void rainbowCycle(uint8_t pattern, uint8_t wait, uint8_t nums) {
	uint16_t i, j;
	switch(pattern){
		case 0:
			for(j=255*nums; j>0; j--) { // 5 cycles of all colors on wheel
			if(playGame){break;}
				for(i=0; i< column0.numPixels(); i++) {
				if(playGame){break;}
					column0.setPixelColor(i, wheelTranslate(((i * 256 / column0.numPixels()) + j + 0x00 ) & 255));
					column1.setPixelColor(i, wheelTranslate(((i * 256 / column0.numPixels()) + j + 0x08 ) & 255));
					column2.setPixelColor(i, wheelTranslate(((i * 256 / column0.numPixels()) + j + 0x0F ) & 255));
					column3.setPixelColor(i, wheelTranslate(((i * 256 / column0.numPixels()) + j + 0x10 ) & 255));
					column4.setPixelColor(i, wheelTranslate(((i * 256 / column0.numPixels()) + j + 0x18 ) & 255));
					column5.setPixelColor(i, wheelTranslate(((i * 256 / column0.numPixels()) + j + 0x1F ) & 255));
					column6.setPixelColor(i, wheelTranslate(((i * 256 / column0.numPixels()) + j + 0x20 ) & 255));
					column7.setPixelColor(i, wheelTranslate(((i * 256 / column0.numPixels()) + j + 0x28 ) & 255));
					column8.setPixelColor(i, wheelTranslate(((i * 256 / column0.numPixels()) + j + 0x2F ) & 255));
					column9.setPixelColor(i, wheelTranslate(((i * 256 / column0.numPixels()) + j + 0x30 ) & 255));
				}
				showframe();
				//delay(wait);
			}
			break;

		case 2:
			for(j=255*nums; j>0; j--) { // 5 cycles of all colors on wheel
			if(playGame){break;}
				for(i=0; i< column0.numPixels(); i++) {
				if(playGame){break;}
					column0.setPixelColor(i, wheelTranslate(((i * 256 / column0.numPixels()) + j + 0x00 ) & 255));
					column1.setPixelColor(i, wheelTranslate(((i * 256 / column0.numPixels()) + j + 0x18 ) & 255));
					column2.setPixelColor(i, wheelTranslate(((i * 256 / column0.numPixels()) + j + 0x2F ) & 255));
					column3.setPixelColor(i, wheelTranslate(((i * 256 / column0.numPixels()) + j + 0x30 ) & 255));
					column4.setPixelColor(i, wheelTranslate(((i * 256 / column0.numPixels()) + j + 0x48 ) & 255));
					column5.setPixelColor(i, wheelTranslate(((i * 256 / column0.numPixels()) + j + 0x5F ) & 255));
					column6.setPixelColor(i, wheelTranslate(((i * 256 / column0.numPixels()) + j + 0x60 ) & 255));
					column7.setPixelColor(i, wheelTranslate(((i * 256 / column0.numPixels()) + j + 0x78 ) & 255));
					column8.setPixelColor(i, wheelTranslate(((i * 256 / column0.numPixels()) + j + 0x8F ) & 255));
					column9.setPixelColor(i, wheelTranslate(((i * 256 / column0.numPixels()) + j + 0x90 ) & 255));
				}
				showframe();
			}
			break;
			
		case 3:
			for( int j=0; j < 1 * nums; j++) { 
			if(playGame){break;}
				for( int i = 29; i >= 0; i -= 2) {
				int color = random(0x00,0xFF);
				if(playGame){break;}
					column0.setPixelColor(i, wheelTranslate(color + random(0x00,0xFF)));column0.setPixelColor(i - 1, column0.getPixelColor(i));
					column1.setPixelColor(i, wheelTranslate(color + random(0x00,0xFF)));column1.setPixelColor(i - 1, column1.getPixelColor(i));
					column2.setPixelColor(i, wheelTranslate(color + random(0x00,0xFF)));column2.setPixelColor(i - 1, column2.getPixelColor(i));
					column3.setPixelColor(i, wheelTranslate(color + random(0x00,0xFF)));column3.setPixelColor(i - 1, column3.getPixelColor(i));
					column4.setPixelColor(i, wheelTranslate(color + random(0x00,0xFF)));column4.setPixelColor(i - 1, column4.getPixelColor(i));
					column5.setPixelColor(i, wheelTranslate(color + random(0x00,0xFF)));column5.setPixelColor(i - 1, column5.getPixelColor(i));
					column6.setPixelColor(i, wheelTranslate(color + random(0x00,0xFF)));column6.setPixelColor(i - 1, column6.getPixelColor(i));
					column7.setPixelColor(i, wheelTranslate(color + random(0x00,0xFF)));column7.setPixelColor(i - 1, column7.getPixelColor(i));
					column8.setPixelColor(i, wheelTranslate(color + random(0x00,0xFF)));column8.setPixelColor(i - 1, column8.getPixelColor(i));
					column9.setPixelColor(i, wheelTranslate(color + random(0x00,0xFF)));column9.setPixelColor(i - 1, column9.getPixelColor(i));
					showframe();
					delay(wait * 3);
				}
			}
		break;
		case 4:
			for(j=256*nums; j>0; j--) { // 5 cycles of all colors on wheel
			if(playGame){break;}
				for(i=0; i< column0.numPixels(); i++) {
				if(playGame){break;}
					column0.setPixelColor(i, wheelTranslate(((i * 256 / column0.numPixels()) + j + random(0x00,0x2F)) & 255));
					column1.setPixelColor(i, wheelTranslate(((i * 256 / column0.numPixels()) + j + random(0x00,0x2F)) & 255));
					column2.setPixelColor(i, wheelTranslate(((i * 256 / column0.numPixels()) + j + random(0x00,0x2F)) & 255));
					column3.setPixelColor(i, wheelTranslate(((i * 256 / column0.numPixels()) + j + random(0x00,0x2F)) & 255));
					column4.setPixelColor(i, wheelTranslate(((i * 256 / column0.numPixels()) + j + random(0x00,0x2F)) & 255));
					column5.setPixelColor(i, wheelTranslate(((i * 256 / column0.numPixels()) + j + random(0x00,0x2F)) & 255));
					column6.setPixelColor(i, wheelTranslate(((i * 256 / column0.numPixels()) + j + random(0x00,0x2F)) & 255));
					column7.setPixelColor(i, wheelTranslate(((i * 256 / column0.numPixels()) + j + random(0x00,0x2F)) & 255));
					column8.setPixelColor(i, wheelTranslate(((i * 256 / column0.numPixels()) + j + random(0x00,0x2F)) & 255));
					column9.setPixelColor(i, wheelTranslate(((i * 256 / column0.numPixels()) + j + random(0x00,0x2F)) & 255));
				}
				showframe();
			}
			break;
	}
	//Serial.println("no more rain");
}

void showLogos(bool del) {
	for(uint8_t x = 0; x < 10; x++){
		for(uint8_t y = 0; y < 15; y++){
			gameField[x][y] = NFS_LOGO[x][y];
		}
	}
	
	updateFrame();
	refreshScreen();
	if(del) {delay(2000);}
	
	for(uint8_t x = 0; x < 10; x++){
		for(uint8_t y = 0; y < 15; y++){
			gameField[x][y] = TETRIS_LOGO[x][y];
		}
	}
	
	updateFrame();
	refreshScreen();
	if(del) {delay(2000);}
}

// // This function intercepts key press
// void keyPressed() {
//   Serial.print("Pressed:  ");
//   printKey();
// }

// // This function intercepts key release
// void keyReleased() {
//   Serial.print("Released: ");
//   printKey();
// }

// void printKey() {
//   // getOemKey() returns the OEM-code associated with the key
//   Serial.print(" key:");
//   Serial.print(keyboard.getOemKey());

//   // getModifiers() returns a bits field with the modifiers-keys
//   int mod = keyboard.getModifiers();
//   Serial.print(" mod:");
//   Serial.print(mod);

//   Serial.print(" => ");

//   if (mod & LeftCtrl)
//     Serial.print("L-Ctrl ");
//   if (mod & LeftShift)
//     Serial.print("L-Shift ");
//   if (mod & Alt)
//     Serial.print("Alt ");
//   if (mod & LeftCmd)
//     Serial.print("L-Cmd ");
//   if (mod & RightCtrl)
//     Serial.print("R-Ctrl ");
//   if (mod & RightShift)
//     Serial.print("R-Shift ");
//   if (mod & AltGr)
//     Serial.print("AltGr ");
//   if (mod & RightCmd)
//     Serial.print("R-Cmd ");

//   // getKey() returns the ASCII translation of OEM key
//   // combined with modifiers.
//   Serial.write(keyboard.getKey());
//   Serial.println();
// }







