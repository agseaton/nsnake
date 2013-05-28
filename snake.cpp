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
	time_t initTime;
	time_t expiryTime; //-1 means infinite
	int fruitType;
	
	fruit(int y0,int x0, time_t initTime0, time_t expiryTime0, int fruitType0) : position(y0,x0)
	{
		initTime = initTime0;
		expiryTime = expiryTime0;
		fruitType = fruitType0;
	}
	fruit(coord position0, time_t initTime0, time_t expiryTime0, int fruitType0) : position(position0)
	{
		initTime = initTime0;
		expiryTime = expiryTime0;
		fruitType = fruitType0;
	}
	
};

bool isFruitReady(int gameTime, list<fruit> &fruitMarket); //Checks whether it is time to produce a fruit
void placeFruit(list<fruit> &fruitList); //Adds a fruit to the list - the fruits get drawn later
void gameOver();

int main()
{
	deque<coord> snake; //Position of snake
	list<fruit> fruitMarket; //List of fruits currently in use
	
	time_t initTime = time(NULL); //Epoch time of start of game (seconds)
	time_t gameTime = 0; //Current time, measured in seconds with 0 as time game started
	int ch; //Stores latest character from stdin
	int row,col; //Size of play area (currently dynamic)
	
	int direction=-1; //Direction of motion of snake (-1: uninitialised, 0: up, 1: down, 2: right, 3: left)
	coord predictor(-1,-1); //Predicted position of snake
	bool gotFruit = false;
	bool growSnake = false;
	
	//Initialise ncurses
	initscr();
	raw();
	keypad(stdscr,TRUE);
	noecho();
	nodelay(stdscr,TRUE);
	
	//Get size of window
	getmaxyx(stdscr,row,col);
	
	//And God created the snake, saying, "Be fruitful and multiply"
	snake.push_front(coord(row/2,col/2));
	snake.push_front(coord(row/2,col/2+1));
	snake.push_front(coord(row/2-1,col/2+1));
	snake.push_front(coord(row/2-1,col/2));
	
	//Add a test fruit!
	fruitMarket.push_front(fruit(row/2,col/2,gameTime,-1,0));
	
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
			//Get the time
			gameTime = time(NULL)-initTime;
			
			//Calculate where the snake will move
			predictor = snake.front();
			
			if(direction == 0) predictor.y--;
			else if(direction == 1)	predictor.y++;
			else if(direction == 2) predictor.x++;
			else if(direction == 3)	predictor.x--;
			
			//Sort out fruit related issues
			if(isFruitReady(gameTime, fruitMarket)) placeFruit(fruitMarket); //If a fruit is ready to be placed, place it!
			
			if(gotFruit)
			{
				growSnake = true;
				gotFruit = false;
			}
			
			//Run through fruit and remove any the snake is about to eat or that are about to expire
			for(list<fruit>::iterator i=fruitMarket.begin(); i != fruitMarket.end(); i++)
			{
				//Warning: Iterator after an item is removed may not be valid!
				//Remove expiring fruit
				if((gameTime > (*i).expiryTime) && ((*i).expiryTime != -1))
				{
					i = fruitMarket.erase(i);
					i--;
					continue;
				}
				
				//Remove fruit that are in the path of the snake
				if(predictor == (*i).position)
				{
					i = fruitMarket.erase(i);
					i--;
					gotFruit = true;
				}
			}
			
			//Check if snake is about to hit a wall
			if(predictor.y < 2 || predictor.y > row-2 || predictor.x < 1 || predictor.x > col-2) gameOver();
			
			//Check if snake is about to hit itself
			//Note: the snake can move into the space currently occupied by the last part of its tail, unless it has just received a fruit.
			for(deque<coord>::iterator i=snake.begin();
			    ((i != (--snake.end())) && (!growSnake)) || ((i != snake.end()) && growSnake);
			    i++)
			{ if(predictor == *i) gameOver(); }
			
			//Move snake
			if(growSnake != true) snake.pop_back();
			else growSnake = false;
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
		for(deque<coord>::iterator i=snake.begin(); i != snake.end(); i++)
		{
			mvprintw((*i).y,(*i).x,"*");
		}
		mvprintw((snake.front()).y,(snake.front()).x,"O");
		
		//Draw fruit!
		for(list<fruit>::iterator i=fruitMarket.begin(); i != fruitMarket.end(); i++) mvprintw((*i).position.y,(*i).position.x,"F");
		
		//Draw timer
		mvprintw(0,col/2,"%i",gameTime);
		
		//Move cursor back to top left hand corner
		move(0,0);
		
		//Draw to console
		refresh();
		
		//Wait for a second
		usleep(1*1000000);
	}
	
	while(ch != 'q') ch = getch();
	
	endwin();
	
	return 0;
}

bool isFruitReady(int gameTime, list<fruit> &fruitMarket)
{
	//Find the time at which the last fruit was placed
	int latestFruitTime = 0;
	for(list<fruit>::iterator i=fruitMarket.begin(); i != fruitMarket.end(); i++) if(latestFruitTime > (*i).initTime) latestFruitTime = (*i).initTime;
	
	return 0;
}

void placeFruit(list<fruit> &fruitList)
{
	//Create new fruit
	//Add it to some vector/list thing
}

void gameOver()
{
	int row, col;
	char ch;
	
	//Clear window
	clear();
	refresh();
	
	//Get size of window
	getmaxyx(stdscr,row,col);
	
	//Print game over text
	mvprintw(row/2-3,col/2-38,"  ____       _         _  _      _____          ___             _____  ___");
	mvprintw(row/2-2,col/2-38," /    \\     / \\       / \\/ \\    |              /   \\  \\      / |      |   \\ ");
	mvprintw(row/2-1,col/2-38,"|          /___\\     /      \\   |___          /     \\  \\    /  |___   |___/");
	mvprintw(row/2,col/2-38,"|    ___  /     \\   /        \\  |             \\     /   \\  /   |      |   \\ ");
	mvprintw(row/2+1,col/2-38," \\____/  /       \\ /          \\ |_____         \\___/     \\/    |_____ |    \\ ");
	mvprintw(row-1,col/2-strlen("Press 'q' to return to menu")/2,"Press 'q' to return to menu");
	
	//Move pointer back to top left hand corner
	move(0,0);
	
	//Draw to console
	refresh();
	
	while(ch != 'q') ch = getch();
	
	//End the game
	endwin();
	exit(0);
}
