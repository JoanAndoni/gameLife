/*
    Name: Joan Andoni González Rioz

   Definition of an image data structure for PGM format images
   Uses typedef, struct
   Uses functions to read and write a file in PGM format, described here:
    http://netpbm.sourceforge.net/doc/pgm.html
    http://rosettacode.org/wiki/Bitmap/Write_a_PGM_file#C

   Gilberto Echeverria
   gilecheverria@yahoo.com
   06/10/2016
*/

#include "pgm_image.h"

// Initialize the mutex to help the threads to not share memory
pthread_mutex_t lock;

// Generate space to store the image in memory
void allocateImage(image_t * image)
{
    // Allocate the memory for INDEX array
    image->pixels = malloc (image->height * sizeof(pixel_t *));
    // Allocate the memory for all the DATA array
    image->pixels[0] = calloc (image->height * image->width, sizeof(pixel_t));

    // Add the rest of the pointers to the INDEX array
    for (int i=1; i<image->height; i++)
    {
        // Add an offset from the beginning of the DATA array
        image->pixels[i] = image->pixels[0] + image->width * i;
    }
}

// Release the dynamic memory used by an image
void freeImage(image_t * image)
{
    // Free the DATA array
    free (image->pixels[0]);
    // Free the INDEX array
    free (image->pixels);

    // Set the values for an empty image
    image->width = 0;
    image->height = 0;
    image->pixels = NULL;
}

void readBoard(const char * filename, image_t * image)
{
    FILE * file_ptr = NULL;

    printf("\nReading file: '%s'\n", filename);
    // Open the file
    file_ptr = fopen(filename, "r");
    if (file_ptr == NULL)
    {
        printf("Error: Unable to open file '%s'\n", filename);
        exit(EXIT_FAILURE);
    }

    fscanf(file_ptr, "%d", &image->height);
    fscanf(file_ptr, "%d", &image->width);

    printf("Width: %d, Height: %d\n",image->width, image->height );

    // Allocate the memory for the pixels in the board
    allocateImage(image);

    // Read the data for the pixels
    for (int i=0; i<image->height; i++)
    {
        for (int j=0; j<image->width; j++)
        {
            // Read the value for the pixel
            fscanf(file_ptr, "%hhu", &(image->pixels[i][j].value));
        }
    }

    // Close the file
    fclose(file_ptr);
    printf("Done!\n");
}

void playGame(image_t * image)
{
    // Local variable for an image structure
    image_t destination = {0, 0, NULL};

    // Local variable for using easier the matrix array
    int double_matrix[3][3] = {0};

    // Copy the size of the image
    destination.height = image->height;
    destination.width = image->width;

    // Get the memory for the image data
    allocateImage(&destination);

    // Check the neighbors of all pixels
    for (int row = 0; row < image->height; row++)
    {
        for (int column = 0; column < image->width; column++)
        {
            for (int matrixRow = -1; matrixRow <= 1; matrixRow++) {
                for (int matrixColumn = -1; matrixColumn <= 1; matrixColumn++) {
                    int r = row+matrixRow;
                    int c = column+matrixColumn;
                    if (r < 0) {
                        r = image->height - 1;
                    } else if (r == image->height) {
                        r = 0;
                    }
                    if (c < 0) {
                        c = image->width - 1;
                    } else if (r == image->width) {
                        c = 0;
                    }
                    double_matrix[matrixRow+1][matrixColumn+1] = image->pixels[r][c].value;
                }
            }
            destination.pixels[row][column].value = checkNeighbors(double_matrix);
        }
    }

    // Free the previous memory data
    freeImage(image);
    // Copy the results back to the pointer received
    *image = destination;
}

void playGameOMP(image_t * image)
{
    // Local variable for an image structure
    image_t destination = {0, 0, NULL};

    // Local variable for using easier the matrix array
    int double_matrix[3][3] = {0};

    // Copy the size of the image
    destination.height = image->height;
    destination.width = image->width;

    // Get the memory for the image data
    allocateImage(&destination);

    // Make the paralelism with Open MP
    #pragma omp parallel for collapse(2) private(double_matrix)
    // Check the neighbors of all pixels
    for (int row = 0; row < image->height; row++)
    {
        for (int column = 0; column < image->width; column++)
        {
            for (int matrixRow = -1; matrixRow <= 1; matrixRow++) {
                for (int matrixColumn = -1; matrixColumn <= 1; matrixColumn++) {
                    int r = row+matrixRow;
                    int c = column+matrixColumn;
                    if (r < 0) {
                        r = image->height - 1;
                    } else if (r == image->height) {
                        r = 0;
                    }
                    if (c < 0) {
                        c = image->width - 1;
                    } else if (r == image->width) {
                        c = 0;
                    }
                    double_matrix[matrixRow+1][matrixColumn+1] = image->pixels[r][c].value;
                }
            }
            destination.pixels[row][column].value = checkNeighbors(double_matrix);
        }
    }

    // Free the previous memory data
    freeImage(image);
    // Copy the results back to the pointer received
    *image = destination;
}

void playGameThreads(image_t * image)
{
    // Set the data that is going to be sended to the threads
    data_t * thread_data = NULL;
    pthread_t tid[NUM_THREADS];

    // Make the calculations to split the iterations
    int stepsPerThread = image->height / NUM_THREADS;
    int stepsLeft = image->height % NUM_THREADS;
    int start = 0;

    for (int i = 0; i < NUM_THREADS; i++) {
        // Allocate memory for the information that is going to be
        // sended to the thread
        thread_data = malloc (sizeof (data_t));

        // Fill the data of the thread with the iterations
        thread_data->start = start;
        thread_data->steps = stepsPerThread;
        thread_data->image = image;

        // Check if there is some iterations that have to be done
        if (stepsLeft > 0) {
            thread_data->steps++;
            stepsLeft--;
        }

        //Make thread
        pthread_create(&tid[i], NULL, threadsGame, (void *)thread_data);
        start += thread_data->steps;
    }

    // Wait the end of the threads
    for (int i = 0; i < NUM_THREADS; i++) {
      pthread_join(tid[i], NULL);
    }
}

void * threadsGame(void * arg)
{
    // Start the lock of the mutex here cause the threads are sharing
    // information and the most important is that if i dont lock it it frees
    // the memory of image multiple times and makes a sementation fault
    pthread_mutex_lock(&lock);

    // Recive the information of the thread
    data_t * thread_data = (data_t *) arg;

    // Local variable for an image structure
    image_t destination = {0, 0, NULL};

    // Local variable for using easier the matrix array
    int double_matrix[3][3] = {0};

    // Copy the size of the image
    destination.height = thread_data->image->height;
    destination.width = thread_data->image->width;

    // Get the memory for the image data
    allocateImage(&destination);

    // Check the neighbors of all pixels
    for (int row = 0; row < thread_data->image->height; row++)
    {
        for (int column = 0; column < thread_data->image->width; column++)
        {
            for (int matrixRow = -1; matrixRow <= 1; matrixRow++) {
                for (int matrixColumn = -1; matrixColumn <= 1; matrixColumn++) {
                    int r = row+matrixRow;
                    int c = column+matrixColumn;
                    if (r < 0) {
                        r = thread_data->image->height - 1;
                    } else if (r == thread_data->image->height) {
                        r = 0;
                    }
                    if (c < 0) {
                        c = thread_data->image->width - 1;
                    } else if (r == thread_data->image->width) {
                        c = 0;
                    }
                    double_matrix[matrixRow+1][matrixColumn+1] = thread_data->image->pixels[r][c].value;
                }
            }
            destination.pixels[row][column].value = checkNeighbors(double_matrix);
        }
    }

    // Free the previous memory data
    freeImage(thread_data->image);

    // Copy the results back to the pointer received
    *thread_data->image = destination;

    // Unlock the mutex
    pthread_mutex_unlock(&lock);

    // Exit the thread
    pthread_exit(NULL);
}

// Function to check the neighbors and return the value of the
// statement for the pixel
int checkNeighbors(int matrix[3][3])
{
    // Initialize the value of the neighbors and the count for
    // the number of neighbors 1 and the neighbors 2

    int neighbors = 0, repetitions[2] = { 0, 0 }, value = 0, live = 0;

    for (int matrixRow = 0; matrixRow < 3; matrixRow++) {
        for (int matrixColumn = 0; matrixColumn < 3; matrixColumn++) {
            if (matrixRow == 1 && matrixColumn == 1) {
                continue;
            }
            if (matrix[matrixRow][matrixColumn] > 0) {
                switch (matrix[matrixRow][matrixColumn]) {
                    case 1:
                        repetitions[0] += 1;
                    break;

                    case 2:
                        repetitions[1] += 1;
                    break;
                }
                neighbors += 1;
            }
        }
    }

    if (repetitions[1] >= repetitions[0]) {
        value = 2;
    } else if (repetitions[1] < repetitions[0]) {
        value = 1;
    }

    if (neighbors >= 4) {
        // Over population
        live = 0;
    } else if ( neighbors == 3 ) {
        // Revive or stay with the live value
        live = 1;
    } else if ( neighbors == 2 ){
        // Stay with the value that it has
        if ( matrix[1][1] >= 1 ) {
            // The pixel is alive
            live = 1;
        } else {
            // The pixel is dead
            live = 0;
        }
    } else if ( neighbors < 2 ) {
        // Underpopulation or dead pixel
        live = 0;
    }

    return live * value;
}

// Write the data in the image structure into a new PGM file
// Receive a pointer to the image, to avoid having to copy the data
void writePGMFile(const char * filename, const pgm_t * pgm_image)
{
    FILE * file_ptr = NULL;

    //printf("\nWriting file: '%s'\n", filename);
    // Open the file
    file_ptr = fopen(filename, "w");
    if (file_ptr == NULL)
    {
        printf("Error: Unable to open file '%s'\n", filename);
        exit(EXIT_FAILURE);
    }

    // Write the header for the file
    fprintf(file_ptr, "%s\n", pgm_image->magic_number);
    fprintf(file_ptr, "# %s\n", filename);
    fprintf(file_ptr, "%d %d\n", pgm_image->image.width, pgm_image->image.height);
    fprintf(file_ptr, "%d\n", pgm_image->max_value);

    // Write the data acording to the type
    if ( !strncmp(pgm_image->magic_number, "P2", 3) )
    {
        writePGMTextData(pgm_image, file_ptr);
    }
    else if ( !strncmp(pgm_image->magic_number, "P5", 3) )
    {
        writePGMBinaryData(pgm_image, file_ptr);
    }
    else
    {
        printf("Invalid file format. Unknown type: %s\n", pgm_image->magic_number);
        exit(EXIT_FAILURE);
    }
    fclose(file_ptr);
    //printf("Done!\n");
}

// Write the data in the image structure into a new PGM file
// Receive a pointer to the image, to avoid having to copy the data
void writePGMTextData(const pgm_t * pgm_image, FILE * file_ptr)
{
    // Write the pixel data
    for (int i=0; i<pgm_image->image.height; i++)
    {
        for (int j=0; j<pgm_image->image.width; j++)
        {
            fprintf(file_ptr, "%d", pgm_image->image.pixels[i][j].value);
            // Separate pixels in the same row with tabs
            if (j < pgm_image->image.width-1)
                fprintf(file_ptr, "\t");
            else
                fprintf(file_ptr, "\n");
        }
    }
}

// Write the data in the image structure into a new PGM file
// Receive a pointer to the image, to avoid having to copy the data
void writePGMBinaryData(const pgm_t * pgm_image, FILE * file_ptr)
{
    unsigned char * data = NULL;
    int k = 0;
    // Write the pixel data
    size_t pixels = pgm_image->image.height * pgm_image->image.width;
    // Create an array with the raw data for the image
    data = (unsigned char *) malloc (pixels * 3 * sizeof (unsigned char));
    // Copy the data into the new array
    for (int i=0; i<pgm_image->image.height; i++)
    {
        for (int j=0; j<pgm_image->image.width; j++)
        {
            // Copy the data into the correct structure
            data[k++] = pgm_image->image.pixels[i][j].value;
        }
    }
    // Write the binary data to the file
    fwrite(data, sizeof (unsigned char), pixels, file_ptr);
    // Release the temporary array
    free(data);
}
