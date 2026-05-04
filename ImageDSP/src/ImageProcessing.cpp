
#include "ImageProcessing.h"
#include "ColorSpaces.h"
#include "JPEG.h"

#include <cmath>

#include <QDebug>
#include <QString>
#include <QImage>


void imageProcessingFun(const QString& progName, QImage& outImgs, const QImage& inImgs, const QVector<double>& params)
{
	// TO DO

	/* Create buffers for YUV image */
    int x = inImgs.width();
    int y = inImgs.height();
    uchar* Y_buff = new uchar[x*y];
    char* V_buff = new char[x*y/4];
    char* U_buff = new char[x*y/4];


	/* Create empty output image */
	outImgs = QImage(inImgs.width(), inImgs.height(), inImgs.format());

	/* Convert input image to YUV420 image */
    RGBtoYUV420(inImgs.bits(), x, y, Y_buff, U_buff, V_buff);

    if(progName == QString("JPEG Encoder"))
	{	
		/* Perform NxN DCT */

        //decimate_Y(Y_buff, x, y);

        performJPEGEncoding(Y_buff, U_buff, V_buff, x, y, params[0]);
	}

    /* Convert YUV image back to RGB */
    procesing_YUV420(Y_buff, U_buff, V_buff, x, y, 1, 0, 0);
    YUV420toRGB(Y_buff, U_buff, V_buff, inImgs.width(), inImgs.height(), outImgs.bits());

    //outImgs = QImage("example.jpg");

	/* Delete used memory buffers */
    delete[] Y_buff;
    delete[] U_buff;
    delete[] V_buff;
}

