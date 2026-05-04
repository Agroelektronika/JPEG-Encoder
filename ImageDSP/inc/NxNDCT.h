#ifndef NXNDCT_H_
#define NXNDCT_H_
#include <QDebug>
		

void GenerateDCTmatrix(double* DCTKernel, int order);

void DCT(const char input[], int16_t output[], int N);

void IDCT(const short input[], char output[], int N);

template<typename T>
void extendBorders(T* input, int xSize, int ySize, int N, T** p_output, int* newXSize, int* newYSize)
{
    int deltaX = N-xSize%N;
    int deltaY = N-ySize%N;

    *newXSize = xSize+deltaX;
    *newYSize = ySize+deltaY;

    T* output = new T[(xSize+deltaX)*(ySize+deltaY)];

    for (int i=0; i<ySize; i++)
    {
        memcpy(&output[i*(xSize+deltaX)], &input[i*(xSize)], xSize);
        if(deltaX != 0)
        {
            memset(&output[i*(xSize+deltaX)+xSize], output[i*(xSize+deltaX)+xSize - 1], deltaX);
        }
    }

    for (int i=0; i<deltaY; i++)
    {
        memcpy(&output[(i + ySize)*(xSize+deltaX)], &output[(ySize)*(xSize+deltaX)], xSize+deltaX);
    }

    *p_output = output;
}
template<typename T>
void cropImage(T* input, int xSize, int ySize, T* output, int newXSize, int newYSize)
{
    for (int i=0; i<newYSize; i++)
    {
        memcpy(&output[i*(newXSize)], &input[i*(xSize)], newXSize*sizeof(T));
    }
}
#endif // NXNDCT_H_
