//
//  main.cpp
//  DVA336Project
//

#include <stdlib.h>
#include <stdio.h>
#include "bitmap_image.hpp"
#include <sys/time.h>
#include <pthread.h>

// Number of threads
#define P 4

bitmap_image image = bitmap_image("images/MARBLES.bmp");
bitmap_image out;
rgb_t** inImage;

struct args
{
    int hStart;
    int hEnd;
    int wStart;
    int wEnd;
    int height;
    int width;
};

void* mean(void * args){
    struct args params = *((struct args*)args);
    
    for (int y = params.hStart; y < params.hEnd; y++){
        for (int x = params.wStart; x < params.wEnd; x++){
            int red = 0;
            int green = 0;
            int blue = 0;
            int neighbors = 0;
            rgb_t colour;
            
            for(int i = x-1; i <= x+1; i++){
                for(int j = y-1; j <= y+1; j++){
                    if(i >= 0 && j >= 0 && i < params.width && j < params.height){
                        neighbors++;
                        colour = inImage[i][j];
                        
                        red   += colour.red;
                        green += colour.green;
                        blue  += colour.blue;
                    }
                }
            }
            red   /= neighbors;
            green /= neighbors;
            blue  /= neighbors;
            colour = make_colour(red, green, blue);
            out.set_pixel(x, y, colour);
        }
    }
    return NULL;
}



void loadPixelsToArray(){
    inImage = (rgb_t**) malloc(image.height()*sizeof(rgb_t*));
    
    for (int i = 0; i < image.width(); i++) {
        inImage[i] = (rgb_t*) malloc(image.width()*sizeof(rgb_t));
    }
    
    for (int y = 0; y < image.height(); y++){
        for (int x = 0; x < image.width(); x++){
            image.get_pixel(y, x, inImage[y][x]);
        }
    }
}

int main(int argc, const char * argv[]) {
    pthread_t thread[P];
    pthread_attr_t attr;
    time_t time;
    struct timeval start, end;
    image = bitmap_image("images/MARBLES.bmp");
    
    if (!image){
        printf("test01() - Error - Failed to open image\n");
        return 1;
    }
    loadPixelsToArray();
    
    int height = image.height();
    int width = image.width();
    
    out = bitmap_image(width, height);
    
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    
    gettimeofday(&start, NULL);
    
    for (int i = 0; i < P; i++)
    {
        struct args* params = (struct args*)malloc(sizeof(struct args));
        params->hStart = (i*height/P);
        params->hEnd = (i*height/P)+(height/P);
        params->wStart = 0;
        params->wEnd = width;
        params->height = height;
        params->width = width;
        
        pthread_create(&thread[i], &attr, &mean, (void*)params);
    }
    
    //Join the threads
    for (int i = 0; i < P; i++) {
        pthread_join(thread[i], NULL);
    }

    gettimeofday(&end, NULL);
    float delta = ((end.tv_sec  - start.tv_sec) * 1000000u + end.tv_usec - start.tv_usec) / 1.e6;

    printf("Duration: %f seconds\n", delta);
    
    out.save_image("images/out.bmp");
    
    return 0;
}
