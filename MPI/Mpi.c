#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <mpi.h> 
#define ROWS 1024
#define COLS 1024
  
// process number must be multipe of ROWS

int nbprocess,processId,processCount,generation = 0;
int population = 0;
int populationMax = 0;
int populationMin = 0;
int grid[ROWS][COLS];
int tempGrid[ROWS][COLS];

int getUserInput();
void initGrid(int, int, int[ROWS][COLS]);
void processGeneration(int, int, int[ROWS][COLS]);
void populationUpdate(int, int, int[ROWS][COLS],int);
int countNeighbors(int, int, int[ROWS][COLS], int, int);
void printGrid(int, int, int[ROWS][COLS]);
void sleep(unsigned int);

    //main function
int main()
{
    
    int res,i,g,size=COLS*ROWS,tmpMax,tmpMin;
    MPI_Init(NULL, NULL);
    double start,end,globaltime; 
    start = MPI_Wtime(); 
    srand((unsigned int) time(NULL));
	MPI_Status status;
    MPI_Request request;
	MPI_Comm_size(MPI_COMM_WORLD, &nbprocess);
	MPI_Comm_rank(MPI_COMM_WORLD, &processId);
    processCount=ROWS/nbprocess;
    if(processId==0) {
        initGrid(ROWS, COLS, grid);
        //use this function for small sizes -> printGrid(ROWS,COLS,grid);
        g = getUserInput();
    };
    /** wait the first process to init grid ***/
    MPI_Barrier(MPI_COMM_WORLD);
    /** broadcast data to all process ***/
    MPI_Bcast(&g, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&grid, size, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&tempGrid, size, MPI_INT, 0, MPI_COMM_WORLD);
    for (i = 0; i < g; i++)
    {
        /** calculate part of generation  ***/
        processGeneration(ROWS, COLS, grid);
        if(processId!=0) {
            /** send result to the first process  ***/
            MPI_Isend(&tempGrid, size, MPI_INT,0, 0, MPI_COMM_WORLD,&request);
        }
        MPI_Barrier(MPI_COMM_WORLD);
        if(processId==0){
            //printGrid(ROWS,COLS,tempGrid);
            int recievedGrid[ROWS][COLS],k;
            generation++;
            population = 0;
            /** Update population  ***/
            populationUpdate(ROWS,COLS,tempGrid,0);
            for(k=1;k<nbprocess;k++){
                MPI_Recv(&recievedGrid, size, MPI_INT, k, 0, MPI_COMM_WORLD,&status);
                populationUpdate(ROWS,COLS,recievedGrid,k);
            }
            //use this function for small sizes -> printGrid(ROWS,COLS,grid);   
        }
        MPI_Barrier(MPI_COMM_WORLD);
        /** broadcast grids  ***/
        MPI_Bcast(&grid, size, MPI_INT, 0, MPI_COMM_WORLD); 
        MPI_Bcast(&tempGrid, size, MPI_INT, 0, MPI_COMM_WORLD); 
    }
    end = MPI_Wtime(); 
    start=end-start;
    MPI_Reduce(&start, &globaltime, 1, MPI_DOUBLE, MPI_MAX, 0,MPI_COMM_WORLD);
    if(processId==0) printf( "The process time is %f\n", globaltime );
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
void initGrid(int rows, int cols, int g[rows][cols])
{
	int i, j;
	for (i =0 ; i < rows; i++)
	{
		for (j = 0; j < cols; j++)
		{
                //array is bounded by [-1]'s
			if (i == 0 || j == 0 || i == (rows - 1) || j == (cols - 1))
			{
				tempGrid[i][j] = -1;
                g[i][j] = -1;
			}
			else
			{
                if (rand() % 2)
                {
                    tempGrid[i][j] = 1;
                    g[i][j] = 1;
                    population++;
                }
                else
                {
                    tempGrid[i][j] = 0;
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

void populationUpdate(int rows, int cols, int g[rows][cols],int rankProcess)
{
    int i, j;
    int start=rankProcess*processCount,end=(rankProcess+1)*processCount;
    for(i = start; i < end; i++)
    {
        for(j = 0; j < cols; j++)
        {
            if(g[i][j] == -1) continue;
            if(grid[i][j] == 1) population++;
            grid[i][j] = g[i][j];
            tempGrid[i][j]=g[i][j];
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


void sleep(unsigned int mill)
{
    clock_t start = clock();
    while (clock() - start < mill) { }
}
