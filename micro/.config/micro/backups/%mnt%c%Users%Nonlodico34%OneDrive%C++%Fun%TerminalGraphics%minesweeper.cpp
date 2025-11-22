#include "pwetty.h"
#include "uteels.h"
#include "vektow2.h"

using namespace std;

#define TW (terminalWidth())
#define TH (terminalHeight())
#define CX (TW / 2)
#define CY (TH / 2)

#define SIZE 11
#define MINES ((SIZE*SIZE)*30)/100;

struct Cell {
	bool empty = true;
	int counter = 0;
	bool mine = false;
	bool flagged = false;
	bool digged = false;
};

int frame;
double start_time;
double game_time;
Cell field[SIZE][SIZE];
bool generated = false;
int cursorX = SIZE/2;
int cursorY = SIZE/2;
bool lost = false;
bool win = false;

void reset(){
	Cell emptyCell;  // viene inizializzata con i valori di default del struct
	
	for (int i = 0; i < SIZE; ++i) {
	    for (int j = 0; j < SIZE; ++j) {
	        field[i][j] = emptyCell;
	    }
	}

	start_time = getTime();
	generated = false;
	lost = false;
	win = false;
}

bool checkWin(){
	for (int i = 0; i < SIZE; ++i) {
	    for (int j = 0; j < SIZE; ++j) {
	    	if(!(field[i][j].digged || field[i][j].mine)) 
	    		return false;
	    }
	}
	return true;
}

void uncover(int x, int y){
	if(!(x>=0 && x<SIZE && y>=0 && y<SIZE)) return; // Bounds
	
	if(!field[x][y].mine && !field[x][y].digged){
		field[x][y].flagged = false;
	    field[x][y].digged = true;
	
	    if(field[x][y].counter == 0){
	        for(int i = x - 1; i <= x + 1; i++){
	            for(int j = y - 1; j <= y + 1; j++){
	                if(i >= 0 && i < SIZE && j >= 0 && j < SIZE){
	                    if(!(i == x && j == y)) { // evita la cella stessa
	                        uncover(i, j);
	                    }
	                }
	            }
	        }
	    }
	}
}

void generateField(){
	int mines = MINES;

	while(mines>0){
		int randX = rand()%SIZE;
		int randY = rand()%SIZE;
		
		// Controlla di non essere nel 3x3 intorno al cursore
   		if(!(abs(cursorX - randX) <= 1 && abs(cursorY - randY) <= 1)){
   			field[randX][randY].mine = true;
   			mines--;
   		}
	}

	// Itera tutte le celle per contare le mine adiacenti
	for(int x=0; x<SIZE; x++){
    	for(int y=0; y<SIZE; y++){
    		if(field[x][y].mine) continue;
    		// Itera il 3x3 intorno alla cella attuale
    		for(int i=x-1; i<=x+1; i++){
    			for(int j=y-1; j<=y+1; j++){
    				if(i>=0 && i<SIZE && j>=0 && j<SIZE){ // Controlla bounds
    					if(field[i][j].mine) field[x][y].counter++;
    				}
    			}
    		}
    	}
    }

    uncover(cursorX, cursorY);
}

void explode(){
	for(int x=0; x<SIZE; x++){
	   	for(int y=0; y<SIZE; y++){
	   		field[x][y].digged = true;
	   	}
	}

	// lost = true;
}

void dig(int x = cursorX, int y = cursorY){
	if(!field[x][y].flagged){
		if(!generated){
			generateField();
			generated = true;
		}

		// Se scavi su un counter completo, QoL
		if(field[x][y].digged && field[x][y].counter != 0){
			int flags = 0;
			for(int i=x-1; i<=x+1; i++){
    			for(int j=y-1; j<=y+1; j++){
    				if(i>=0 && i<SIZE && j>=0 && j<SIZE){ // Controlla bounds
    					if(field[i][j].flagged) flags++;
    				}
    			}
    		}
    		if(flags == field[x][y].counter){
    			for(int i=x-1; i<=x+1; i++){
	    			for(int j=y-1; j<=y+1; j++){
	    				if(i>=0 && i<SIZE && j>=0 && j<SIZE){ // Controlla bounds
	    					if(!field[i][j].digged)
	    						dig(i, j);
	    				}
	    			}
	    		}
    		}
		}

		if(!field[x][y].mine && field[x][y].counter == 0) uncover(x, y);
		field[x][y].digged = true;

		if(field[x][y].mine) explode();
	}
}

void flag(){
	if(!field[cursorX][cursorY].digged){
		field[cursorX][cursorY].flagged = !field[cursorX][cursorY].flagged;
	}
}

void draw()
{
	for(int x=0; x<SIZE; x++){
    	for(int y=0; y<SIZE; y++){
    		Cell &cell = field[x][y];
    		char c = ' ';
    		Color color = WHITE;
    		if(cell.flagged){
    			c = '?';
    			color = BROWN;
    		}else if(!cell.digged){
    			c = '.';
    		} else if(cell.mine){
				c = '@';
				color = RED;
    		} else if(cell.counter != 0){
    			c = cell.counter + '0';
    			switch(cell.counter){
    				case 1:
    					color = TURQUOISE;
    					break;
    				case 2:
    					color = GREEN;
    					break;
    				case 3:
    					color = BLUE;
    					break;
    				default:
    					color = PINK;
    			}
    		}
    		write(x*4 + (CX-SIZE*2) + 2, y*2 + (CY-SIZE) + 1, c, color);
    	}
    }

    write(cursorX*4 + (CX-SIZE*2) + 1, cursorY*2 + (CY-SIZE) + 1, '[');
    write(cursorX*4 + (CX-SIZE*2) + 3, cursorY*2 + (CY-SIZE) + 1, ']');
}

int main()
{
    reset();
    frame = 0;

    showCursor(false);

    while (1)
    {
        clearScreen();

        if(TW < (SIZE * 2 + 3) || TH < (SIZE * 2 + 6)){
       		writeAligned(Alignment::Center, CY, "TERMINAL IS TOO SMALL", RED);
       		render();
	        sleepMs(16); // 60 fps
	        frame++;
       		continue;
       	}

		if(!lost && !win){
	        if (keyPressed())
	        {
	            int c = getKey();
	            switch (c)
	            {
	            case 'c':
	            case 'q':
	            	clearScreen();
				    showCursor(true);
				    return 0;
	            case 'w':
	            case KEY_UP: 
	            	cursorY--;
	            	break;
	            case 'a':
	            case KEY_LEFT:
	            	cursorX--;
	            	break;
	            case 's':
	            case KEY_DOWN:
	            	cursorY++;
	            	break;
	            case 'd':
	            case KEY_RIGHT:
	            	cursorX++;
	            	break;
	            case 'f':
	            	flag();
	            	break;
	            case ' ':
	            case KEY_SPACE:
	            case 'e':
	            case KEY_ENTER:
	            	dig();
	            	break;
	            }
	        }

	        cursorX = clampInt(cursorX, 0, SIZE-1);
	        cursorY = clampInt(cursorY, 0, SIZE-1);
	    } else {
	    	if (keyPressed()){
	    		int c = getKey();
	            switch (c)
	            {
	            	case ' ':
	            	case KEY_SPACE:
						reset();
						break;
				}
	    	}
	    }

        // setMouseInputEnabled(true);
        updateInput();

        win = checkWin();

       	draw();
       	if(lost){
			writeAligned(Alignment::Center, (CY-SIZE) - 2, "Game Over!", RED);
			writeAligned(Alignment::Center, (CY-SIZE) - 1, "Press [SPACE] to restart", GRAY);
       	} else if(win){
       		writeAligned(Alignment::Center, (CY-SIZE) - 2, "You Won!", LIME);
       		writeAligned(Alignment::Center, (CY-SIZE) - 1, "Press [SPACE] to restart", GRAY);
       	} else {
       		game_time = getTime() - start_time;
       	}

       	writeAligned(Alignment::Center, (CY-SIZE) + (SIZE*2) + 2, to_string((int)(game_time)) + "s", WHITE);

        if (getTime() - start_time < 10)
        {
            int i = 0;
            write(1, i++, "[SPACE] Dig", YELLOW);
            write(1, i++, "[F] Flag", YELLOW);
            write(1, i++, "[Q] Quit", YELLOW);
        }

        render();
        sleepMs(16); // 60 fps
        frame++;
    }

    clearScreen();
    showCursor(true);
    return 0;
}
