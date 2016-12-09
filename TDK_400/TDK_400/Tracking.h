#ifndef TRACKING
#define TRACKING

using namespace cv;
using namespace std;




/*
How to get from the first Point to the second
.x grows to the right
.y grows down
.distance is sqrt(x^2+y^2)
*/

typedef struct
{
	//Where to go to the destination
	int x_dir;
	//Where to go to the destination
	int y_dir;
	double distance;

} Distance_Data;



Distance_Data DistanceDirectionTwoPoints(Point a, Point b);
Point ReturnMiddleOfRect(Rect rect);
void GetMinMaxXPixels(vector<Point> contour, int &x_left, int &x_right);
void GetMinMaxYPixels(vector<Point> contour, int x, int &y_up, int &y_down);
int ReturnMinMaxYPixelsDistance(vector<Point> contour, int x);
double ReturnBubbleVolume(vector<Point> contour);


#endif