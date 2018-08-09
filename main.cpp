//OpenCV
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/core.hpp>
#include <opencv2/ml.hpp>

//Standard Libraries
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <time.h>
#include <locale>
#include <stdexcept>
#include <unistd.h>

//Thread Libraries
#include <pthread.h>
#include <thread>

//From C Libraries
#include <cstdio>
#include <cstdlib>
#include <ctime>

using namespace std;
using namespace cv;

// "rtsp://admin:pvllck@10.110.1.56:554/cam/realmonitor?channel=1&subtype=1"

//Prototypes
void* runEvents(void* thread_id);

int main(int argc, char ** argv)
{
	//=== Arguments ===
	if(argc<3)
	{
	    cerr<<"./program.o <type> <id> <id> ... <id>"<<endl;
		cerr<<"example: ./main.o MovEvents 272707 676706"<<endl;
	    exit(-1);
	}
	//Define ids vector
	int i;
	vector<string> ids;
	for(i=2; i<argc; i++)
	{
		ids.push_back( argv[i] );
	}

	//=== Types: Training, Events or Analysis ===
	if(strcmp(argv[1],"MovEvents")==0)
	{
			//=== Init Threads ===
			pthread_t threads[argc];

			for(i=0; i<(int)ids.size(); i++)
			{
				pthread_create(&threads[i], NULL, &runEvents, (void*)&ids[i]);
				sleep(1);
			}
			for(i=0; i<(int)ids.size(); i++)
			{
				pthread_join(threads[i], NULL);
			}
	}
	else
	{
		cerr<<"yep"<<endl;
		exit(-1);
	}

	return 0;
}

//string command = "mkdir -p imagens/" + to_string(id) + to_string(id) + "_" + to_string(cam);
//system(command.c_str());
//command.clear();

void* runEvents(void* thread_id)
{
	//=== System Variables ===
	Mat frame;
	string *t_id;
	//convert void* to string*
	t_id = (string *)thread_id;
	string sysid = *t_id;

	//=== Runing Application ===
	Mat before,now,sub;
	VideoCapture cap("rtsp://admin:pvllck@10.110.1.56:554/cam/realmonitor?channel=1&subtype=1"); //sysid.c_str());

	Mat element = getStructuringElement( MORPH_RECT, Size( 3, 3 ),Point( -1,-1 ) );
	cap.read(before);
	cvtColor(before, before, CV_RGB2GRAY);
	medianBlur(before,before,3);
	while(true)
	{
		if(cap.read(frame))
		{
			frame.copyTo(now);
			cvtColor(now, now, CV_RGB2GRAY);
			medianBlur(now,now,3);
			sub = abs( now -  before );
			erode( sub, sub, element );
			for(int y=0;y<sub.rows;y++)
			{
					for(int x=0;x<sub.cols;x++)
					{
								if(sub.at<uchar>(y,x)>10)
								{
									sub.at<uchar>(y,x) = 255;
								}
								else
								{
									sub.at<uchar>(y,x) = 0;
								}
					}
			}
			float dif =  float(countNonZero(sub))/float(sub.cols*sub.rows);

			imshow("frame",frame);
			waitKey(1);

			if( dif > 0.01 )
			{
				// save
			}

			now.copyTo(before);
		}
	}
	frame.release();

	pthread_exit(NULL);
}
