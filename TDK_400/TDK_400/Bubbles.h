#ifndef BUBBLES
#define BUBBLES

#include <vector>
#include <opencv2/opencv.hpp>

#include "Tracking.h"
#include "Global.h"

//////////////////////////////////////////////////////////////////////////////////////////////////

using namespace cv;
using namespace std;

// Contour datas, like contour points, covering hull, bounding rect and midpoint of said rect
typedef struct
{
	vector<Point> contour;
	vector<Point> hull;
	Rect bounding_rect;
	Point midpoint;

} ObjectContourDatas;

// Inner and Outter contour datas, like contour points, covering hull, bounding rect and midpoint of said rect
typedef struct
{
	ObjectContourDatas Inner;
	ObjectContourDatas Outter;

} BubbleContourDatas;

//////////////////////////////////////////////////////////////////////////////////////////////////

//A bubble with it's ID, Colour, LifeSpan, Inner and Outter Contour, and that contour's Volume, procession status
class Bubble
{
public:

	void setCurrent(ObjectContourDatas outter, ObjectContourDatas inner);
	void setCurrent(BubbleContourDatas bubble);
	void setProcessed(void);
	void prepareForNextFrame(void);

	uint getID(void);
	Scalar getColour(void);
	uint getLifeSpan(void);
	double getVolumeOutter(void);
	double getVolumeInner(void);

	bool getIsProcessed(void);

	ObjectContourDatas getPreviousInnerBubbleData(void);//Returns previous bubble datas like hull, rect or middle point
	ObjectContourDatas getPreviousOutterBubbleData(void);//Returns previous bubble datas like hull, rect or middle point
	ObjectContourDatas getCurrentInnerBubbleData(void);//Gets previous bubble datas like hull, rect or middle point
	ObjectContourDatas getCurrentOutterBubbleData(void);//Gets previous bubble datas like hull, rect or middle point

	Bubble(uint id);
	Bubble(uint id, BubbleContourDatas bubble);
	Bubble(uint id, ObjectContourDatas outter, ObjectContourDatas inner);

private:
	uint ID;
	Scalar Colour;

	BubbleContourDatas Previous;
	BubbleContourDatas Current;

	bool isProcessed;
	uint LifeSpan;
	double Volume_outter;
	double Volume_inner;
};

#endif