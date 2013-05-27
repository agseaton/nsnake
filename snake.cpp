#include <ncurses.h>
#include <string.h>
#include <deque>
#include <list>
#include <cstdlib>
#include <time.h>
#include <unistd.h>

using namespace std;

class coord
{
	public:
	int x,y;
	coord(int y0,int x0)
	{
		y=y0;
		x=x0;
	}
	bool operator==(coord other)
	{
		if (x == other.x && y == other.y) return true;
		else return false;
	}
};

class fruit
{
	public:
	coord position;
	int initTime;
	int lifeTime; //-1 means infinite
	int fruitType;
	
	fruit(int y0,int x0, int initTime0, int lifeTime0, int fruitType0) : position(y0,x0)
	{
		initTime = initTime0;
		lifeTime = lifeTime0;
		fruitType = fruitType0;
	}
	fruit(coord position0, int initTime0, int lifeTime0, int fruitType0) : position(position0)
	{
		initTime = initTime0;
		lifeTime = lifeTime0;
		fruitType = fruitType0;
	}
	
};

bool isFruitReady(int gameTime, list<fruit> &fruitMarket); //Checks whether it is time to produce a fruit
void placeFruit(list<fruit> &fruitList); //Adds a fruit to the list - the fruits get drawn later
void gameOver();

int main()
{
	deque<coord> snake; //Position of snake
	deque<coord>::iterator snakeIterator; //Iterator for snake
	list<fruit> fruitMarket; //List of fruits currently in use
	list<fruit>::iterator fruitIterator; //Iterator for fruits
	
	int initTime = 0; //Epoch time of start of game (seconds)
	int curTime = 0; //Current time, measured in seconds with 0 as time game started
	int ch; //Stores latest character from stdin
	int row,col; //Size of play area (currently dynamic)
	
	int direction=-1; //Direction of motion of snake (-1: uninitialised, 0: up, 1: down, 2: right, 3: left)
	coord predictor(-1,-1); //Predicted position of snake
	
	//Initialise ncurses
	initscr();
	raw();
	keypad(stdscr,TRUE);
	noecho();
	nodelay(stdscr,TRUE);
	
	//Get size of window
	getmaxyx(stdscr,row,col);
	
	snake.push_front(coord(row/2,col/2));
	snake.push_front(coord(row/2,col/2+1));
	
	while(true)
	{
		//Read character from input buffer
		ch=getch();
		
		//Clear the rest of the buffer
		while(getch() != ERR);
		
		//Interpret user input
		if(ch == 'q') break;
		else if(ch == KEY_UP) { if(direction != 1) direction=0; }
		else if(ch == KEY_DOWN) { if(direction != 0) direction=1; }
		else if(ch == KEY_RIGHT) { if(direction != 3) direction=2; }
		else if(ch == KEY_LEFT) { if(direction != 2) direction=3; }
		
		if(direction != -1) //Only do this stuff if the game has started!
		{			
			//Calculate where the snake will move
			predictor = snake.front();
			
			if(direction == 0) predictor.y--;
			else if(direction == 1)	predictor.y++;
			else if(direction == 2) predictor.x++;
			else if(direction == 3)	predictor.x--;
			
			//Sort out fruit related issues
			if(isFruitReady(curTime, initTime)) placeFruit(fruitMarket); //If a fruit is ready to be placed, place it!
			
			//Run through fruit and remove any the snake is about to eat or that are about to expire
			for(fruitIterator=fruitMarket.begin(); fruitIterator != fruitMarket.end(); fruitIterator++)
			{
				if(predictor == (*fruitIterator).position)
				{
					fruitIterator = fruitMarket.erase(fruitIterator);
					fruitIterator--;
				}
				
				if((curTime > ((*fruitIterator).initTime+(*fruitIterator).lifeTime)) && ((*fruitIterator).lifeTime > -1))
				{
					fruitIterator = fruitMarket.erase(fruitIterator);
					fruitIterator--;
				}
			}
			
			//Check if snake is about to hit a wall
			if(predictor.y < 2 || predictor.y > row-2 || predictor.x < 1 || predictor.x > col-2) gameOver();
			
			//Check if snake is about to hit itself
			for(snakeIterator=snake.begin(); snakeIterator != snake.end(); snakeIterator++)	if(predictor == *snakeIterator) gameOver();
			
			//Move snake
			snake.pop_back();
			snake.push_front(predictor);
		}
				
		//Clear window
		clear();
		refresh();
		
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
		for(snakeIterator=snake.begin(); snakeIterator != snake.end(); snakeIterator++)
		{
			mvprintw((*snakeIterator).y,(*snakeIterator).x,"*");
		}
		mvprintw((snake.front()).y,(snake.front()).x,"O");
		
		//Draw fruit!
		for(fruitIterator=fruitMarket.begin(); fruitIterator != fruitMarket.end(); fruitIterator++) mvprintw((*fruitIterator).position.y,(*fruitIterator).position.x,"F");
		
		//Move pointer back to top left hand corner
		move(0,0);
		
		//Draw to console
		refresh();
		
		//Wait for a second
		usleep(0.5*1000000);
	}
	
	while(ch != 'q') ch = getch();
	
	endwin();
	
	return 0;
}

bool isFruitReady(int gameTime, list<fruit> &fruitMarket)
{
	//Fill this in!
	
	return 0;
}

void placeFruit(list<fruit> &fruitList)
{
	//Create new fruit
	//Add it to some vector/list thing
}

void gameOver()
{
	endwin();
	exit(0);
}
