//***************************************************************************//
//                                  HEADERS                                  //
//***************************************************************************//

//Platform independent headers
#include <cstring>
#include <deque>
#include <list>
#include <cstdlib>
#include <time.h>
#include <cmath>
#include <fstream>
#include <string>

//Platform specific headers :(
#include <ncurses.h>
#include <unistd.h>

using namespace std;

//***************************************************************************//
//                             CLASS DEFINITIONS                             //
//***************************************************************************//

//Define a coordinate
class coord_t
{
	public:
	int y,x;
	coord_t(int y0,int x0)
	{
		y=y0;
		x=x0;
	}
	bool operator==(coord_t other)
	{
		if(x == other.x && y == other.y) return true;
		else return false;
	}
	bool operator!=(coord_t other)
	{
		if(x != other.x || y != other.y) return true;
		else return false;
	}
};

//Define a fruit
class fruit_t
{
	public:
	coord_t position;
	time_t initTime;
	time_t expiryTime; //-1 means infinite
	int fruitPoints;
	
	fruit_t(int y0,int x0, time_t initTime0, time_t expiryTime0, int fruitPoints0) : position(y0,x0)
	{
		initTime = initTime0;
		expiryTime = expiryTime0;
		fruitPoints = fruitPoints0;
	}
	fruit_t(coord_t position0, time_t initTime0, time_t expiryTime0, int fruitPoints0) : position(position0)
	{
		initTime = initTime0;
		expiryTime = expiryTime0;
		fruitPoints = fruitPoints0;
	}
};

//Define a high score
class highScore_t
{
	char* name;
	int score;
	
	public:
	//Constructors
	highScore_t()
	{
		name = new char[1];
		name[0] = 0;
		score = 0;
	}
	highScore_t(char* newName,int newScore)
	{
		name = new char[strlen(newName)+1];
		strcpy(name,newName);
		score = newScore;
	}
	
	//Setter function
	void setNameScore(char* newName,int newScore)
	{
		delete [] name;
		name = new char[strlen(newName)+1];
		strcpy(name,newName);
		score = newScore;
	}
	
	//Getter functions
	int getScore() { return score; }
	char* getName() { return name; }
};

//***************************************************************************//
//                          FUNCTION PROTOTYPES                              //
//***************************************************************************//

void playGame(list<highScore_t> &highScores); //Function to handle the game
bool isFruitReady(time_t gameTime, list<fruit_t> &fruitMarket); //Checks whether it is time to produce a fruit
void placeFruit(time_t gameTime, list<fruit_t> &fruitMarket, deque<coord_t> &snake); //Adds a fruit to the list - the fruits get drawn later
void gameOver(int score, list<highScore_t> &highScores); //Function to display game over screen

void optionsMenu();	//Function to display options menu

void highScoresScreen(list<highScore_t> &highScores); //Function to display high scores
int loadHighScores(list<highScore_t> &highScores); //Function to retrive high scores from file
int saveHighScores(list<highScore_t> &highScores); //Function to save a high score to file

void streetCred(); //Function to display credits

double exponential(double rate); //Function to generate an exponential distribution

//***************************************************************************//
//                           NON-STRING CONSTS                               //
//***************************************************************************//

const double gameTurnTime = 0.25; //Length of a turn (seconds)
const double endWaitTime = 1.5; //Length of time to show players their demise
double rate = 1.0/10; //rate at which fruits will be generated (in units of /second)
int youngest = 0;

//***************************************************************************//
//                            STRING CONSTANTS                               //
//***************************************************************************//

// -main menu
const char gameName[] = "SNAKE!";
const char menuOptionPlay[] = "Start game ('p')";
const char menuOptionOptions[] = "Options ('o')";
const char menuOptionHighScore[] = "High Scores ('s')";
const char menuOptionCredits[] = "Credits ('c')";
const char menuOptionQuit[] = "Quit ('q')";

const char* menuOptions[] = {menuOptionPlay, menuOptionOptions, menuOptionHighScore, menuOptionCredits, menuOptionQuit};

// -Game
const char gameOverText[] = "Press 'q' to return to the main menu";
const char snakeHeadChar[] = "O";
const char snakeBodyChar[] = "*";
const char snakeTailChar[] = "";
const char fruitChar[] = "F";

// -Options menu
const char optionsTitle[] = "OPTIONS";
const char optionsQuit[] = "Quit ('q')";

const char* optionsOptions[] = {optionsQuit};

// -High scores
const char scoresTitle[] = "HIGH SCORES";
const char scoresQuit[] = "Press 'q' to return to the main menu";
const char scoresFile[] = "~/.snakeHighScores";

const char* scoresOptions[] = {scoresTitle,scoresQuit};

// -Credits
const char creditsTitle[] = "CREDITS";
const char* creditsText[] = {"PROCRASTINATION","Alex Seaton","","ARCANE MATHS","Si Chen","","GURU","Aviv Beeri","","BUG RESPONSIBILITY","ERROR: Unhandled exception #423987"};
const char creditsQuit[] = "Return to main menu ('q')";

//***************************************************************************//
//                                   MAIN()                                  //
//***************************************************************************//

//main() handles main menu and calls functions to display other screens (e.g. game, submenus etc.)
int main()
{
	int ch;
	int row,col; //Size of menu area (currently dynamic)
	
	int highlight = 0; //Item highlighted
	
	//Create container to store high scores, and load them from a file
	list<highScore_t> highScores;
	loadHighScores(highScores);

	//Initialise ncurses
	initscr();
	raw();
	keypad(stdscr,TRUE);
	noecho();
	
	//Initialise random seed
	srand(time(NULL));
	
	while(true)
	{
		//Clear display
		clear();
		
		//Get size of console
		getmaxyx(stdscr,row,col);
		
		//Display main menu
		attron(A_UNDERLINE | A_BOLD);
		mvprintw(row/5,col/2-strlen(gameName)/2,"%s",gameName);
		attroff(A_UNDERLINE | A_BOLD);
		mvprintw(row/5+2,col/2-strlen(menuOptionPlay)/2,"%s",menuOptionPlay);
		mvprintw(row/5+4,col/2-strlen(menuOptionOptions)/2,"%s",menuOptionOptions);
		mvprintw(row/5+6,col/2-strlen(menuOptionHighScore)/2,"%s",menuOptionHighScore);
		mvprintw(row/5+8,col/2-strlen(menuOptionCredits)/2,"%s",menuOptionCredits);
		mvprintw(row/5+10,col/2-strlen(menuOptionQuit)/2,"%s",menuOptionQuit);
		
		mvprintw(row/5+2*highlight+2,col/2-strlen(menuOptions[highlight])/2-2,"*");
		mvprintw(row/5+2*highlight+2,col/2-strlen(menuOptions[highlight])/2+strlen(menuOptions[highlight])+1,"*");
		
		//Move cursor to (0,0)
		move(0,0);
		
		//Write all output to console
		refresh();
		
		//Set character input as blocking
		nodelay(stdscr,FALSE);
		
		//Get character from user
		ch=getch();
		
		//Interpret user input
		if(ch == 'p')
		{
			playGame(highScores);
			continue;
		}
		else if(ch == 'o') optionsMenu();
		else if(ch == 's') highScoresScreen(highScores);
		else if(ch == 'c') streetCred();
		else if(ch == 'q') break;
		else if(ch == KEY_UP)
		{
			if(highlight == 0) highlight = 4;
			else highlight--;
		}
		else if(ch == KEY_DOWN)
		{
			if(highlight == 4) highlight = 0;
			else highlight++;
		}
		else if(ch == '\n')
		{
			if(highlight == 0)
			{
				playGame(highScores);
				continue;
			}
			else if(highlight == 1) optionsMenu();
			else if(highlight == 2) highScoresScreen(highScores);
			else if(highlight == 3) streetCred();
			else if(highlight == 4) break;
		}
	}
	
	endwin();
	
	return 0;
}

//***************************************************************************//
//                          FUNCTION DEFINITONS                              //
//***************************************************************************//

//TODO: Sort out these two functions!

//Checks whether a fruit is ready to be placed
bool isFruitReady(time_t gameTime, list<fruit_t> &fruitMarket)
{
	//If no fruits are present then we need a new one.
	if(fruitMarket.empty()) return 1;
	//if all fruits are younger than the current time of the game, make a new one
	for(list<fruit_t>::iterator i = fruitMarket.begin(); i != fruitMarket.end(); i++)
	{
		if((*i).initTime > youngest) youngest = (*i).initTime;
	}
	if (youngest <= gameTime) return 1;
	else return 0;
}

//Places (i.e. generates coordinates for) a fruit
void placeFruit(time_t gameTime, list<fruit_t> &fruitMarket,deque<coord_t> &snake)
{
	//Get size of window
	int row,col;
	getmaxyx(stdscr,row,col);
	coord_t randomCoord(-1,-1);
	bool inSomething = true;
	
	//Generate coordinates of fruit such that they aren't in the snake or on any other fruit
	while(inSomething == true)
	{
		randomCoord = coord_t((rand() % (row-3))+2,(rand() % (col-2))+1);
		
		for(deque<coord_t>::iterator i = snake.begin(); i != snake.end(); i++)
		{
			if((*i) == randomCoord)
			{
				inSomething = true;
				break;
			}
			else inSomething = false;
		}
		if(inSomething == true) continue;
		
		for(list<fruit_t>::iterator i = fruitMarket.begin(); i != fruitMarket.end(); i++)
		{
			if((*i).position == randomCoord)
			{
				inSomething = true;
				break;
			}
			else inSomething = false;
		}
	}
	
	//Create the fruit
	int creation_time;
	do {
		creation_time = youngest + int(exponential(rate));
	} while (creation_time <= gameTime); //generate next birthday of fruit; make sure it is in the future
	fruitMarket.push_front(fruit_t(randomCoord,creation_time,creation_time+30,10));//put new fruit on market
}

void playGame(list<highScore_t> &highScores)
{
	deque<coord_t> snake; //Position of snake
	list<fruit_t> fruitMarket; //List of fruits currently in use
	
	time_t initTime; //Epoch time of start of game (seconds)
	time_t gameTime = 0; //Current time, measured in seconds with 0 as time game started
	int ch; //Stores latest character from stdin
	int row,col; //Size of play area (currently dynamic) TODO: Fix these values in some way
	
	int direction=-1; //Direction of motion of snake (-1: uninitialised, 0: up, 1: down, 2: right, 3: left)
	coord_t predictor(-1,-1); //Predicted position of snake
	bool gotFruit = false; //If true, signals that snake will eat a fruit *next* turn
	bool growSnake = false; //If true, signals that snake has eaten a fruit this turn and should grow
	int score = 0;
	
	//Set character reading to be non-blocking
	nodelay(stdscr,TRUE);
	
	//Get size of window
	getmaxyx(stdscr,row,col);
	
	//And God created the snake, saying, "Be fruitful and multiply"
	snake.push_front(coord_t(row/2,col/2));
	snake.push_front(coord_t(row/2,col/2+1));
	snake.push_front(coord_t(row/2-1,col/2+1));
	snake.push_front(coord_t(row/2-1,col/2));
	
	//Add a test fruit!
	fruitMarket.push_front(fruit_t(row/2,col/2,gameTime,-1,100));
	
	while(true)
	{
		//Read character from input buffer
		ch=getch();
		
		//Clear the rest of the buffer
		while(getch() != ERR);
		
		//Interpret user input
		if(ch == 'q') return;
		else if(ch == KEY_UP) { if(direction != 1) direction=0; }
		else if(ch == KEY_DOWN) { if(direction != 0) direction=1; }
		else if(ch == KEY_RIGHT) { if(direction != 3) direction=2; }
		else if(ch == KEY_LEFT) { if(direction != 2) direction=3; }
		
		if(direction == -1) initTime = time(NULL); //Record time to mark start of game (last time this executes will be the start of the game)
		
		if(direction != -1) //Only do this stuff if the game has started!
		{
			//Get the time
			gameTime = time(NULL)-initTime;
			
			//Calculate where the snake will move
			predictor = snake.front();
			
			if(direction == 0) predictor.y--;
			else if(direction == 1)	predictor.y++;
			else if(direction == 2) predictor.x++;
			else if(direction == 3)	predictor.x--;
			
			//Sort out fruit related issues
			if(isFruitReady(gameTime, fruitMarket)) placeFruit(gameTime, fruitMarket, snake); //If a fruit is ready to be placed, place it!
			
			if(gotFruit)
			{
				growSnake = true;
				gotFruit = false;
			}
			
			//Run through fruit and remove any the snake is about to eat or that are about to expire
			for(list<fruit_t>::iterator i=fruitMarket.begin(); i != fruitMarket.end(); i++)
			{
				//Warning: Iterator after an item is removed may not be valid!
				//Remove expiring fruit
				if((gameTime > (*i).expiryTime) && ((*i).expiryTime != -1))
				{
					i = fruitMarket.erase(i);
					i--;
					continue;
				}
				
				//Remove fruits that are in the path of the snake
				if(predictor == (*i).position)
				{
					score += (*i).fruitPoints;
					i = fruitMarket.erase(i);
					i--;
					gotFruit = true;
				}
			}
			
			//Check if snake is about to hit a wall
			if(predictor.y < 2 || predictor.y > row-2 || predictor.x < 1 || predictor.x > col-2)
			{
				gameOver(score, highScores);
				return;
			}
			
			//Check if snake is about to hit itself
			//Note: the snake can move into the space currently occupied by the last part of its tail, unless it has just received a fruit.
			for(deque<coord_t>::iterator i=snake.begin();
			    ((i != (--snake.end())) && (!growSnake)) || ((i != snake.end()) && growSnake);
			    i++)
			{
				if(predictor == *i)
				{
					gameOver(score, highScores);
					return;
				}
			}
			
			//Move snake
			if(growSnake != true) snake.pop_back();
			else growSnake = false;
			snake.push_front(predictor);
		}
				
		//Clear window
		clear();
		
		//Get size of window
		getmaxyx(stdscr,row,col);

		//Draw edges of play area
		for(int i=0; i<col; i++) mvprintw(1,i,"%s","-");
		for(int i=0; i<col; i++) mvprintw(row-1,i,"%s","-");
		for(int i=2; i<row-1; i++) mvprintw(i,0,"%s","|");
		for(int i=2; i<row-1; i++) mvprintw(i,col-1,"%s","|");
		mvprintw(1,0,"O");
		mvprintw(row-1,0,"O");
		mvprintw(1,col-1,"O");
		mvprintw(row-1,col-1,"O");
		
		//Draw snake!
		for(deque<coord_t>::iterator i=snake.begin(); i != snake.end(); i++)
		{
			mvprintw((*i).y,(*i).x,"%s",snakeBodyChar);
		}
		mvprintw((snake.front()).y,(snake.front()).x,"%s",snakeHeadChar);
		if((snake.front() != snake.back()) && (strcmp(snakeTailChar,"") != 0)) mvprintw((snake.back()).y,(snake.back()).x,"%s",snakeTailChar);
		
		//Draw fruit!
		for(list<fruit_t>::iterator i=fruitMarket.begin(); i != fruitMarket.end(); i++)
		{
			for(deque<coord_t>::iterator j = snake.begin(); j != snake.end(); j++)
			{
				if( ((*i).position != (*j)) && (*i).initTime <= gameTime)
				{
					mvprintw((*i).position.y,(*i).position.x,"%s",fruitChar); //if the fruit's creation time is now or in the past, and its position does not conflict with the snake's, draw it
				}
			}
		}
		
		//Draw timer and score
		mvprintw(0,col/4-(strlen("Timer: ")+(int)log10(gameTime+0.1)+1)/2,"Timer: %i",gameTime);
		mvprintw(0,col-1-col/4-(strlen("Score: ")+(int)log10(score+0.1)+1)/2,"Score: %i",score);
		
		//Move cursor back to top left hand corner
		move(0,0);
		
		//Copy virtual buffer to console and display everything!
		refresh();
		
		//Wait for a second
		usleep(gameTurnTime*1000000);
	}
}

void gameOver(int score,list<highScore_t> &highScores)
{
	int row, col;
	char ch = 0;
	
	//Wait a little to show players their demise
	usleep(endWaitTime*1000000);
	
	//Set character reading to be blocking
	nodelay(stdscr,FALSE);
	
	//Clear window
	clear();
	
	//Get size of window
	getmaxyx(stdscr,row,col);
	
	//Print game over text
	mvprintw(row/2-3,col/2-38,"  ____       _         _  _      _____          ___             _____  ___");
	mvprintw(row/2-2,col/2-38," /    \\     / \\       / \\/ \\    |              /   \\  \\      / |      |   \\ ");
	mvprintw(row/2-1,col/2-38,"|          /___\\     /      \\   |___          /     \\  \\    /  |___   |___/");
	mvprintw(row/2,col/2-38,"|    ___  /     \\   /        \\  |             \\     /   \\  /   |      |   \\ ");
	mvprintw(row/2+1,col/2-38," \\____/  /       \\ /          \\ |_____         \\___/     \\/    |_____ |    \\ ");
	mvprintw(row-1,col/2-strlen(gameOverText)/2,"%s",gameOverText);
	
	mvprintw(row/2+4,col/2-strlen("Congratulations! High score!")/2,"Contratulations! High Score!");
	mvprintw(row/2+5,col/2-strlen("Please enter your name: ")/2,"Please enter your name: ");
	
	raw();
	keypad(stdscr,TRUE);
	
	string name = "";
	while(ch != '\n')
	{
		ch = getch();
		if(ch == '\a') name.erase(--name.end());
		else name += ch;
		for(int i=0; i<name.length()+2; i++) mvprintw(row/2+6,col/2-(name.length()+1)/2-1+i," ");
		mvprintw(row/2+6,col/2-(name.length()+1)/2,"%s",name.c_str());
		
		refresh();
	}
	
	//Move pointer back to top left hand corner
	move(0,0);
	
	//Draw to console
	refresh();
	
	while(ch != 'q') ch = getch();
}

//Function to display options menu
void optionsMenu()
{
	int ch;
	int row,col; //Size of menu area (currently dynamic)
	
	int highlight = 0; //Item highlighted
	
	while(true)
	{
		//Clear display
		clear();
		
		//Get size of console
		getmaxyx(stdscr,row,col);
		
		//Display main menu
		attron(A_UNDERLINE | A_BOLD);
		mvprintw(row/5,col/2-strlen(optionsTitle)/2,"%s",optionsTitle);
		attroff(A_UNDERLINE | A_BOLD);
		mvprintw(row/5+2,col/2-strlen(optionsQuit)/2,"%s",optionsQuit);
		
		mvprintw(row/5+2*highlight+2,col/2-strlen(optionsOptions[highlight])/2-2,"*");
		mvprintw(row/5+2*highlight+2,col/2-strlen(optionsOptions[highlight])/2+strlen(optionsOptions[highlight])+1,"*");
		
		//Move cursor to (0,0)
		move(0,0);
		
		//Write all output to console
		refresh();
		
		//Set character input as blocking
		nodelay(stdscr,FALSE);
		
		//Get character from user
		ch=getch();
		
		//Interpret user input
		if(ch == 'q') break;
		else if(ch == KEY_UP)
		{
			if(highlight == 0) highlight = 0;
			else highlight--;
		}
		else if(ch == KEY_DOWN)
		{
			if(highlight == 0) highlight = 0;
			else highlight++;
		}
		else if(ch == '\n')
		{
			if(highlight == 0)
			{
				break;
			}
		}
	}
}

//Function to display high scores list
void highScoresScreen(list<highScore_t> &highScores)
{
	int ch;
	int row,col; //Size of menu area (currently dynamic)
	
	while(true)
	{
		//Clear display
		clear();
		
		//Get size of console
		getmaxyx(stdscr,row,col);
		
		//Print all high scores text
		attron(A_UNDERLINE | A_BOLD);
		mvprintw(row/5,col/2-strlen(scoresTitle)/2,"%s",scoresTitle);
		attroff(A_UNDERLINE | A_BOLD);
		mvprintw(row-1,col/2-strlen(scoresQuit)/2,"%s",scoresQuit);
		
		//Move cursor to (0,0)
		move(0,0);
		
		//Write all output to console
		refresh();
		
		//Set character input as blocking
		nodelay(stdscr,FALSE);
		
		//Get character from user
		ch=getch();
		
		//Interpret user input
		if(ch == 'q') break;
		else if(ch == KEY_UP)
		{
			//Scroll up
		}
		else if(ch == KEY_DOWN)
		{
			//Scroll down
		}
	}
}

//Function to retrive high scores from file (appends them to list of high scores given)
int loadHighScores(list<highScore_t> &highScores)
{
	ifstream inputStream;
	
	//Check input file correctly opened
	inputStream.open(scoresFile, ios::in);
	if(inputStream.is_open() == false) return 1;
	
	char ch; //Holds last char read from stream
	char* currentLine = NULL; //Line we're currently working on
	char* tempName = NULL; //Name extracted from current line
	int tempScore; //Score extracted from current line
	streampos lineStart; //Position of start of line
	streampos lineEnd; //EOL
	
	while(inputStream.peek() != EOF)
	{
		//Remove all newline characters before proceeding
		while(inputStream.peek() == '\n') inputStream.get(ch);
		
		//Find start and end of line
		lineStart = inputStream.tellg();
		do{ inputStream.get(ch); } while(ch != '\n');
		lineEnd = inputStream.tellg();
		
		//Return to start of line
		inputStream.seekg(lineStart);
		
		//Create new char string to store line
		currentLine = new char[lineEnd-lineStart];
		
		//Read the line
		inputStream.getline(currentLine,lineEnd-lineStart);
		
		//Find position of delimiter (',') in line and attempt to parse the name and score
		//Expected format: <name>,<score>
		for(int i=0; i<strlen(currentLine); i++)
		{			
			if(currentLine[i] == ',')
			{
				tempName = new char[i+1];
				strncpy(tempName,currentLine,i);
				tempName[i] = 0;
				
				//Check for and discard empty strings
				if(strcmp(tempName,"") == 0) break;
				
				bool isWhiteSpace = true;
				for(int j=0; j<strlen(tempName); j++) if(tempName[j] != ' ') isWhiteSpace = false;
				
				if(isWhiteSpace == true) break;
				
				//Record score
				tempScore = atoi(currentLine+i+1);
				
				highScores.push_back(highScore_t(tempName,tempScore));
				break;
			}
		}
		
		//Free memory
		if(currentLine != NULL)	delete [] currentLine;
		if(tempName != NULL) delete [] tempName;
		
		currentLine = NULL;
		tempName = NULL;
	}
	
	//Close file
	inputStream.close();

	return 0;
}

//Function to save high scores to file
int saveHighScores(list<highScore_t> &highScores)
{
	ofstream outputStream;
	
	//Check output file correctly opened
	outputStream.open(scoresFile, ios::out | ios::trunc);
	if(outputStream.is_open() == false) return 1;
	
	for(list<highScore_t>::iterator i = highScores.begin(); i != highScores.end(); i++)
	{
		outputStream << (*i).getName() << "," << (*i).getScore() << endl;
	}
	
	return 0;
}

//Function to display credits
void streetCred()
{
	int ch;
	int row,col; //Size of console
	
	while(true)
	{
		//Clear display
		clear();
		
		//Get size of console
		getmaxyx(stdscr,row,col);
		
		//Print all credits text
		attron(A_UNDERLINE | A_BOLD);
		mvprintw(row/5,col/2-strlen(creditsTitle)/2,"%s",creditsTitle);
		attroff(A_UNDERLINE | A_BOLD);
		mvprintw(row-1,col/2-strlen(creditsQuit)/2,"%s",creditsQuit);
		
		for(int i=0; (i<(sizeof(creditsText)/sizeof(const char*))) && (row/5+3+i < row-3); i++)
		{ mvprintw(row/5+3+i,col/2-strlen(creditsText[i])/2,"%s",creditsText[i]); }
		
		//Move cursor to (0,0)
		move(0,0);
		
		//Write all output to console
		refresh();
		
		//Set character input as blocking
		nodelay(stdscr,FALSE);
		
		//Get character from user
		ch=getch();
		
		//Interpret user input
		if(ch == 'q') break;
		else if(ch == KEY_UP)
		{
			//Scroll up
		}
		else if(ch == KEY_DOWN)
		{
			//Scroll down
		}
	}
}

double exponential(double rate)// function generating an exponential distribution
{
	double x;
	do{
		x= -1.0*log(1.0*rand()/RAND_MAX)/rate;
	} while ((x<=5) || (x>=30)); //wating time between fruits must be between 5 to 30 seconds
	return x;
}
