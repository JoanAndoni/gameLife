/*
    WRITE YOUR NAME HERE: Joan Andoni González Rioz

    Simulation of Conway's Game of Life using OpenMP
    Based on the explanations at:
        https://en.wikipedia.org/wiki/Conway%27s_Game_of_Life
    Online versions:
        http://pmav.eu/stuff/javascript-game-of-life-v3.1.1/
        https://bitstorm.org/gameoflife/

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pgm_image.h"
// ADD YOUR EXTRA LIBRARIES HERE
#include <omp.h>
#include <time.h>

#include <pthread.h>
#include <semaphore.h>
#include <sys/time.h>

#define STRING_SIZE 50

///// Function declarations /////
void usage(const char * program);
void lifeSimulation(int iterations, char * start_file, int option);
void lifeSimulationThreads(int iterations, char * start_file);

int main(int argc, char * argv [])
{
    char * start_file = "Boards/pulsar.txt";
    int iterations = 5;

    printf("\n=== GAME OF LIFE ===\n");
    printf("{By: Joan Andoni Gonzalez Rioz}\n");

    // Parsing the arguments
    if (argc == 2)
    {
        iterations = atoi(argv[1]);
    }
    else if (argc == 3)
    {
        iterations = atoi(argv[1]);
        start_file = argv[2];
    }
    else if (argc != 1)
    {
        usage(argv[0]);
        return 1;
    }

    int option = 0;

    printf("Ok... so we are going to make it with paralelism choose wich option ");
    printf("you want to use or if you dont want to use it chose the correct option.\n");
    printf("1.- OpenMP\n");
    printf("2.- Manual Threads\n");
    printf("0.- No paralelism\n\tR: ");
    scanf("%d", &option);

    if (option == 1) {
        // Run the simulation with the iterations specified
        printf("\nRunning the simulation with file '%s' using %d iterations and OpenMP\n", start_file, iterations);
        lifeSimulation(iterations, start_file, option);
    } else if (option == 2) {
        // Run the simulation with the iterations specified
        printf("\nRunning the simulation with file '%s' using %d iterations and manual threads\n", start_file, iterations);
        lifeSimulationThreads(iterations, start_file);
    } else {
        // Run the simulation with the iterations specified
        printf("\nRunning the simulation with file '%s' using %d iterations without threads\n", start_file, iterations);
        lifeSimulation(iterations, start_file, 0);
    }

    return 0;
}

// Print usage information for the program
void usage(const char * program)
{
    printf("Usage:\n");
    printf("%s [iterations] [board file]\n", program);
}

// Main loop for the simulation
void lifeSimulation(int iterations, char * start_file, int option)
{
    // Create the image variables and initialize to be empty
    image_t board = {0, 0, NULL};

    // Put the board into a image
    pgm_t imagePGM = { "P5", 2};

    //Read the board to put it into the image format
    readBoard(start_file, &board);

    // Erase the last 4 characters of the name of the file
    start_file[strlen(start_file) - 4] = '\0' ;
    start_file = &start_file[7];

    // Start the time to count the execution
    double start = omp_get_wtime();

    for (int i = 0; i < iterations; i++) {
        // Create an array that will have the name of the files on every iteration
        char buffer[32] = { 0 };

        if (option == 1) {
            // Play the game with OMP
            playGameOMP(&board);
        } else {
            // Play the game
            playGame(&board);
        }

        // Add the file name and the sub carpet to save the files
        sprintf(buffer, "Images/%s_%d.pgm", start_file, i);

        // Add the data of the board to the image
        imagePGM.image = board;

        // Save the new image
        writePGMFile(buffer, &imagePGM);
    }

    // Liberate the memory used for the structure
    freeImage(&(imagePGM.image));
    printf("\nThe execution time is : %f s\n", omp_get_wtime()-start);
    printf("\nThe images are successfully write in the folder of Images\n");
}

// Main loop for the simulation
void lifeSimulationThreads(int iterations, char * start_file)
{
    // Create the image variables and initialize to be empty
    image_t board = {0, 0, NULL};

    // Put the board into a image
    pgm_t imagePGM = { "P5", 2};

    //Read the board to put it into the image format
    readBoard(start_file, &board);

    // Erase the last 4 characters of the name of the file
    start_file[strlen(start_file) - 4] = '\0' ;
    start_file = &start_file[7];

    // Start the time to count the execution
    double start = omp_get_wtime();

    for (int i = 0; i < iterations; i++) {
        // Create an array that will have the name of the files on every iteration
        char buffer[32] = { 0 };

        //Play the game without paralelism
        playGame(&board);

        // Add the file name and the sub carpet to save the files
        sprintf(buffer, "Images/%s_%d.pgm", start_file, i);

        // Add the data of the board to the image
        imagePGM.image = board;

        // Save the new image
        writePGMFile(buffer, &imagePGM);
    }

    // Liberate the memory used for the structure
    freeImage(&(imagePGM.image));
    printf("\nThe execution time is : %f s\n", omp_get_wtime()-start);
    printf("\nThe images are successfully write in the folder of Images\n");
}
