#include <vector>
#include <opencv2/opencv.hpp>
#include <math.h>

#include "Tracking.h"
#include "Global.h"

using namespace cv;
using namespace std;

//Returns the distance and the route from point a to point b, x grows to the right and y down.
Distance_Data DistanceDirectionTwoPoints(Point a, Point b)
{
	Distance_Data data;
	data.x_dir = b.x - a.x;
	data.y_dir = b.y - a.y;
	data.distance = sqrt((b.x - a.x)*(b.x - a.x) + (b.y - a.y)*(b.y - a.y));
	return data;
}

//Returns the middle point of a rectangle
Point ReturnMiddleOfRect(Rect rect)
{
	return Point(rect.x + rect.width / 2, rect.y + rect.height / 2);
}

//Gets the most left and most right pixels from an objects contours into the two int& parameters
void GetMinMaxXPixels(vector<Point> contour, int &x_left, int &x_right)
{
	x_left = contour[0].x;
	x_right = contour[0].x;

	for (int i = 1; i < contour.size(); i++)
	{
		if (x_left > contour[i].x)
			x_left = contour[i].x;

		if (x_right < contour[i].x)
			x_right = contour[i].x;
	}

	return;
}

//Gets the upper and lower pixels from a contour at a given horizontal x value into the int& parameters
void GetMinMaxYPixels(vector<Point> contour, int x, int &y_up, int &y_down)
{
	bool first = true;

	for (int i = 0; i < contour.size(); i++)
	{
		if (contour[i].x == x)
		{
			if (first)
			{
				y_up = contour[i].y;
				y_down = contour[i].y;
			}

			else
			{
				//If we get a smaller value then it is higher in the picture
				if (y_up > contour[i].y)
					y_up = contour[i].y;

				//If we get a bigger value then it is lower in the picture
				if (y_down < contour[i].y)
					y_down = contour[i].y;
			}
		}
	}

	return;
}


//Returns the pixel distance of the upper and lower pixels from a contour at a given horizontal x value
int ReturnMinMaxYPixelsDistance(vector<Point> contour, int x)
{
	bool first = true;
	int y_up;
	int y_down;

	for (int i = 0; i < contour.size(); i++)
	{
		if (contour[i].x == x)
		{
			if (first)
			{
				first = false;
				y_up = contour[i].y;
				y_down = contour[i].y;
			}

			else
			{
				//If we get a smaller value then it is higher in the picture
				if (y_up > contour[i].y)
					y_up = contour[i].y;

				//If we get a bigger value then it is lower in the picture
				if (y_down < contour[i].y)
					y_down = contour[i].y;
			}
		}
	}
	return (y_down - y_up) - 1;
}

//Returns the volume of the bubble
double ReturnBubbleVolume(vector<Point> contour)
{
	//The horizontal boundaries of the bubble's contour
	int x_left, x_right;

	//We get the vector of how much did we get from different radiuses in pixels, all elements are 0 at this point
	vector<int> TableNumberOfDiametersPx(100, 0);

	//Get the horizontal boundaries of the bubble's contour
	GetMinMaxXPixels(contour, x_left, x_right);

	//We fill up the vector containing the number of the specific diameters
	//TableNumberOfDiametersPx[5] = 2 means there are two cylinders with the diameter of five pixels

	//its f.ing slow so not all of the bubble should be get like this
	for (int i = x_left; i <= x_right; i++)
	{
		TableNumberOfDiametersPx[ReturnMinMaxYPixelsDistance(contour, i)]++;
	}


	double SUM = 0;

	for (int i = 1; i < 100; i++)
	{
		SUM = SUM + TableNumberOfDiametersPx[i] * TableSquareOfNumbers[i];
	}

	return SUM*ConstantNumberVolume;
}