//
//  peak_picker.c
//  
//
//  Created by Yoshiharu Nishiyama on 16/11/2023.
//
#include <Accelerate/Accelerate.h>
#include <iostream>
#include "peak_picker.h"
using namespace std;
static double mul_threshold =15.;
static double threshold = 5.;

void set_mul_threshold(double a) {mul_threshold = a;}
void set_threshold(double a) {threshold = a;}

double sqr(double &x){return x*x;}
int
calc_stdev(double *x0, double *x1, int count,  double &stdev, double &average)
{
   double sum  = 0;
   double sum2 = 0;
   
   for(double *ptr = x0; ptr !=x1; ptr++){
      sum += *ptr;
      sum2+= sqr(*ptr);
   }
   sum/=count;
   sum2/=count;
   stdev = sqrt(sum2-(sum*sum));
    average = sum;
    return 0;
}

int get_peak(double *data, int box, int &x, int &y)
{
    int box_size = box*box;
    int middle = box_size/2;
    int border = box-2;
    double *max_val = max_element(data, data+box_size);
    int pos = max_val-data;
    div_t t = div(pos, box);
    if(!t.quot ||( t.quot > border) ) return 0;
    if(!t.rem  ||( t.rem  > border) ) return 0;
    double intensity = *max_val;
    double stdev, average;
    nth_element(data,data+middle, data+box_size);
    calc_stdev(data, data+middle, middle,  stdev, average);
    intensity -= average;
//    cout << intensity <<" "<< average <<" "<< stdev<<endl;
    if(intensity < (max(mul_threshold * stdev, threshold))) return 0;
    x = t.rem;
    y = t.quot;
//    cout <<" xy "<< x <<" "<<y <<endl;
    return 1;
}


int get_peak_list(double *data, int w, int h, int line_len, int box, 
                     int *peaklist, int max_size,  int &count)
{
    int jshift = box-2;
    int ishift = box-2;
    int tilej = h/(box-2);
    int tilei = w/(box-2);
    double *clip = new double[ box*box];
    count = 0;
    for(int j = 0; j < tilej; j++ ){
        double *ptr = data + jshift*j*line_len;;
        char A = 'T';
        for(int i = 0; i < tilei; i++ ){
            for(int k = 0; k < box; k++) memcpy(clip + box*k, ptr+line_len*k+ i*ishift, sizeof(double)*box);
//            dlacpy_(&A, &box, &box, ptr, &line_len, clip, &box);
//            copy_data(ptr, clip, box, line_len);
            int x, y;
            if(get_peak(clip, box, x, y))
            {
                x+= i * ishift;
                y+= j * jshift;
                cout << x  <<" "<<y << endl;
                if(count > max_size-3){ cerr <<"list size too small "<<count <<endl; return 0;} 
                peaklist[count] = x;
                peaklist[count+1] = y;
                count+=2;
            }
        }
    }
    delete[] clip;
    
}
