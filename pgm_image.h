/*
   Definition of an image data structure for PGM format images
   Uses typedef, struct
   Uses functions to read and write a file in PGM format, described here:
    http://netpbm.sourceforge.net/doc/pgm.html
    http://rosettacode.org/wiki/Bitmap/Write_a_PGM_file#C

   Gilberto Echeverria
   gilecheverria@yahoo.com
   04/15/2017
*/

#ifndef PGM_IMAGE_H
#define PGM_IMAGE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "string_functions.h"

// Constant for the size of strings to be read from a PGM file header
#define LINE_SIZE 255

//// TYPE DECLARATIONS

// Structure for a pixel color information, using RGB components
typedef struct pixel_struct
{
    unsigned char value;
} pixel_t;

// Structure to store full image data of any size
typedef struct image_struct
{
    int width;
    int height;
    pixel_t ** pixels;
} image_t;

// Structure for an image in PGM format
typedef struct pgm_struct
{
    char magic_number[3];           // String for the code indicating the type of PGM file
    int max_value;                  // Maximum value for pixel data in each component
    image_t image;
} pgm_t;

//Struct for thread data
typedef struct data_struct {
    int startingLine;
    int step;
    pgm_t * image;
} data_t;

//// FUNCTION PROTOTYPES
void allocateImage(image_t * image);
void freeImage(image_t * image);
void readBoard(const char * filename, image_t * image);
void playGame(image_t * image);
void playGameOMP(image_t * image);
int checkNeighbors(int matrix[3][3]);
void writePGMFile(const char * filename, const pgm_t * pgm_image);
void writePGMTextData(const pgm_t * pgm_image, FILE * file_ptr);
void writePGMBinaryData(const pgm_t * pgm_image, FILE * file_ptr);

#endif
