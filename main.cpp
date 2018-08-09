// g++ -std=c++11 main.cpp -o main.o `pkg-config --cflags --libs opencv` -lpthread

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

class LCTEventVideoWriter
{
	private:
		VideoWriter vid;
		int prevEvent;
		int ifIsSaving;
		vector<Mat> prev;
		struct tm *T;
		char videoname[300];
		int nCarryFrames, fps;
		string pathFolder;
		int nrows;
		int ncols;
		
	public:
		LCTEventVideoWriter();
		LCTEventVideoWriter(int rows,int cols,int ncarryframes,int _fps,string path);
		void runEventVideoWriterDateTime(int _event, Mat frame);
		void runEventVideoWriterCounter(int _event, Mat frame);
		void runEventVideoWriterDateTime(int _event, Mat frame, int sysfps);
		void runEventVideoWriterDateTime(int _event, Mat frame, int sysfps, int id_db);
};

//==== public ====
LCTEventVideoWriter::LCTEventVideoWriter(){ // @suppress("Class members should be properly initialized")
}

LCTEventVideoWriter::LCTEventVideoWriter(int rows,int cols,int ncarryframes,int _fps,string path){ // @suppress("Class members should be properly initialized")
	nrows = rows;
	ncols = cols;
	nCarryFrames = ncarryframes;
	fps = _fps;
	pathFolder = path;

	ifIsSaving = 0;
	prevEvent = -1;
}

void LCTEventVideoWriter::runEventVideoWriterDateTime(int _event, Mat frame)
{
	if(frame.rows!=nrows || frame.cols!=ncols){return ;}

	// Update Carry Frame
	if(prev.empty())
	{
		prev = vector<Mat> (nCarryFrames);
	}
	for(int i=0;i<int(prev.size())-1;i++)
	{
		prev[i+1].copyTo(prev[i]);
	}
	frame.copyTo(prev[prev.size()-1]);

	// Verify write process
	if( prevEvent == -1 )
	{
		prevEvent = _event;
	}
	else if( ifIsSaving > 0 )
	{
		if( ifIsSaving < nCarryFrames && _event==1 )
		{
			vid.write(frame);
			ifIsSaving++;
			// significa q ainda falta salvar frames do final...
			// porem se ja mudou para status de aberto deve dar release
		}
		else
		{
			vid.release();
			ifIsSaving = 0;
		}
	}
	else if( ifIsSaving == 0 )
	{
		if( prevEvent==1  && _event==0 )
		{
			time_t my_time;
			time (&my_time);
			T = localtime (&my_time);

			sprintf(videoname,"%s/(%d_%d_%d)_(%dh_%dmin_%ds).avi",pathFolder.c_str(),T->tm_year+1900,T->tm_mon+1,T->tm_mday,T->tm_hour,T->tm_min,T->tm_sec);
			vid.open(videoname,CV_FOURCC('M','J','P','G'),fps,frame.size(),true);

			for(int i=0; i<int(prev.size()); i++)
			{
				vid.write( prev[i] );
			}
		}
		else if( prevEvent==0 && _event==0 )
		{
			vid.write(frame);
		}
		else if( prevEvent==0  && _event==1 )
		{
			vid.write(frame);
			ifIsSaving = 1;
		}
		prevEvent = _event;
	}
}

//Prototype
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

// "rtsp://admin:pvllck@10.110.1.56:554/cam/realmonitor?channel=1&subtype=1"

void* runEvents(void* thread_id)
{
	time_t timeropen,timernow;
	time (&timernow);
	time (&timeropen);
	
	string command = "mkdir -p videos/";
	system(command.c_str());
	command.clear();
	string folder = "videos";

	//=== System Variables ===
	Mat frame;
	string *t_id;
	//convert void* to string*
	t_id = (string *)thread_id;
	string sysid = *t_id;

	//=== Runing Application ===
	int fps = 10;
	int event = 1;
	Mat before,now,sub;
	LCTEventVideoWriter vw;
	VideoCapture cap("rtsp://admin:pvllck@10.110.1.56:554/cam/realmonitor?channel=1&subtype=1"); //sysid.c_str());

	Mat element = getStructuringElement( MORPH_RECT, Size( 3, 3 ),Point( -1,-1 ) );
	cap.read(before);
	cvtColor(before, before, CV_RGB2GRAY);
	medianBlur(before,before,3);
	vw  = LCTEventVideoWriter(before.rows, before.cols, 3, fps,folder);


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
								if(sub.at<uchar>(y,x)>3)
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
			//imshow("sub",sub);
			waitKey(1);
		

			time (&timernow);
			if( dif > 0.0001 )
			{
				event = 0;
				time (&timeropen);
			}
			else if( difftime(timernow, timeropen) > 10 )
			{
				event = 1;
			}	
			printf("%f\n",float(difftime(timernow, timeropen)));
			
			vw.runEventVideoWriterDateTime(event, frame);
			now.copyTo(before);
		}
	}
	frame.release();

	pthread_exit(NULL);
}
