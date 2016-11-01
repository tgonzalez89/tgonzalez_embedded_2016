//gcc -o conv1 conv.c -lm -O3 [-pg]
//gcc -o conv2 conv.c -lm -O3 -fopenmp -DUSEOPENMP [-pg]

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <omp.h>
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

void run_kernel(const double* kernel, const unsigned char* input_data, unsigned char* output_data, const int w, const int h) {
    int j;
    #ifdef USEOPENMP
    #pragma omp parallel for
    #endif
    for (j = 0; j < h; j++) {
        int i;
        for (i = 0; i < w; i++) {
            int pos = i + j*w;
            if (i == 0 || i == w-1 || j == 0 || j == h-1) {
                *(output_data + pos) = *(input_data + pos);
                continue;
            }
            double temp = *(input_data + pos-1-w)*kernel[0] + *(input_data + pos-w)*kernel[1] + *(input_data + pos+1-w)*kernel[2] +
                          *(input_data + pos-1)*kernel[3]   + *(input_data + pos)*kernel[4]   + *(input_data + pos+1)*kernel[5] +
                          *(input_data + pos-1+w)*kernel[6] + *(input_data + pos+w)*kernel[7] + *(input_data + pos+1+w)*kernel[8];
            if (temp > 255) temp = 255;
            else if (temp < 0) temp = 0;
            *(output_data + pos) = (unsigned char)temp;
        }
    }
    return;
}

void help() {
    printf("Usage: conv [options]\nOptions:\n");
    printf("  -i INPUT_IMAGE\tInput image to be processed.\n");
    printf("  -o OUTPUT_IMAGE\tName for the output image generated.\n");
    printf("  -k KERNEL_NUMBER\tKernel to execute: 1 = blur, 2 = sharpen.\n");
    printf("  -a\t\t\tPrint the information of the author of this program and exit.\n");
    printf("  -h\t\t\tPrint this message and exit.\n");
    return;
}

int main(int argc, char* argv[]) {
    struct timeval start, end;
    gettimeofday(&start, NULL);

    // Process arguments
    char* input_image = NULL;
    char* output_image = NULL;
    int kernel_num = 0;
    int arg;
    while ((arg = getopt(argc, argv, "hak:o:i:")) != -1) {
        switch (arg) {
            case 'h':
                help();
                return 0;
            case 'a':
                printf("Author: Tomas Gonzalez Aragon\n");
                return 0;
            case 'i':
                input_image = optarg;
                break;
            case 'o':
                output_image = optarg;
                break;
            case 'k':
                kernel_num = atoi(optarg);
                break;
            case '?':
                help();
                return -1;
            default:
                help();
                return -1;
        }
    }
    int i;
    for (i = optind; i < argc; i++)
        printf("-W- Ignoring non-option argument: '%s'\n", argv[i]);
    if (input_image == NULL) {
        printf("-E- No input image provided.\n");
        return -1;
    } else if (output_image == NULL) {
        printf("-E- No name for output image provided.\n");
        return -1;
    } else if (kernel_num < 1 || kernel_num > 2) {
        printf("-E- Invalid kernel number.\n");
        return -1;
    }

    // Read input image
    int w, h, n;
    unsigned char* input_data = stbi_load(input_image, &w, &h, &n, 1);
    if (input_data == NULL) {
        printf("-E- Can't read input image %s\n", input_image);
        return -1;
    }

    // Apply kernel
    double kernel[] = {0, 0, 0, 0, 1, 0, 0, 0, 0};
    if (kernel_num == 1) { // blur
        const double kernel_data[] = {0.0625, 0.125, 0.0625, 0.125, 0.25, 0.125, 0.0625, 0.125, 0.0625};
        memcpy(kernel, kernel_data, sizeof(kernel_data));
    } else if (kernel_num == 2) { // sharpen
        const double kernel_data[] = {0, -1, 0, -1, 5, -1, 0, -1, 0};
        memcpy(kernel, kernel_data, sizeof(kernel_data));
    }
    unsigned char* output_data = (unsigned char*)malloc(sizeof(unsigned char)*w*h);
    struct timeval k_start, k_end;
    gettimeofday(&k_start, NULL);
    run_kernel(kernel, input_data, output_data, w, h);
    gettimeofday(&k_end, NULL);
    printf("run_kernel elapsed time: %f seconds\n", (double)(k_end.tv_sec - k_start.tv_sec) + (double)(k_end.tv_usec - k_start.tv_usec)/1000000);
    stbi_image_free(input_data);

    // Write output image
    if (!stbi_write_png(output_image, w, h, 1, output_data, sizeof(char)*w)) {
        printf("-E- Can't write output image %s\n", output_image);
        return -1;
    }
    free(output_data);

    gettimeofday(&end, NULL);
    printf("Total elapsed time: %f seconds\n", (double)(end.tv_sec - start.tv_sec) + (double)(end.tv_usec - start.tv_usec)/1000000);

    return 0;
}
