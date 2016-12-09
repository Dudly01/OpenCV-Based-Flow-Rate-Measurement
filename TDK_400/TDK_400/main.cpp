//Továbbfejlesztett változata a már mûködõ programnak
//amiben a ROI-t a konkrét csõre illesztem nem a környezetére ezzel még pontosabb vizsgálatot tehetek, zavarelnyomásilag.

#include <stdio.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <string.h>
#include <chrono>
#include <ctime>

#include <opencv2\opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/background_segm.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2\imgcodecs.hpp>

#include "Bubbles.h"
#include "Tracking.h"
#include "Global.h"

using namespace cv;
using namespace std;

vector<Bubble> IdentifiedBubbles;

double OnePixelLength = -1;
double ConstantNumberVolume = -1;

////The physical values of the system in milimeter!
//const double TubeRadius = 2.8;
//const double OnePixelLength = 68 / 678;
//const double ConstantNumberVolume = OnePixelLength*OnePixelLength*OnePixelLength / 4 * 3.1415;


int main()
{
	///////////////////////////////////////////////////////////////////////////////////////

	cv::Rect RangeOfInterest(239, 200, 795, 350);
	cv::Rect RangeOfInterestBubble(68, 80, 676, 95);
	Size CaptureSize(RangeOfInterest.width, RangeOfInterest.height);

	cv::Rect ResolutionRect = RangeOfInterestBubble;
	ResolutionRect.height += 160;

	Point A(ResolutionRect.x, ResolutionRect.y + ResolutionRect.height - 6);
	Point B(ResolutionRect.x + ResolutionRect.width, ResolutionRect.y + ResolutionRect.height - 17);

	//68mm divided by the number of pixels, so we get one pixel's dimension
	OnePixelLength = 68.0 / (double)DistanceDirectionTwoPoints(A, B).distance;
	ConstantNumberVolume = OnePixelLength*OnePixelLength*OnePixelLength / 4.0 * 3.1415;

	Size BlurSize(8, 8);
	int BlurThreshold = 15;

	int MinFollowDistance = 25;

	///////////////////////////////////////////////////////////////////////////////////////

	for (int i = 0; i < 100; i++)
	{
		TableSquareOfNumbers.push_back(i*i);
	}

	//The counter used to give ID to the bubbles
	uint Counter = 1;

	uint NumberOfProcessedBubbles = 0;

	Mat FrameCurrent, FrameROI, FrameFG, FrameBG;
	Mat FrameBubbleROI;
	Mat FrameBlur;
	Mat FrameHull;

	std::chrono::time_point<std::chrono::system_clock> end;
	end = std::chrono::system_clock::now();
	std::time_t end_time = std::chrono::system_clock::to_time_t(end);

	time_t rawtime;
	struct tm * timeinfo;
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	char filename[200];
	size_t l = strftime(filename,
		sizeof(filename),
		"MVI_0943_%Y_%m_%d_%Hh_%Mm_%Ss.avi",
		timeinfo);
	assert(l != 0);

	//Smaller file size
	VideoWriter Record(filename, CV_FOURCC('X', 'V', 'I', 'D'), 50., CaptureSize);

	//Good quality large file size
	/*VideoWriter Record("teszt11_LifeSpan.avi", CV_FOURCC('M', 'J', 'P', 'G'), 50., Size(678, 62 * 2));*/

	char fileName[100] = "C:/Users/Andris-W520/Videos/2016 TDK 2.0/MVI_0943.MOV";

	VideoCapture TheVideo;   //0 is the id of video device.0 if you have only one camera

	TheVideo.open(fileName);

	TheVideo.set(CV_CAP_PROP_POS_MSEC, (60 * 2 + 14) * 1000);// Skip to the first bubble

	Ptr<BackgroundSubtractor> pMOG2; //MOG2 Background subtractor
	pMOG2 = createBackgroundSubtractorMOG2(500, 200, false); //35, 50, 100, 1000

															  // Loop through video  
	bool FirstFrame = true;
	int index = 0;
	while (true)
	{
		if (!(TheVideo.read(FrameCurrent))) //get one frame form video
		{
			//The end of the video
			Record.release();
			TheVideo.release();
			destroyAllWindows();
			break;
		}

		FrameROI = FrameCurrent(RangeOfInterest);
		FrameBubbleROI = FrameROI(RangeOfInterestBubble);

		if (FirstFrame)
		{
			FirstFrame = false;
			pMOG2->apply(FrameBubbleROI, FrameFG, 1);
			pMOG2->getBackgroundImage(FrameBG);
		}

		pMOG2->apply(FrameBubbleROI, FrameFG, 0.);

		Mat Rect3 = getStructuringElement(MORPH_RECT, Size(3, 3));
		Mat Rect2 = getStructuringElement(MORPH_RECT, Size(2, 2));

		erode(FrameFG, FrameFG, Rect2, Point(-1, -1), 0);
		dilate(FrameFG, FrameBlur, Rect2, Point(-1, -1), 3);


		//blur(FrameFG, FrameBlur, BlurSize);
		//threshold(FrameBlur, FrameBlur, BlurThreshold, 255, THRESH_BINARY);

		//GaussianBlur(FrameFG, FrameBlur, Size(7, 3), 40, 0);

		//the line have to be put in the second columns so the countour is in the frame
		cv::line(FrameBlur, Point(1, 2), Point(1, RangeOfInterestBubble.height - 2), Scalar(255, 255, 255), 1);//Add a line so the end of the tube won't fuck up the complex hull;
		cv::line(FrameBlur, Point(RangeOfInterestBubble.width - 2, 2), Point(RangeOfInterestBubble.width - 2, RangeOfInterestBubble.height - 2), Scalar(255, 255, 255), 1);

		///Let's process the contours

		vector<vector<Point> > contours;
		vector<Vec4i> hierarchy;

		/// Find contours
		Mat Temp = FrameBlur.clone();
		findContours(Temp, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_NONE, Point(0, 0));

		FrameHull = Mat::zeros(FrameBlur.size(), CV_8UC3);
		vector<BubbleContourDatas> UnidentifiedBubbleDatas;
		BubbleContourDatas TempBubbleData;

		///Process the contours, get the ones that are possible bubbles
		if (contours.size() != 0)
			for (int i = 0; i != -1;)
			{
				double outterArea = boundingRect(Mat(contours[i])).area();

				//If they are not too big nor too small and they have inner contours
				if (outterArea > 400. && hierarchy[i][2] != -1)
				{
					for (int j = hierarchy[i][2]; j != -1;)
					{
						double innerArea = boundingRect(Mat(contours[j])).area();

						if (innerArea > 200.)
						{
							TempBubbleData.Inner.contour = contours[j];
							TempBubbleData.Inner.bounding_rect = boundingRect(Mat(contours[j]));
							convexHull(Mat(contours[j]), TempBubbleData.Inner.hull, true);
							TempBubbleData.Inner.midpoint = ReturnMiddleOfRect(TempBubbleData.Inner.bounding_rect);

							TempBubbleData.Outter.contour = contours[i];
							TempBubbleData.Outter.bounding_rect = boundingRect(Mat(contours[i]));
							convexHull(Mat(contours[i]), TempBubbleData.Outter.hull, true);
							TempBubbleData.Outter.midpoint = ReturnMiddleOfRect(TempBubbleData.Outter.bounding_rect);

							UnidentifiedBubbleDatas.push_back(TempBubbleData);
						}

						j = hierarchy[j][0];

					}


				}
				//We go through the outter contours
				i = hierarchy[i][0];
			}


		///Here we got all the possible bubbles put into UnidentifiedBubbleDatas, now we have to identify them

		//If there are no bubbles to match with, put every bubble into the Identified Bubbles
		if (IdentifiedBubbles.empty())
		{
			for (int j = 0; j < UnidentifiedBubbleDatas.size(); j++)
			{
				//Call the constructor with the ID and the BubbleContourDatas
				Bubble temp(Counter, UnidentifiedBubbleDatas[j]);
				Counter++;
				//Put the bubble into the identified ones
				IdentifiedBubbles.push_back(temp);
			}
		}

		//If there were no unidentified bubbles we shall delete all identified bubbles
		else if (UnidentifiedBubbleDatas.empty())
		{
			IdentifiedBubbles.clear();
		}

		//If there were identified bubbles we have to match them up with the unidentified ones
		else
		{
			//Make the matrix for the hungarian-like algorithm
			//The identified bubbles are the rows, the columns are the unidentified ones
			vector<vector<Distance_Data>> DistanceMtrx;

			//Calculate the Distance_Data between the bubble middle points
			for (int k = 0; k < IdentifiedBubbles.size(); k++)
			{
				//Get a temporary row
				vector<Distance_Data> row;

				//Fill up the row
				for (int l = 0; l < UnidentifiedBubbleDatas.size(); l++)
				{
					//Get the Distance_Datas regarding the middle point of the identified- and the unidentified bubbles
					Distance_Data data = DistanceDirectionTwoPoints(IdentifiedBubbles[k].getPreviousInnerBubbleData().midpoint, UnidentifiedBubbleDatas[l].Inner.midpoint);
					row.push_back(data);
				}

				//Save the filled up row into the matrix
				DistanceMtrx.push_back(row);
			}

			///The DistanceMtrx has been filled with the Distance_Datas at this point

			//Lets make variables where we store which rows and colmuns have been processed, for now all is unprocessed hence false
			vector<bool> PairedIBubbles(IdentifiedBubbles.size(), false);
			//Lets make variables where we store which rows and colmuns have been processed, for now all is unprocessed hence false
			vector<bool> PairedUBubbles(UnidentifiedBubbleDatas.size(), false);

			//Pair up the bubbles
			for (int k = 0; k < IdentifiedBubbles.size(); k++)
			{
				//For every row we search for the coulmn number containing the lowest distance
				int min_index = 0;
				double min_value = DistanceMtrx[k][0].distance; //The first minimum is in the first column

				for (int l = 1; l < UnidentifiedBubbleDatas.size(); l++)
				{
					if (DistanceMtrx[k][l].distance < min_value)
					{
						min_value = DistanceMtrx[k][l].distance;
						min_index = l;
					}
				}

				///We checked through all the columns so got the minimum values

				//Check whether the minimum value is close enough to  be a possible solution
				if (min_value < MinFollowDistance)
				{
					//The distance is possible so we match up the IdentifiedBubble with the UnidentifiedBubble having the index min_index
					IdentifiedBubbles[k].setCurrent(UnidentifiedBubbleDatas[min_index]);
					//Set the process status
					PairedIBubbles[k] = true;
					PairedUBubbles[min_index] = true;
				}
				else
				{
					;//The distance is too big, the closest UnidentifiedBubble's middle point don't belong to this bubble
				}
			}

			///The pair-ups have been made, we have the data which rows and columns have not found their pair

			//The left alone unidentified bubbles have to be made identified as new bubbles
			for (int m = 0; m < PairedUBubbles.size(); m++)
			{
				if (PairedUBubbles[m] == false)
				{
					//Call the constructor with the ID and the ObjectContourDatass
					Bubble temp(Counter, UnidentifiedBubbleDatas[m]);
					Counter++;
					//Put the bubble into the identified ones
					IdentifiedBubbles.push_back(temp);
				}
			}

			//The left alone identified bubbles have to be thrown out
			//We check from the bigger indexes, so upon erasing the indexes won't get fucked up
			for (int m = PairedIBubbles.size(); m >= 1; m--)
			{
				if (PairedIBubbles[m - 1] == false)
				{
					IdentifiedBubbles.erase(IdentifiedBubbles.begin() + m - 1);
				}
			}
		}


		UnidentifiedBubbleDatas.clear();


		///The bubbles have been identified

		for (int i = 0; i < IdentifiedBubbles.size(); i++)
		{
			if (IdentifiedBubbles[i].getLifeSpan() > 25)
			{
				vector<vector<Point> > contour_inner;
				vector<vector<Point> > hull_inner;
				vector<Rect> rect_inner;

				vector<vector<Point> > contour_outter;
				vector<vector<Point> > hull_outter;
				vector<Rect> rect_outter;

				contour_inner.push_back(IdentifiedBubbles[i].getCurrentInnerBubbleData().contour);
				hull_inner.push_back(IdentifiedBubbles[i].getCurrentInnerBubbleData().hull);
				rect_inner.push_back(IdentifiedBubbles[i].getCurrentInnerBubbleData().bounding_rect);

				contour_outter.push_back(IdentifiedBubbles[i].getCurrentOutterBubbleData().contour);
				hull_outter.push_back(IdentifiedBubbles[i].getCurrentOutterBubbleData().hull);
				rect_outter.push_back(IdentifiedBubbles[i].getCurrentOutterBubbleData().bounding_rect);

				//Draw the hulls, rects and middle points on a picture for debugging reasons

				drawContours(FrameHull, hull_outter, 0, IdentifiedBubbles[i].getColour()*0.5, -1, 8, vector<Vec4i>(), 0, Point());
				drawContours(FrameHull, contour_outter, 0, Scalar(0, 153, 255), 1);

				drawContours(FrameHull, hull_inner, 0, IdentifiedBubbles[i].getColour(), -1, 8, vector<Vec4i>(), 0, Point());
				drawContours(FrameHull, contour_inner, 0, Scalar(0, 0, 255), 1);
				//rectangle(FrameHull, rect[0], Scalar(0, 0, 255), 1);

				//If it reaches the middle line then let's process it
				if (IdentifiedBubbles[i].getCurrentInnerBubbleData().midpoint.x < 405 - 66 && IdentifiedBubbles[i].getIsProcessed() == false)
				{
					IdentifiedBubbles[i].setProcessed();

					SumInnerBubbleVolume += IdentifiedBubbles[i].getVolumeInner();
					SumOutterBubbleVolume += IdentifiedBubbles[i].getVolumeOutter();
					NumberOfProcessedBubbles++;
				}

				//Colour the middle point whether it is processed
				if (IdentifiedBubbles[i].getIsProcessed() == true)
				{
					//Turn the Volume souble into string
					ostringstream InnerStrS;
					InnerStrS << IdentifiedBubbles[i].getVolumeInner() << " mm^3";
					string InnerStr = InnerStrS.str();

					ostringstream OutterStrS;
					OutterStrS << IdentifiedBubbles[i].getVolumeOutter() << " mm^3";
					string OutterStr = OutterStrS.str();

					circle(FrameHull, IdentifiedBubbles[i].getCurrentInnerBubbleData().midpoint, 4, Scalar(0, 255, 0), -1);

					putText(FrameHull, InnerStr, Point(IdentifiedBubbles[i].getCurrentInnerBubbleData().midpoint.x + 2, IdentifiedBubbles[i].getCurrentInnerBubbleData().midpoint.y + 8), CV_FONT_HERSHEY_SIMPLEX, 0.8, Scalar(0, 0, 0), 5);
					putText(FrameHull, InnerStr, Point(IdentifiedBubbles[i].getCurrentInnerBubbleData().midpoint.x + 2, IdentifiedBubbles[i].getCurrentInnerBubbleData().midpoint.y + 8), CV_FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 1);

					putText(FrameHull, OutterStr, Point(IdentifiedBubbles[i].getCurrentInnerBubbleData().midpoint.x + 2, IdentifiedBubbles[i].getCurrentInnerBubbleData().midpoint.y + 8 - 30), CV_FONT_HERSHEY_SIMPLEX, 0.8, Scalar(0, 0, 0), 5);
					putText(FrameHull, OutterStr, Point(IdentifiedBubbles[i].getCurrentInnerBubbleData().midpoint.x + 2, IdentifiedBubbles[i].getCurrentInnerBubbleData().midpoint.y + 8 - 30), CV_FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 1);
				}
				else
				{
					circle(FrameHull, IdentifiedBubbles[i].getCurrentInnerBubbleData().midpoint, 4, Scalar(0, 0, 255), -1);
				}

			}

			IdentifiedBubbles[i].prepareForNextFrame();
		}

		ostringstream StrS;
		StrS << "Inner:" << SumInnerBubbleVolume << " Outter:" << SumOutterBubbleVolume << " [mm^3] = [uL]";
		string Str = StrS.str();

		putText(FrameROI, Str, Point(3, 30), CV_FONT_HERSHEY_SIMPLEX, 0.8, Scalar(0, 0, 0), 5);
		putText(FrameROI, Str, Point(3, 30), CV_FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 1);

		ostringstream StrS0;
		StrS0 << "BubbleCount:" << NumberOfProcessedBubbles;
		string Str0 = StrS0.str();

		putText(FrameROI, Str0, Point(3, 60), CV_FONT_HERSHEY_SIMPLEX, 0.8, Scalar(0, 0, 0), 5);
		putText(FrameROI, Str0, Point(3, 60), CV_FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 1);



		Mat FrameCountLine = FrameROI.clone();

		cv::line(FrameCountLine, Point(405, 0), Point(405, 250), Scalar(0, 255, 0), 5);
		//cv::line(FrameBubbleROI, Point(405-66, 0), Point(405-66, 250), Scalar(0, 255, 0), 5);

		cv::rectangle(FrameCountLine, ResolutionRect, Scalar(0, 0, 0), 1);

		//The red rectangle is where the magic happens, it is 678 pixels wide and it equals to 6,8cm
		cv::rectangle(FrameCountLine, RangeOfInterestBubble, Scalar(0, 0, 255), 1);

		//Draws the line which the pixeldistance is calculated from
		cv::line(FrameCountLine, A, B, Scalar(0, 0, 255), 1);

		//Mivel a ROI kiválasztásánál nem clone-ozom az eredeti képemet, mert azzal csak lassítok, ezért a BubbleRoi-n is változást hozok létre, amit szintén nem clone-ozva hozok létre, vagyis csak az eredeti adatok egy megfelelõ részére hivatkozom.
		cv::addWeighted(FrameCountLine, 0.7, FrameROI, 1 - 0.7, 0, FrameROI);

		//FrameROI.setTo(Scalar(0, 0, 255), FrameHull);




		if (1)
		{
			//imshow("ROI1", FrameROI);
			//imshow("Bubble RO1", FrameBubbleROI);
			//imshow("foreground", FrameFG);
			//imshow("Blurred", FrameBlur);
			//imshow("Hull", FrameHull);

			FrameBubbleROI = FrameBubbleROI + FrameHull;

			//imshow("ROI + hull", FrameROI);

			Record.write(FrameROI);

			//if (waitKey(3) >= 0)
			//	break;
		}



	}

}




