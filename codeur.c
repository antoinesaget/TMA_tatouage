#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "pixmap_io.h"
#include "shared.h"

/**
 * Write a full byte starting at i of pixels.
 * This will write at pixels[i], pixels[i+1], pixels[i+2], and pixels[i+3]
 */
void write_byte(unsigned char *pixels, int i, char byte) {
    for(int j = 3; j >= 0; j--) {
        pixels[i + j] = (pixels[i + j] & 0xFC) | (byte & 0x03);
        byte >>= 2;
    }
}

/**
 * Write a full int starting at i of pixels.
 * This will write from pixels[i] to pixels[i+15]
 */
void write_int(unsigned char *pixels, int i, int val) {
    for(int j = 15; j >=0; j--) {
        pixels[i + j] = (pixels[i + j] & 0xFC) | (val & 0x03);
        val >>= 2;
    }
}

/** 
 * Write the file size at the beginning of the file
 */
void write_size(unsigned char *pixels, unsigned int size) {
    write_int(pixels, 0, size);
}

/** 
 * Write the filename after the file size header
 */
int write_filename(unsigned char *pixels, char* filename) {
    if(strlen(filename)>=FILENAME_HEADER_SIZE) {
        fprintf(stderr, "Error: filename too long!\n");
        return -1;
    }

    // Start after the 4 bytes needed for the file size
    int start = SIZE_HEADER_SIZE*4;
    
    int i = 0;
    char val = filename[i];
    // Write the filename until then end of the string
    while (val != '\0') {
        write_byte(pixels, start + i*4, val);
        val = filename[++i];
    }
    
    // Adding a '\0'
    write_byte(pixels, start + i*4, '\0');
    return 0;
}

/**
 * Write the file data after the file size+name header
 */
void write_data(unsigned char *pixels, FILE* f, unsigned int size) {
    // Start after the header
    int start = TOT_HEADER_SIZE*4;
    
    // Read the file bytes one by one
    char val;
    fread(&val, 1, 1, f);

    // Write each byte after the header
    for(int i = 0; i < size; i++) {
        write_byte(pixels, start + i*4, val);
        fread(&val, 1, 1, f);
    }
}

int main(int argc, char const *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "Error: incorrect number of arguments.\n");
        printf("Usage: %s <filename> <msg filename>\n", argv[0]);
        return -1;
    }
    
    char* filename = argv[1];
    printf("File name:\t%s\n", filename);
    char* msg_filename = argv[2];
    printf("Tatou file name: %s\n", msg_filename);

    unsigned char *pixels;
    int width, height;

    if((pixels = load_pixmap(filename, &width, &height)) != NULL) {
        printf("Image size:\t%d %d\n", width, height);
        unsigned long available = width*height / 4 - TOT_HEADER_SIZE;
        printf("Available size:\t%ld\n", available);

        FILE* msg = fopen(msg_filename, "rb");
        if (msg == NULL) {
            fprintf(stderr, "Error when opening message file : %s\n", msg_filename);
            return -1;
        }
        
        // Go to the end of the file to get the file size.
        fseek(msg, 0, SEEK_END);          
        long msg_size_l = ftell(msg);            
        rewind(msg);

        // Make sure the file size can be encoded with 4 bytes
        // The max size encoded with 4 bytes is 2^33-1 bytes =~ 8.5 Gb
        // And would need an image of at least sqrt(2^33-1)*4 = 370728*370728 pixels to be hidden in
        if (msg_size_l >= (pow(2, SIZE_HEADER_SIZE*8 + 1) -1)) {
            fprintf(stderr, "Error: the mesage file size is too big and cannnot be encoded with 4 bytes.\n");
            return -1;
        }
        unsigned int msg_size = msg_size_l;
        printf("Message size:\t%d\n", msg_size);

        // If the msg size is greater than the available space in the file, stop
        if (msg_size_l > available) {
            fprintf(stderr, "Error: the message file is too big: %ld > %ld\n", msg_size_l, available);
            return -1;
        }

        // Hide header within pixels
        write_size(pixels, msg_size);
        printf("Message size successfully encoded.\n");

        if(write_filename(pixels, msg_filename) != 0) {
            return -1;
        } else {
            printf("Message file name successfully encoded.\n");
        }

        // Hide data within pixels
        write_data(pixels, msg, msg_size);
        printf("Message data successfully encoded.\n");

        // Write the pixels to a new image
        char* prefix = "coded_";
        char out_filename[strlen(filename)+strlen(prefix)];
        sprintf(out_filename, "%s%s", prefix, filename);

        store_pixmap(out_filename, pixels, width, height);
        fclose(msg);
        
        printf("Message successfully hidden in %s\n", out_filename);
    } else {
        fprintf(stderr, "Error when reading %s pixels.\n", filename);
        return -1;
    }

    return 0;
}
