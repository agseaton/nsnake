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
#include <chrono>

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
	highScore_t(const char* newName,int newScore)
	{
		name = new char[strlen(newName)+1];
		strcpy(name,newName);
		score = newScore;
	}
	
	//Setter function
	void setNameScore(const char* newName,int newScore)
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
bool isFruitReady(int gameTime, list<fruit_t> &fruitMarket, int &youngest); //Checks whether it is time to produce a fruit
void placeFruit(int gameTime, list<fruit_t> &fruitMarket, deque<coord_t> &snake, int youngest); //Adds a fruit to the list - the fruits get drawn later
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
const unsigned int maxNumHighScores = 10; // Maximum number of high scores allowed
double rate = 1.0/10; //rate at which fruits will be generated (in units of /second)

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
const char scoresFile[] = "./.snakeHighScores";

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
		ch=wgetch(stdscr);
		
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
bool isFruitReady(int gameTime, list<fruit_t> &fruitMarket, int &youngest)
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
void placeFruit(int gameTime, list<fruit_t> &fruitMarket,deque<coord_t> &snake, int youngest)
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
	//In game objects
	deque<coord_t> snake; //Position of snake
	list<fruit_t> fruitMarket; //List of fruits currently in use
	int youngest = 0; //age of youngest fruit
	
	//Variables for tracking motion of snake
	int direction=-1; //Direction of motion of snake (-1: uninitialised, 0: up, 1: down, 2: right, 3: left)
	coord_t predictor(-1,-1); //Predicted position of snake
	bool gotFruit = false; //If true, signals that snake will eat a fruit *next* turn
	bool growSnake = false; //If true, signals that snake has eaten a fruit this turn and should grow
	
	//Timing variables
	chrono::system_clock::time_point gameInitTime; //Time at start of game
	chrono::system_clock::time_point loopFinishTime; //Time at end of main loop
	double totalElapsedTime; //Time elapsed since start of game by end of main loop
	unsigned int gameTime = 0; //Time in seconds since beginning of game
	unsigned int turnNum = 0; //Which turn is this?
	
	//Input variables
	int ch; //Stores latest character from stdin
	
	//Window parameters
	int row,col; //Size of play area (currently dynamic) TODO: Fix these values in some way
	
	//The score
	int score = 0;
	
/*****************************************************************************/
	//Set character reading to be blocking
	nodelay(stdscr,FALSE);
	
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
	
	//And God created the snake, saying, "Be fruitful and multiply"
	snake.push_front(coord_t(row/2,col/2));
	snake.push_front(coord_t(row/2,col/2+1));
	snake.push_front(coord_t(row/2-1,col/2+1));
	snake.push_front(coord_t(row/2-1,col/2));
	
	//Draw the snake's initial position
	for(deque<coord_t>::iterator i = ++snake.begin(); i != snake.end(); i++) mvprintw((*i).y,(*i).x,"%s",snakeBodyChar);
	mvprintw(snake.front().y,snake.front().x,"%s",snakeHeadChar);
	if((snake.front() != snake.back()) && (strcmp(snakeTailChar,"") != 0)) mvprintw((snake.back()).y,(snake.back()).x,"%s",snakeTailChar);
	
	//Add a test fruit!
	fruitMarket.push_front(fruit_t(row/2,col/2,gameTime,-1,100));
	mvprintw(fruitMarket.front().position.y,fruitMarket.back().position.x,"%s",fruitChar); //Draw it
	
	//Draw timer and score
	for(int i=0; i<col; i++) mvprintw(0,i," ");
	mvprintw(0,col/4-(strlen("Timer: ")+2)/2,"Timer: 0");
	mvprintw(0,col-1-col/4-(strlen("Score: ")+2)/2,"Score: 0");
	
	//Move cursor back to top left hand corner
	move(0,0);
	
	//Copy virtual buffer to console and display everything!
	refresh();

/*****************************************************************************/
	//Wait until the user starts the game
	while(true)
	{
		//Read character from input buffer
		ch=wgetch(stdscr);
		
		//Interpret user input
		if(ch == 'q') return;
		else if(ch == KEY_UP) { direction=0; break; }
		else if(ch == KEY_DOWN) { direction=1; break; }
		else if(ch == KEY_RIGHT) { direction=2; break; }
		else if(ch == KEY_LEFT) { direction=3; break; }
	}
	
	//Get ready to start the game
	//Set character reading to be non-blocking
	nodelay(stdscr,TRUE);
	
	gameInitTime = chrono::system_clock::now(); //Record time to mark start of game

/*****************************************************************************/
	//Game main loop
	while(true)
	{
		//Increment turn counter
		turnNum++;
		
		//Get time in seconds since start of game
		gameTime = turnNum*gameTurnTime;
		
		//Read character from input buffer
		ch=wgetch(stdscr);
		
		//Clear the rest of the buffer
		while(wgetch(stdscr) != ERR);
		
		//Interpret user input
		if(ch == 'q') return;
		else if(ch == KEY_UP) { if(direction != 1) direction=0; }
		else if(ch == KEY_DOWN) { if(direction != 0) direction=1; }
		else if(ch == KEY_RIGHT) { if(direction != 3) direction=2; }
		else if(ch == KEY_LEFT) { if(direction != 2) direction=3; }
		
		//Calculate where the snake will move
		predictor = snake.front();
		
		if(direction == 0) predictor.y--;
		else if(direction == 1)	predictor.y++;
		else if(direction == 2) predictor.x++;
		else if(direction == 3)	predictor.x--;
		
		//Sort out fruit related issues
		if(isFruitReady(gameTime, fruitMarket,youngest)) placeFruit(gameTime, fruitMarket, snake,youngest); //If a fruit is ready to be placed, place it!
		
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
				mvprintw((*i).position.y,(*i).position.x," ");
				i = fruitMarket.erase(i);
				i--;
				continue;
			}
			
			//Remove fruits that are in the path of the snake
			if(predictor == (*i).position)
			{
				score += (*i).fruitPoints;
				mvprintw((*i).position.y,(*i).position.x," ");
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
		if(growSnake != true)
		{
			mvprintw((snake.back()).y,(snake.back().x)," ");
			snake.pop_back();
		}
		else growSnake = false;
		mvprintw((snake.front()).y,(snake.front().x),"%s",snakeBodyChar);
		snake.push_front(predictor);
		
		//Draw snake's head and tail
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
		for(int i=0; i<col; i++) mvprintw(0,i," ");
		mvprintw(0,col/4-(strlen("Timer: ")+(int)log10(gameTime+0.1)+1)/2,"Timer: %i",gameTime);
		mvprintw(0,col-1-col/4-(strlen("Score: ")+(int)log10(score+0.1)+1)/2,"Score: %i",score);
		
		//Move cursor back to top left hand corner
		move(0,0);
		
		//Copy virtual buffer to console and display everything!
		refresh();
		
		//Determine the time and thus time elapsed since beginning of game
		loopFinishTime = chrono::system_clock::now();
		totalElapsedTime = (chrono::duration_cast<chrono::duration<double>>(loopFinishTime-gameInitTime)).count();
		
		//Sleep for the amount of time remaining in the turn
		usleep(((gameTurnTime*turnNum)-totalElapsedTime)*1000000);
	}
}

void gameOver(int score,list<highScore_t> &highScores)
{
	int row, col;
	int ch = 0;
	bool isHighScore = false;
	
	//Do we have a high score?
	if(highScores.empty() && (score != 0)) isHighScore = true; //Yes, if you're the first to get a non-zero score. Impressive.
	else for(list<highScore_t>::iterator i=highScores.begin(); i != highScores.end(); i++)
	{
		if((*i).getScore() < score)
		{
			isHighScore = true;
			break;
		}
		else isHighScore = false;
	}
	
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
	
	if(isHighScore)
	{
		mvprintw(row/2+4,col/2-strlen("Congratulations! High score!")/2,"Contratulations! High Score!");
		mvprintw(row/2+5,col/2-strlen("Please enter your name: ")/2,"Please enter your name: ");
		move(row/2+6,col/2);
	
		string name = "";
		while(true)
		{
			ch = wgetch(stdscr); //Read character
		
			//Interpret it correctly	
			if(ch == KEY_BACKSPACE) { if(name.length() > 0) name.erase(--name.end()); }
			else if(ch == '\n')
			{
				refresh();
				break;
			}
			else if((ch == ' ') && (name.length() == 0)) { /*Do nothing - I'm onto you*/ }
			else if((ch >= KEY_MIN) && (ch <= KEY_MAX)) { ch = 0; }
			else name += ch;
		
			for(unsigned int i=0; i<name.length()+2; i++) mvprintw(row/2+6,col/2-(name.length()+1)/2-1+i," ");
			mvprintw(row/2+6,col/2-(name.length()+1)/2,"%s",name.c_str());
		
			refresh();
		}
	
		//Remove trailing space characters
		for(string::iterator i=(--name.end()); i != name.begin(); i--)
		{
			if((*i) == ' ') i = name.erase(i);
			else break;
		}
		
		//Record the score in our list of high scores, preserving ordering
		if(highScores.empty()) highScores.push_front(highScore_t(name.c_str(),score));
		else for(list<highScore_t>::iterator i = highScores.begin(); i != highScores.end(); i++)
		{
			if(score > (*i).getScore())
			{
				highScores.insert(i,highScore_t(name.c_str(),score));
				break;
			}
			else if(score == (*i).getScore()) //Compare Names
			{
				if(strcmp(name.c_str(),(*i).getName()) < 0)
				{
					highScores.insert(i,highScore_t(name.c_str(),score));
					break;
				}
				else if(strcmp(name.c_str(),(*i).getName()) == 0) break; //If name AND score is the same then ignore this score to prevent it being inserted multiple times
			}
			
			//If we're about to get to the end and haven't found a place for the high score, place it at the end
			if(i == --highScores.end())
			{
				highScores.insert(highScores.end(),highScore_t(name.c_str(),score));
				break;
			}
		}
		
		//Remove a high score from the list if there are too many
		while(highScores.size() > maxNumHighScores) highScores.pop_back();
		
		//Save the high scores to a file
		if(saveHighScores(highScores) == 1) mvprintw(row-2,col/2-strlen("ERROR: Couldn't save to file")/2,"ERROR: Couldn't save to file");
	}
	
	mvprintw(row-1,col/2-strlen(gameOverText)/2,"%s",gameOverText);
	
	//Move pointer back to top left hand corner
	move(0,0);
	
	//Draw to console
	refresh();
	
	while(ch != 'q') ch = wgetch(stdscr);
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
		ch=wgetch(stdscr);
		
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
		
		//Print title and quit text
		attron(A_UNDERLINE | A_BOLD);
		mvprintw(row/5,col/2-strlen(scoresTitle)/2,"%s",scoresTitle);
		attroff(A_UNDERLINE | A_BOLD);
		mvprintw(row-1,col/2-strlen(scoresQuit)/2,"%s",scoresQuit);
		
		int maxScoreLength = (int)log10((float)highScores.front().getScore()+0.1)+1;
		int maxNameLength = 0;
		//Find what the largest high score name length is to position the list on the screen
		for(list<highScore_t>::iterator i = highScores.begin(); i != highScores.end(); i++)
		{
			int currLength = strlen((*i).getName());
			if(currLength > maxNameLength) maxNameLength = currLength;
		}
		
		//Print the high scores
		int listPos = 1;
		for(list<highScore_t>::iterator i = highScores.begin(); i != highScores.end(); i++)
		{
			if(maxScoreLength > maxNameLength)
			{
				mvprintw(row/5+2+listPos,col/2-maxScoreLength-2,"%s",(*i).getName());
				mvprintw(row/5+2+listPos,col/2+maxScoreLength+2-(int)log10((float)(*i).getScore()+0.1),"%i",(*i).getScore());
				mvprintw(row/5+2+listPos,col/2-maxScoreLength-(int)log10((float)listPos+0.1)-5,"%i. ",listPos);
			}
			else
			{
				mvprintw(row/5+2+listPos,col/2-maxNameLength-2,"%s",(*i).getName());
				mvprintw(row/5+2+listPos,col/2+maxNameLength+2-(int)log10((float)(*i).getScore()+0.1),"%i",(*i).getScore());
				mvprintw(row/5+2+listPos,col/2-maxNameLength-(int)log10((float)listPos+0.1)-5,"%i. ",listPos);
			}
			listPos++;
			if(row/5+2+listPos > row-3) break;
		}
		
		//Move cursor to (0,0)
		move(0,0);
		
		//Write all output to console
		refresh();
		
		//Set character input as blocking
		nodelay(stdscr,FALSE);
		
		//Get character from user
		ch=wgetch(stdscr);
		
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
		for(unsigned int i=0; i<strlen(currentLine); i++)
		{			
			if(currentLine[i] == ',')
			{
				tempName = new char[i+1];
				strncpy(tempName,currentLine,i);
				tempName[i] = 0;
				
				//Check for and discard empty strings
				if(strcmp(tempName,"") == 0) break;
				
				bool isWhiteSpace = true;
				for(unsigned int j=0; j<strlen(tempName); j++) if(tempName[j] != ' ') isWhiteSpace = false;
				
				if(isWhiteSpace == true) break;
				
				//Record score
				tempScore = atoi(currentLine+i+1);
				if((tempScore == 0) || (tempScore < 0)) break;
				
				//Place new high score record in correct position in list - preserve ordering of list.
				if(highScores.empty()) highScores.push_front(highScore_t(tempName,tempScore));
				else for(list<highScore_t>::iterator j = highScores.begin(); j != highScores.end(); j++)
				{
					if(tempScore > (*j).getScore())
					{
						highScores.insert(j,highScore_t(tempName,tempScore));
						break;
					}
					else if(tempScore == (*j).getScore()) //Compare Names
					{
						if(strcmp(tempName,(*j).getName()) < 0)
						{
							highScores.insert(j,highScore_t(tempName,tempScore));
							break;
						}
						else if(strcmp(tempName,(*j).getName()) == 0) break; //If name AND score is the same then ignore this score to prevent it being inserted multiple times
					}
					
					//If we're about to get to the end and haven't found a place for the high score, place it at the end
					if(j == --highScores.end())
					{
						highScores.insert(highScores.end(),highScore_t(tempName,tempScore));
						break;
					}
				}
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
	unsigned int row,col; //Size of console
	
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
		
		for(unsigned int i=0; (i<(sizeof(creditsText)/sizeof(const char*))) && (row/5+3+i < row-3); i++)
		{ mvprintw(row/5+3+i,col/2-strlen(creditsText[i])/2,"%s",creditsText[i]); }
		
		//Move cursor to (0,0)
		move(0,0);
		
		//Write all output to console
		refresh();
		
		//Set character input as blocking
		nodelay(stdscr,FALSE);
		
		//Get character from user
		ch=wgetch(stdscr);
		
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
