#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pixmap_io.h"
#include "shared.h"

/**
 * Read a full byte starting at i of pixels.
 * This will read at pixels[i], pixels[i+1], pixels[i+2], and pixels[i+3]
 */
char read_byte(unsigned char *pixels, int i) {
    char n;
    for (int j = 0; j < 4; j++) {
        n = n<<2 | (pixels[i+j] & 0x03);
    }
    return n;
}

/**
 * Read a full int starting at i of pixels.
 * This will read from pixels[i] to pixels[i+15]
 */
int read_int(unsigned char *pixels, int i) {
    int n = 0;
    for (int j = 0; j < 16; j++) {
        n = n<<2 | (pixels[i+j] & 0x03);
    }
    return n;
}

void read_filename(unsigned char *pixels, char* filename) {
    for(int i = 0; i < FILENAME_HEADER_SIZE; i++) {
        filename[i] = read_byte(pixels, SIZE_HEADER_SIZE*4 + i*4);
    }
}

void read_file(unsigned char *pixels, char* bytes, int n) {
    for(int i = 0; i < n; i++) {
        bytes[i] = read_byte(pixels, TOT_HEADER_SIZE*4 + i*4);
    }
}

int main(int argc, char const *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Error: Incorrect number of arguments.\n");
        printf("Usage: %s <filename>\n", argv[0]);
        return -1;
    }

    char* filename = argv[1];
    printf("Filename: %s\n", filename);

    unsigned char *pixels;
    int width, height;

    if((pixels = load_pixmap(filename, &width, &height)) != NULL) {
        printf("Image size: %d %d\n", width, height);
        unsigned long available = width*height/4 - TOT_HEADER_SIZE;
        printf("Available size: %ld\n", available);

        // Reading the size
        unsigned int n = read_int(pixels, 0);
        printf("Message file size: %u\n",n);

        // If the read message size is greater than the available size, there is a problem
        if (n > available) {
            fprintf(stderr, "Error: the message size is greater than the available space in the file.\nEither %s is not hidding anything or corrupted.\n", argv[1]);
            return -1;
        }
        
        // Reading the filename
        char filename[FILENAME_HEADER_SIZE];
        read_filename(pixels, filename);
        printf("Message file name: %s\n", filename);

        // Reading the file 
        char bytes[n];
        read_file(pixels, bytes, n);
        
        // Writing the file
        char* prefix = "tatou_";
        char out_filename[FILENAME_HEADER_SIZE+strlen(prefix)];
        sprintf(out_filename, "%s%s", prefix, filename);

        FILE* fout = fopen(out_filename, "wb");
        fwrite(bytes, 1, n, fout);
        fclose(fout);

        printf("Message successfully written into %s\n", out_filename);
    } else {
        fprintf(stderr, "Error when reading %s pixels.\n", filename);
        return -1;
    }

    return 0;
}
