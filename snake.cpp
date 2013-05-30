//***************************************************************************//
//                                  HEADERS                                  //
//***************************************************************************//

//Platform independent headers
#include <string.h>
#include <deque>
#include <list>
#include <cstdlib>
#include <time.h>
#include <cmath>

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
		name = new char[strlen(newName)];
		strcpy(name,newName);
	}
	
	//Setter function
	void setNameScore(char* newName,int newScore)
	{
		delete [] name;
		name = new char[strlen(newName)];
		strcpy(name,newName);
	}
	
	//Getter functions
	int getScore() { return score; }
	char* getName() { return name; }
};

//***************************************************************************//
//                          FUNCTION PROTOTYPES                              //
//***************************************************************************//

int playGame(); //Function to handle the game
bool isFruitReady(time_t gameTime, list<fruit_t> &fruitMarket); //Checks whether it is time to produce a fruit
void placeFruit(time_t gameTime, list<fruit_t> &fruitMarket, deque<coord_t> &snake); //Adds a fruit to the list - the fruits get drawn later
void gameOver(); //Function to display game over screen

void optionsMenu();	//Function to display options menu

void highScoresScreen(); //Function to display high scores
void loadHighScores(); //Function to retrive high scores from file
void saveHighScore(); //Function to save a high score to file

void streetCred(); //Function to display credits

//***************************************************************************//
//                           NON-STRING CONSTS                               //
//***************************************************************************//

const double gameTurnTime = 0.25; //Length of a turn (seconds)

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
	
	list<highScore_t> highScores;

	//Initialise ncurses
	initscr();
	raw();
	keypad(stdscr,TRUE);
	noecho();
	
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
			int outcome = playGame();
			continue;
		}
		else if(ch == 'o') optionsMenu();
		else if(ch == 's') highScoresScreen();
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
				int outcome = playGame();
				continue;
			}
			else if(highlight == 1) optionsMenu();
			else if(highlight == 2) highScoresScreen();
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
	fruitMarket.push_front(fruit_t(randomCoord,gameTime,gameTime+30,10));
}

int playGame()
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
		if(ch == 'q') return 0;
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
				gameOver();
				return score;
			}
			
			//Check if snake is about to hit itself
			//Note: the snake can move into the space currently occupied by the last part of its tail, unless it has just received a fruit.
			for(deque<coord_t>::iterator i=snake.begin();
			    ((i != (--snake.end())) && (!growSnake)) || ((i != snake.end()) && growSnake);
			    i++)
			{
				if(predictor == *i)
				{
					gameOver();
					return score;
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
		for(list<fruit_t>::iterator i=fruitMarket.begin(); i != fruitMarket.end(); i++) mvprintw((*i).position.y,(*i).position.x,"%s",fruitChar);
		
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

void gameOver()
{
	int row, col;
	char ch;
	
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

//Function to display high scores
void highScoresScreen()
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

