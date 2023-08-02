#include "seamcarving.h"
#include "c_img.h"
#include "c_img.c"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>

void calc_energy(struct rgb_img *im, struct rgb_img **grad){
    int w_plus;
    int w_sub;
    int h_plus;
    int h_sub;
    int sum;
    int h = im->height - 1;
    int w = im->width - 1;

    create_img(grad, im->height, im->width);

    for(int k = 0; k <= h; k++){ 
        for(int j = 0; j <= w; j++){
            for(int i = 0; i < 3; i++){
                int w_one = j+1;
                int w_minus_one = j-1;
                int h_one = k+1;
                int h_minus_one = k-1;

                if(j==0){
                    w_minus_one = w;
                }
                if(j==w){
                    w_one = 0;
                }
                if(k==0){
                    h_minus_one = h;
                }
                if(k==h){
                    h_one = 0;
                }
                
                w_plus = get_pixel(im,k,w_one,i);
                w_sub = get_pixel(im,k,w_minus_one,i);
                h_plus = get_pixel(im,h_one,j,i);
                h_sub = get_pixel(im,h_minus_one,j,i);
                //printf("color is: %d, %d, %d, %d, %d\n", i, w_plus, w_sub, h_plus, h_sub);
                sum += pow((w_plus-w_sub),2) + pow((h_plus-h_sub),2);
            }
            int energy = (uint8_t) (pow(sum,0.5)/10);
            set_pixel(*grad, k, j, energy, energy, energy);
            //printf("%d\n", get_pixel(*grad,k,j,0));
            sum = 0;
        }
    }
}

void dynamic_seam(struct rgb_img *grad, double **best_arr){
    int h = grad->height;
    int w = grad->width;
    double temp1;
    double temp2;
    double temp3;
    double min;
    *best_arr = (double *)malloc((h*w) * sizeof(double));

    for (int i = 0; i < h; i++){
        for(int j = 0; j < w; j++){
            if (i == 0){
                (*best_arr)[j] = get_pixel(grad, 0,j,0);
            }else{
                temp1 = (*best_arr)[(i-1)*w+j];
                temp2 = temp1;
                temp3 = temp1;
                if(j!=0){
                    temp2 = (*best_arr)[(i-1)*w+j-1];
                }
                if(j!=w-1){
                    temp3 = (*best_arr)[(i-1)*w+j+1];
                }

                if(temp1 >= temp3 && temp2 >= temp3){
                    min = temp3;
                }else if(temp2 >= temp1 && temp3 >= temp1){
                    min = temp1;
                }else if(temp3 >= temp2 && temp1 >= temp2){
                    min = temp2;
                }

                double sum = get_pixel(grad, i,j,0) + min;
                (*best_arr)[i*w+j] = sum;
                //printf("(%d,%d), %lf\n", i,j,(*best_arr)[i*w+j]);

            }
        }
    }
}

void recover_path(double *best, int height, int width, int **path){
    double minVal;
    int minInd;
    int h = height - 1;
    int prevInd;

    *path = (int *)malloc(height * sizeof(int));

    for (int j = 0; j < width; j++){
        if(j==0){
            minVal = best[h*width];
            minInd = 0;
        }else if (minVal > best[h*width+j]){
            minVal = best[h*width+j];
            minInd = j;
        }
    }

    (*path)[h] = minInd;
    h--;
    
    for (int i = h; i >= 0; i --){
        prevInd = (*path)[i+1];
        minVal = best[i*width + prevInd];
        minInd = prevInd;
        for (int j = prevInd - 1; j <= prevInd + 1; j++){
            if (minVal > best[i*width + j]){
                minVal = best[i*width + j];
                minInd = j;
            }
        }
        (*path)[i] = minInd;
    }
    /*for (int i = 0; i < height; i++){
        printf("%d\n", (*path)[i]);
    }*/
}

void remove_seam(struct rgb_img *src, struct rgb_img **dest, int *path){
    int h = src->height;
    int w = src->width - 1;
    int r;
    int g;
    int b;
    int skip;
    create_img(dest, h, w);

    for(int i = 0; i < h; i++){
        skip = 0;
        for(int j = 0; j <= w; j++){
            r = get_pixel(src, i,j,0);
            g = get_pixel(src, i,j,1);
            b = get_pixel(src, i,j,2);
            if (j==path[i]){
                skip = 1;
            }else{
                if(skip){
                    set_pixel(*dest, i, j-1, r, g, b);
                }else{
                    set_pixel(*dest, i, j, r, g, b);
                }
            }
        }
    }
}

int main(void){
    
    struct rgb_img *image;
    struct rgb_img *cur_im;
    struct rgb_img *grad;
    double *best;
    int *path;

    read_in_img(&image, "6x5.bin");
    calc_energy(image, &grad);
    dynamic_seam(grad,&best); 
    recover_path(best, grad->height, grad->width, &path);
    for (int i = 0; i < grad->height; i ++){
        for(int j = 0; j< grad->width; j++){
            printf("ENERGY: (%d,%d,) %d\n", i,j, get_pixel(grad,i,j,0));
        }
    }
     for (int i = 0; i < grad->height; i ++){
        for(int j = 0; j< grad->width; j++){
            printf("PATH: (%d,%d), %lf\n", i,j,(best)[i*(grad->width)+j]);
        }
    }

    for (int i = 0; i < grad->height; i++){
        printf("%d\n", (path)[i]);
    }

    remove_seam(image, &cur_im, path);
    destroy_image(image);
    destroy_image(grad);
    
    
    /*struct rgb_img *im;
    struct rgb_img *cur_im;
    struct rgb_img *grad;
    double *best;
    int *path;

    read_in_img(&im, "HJoceanSmall.bin");
    
    for(int i = 0; i < 150; i++){
        //printf("i = %d\n", i);
        calc_energy(im,  &grad);
        dynamic_seam(grad, &best);
        recover_path(best, grad->height, grad->width, &path);
        remove_seam(im, &cur_im, path);

        char filename[200];
        sprintf(filename, "img%d.bin", i);
        write_img(cur_im, filename);


        destroy_image(im);
        destroy_image(grad);
        free(best);
        free(path);
        im = cur_im;
    }
    destroy_image(im);*/
}