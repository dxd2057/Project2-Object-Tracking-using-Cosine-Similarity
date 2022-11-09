#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define STEP 3

#define STB_IMAGE_IMPLEMENTATION
#include "./headers/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "./headers/stb_image_write.h"

unsigned char *uc_arrayNew_1d(int _size) {
    return (unsigned char *)calloc(_size, sizeof(unsigned char));
}

float object_sqrt(unsigned char *object, int object_width, int object_height) {
    float sum = 0;
    for (int i = 0; i < object_width * object_height; i += STEP) {
        sum += object[i] * object[i];
    }
    return sqrt(sum);
}

void bwConverter(unsigned char *input, unsigned char *output, int width, int height, int channel) {
    for (int i = 0; i < width ; i++) {
        for (int j=0; j< height; j++) {
            int index = j * width * channel + i * channel;
            int index_out = j * width + i;
            unsigned char red = input[index];
            unsigned char green =input[index + 1];
            unsigned char blue = input[index + 2];
            output[index_out] = (red*red + green*green + blue*blue+1)/(red+green+blue+1);
        }
    }
}

int objectDetector(unsigned char *image, int width, int height, unsigned char *object, int object_width, int object_height) {
    float max, similarity;
    int best_x, best_y, img_element, obj_element;
    float sum, sqrt1, sqrt2;
    similarity = 0;
    max = 0.1;
    sqrt2 = object_sqrt(object, object_width, object_height);
    for (int i = 0; i < width - object_width; i += STEP) {
        for (int j = 0; j < height - object_height; j++) {
            sum = 0;
            sqrt1 = 0;
            
            for (int a = 0; a < object_width ; a++){
                for (int b = 0; b < object_height; b++){
                        img_element = image[(b+j) * width + (a+i)];
                        obj_element = object[b * object_width + a];
                        sum += img_element * obj_element;   
                        sqrt1 += img_element * img_element;
                    }
                similarity = sum / (sqrt(sqrt1) * sqrt2);
                
                if (similarity > max) {
                    max = similarity;
                    best_x = i;
                    best_y  = j;
                }
            }
        }
    }
    return best_y * width + best_x;
}

void drawRect(unsigned char *image, int width, int height, int channel, int new_index, int object_width, int object_height) {
    int best_y, best_x;
    best_y = new_index / (width);
    best_x = new_index % (width);
    
    int index;
    for (int i = 0; i <object_width; i++) {
        for (int k = 0; k< 3; k++){
            index = (best_y ) * width * channel + (best_x+i) * channel + k;
            image[index] = 0;
            index = (best_y+ object_height-1) * width * channel + (best_x+i) * channel + k;
            image[index] = 0;
        }
    }
    for (int i = 0; i <object_height; i++) {
        for (int k = 0; k< 3; k++){
            index = (best_y + i) * width * channel + (best_x) * channel + k;
            image[index] = 0;
            index = (best_y+i) * width * channel + (best_x+ object_width-1) * channel + k;
            image[index] = 0;
        }
    }
}

void updateTemplate(int best_index, int object_width, int object_height, unsigned char *object, unsigned char *image, int image_width, int image_height) {
    int best_y, best_x;
    best_y = best_index / (image_width);
    best_x = best_index % (image_width);
    int index;
    for (int i = 0; i < object_height; i++) {
        for (int j = 0; j <object_width; j++) {
            index = (best_y + i) * image_width + (best_x + j);
            object[i*object_width+j] = image[index];
        }
    }
}

int main() {
    //declare variables
    int width, height, channel;
    int width2, height2, channel2;
    char image_path[24];
    char object_path[] = "./data/template (1).jpg";
    char save_path[24];
    unsigned char *image_grey, *object_grey;

    unsigned char *image, *object;
    printf("Starting program...\n");

    //read object
    object = stbi_load(object_path, &width2, &height2, &channel2, 0);
    if(object == NULL) {
        printf("Error: object not found!\n");
        exit(1);
    }
    object_grey = uc_arrayNew_1d(width2*height2);
    bwConverter(object, object_grey, 160, 214, 3);
    
    for (int i = 0; i < 63; i++) {
        sprintf(image_path, "./data/images/img%d.jpg", i);
        sprintf(save_path, "./results/%d.png", i);
        //read image
        image = stbi_load(image_path, &width, &height, &channel, 0);
        if(image == NULL) {
            printf("Error: image not found!\n");
            exit(1);
        }

        image_grey = uc_arrayNew_1d(width*height);
        bwConverter(image, image_grey, width, height, channel);

        int index = objectDetector(image_grey, width, height, object_grey, width2, height2);
        drawRect(image, width, height, channel, index, width2, height2);
        updateTemplate(index, width2, height2, object_grey, image_grey, width, height);
        stbi_write_jpg("./data/template.jpg", width2, height2, 1, object_grey, width);
        
        //save image
        stbi_write_png(save_path, width, height, channel, image, width * channel);
        printf("%s saved\n", save_path);
    }
    
    free(image);
    free(object);
    free(image_grey);
    free(object_grey);

    return 0;
}