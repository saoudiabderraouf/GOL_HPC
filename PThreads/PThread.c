#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <stdlib.h>
#define ROWS 2048
#define COLS 2048

int generation = 0;
int population = 0;
int populationMax = 0;
int populationMin = 0;
int tempGrid[ROWS][COLS];

// Barrier variable
pthread_barrier_t barr;

// Number of threads
unsigned int nthreads=2;
int grid[ROWS][COLS];
// Array for thread ids
int *t_ids;

// Struct variables to measure execution time
struct timeval t_start, t_end;
int g=200;
int getUserInput();
int getThreadsNumber();

void initGrid(int, int, int[ROWS][COLS]);
void processGeneration(int, int, int[ROWS][COLS]);
void populationUpdate(int, int, int[ROWS][COLS]);
int countNeighbors(int, int, int[ROWS][COLS], int, int);
void printGrid(int, int, int[ROWS][COLS]);
void sleep(unsigned int);
// Threads' entry function (calculate raw bounds and play game).
void *entry_function(void *ptr);
void play(int , int ,int[ROWS][COLS]);

    //main function
int main()
{
// Default value for number of threads is 1
	
    srand((unsigned int) time(NULL));
    clock_t start=clock();
    initGrid(ROWS, COLS, grid);
    populationUpdate(ROWS, COLS, grid);
    //use this function for small sizes -> printGrid(ROWS,COLS,grid);
    /*remove read actions to get net time 
        g = getUserInput();
        nthreads=getThreadsNumber();*/
    // Create an array with threads given the input number
	pthread_t thr[nthreads];
    // Allocate memory for the thread ids
	t_ids = malloc(nthreads * sizeof(int));
    pthread_barrier_init(&barr, NULL, nthreads);

    // Create the threads
	for(int i = 0; i < nthreads; i++)
    	{
		t_ids[i]=i;
        	if(pthread_create(&thr[i], NULL, &entry_function, (void *)&t_ids[i]))
        	{
            		printf("Could not create thread %d\n", i);
            		return -1;
        	}
    	}
    
   // Proccess's master thread waits for the execution of all threads
	for(int i = 0; i < nthreads; i++)
    	{
        	if(pthread_join(thr[i], NULL))
        	{
            		printf("Could not join thread %d\n", i);
            		return -1;
        	}
    	}
   
    clock_t end=clock();
    double globaltime=(end-start)/CLOCKS_PER_SEC;
    printf( "The process time is %f\n", globaltime );
    
	return 0;
}

    //functions
int getUserInput()
{
	int g;
	printf("Welcome to the Game of Life.\n");
	printf("How many generations do you want to watch? ");
	scanf("%d", &g);
	return g;
}
int getThreadsNumber()
{
	int t;
	printf("Setting of number of threads.\n");
	printf("How many threads do you want to create? ");
	scanf("%d", &t);
	return t;
}

void initGrid(int rows, int cols, int g[rows][cols])
{
	int i, j;
   
	for (i = 0; i < rows; i++)
	{
		for (j = 0; j < cols; j++)
		{
                //array is bounded by [-1]'s
			if (i == 0 || j == 0 || i == (rows - 1) || j == (cols - 1))
			{
				g[i][j] = -1;
			}
			else
			{
                    //initial random grid
				if (rand() % 2)
                {
                    g[i][j] = 1;
                    population++;
                }
                else
                {
                    g[i][j] = 0;
                }
			}
		}
        
	}
    processGeneration(rows, cols, g);
}

void processGeneration(int rows, int cols, int g[rows][cols])
{
    int i, j, neighbors;
    for(i = 0; i < rows; i++)
    {
        for(j = 0; j < cols; j++)
        {
                //ignore borders
            if (g[i][j] == -1) continue;
                //get number of neighbors
            neighbors = countNeighbors(rows, cols, g, i, j);
                //death conditions
            if(g[i][j] == 1 && (neighbors < 2 || neighbors > 3))
            {
                tempGrid[i][j] = 0;
            }
                //birth conditions
            else if (g[i][j] == 0 && neighbors == 3)
            {
                tempGrid[i][j] = 1;
            }
        }
    }
}

void populationUpdate(int rows, int cols, int g[rows][cols])
{
    int i, j;
        //clean slate for population counting
    population = 0;
        //population counting
    for(i = 0; i < rows; i++)
    {
        for(j = 0; j < cols; j++)
        {
            if(g[i][j] == -1) continue;
            if(tempGrid[i][j] == 1) population++;
            g[i][j] = tempGrid[i][j];
        }
    }
    if (population > populationMax) 
    {
        populationMax = population;
    }
    if (population < populationMin || populationMin == 0)
    {
        populationMin = population;
    }
}

int countNeighbors(int rows, int cols, int g[rows][cols], int x, int y)
{
    int n = 0, i, j;
    
    for (j = y - 1; j < y + 2; j++)
    {
        for (int i = x - 1; i < x + 2; i++)
        {
            if (i == x && j == y)
            {
                continue;
            }
            if (g[i][j] != -1)
            {
                n += g[i][j];
            }
        }
    }
    return n;
}

void printGrid(int rows, int cols, int g[rows][cols])
{
    system("clear");
    printf("Welcome to the Game of Life! Generation %d\n", generation);
    printf("Population: %d [MAX %d] [ MIN %d]\n", population, populationMax, populationMin);
    int i, j;
    for (i = 0; i < rows; i++)
    {
        for (j = 0; j < cols; j++)
        {
            switch (g[i][j])
            {
                case -1: putchar('#'); break;
                case  0: putchar(' '); break;
                case  1: putchar('X'); break;
                default: break;
            }
        }
        putchar('\n');
    }
}

void play (int rows, int cols, int grid[rows][cols]){
    generation++;
    processGeneration(ROWS, COLS, grid);
  
    //use this function for small sizes -> printGrid(ROWS,COLS,grid);
}
void *entry_function(void *t_id)
{
	int *thread_id=(int*)t_id;

	// Calculate the array bounds that each thread will process
	int bound = ROWS / nthreads;
	int start = *thread_id * bound;
	int finish = start + bound;

	int i,bn;

	// exclude extern cells
	if(*thread_id==0) start++;
	if(*thread_id==nthreads-1) finish=COLS-1;

	// Play the game for 100 rounds
	for (i=0; i<g; i++)
	{	
		play (ROWS, COLS, grid);
	
		/* ------ Synchronization point with barries ------
		The pthread_barrier_wait subroutine synchronizes participating threads
		 at the barrier referenced by barrier. 
		The calling thread blocks until the required number of threads have called 
		pthread_barrier_wait specifying the barrier.

		When the required number of threads have called pthread_barrier_wait specifying the barrier, 
		the constant PTHREAD_BARRIER_SERIAL_THREAD is returned to one unspecified thread 
		and 0 is returned to the remaining threads. 

		At this point, the barrier resets to the state it had as a result of 
		the most recent pthread_barrier_init function that referenced it.
		*/
    		bn = pthread_barrier_wait(&barr);
            
    		if(bn != 0 && bn != PTHREAD_BARRIER_SERIAL_THREAD)
    		{
        		printf("Could not wait on barrier\n");
        		exit(-1);
   		}
		
		/* 
		The thread with ID=0 is responsible to swap the pointers and
		print the board's status in each round.
		ps: The screen prints are omitted in bench mode (bench_flag)
		*/
		if(bn==PTHREAD_BARRIER_SERIAL_THREAD)
		{
		  populationUpdate(ROWS, COLS, grid);	
				
		}
		

		//One more barrier is needed in order to ensure
		//that the pointers have been swapped before go to next round
		bn = pthread_barrier_wait(&barr);
    		if(bn != 0 && bn != PTHREAD_BARRIER_SERIAL_THREAD)
    		{
        		printf("Could not wait on barrier\n");
        		exit(-1);
   		}
	}// End of g play rounds for loop
	return 0;
}//End of entry function


void sleep(unsigned int mill)
{
    clock_t start = clock();
    while (clock() - start < mill) { }
}



    //end of the code
