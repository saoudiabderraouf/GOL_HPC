#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <mpi.h> 
#include <omp.h>
#define ROWS 2048
#define COLS 2048
  
// process number must be multipe of ROWS

int nbThreads=2;
int nbprocess,processId,processCount,generation = 0;
int population = 0;
int populationMax = 0;
int populationMin = 0;
int grid[ROWS][COLS];
int tempGrid[ROWS][COLS];

int getUserInput();
int getThreadsNumber();
void initGrid(int, int, int[ROWS][COLS]);
void processGeneration(int, int, int[ROWS][COLS]);
int  countNeighbors(int, int, int[ROWS][COLS], int, int);
void printGrid(int, int, int[ROWS][COLS]);
void mergeResult(int[ROWS][COLS], int[ROWS][COLS], int *);
void sleep(unsigned int);

    //main function
int main()
{
    
    int res,i,g=200,size=COLS*ROWS,tmpMax,tmpMin;
    MPI_Init(NULL, NULL);
    double start,end,globaltime; 
    start = MPI_Wtime(); 
    srand((unsigned int) time(NULL));
    MPI_Op customOp; 
	MPI_Status status;
    MPI_Request request;
	MPI_Comm_size(MPI_COMM_WORLD, &nbprocess);
	MPI_Comm_rank(MPI_COMM_WORLD, &processId);
    MPI_Op_create( (MPI_User_function *)mergeResult, size, &customOp ); 
    processCount=ROWS/nbprocess;
    if(processId==0) {
        initGrid(ROWS, COLS, grid);
        //use this function for small sizes -> printGrid(ROWS,COLS,grid);
        /* remove read actions to get net time
        g = getUserInput();
        nbThreads=getThreadsNumber();*/
        omp_set_num_threads(nbThreads);
    };
    /** wait the first process to init grid ***/
    MPI_Barrier(MPI_COMM_WORLD);
    /// broadcast data to all process 
    MPI_Bcast(&g, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&grid, size, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&tempGrid, size, MPI_INT, 0, MPI_COMM_WORLD);
    for (i = 0; i < g; i++){
        processGeneration(ROWS, COLS, grid);
        MPI_Allreduce( &tempGrid, &grid, size, MPI_INT, customOp, MPI_COMM_WORLD );
        /***********
         *  part to print grid
            if(processId==0) printGrid(ROWS,COLS,grid);
            MPI_Barrier(MPI_COMM_WORLD);
        *//////////
    }
    end = MPI_Wtime(); 
    start=end-start;
    MPI_Reduce(&start, &globaltime, 1, MPI_DOUBLE, MPI_MAX, 0,MPI_COMM_WORLD);
    if(processId==0) printf("The process time is %f\n", globaltime );
    MPI_Finalize();
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
void mergeResult(int in[ROWS][COLS], int out[ROWS][COLS], int *size){
    int i,j,rankProcess=in[1][1];
    int start=rankProcess*processCount,end=(rankProcess+1)*processCount;
    #pragma omp parallel for private(j) schedule(static)
    for(i = start; i < end; i++)
    {
        for(j = 0; j < COLS; j++)
        {
            if(in[i][j] == -1) continue;
            if(in[i][j] == 1) population++;
            out[i][j] = in[i][j];
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

void initGrid(int rows, int cols, int g[rows][cols])
{
	int i, j;
    #pragma omp parallel for private(j) schedule(static)
	for (i =0 ; i < rows; i++)
	{
		for (j = 0; j < cols; j++)
		{
                //array is bounded by [-1]'s
			if (i == 0 || j == 0 || i == (rows - 1) || j == (cols - 1))
			{
                g[i][j] = -1;
                tempGrid[i][j] = -1;
			}
			else
			{
                //
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
}

void processGeneration(int rows, int cols, int g[rows][cols])
{
    int i, j, neighbors;
    int start=processId*processCount,end=(processId+1)*processCount;
    /**** put the process id in the first case to be used in reduce function*/
    if(processId!=0) tempGrid[1][1]=processId;
    #pragma omp parallel for private(j) schedule(static)
    for(i = start; i < end; i++)
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


void sleep(unsigned int mill)
{
    clock_t start = clock();
    while (clock() - start < mill) { }
}
