#include "JPEG.h"
#include "NxNDCT.h"
#include <math.h>

#include "JPEGBitStreamWriter.h"


#define DEBUG(x) do{ qDebug() << #x << " = " << x;}while(0)


uint8_t quantizationMatrixLuminance[64];
uint8_t quantizationMatrixChrominance[64];
JPEGBitStreamWriter stream("example2.jpg");

            // quantization tables from JPEG Standard, Annex K
uint8_t QuantLuminance[8*8] =
    { 16, 11, 10, 16, 24, 40, 51, 61,
      12, 12, 14, 19, 26, 58, 60, 55,
      14, 13, 16, 24, 40, 57, 69, 56,
      14, 17, 22, 29, 51, 87, 80, 62,
      18, 22, 37, 56, 68,109,103, 77,
      24, 35, 55, 64, 81,104,113, 92,
      49, 64, 78, 87,103,121,120,101,
      72, 92, 95, 98,112,100,103, 99 };
uint8_t QuantChrominance[8*8] =
    { 17, 18, 24, 47, 99, 99, 99, 99,
      18, 21, 26, 66, 99, 99, 99, 99,
      24, 26, 56, 99, 99, 99, 99, 99,
      47, 66, 99, 99, 99, 99, 99, 99,
      99, 99, 99, 99, 99, 99, 99, 99,
      99, 99, 99, 99, 99, 99, 99, 99,
      99, 99, 99, 99, 99, 99, 99, 99,
      99, 99, 99, 99, 99, 99, 99, 99 };

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

struct imageProperties{
    int width;
    int height;
    int16_t* coeffs;
};


uint8_t quantQuality(uint8_t quant, uint8_t quality) {
    // Convert to an internal JPEG quality factor, formula taken from libjpeg
    int16_t q = quality < 50 ? 5000 / quality : 200 - quality * 2;
    return clamp((quant * q + 50) / 100, 1, 255);
}
template<typename T>
static void doZigZag(T block[], int N)
{
    /* TO DO */

    int counter = 0;
    bool going_up = true;
    int16_t n = 0, k = 0;
    int16_t* transformed = new int16_t[N*N];  // 1D niz u koji se transformise matrica (blok)

    while(counter < N*N){  // ceo blok

        transformed[counter] = block[n*N+k];  // upis u novi niz vrednosti na n,k poziciji
        if(going_up){    // cik-cak logika za odredjivanje n,k u matrici
            if(n > 0 && k < N-1){
                k += 1;
                n -= 1;
            }
            else{
                if(k < N-1)
                    k += 1;
                else
                    n += 1;
                going_up = false;
            }
        }
        else{
            if(k > 0 && n < N-1){
                n += 1;
                k -= 1;
            }
            else{
                if(n < N-1)
                    n += 1;
                else
                    k += 1;
                going_up = true;
            }
        }
        counter += 1;

    }

    for(int i = 0; i < N*N; i++){
        block[i] = transformed[i];
    }

    delete[] transformed;

}

/* perform DCT */
void performDCT(char input[], short output[], int xSize, int ySize, int N, bool quantType)
{
	// TO DO
    char* inBlock = new char[N*N];


    short* dctCoeffs = new short[N*N]; // blok u koji se upisuju spektralni koeficijenti
    //extended
    int xSize2 = 0;
    int ySize2 = 0;
    char* input2;
    short* output2; // izlaz je cela slika u frekventnom domenu

    extendBorders<char>(input, xSize, ySize, N, &input2, &xSize2, &ySize2);
    extendBorders<short>(output, xSize, ySize, N, &output2, &xSize2, &ySize2); // prosirivanje i ulaza i izlaza, da ne bude neslaganja


    for(int y = 0; y < ySize2/N; y++){
        for(int x = 0; x < xSize2/N; x++){  // po sirini i visini slike
            for(int j = 0;j < N; j++){ // blokovi
                for(int i = 0; i < N; i++){
                    inBlock[j*N+i] = input2[(N*y+j)*(xSize2)+N*x+i];
                }
            }

            DCT(inBlock, dctCoeffs, N);  // DCT za svaki blok


            for(int j = 0; j < N; j++){
                for(int i = 0; i < N; i++){  // kvantizacija bloka
                    if(quantType){
                        dctCoeffs[j*N+i] = round(dctCoeffs[j*N+i] / quantizationMatrixLuminance[j*N+i]);
                    }
                    else{
                        dctCoeffs[j*N+i] = round(dctCoeffs[j*N+i] / quantizationMatrixChrominance[j*N+i]);
                    }
                }
            }

            IDCT(dctCoeffs, inBlock, N);  // inverzno, vracanje u prostorni domen (za lokalni prikaz i proveru)

            for(int j = 0; j < N; j++){
                for(int i = 0; i < N; i++){
                    input2[(N*y+j)*(xSize2)+N*x+i] = inBlock[j*N+i];
                }
            }

            for(int j = 0; j < N; j++){
                for(int i = 0; i < N; i++){
                    output2[(N*y+j)*(xSize2)+N*x+i] = dctCoeffs[j*N+i]; // upis spektralnih koeficijenata u izlazni bafer
                }
            }
        }
    }

    cropImage<char>(input2, xSize2, ySize2, input, xSize, ySize);
    cropImage<short>(output2, xSize2, ySize2, output, xSize, ySize);

    delete[] dctCoeffs;
    delete[] inBlock;
    delete[] input2;
    delete[] output2;

}

//JPEGBitStreamWriter streamer("example.jpg");
void performJPEGEncoding(uchar Y_buff[], char U_buff[], char V_buff[], int xSize, int ySize, int quality)
{
//	DEBUG(quality);
    char* Y_buff_ = new char[xSize*ySize];
    for(int i = 0; i < xSize * ySize; i++){
        Y_buff_[i] = Y_buff[i] - 128;
    }

    short* Y_spectr_coeff = new short[xSize*ySize];
    short* U_spectr_coeff = new short[(xSize/2)*(ySize/2)];
    short* V_spectr_coeff = new short[(xSize/2)*(ySize/2)];

    int N = 8;

    for(int i = 0; i < N*N; i++){
        quantizationMatrixLuminance[i] = quantQuality(QuantLuminance[i], 93);
        quantizationMatrixChrominance[i] = quantQuality(QuantChrominance[i], 93);
    }

    performDCT(Y_buff_, Y_spectr_coeff, xSize, ySize, N, true);    // quantType - luminance true
    performDCT(U_buff, U_spectr_coeff, xSize/2, ySize/2, N, false);
    performDCT(V_buff, V_spectr_coeff, xSize/2, ySize/2, N, false);


    doZigZag<uint8_t>(quantizationMatrixChrominance, N);
    doZigZag<uint8_t>(quantizationMatrixLuminance, N);


    stream.writeHeader();
    stream.writeQuantizationTables(quantizationMatrixLuminance, quantizationMatrixChrominance);
    stream.writeImageInfo(xSize, ySize);
    stream.writeHuffmanTables();


    short* Y_block = new short[N*N];  // blok za upis u jpeg fajl
    short* U_block = new short[N*N];
    short* V_block = new short[N*N];


    int y_uv = 0;
    int x_uv = 0;
    int y = 0;
    int x = 0;
    bool imageProcessed = false;
    while(!imageProcessed){
        if(y >= ySize){
            imageProcessed = true;
            break;
        }
        if(x >= xSize){
            x = 0;
            y += 2*N;
            x_uv = 0;
            y_uv += N;
        }


        for(int i = 0; i < 4; i++){  // 4 Y bloka na 1 U,V blok
            int dy, dx;
            if(i == 0){
                dy = 0;
                dx = 0;
            }
            else if(i == 1){
                dy = 0;
                dx = N;
            }
            else if(i == 2){
                dy = N;
                dx = 0;
            }
            else if(i == 3){
                dy = N;
                dx = N;
            }
            for(int yy = 0; yy < N; yy++){
                for(int xx = 0; xx < N; xx++){
                    Y_block[yy*N+xx] = Y_spectr_coeff[(y+yy+dy)*xSize + x+xx+dx];
                }
            }

            doZigZag<short>(Y_block, N);
            stream.writeBlockY(Y_block);

        }
        x += 2*N;

        for(int yy = 0; yy < N; yy++){
            for(int xx = 0; xx < N; xx++){
                U_block[yy*N+xx] = U_spectr_coeff[(y_uv+yy)*xSize/2 + x_uv+xx];
                V_block[yy*N+xx] = V_spectr_coeff[(y_uv+yy)*xSize/2 + x_uv+xx];

            }
        }
        doZigZag<short>(U_block, N);
        doZigZag<short>(V_block, N);
        stream.writeBlockU(U_block);
        stream.writeBlockV(V_block);

        x_uv += N;
    }


    stream.finishStream();


    for(int i = 0; i < xSize * ySize; i++){
        Y_buff[i] = Y_buff_[i] + 128;
    }

    // TO DO
    delete[] Y_buff_;
    delete[] Y_spectr_coeff;
    delete[] U_spectr_coeff;
    delete[] V_spectr_coeff;
    delete[] Y_block;
    delete[] U_block;
    delete[] V_block;
}
