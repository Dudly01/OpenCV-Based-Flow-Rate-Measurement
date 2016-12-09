#include <vector>
#include <opencv2/opencv.hpp>

#include "Bubbles.h"


//////////////////////////////////////////////////////////////////////////////////////////////////

using namespace cv;
using namespace std;

//////////////////////////////////////////////////////////////////////////////////////////////////

Bubble::Bubble(uint id)
{
	ID = id;
	Colour = Scalar(rand() % 256, rand() % 256, rand() % 256);

	isProcessed = false;
	LifeSpan = 0;
	Volume_outter = -1;
	Volume_inner = -1;
}

Bubble::Bubble(uint id, BubbleContourDatas bubble)
{
	ID = id;
	Colour = Scalar(rand() % 256, rand() % 256, rand() % 256);

	Current.Inner = bubble.Inner;
	Current.Outter = bubble.Outter;

	isProcessed = false;
	LifeSpan = 0;
	Volume_outter = -1;
	Volume_inner = -1;
}

Bubble::Bubble(uint id, ObjectContourDatas outter, ObjectContourDatas inner)
{
	ID = id;
	Colour = Scalar(rand() % 256, rand() % 256, rand() % 256);

	Current.Inner = inner;
	Current.Outter = outter;

	isProcessed = false;
	LifeSpan = 0;
	Volume_outter = -1;
	Volume_inner = -1;
}

//////////////////////////////////////////////////////////////////////////////////////////////////


void Bubble::setCurrent(ObjectContourDatas outter, ObjectContourDatas inner)
{
	Current.Inner = inner;
	Current.Outter = outter;
}

void Bubble::setCurrent(BubbleContourDatas bubble)
{
	Current.Inner = bubble.Inner;
	Current.Outter = bubble.Outter;
}

void Bubble::setProcessed(void)
{
	isProcessed = true;
	Volume_inner = ReturnBubbleVolume(Current.Inner.contour);
	Volume_outter = ReturnBubbleVolume(Current.Outter.contour);
}

void Bubble::prepareForNextFrame(void)
{
	Previous.Inner = Current.Inner;
	Previous.Outter = Current.Outter;
	LifeSpan++;
}

uint Bubble::getID(void)
{
	return ID;
}

Scalar Bubble::getColour(void)
{
	return Colour;
}

uint Bubble::getLifeSpan(void)
{
	return LifeSpan;
}

double Bubble::getVolumeOutter(void)
{
	return Volume_outter;
}

double Bubble::getVolumeInner(void)
{
	return Volume_inner;
}

bool Bubble::getIsProcessed(void)
{
	return isProcessed;
}

ObjectContourDatas Bubble::getPreviousInnerBubbleData(void)
{
	return Previous.Inner;
}

ObjectContourDatas Bubble::getPreviousOutterBubbleData(void)
{
	return Previous.Outter;
}

ObjectContourDatas Bubble::getCurrentInnerBubbleData(void)
{
	return Current.Inner;
}

ObjectContourDatas Bubble::getCurrentOutterBubbleData(void)
{
	return Current.Outter;
}

//////////////////////////////////////////////////////////////////////////////////////////////////


