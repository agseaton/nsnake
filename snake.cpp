#include <ncurses.h>
#include <string.h>
#include <deque>
#include <cstdlib>
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

void gameOver();

int main()
{
	deque<coord> snake;
	deque<coord>::iterator snakeIterator;
	int ch;
	int row,col;
	int direction=-1;
	bool collision=0;
	coord predictor(-1,-1);
	
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
		
		//Work for a Si!
		if(direction != -1)
		{
			//Calculate where the snake will move
			predictor = snake.front();
			
			if(direction == 0) predictor.y--;
			else if(direction == 1)	predictor.y++;
			else if(direction == 2) predictor.x++;
			else if(direction == 3)	predictor.x--;
			
			//Check if snake is about to hit a fruit
			
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

void gameOver()
{
	endwin();
	exit(0);
}
