#include<math.h>
#include<stdio.h>

double pow(double x, int y){
    double ans = 1;
    int index;

    for(index = 0; index<y; index++){
        ans = ans*x;
    }

    return ans;
}

double log(double x){
    double ans = 0;
    int index;

    for(index = 1; index <= 20; index++){
        ans += pow(x,index)/index;
    }
    ans = ans*-1;
    return ans;
}

double expdev(double lambda){
    double dummy;
    do{
        dummy = (double) rand() / RAND_MAX;
    }while(dummy == 0.0);
    return -log(dummy) / lambda;
}