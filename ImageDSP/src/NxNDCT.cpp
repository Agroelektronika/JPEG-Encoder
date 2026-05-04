#define _USE_MATH_DEFINES
#include <math.h>
#include "NxNDCT.h"

#define PI 3.14159265359

static char quantizationMatrix[64] =
{
    16, 11, 10, 16, 24, 40, 51, 61,
    12, 12, 14, 19, 26, 58, 60, 55,
    14, 13, 16, 24, 40, 57, 69, 56,
    14, 17, 22, 29, 51, 87, 80, 62,
    18, 22, 37, 56, 68, 109, 103, 77,
    24, 35, 55, 64, 81, 104, 113, 92,
    49, 64, 78, 87, 103, 121, 120, 101,
    72, 92, 95, 98, 112, 100, 103, 99
};



double alfa(int n, int N){  // part of the 2D DCT formula
    if(n == 0){
        return sqrt(1.0 / (N));
    }
    else{
        return sqrt(2.0 / (N));
    }
    return 1;
}

void DCT(const char input[], int16_t output[], int N)
{  // pozvana za svaki blok NxN, dobijanje spektralnih koeficijenata za svaki blok

    double* temp = new double[N*N];
    double* DCTCoefficients = new double[N*N];

    for (int i = 0; i <= N - 1; i++)
    {
        for (int j = 0; j <= N - 1; j++)
        {
            temp[i*N+j] = input[i*N+j];
            // temp[i*N+j] /= 128;     // [-0.5,0.5]
        }
    }

    double sum = 0;
    for (int n = 0; n <= N - 1; n++)
    {
        for (int k = 0; k <= N - 1; k++)
        {
            sum = 0;
            for (int v = 0; v <= N - 1; v++)
            {
                for (int h = 0; h <= N - 1; h++)
                {
                    sum += temp[v*N+h]*
                           cos(PI*n*(v+0.5)/N)*
                           cos(PI*k*(h+0.5)/N);
                }
            }
            output[n*N+k] = alfa(n, N) * alfa(k, N) * (sum);
        }
    }

    for(int i = 0; i < N*N; i++)
    {
        output[i] = floor(output[i]+0.5);
    }

    delete[] temp;
    delete[] DCTCoefficients;


    return;
}



void IDCT(const short input[], char output[], int N)
{
    /* TO DO */
    double sum = 0;
    for (int v = 0; v <= N - 1; v++)
    {
        for (int h = 0; h <= N - 1; h++)
        {
            sum = 0;
            for (int n = 0; n <= N - 1; n++)
            {
                for (int k = 0; k <= N - 1; k++)
                {
                    sum += input[n*N+k]*cos(PI*n*(v+0.5)/N)*cos(PI*k*(h+0.5)/N)*alfa(n,N)*alfa(k,N);
                }
            }
            output[v*N+h] = sum;
        }
    }

    for(int i = 0; i < N*N; i++)
    {
        output[i] = floor(output[i]+0.5);
    }



}
