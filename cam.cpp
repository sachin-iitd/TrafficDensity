
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include <iostream>
#include <bits/stdc++.h>
#include <opencv2/video/video.hpp>
#include "videoio.hpp"
#include <thread> 
#include <unistd.h>
#include <mutex>
#include <fcntl.h>

using namespace cv;
using namespace std;

int FPS = 18;
const char* ip = "1";

int myFPS = 3, myComp = 1; 
int dump_scale = 2;
int dump_time = 60;
//int rounds = 0;
int output_fd=-1;
double resize_percent=0.2;

mutex g_mutex, g_mutex2;
volatile int outbuf_off = 0;
char outbuf[31*60*(60+6)];

//usr/local/include/opencv4/opencv2/
//int ix=30,fx=200 ;
int caly ;
int calx ;
int X,Y ;
int sx,sy,dx,dy ;
double Vwidth, Vheight;


char* get_current_time_point(void)
{
    time_t timeStamp = chrono::system_clock::to_time_t(chrono::system_clock::now());
    return ctime(&timeStamp);
}

//******************************************************_______________________________________________________________________________________*******************
////PARAMETERS THAT CAN BE ALTERED FOR BETTER ACUURACY : XFIT,YFIT,DYNAMIC_TIME,STATIC_TIME,VARIANCE_FROM_AVG and inside isRED_ arguments of find_line************
//******************************************************_______________________________________________________________________________________*******************

// MUST SET BELOW PARAMETERS FIRST

double BLOB_RATIO=0.3;
int XFIT=7 ; 				//Number of vehicles that would fit in horizontal, now lets us suppose we can fit 3 cars so we may want to have XFIT atleast 3 but since we can also have bikes we can make it 4 or 5 or 6 
int YFIT=7 ; 				//Similar explanation but since bikes and cars have similar length, this is not a problem
// int DYNAMIC_TIME=2 ;   	//Time within which a frame would completely disappear from average, we may want to keep it small so that slow vehicles or stopped vehicles disappear quickly in seconds or FRAMEUNITS
int STATIC_TIME=30 ; 		//in seconds or FRAMEUNITS
int VARIANCE_FROM_AVG =35 ; //variance from avg pixel, the variance from average pixel when it is considered an anomaly, be careful, should be between 10 to 40
int VAR =125 ; 				//variance from avg pixel, useless for now

vector<Point2f> points;  
vector<Point2f> points2;
bool proj_set=false;
bool size_set=false;

Mat staticAVG,dynAVG ;
Mat staticAVG_dif,dynAVG_dif ;
double white_area_variable=0.0, white_area_variable1 = 0.0, stop_density=0.0;
ifstream infile;
ofstream outfile;

Mat prvs;

int GetNextPhaseFLR(float* density, int cur_phase){return 0;}
int GetNextPhaseFLG(float* density, int cur_phase){return 0;}

void on_mouse2( int e, int x, int y, int d, void *ptr ){
	if (e == EVENT_LBUTTONDOWN )
	{
		if(points2.size() < 3 )
		{

			points2.push_back(Point2f(float(x),float(y)));
			outfile << x << " "<< y <<endl;
			cout << x << " "<< y <<endl;
		}
		else if(points2.size()==3)
		{
			points2.push_back(Point2f(float(x),float(y)));
			outfile << x << " "<< y <<endl;
			cout << x << " "<< y <<endl;
			cout << " Done, Thanks! " <<endl;
			// Deactivate callback
			setMouseCallback("Display window2", NULL, NULL);
			size_set=true;
		}

	}
}
void on_mouse( int e, int x, int y, int d, void *ptr ){
	if (e == EVENT_LBUTTONDOWN )
	{
		if(points.size() < 3 )
		{

			points.push_back(Point2f(float(x),float(y)));
			outfile << x << " "<< y <<endl;
			cout << x << " "<< y <<endl;
		}
		else if(points.size()==3)
		{
			points.push_back(Point2f(float(x),float(y)));
			outfile << x << " "<< y <<endl;
			cout << x << " "<< y <<endl;
			cout << " Done, Thanks! " <<endl;
			// Deactivate callback
			setMouseCallback("Display window", NULL, NULL);
			proj_set=true;
		}

	}
}
void size_points(Mat src2final){
   
	cout << "Start up 2" << endl;
	imshow( "Display window2", src2final );
	setMouseCallback("Display window2",on_mouse2, NULL );
	while(size_set == false)
	{
		int key=waitKey(0);
		if(key==13) break;
	}
	cout << "Break up 2" << endl;
	return;
}
void proj_points(Mat ss){
	cout << "Start up" << endl;
	imshow( "Display window", ss );
	setMouseCallback("Display window",on_mouse, NULL );
	while(proj_set == false)
	{
		int key=waitKey(0);
		if(key==13) break;
	}
	cout << "Break up" << endl;
	return;
}
Mat proj(Mat input_Frame,bool colour,Mat ss){
		vector<Point2f> left_image;                 // Stores 4 points(x,y) of the logo image. Here the four points are 4 corners of image.
		vector<Point2f> right_image;        // stores 4 points that the user clicks(mouse left click) in the main image.

		if(!proj_set){
			proj_points(ss);
		}

		left_image.push_back(points[0]);
		left_image.push_back(points[1]);
		left_image.push_back(points[2]);
		left_image.push_back(points[3]);

		//Dont Change////////////////////////////////
		right_image.push_back(Point2f(float(472),float(52)));
		right_image.push_back(Point2f(float(472),float(830)));
		right_image.push_back(Point2f(float(800),float(830)));
		right_image.push_back(Point2f(float(800),float(52)));
		////////////////////////////////////////////

		Mat H = findHomography(left_image,right_image,0 );
		Mat logoWarped;
		warpPerspective(input_Frame,logoWarped,H,Size(1280,875));
		Mat gray,src2final;
		//if(colour) cvtColor(logoWarped,gray,COLOR_BGR2GRAY); //IN COLOUR
		gray=logoWarped.clone(); //IN GRAY_SCALE
		threshold(gray,gray,0,255,THRESH_BINARY);
		logoWarped.copyTo(src2final,gray);
		if(!size_set){
			size_points(src2final);
		}
		float p1x=points2[0].x;
		float p1y=points2[0].y;
		float p2x=points2[1].x;
		float p2y=points2[1].y;
		float p3x=points2[2].x;
		float p3y=points2[2].y;
		float p4x=points2[3].x;
		float p4y=points2[3].y;

		float a1x=(p1x+p2x)/2;
		float a1y=(p1y+p4y)/2;
		float a2x=(p3x+p4x)/2;
		float a2y=(p2y+p3y)/2;

		Mat retImage = src2final(Rect(a1x,a1y,a2x-a1x,a2y-a1y));		
		//img[900:56, 389:831]
   return retImage;
}
Mat proj(Mat input_Frame,bool colour){
		// We need 4 corresponding 2D points(x,y) to calculate homography.
		vector<Point2f> left_image;                 // Stores 4 points(x,y) of the logo image. Here the four points are 4 corners of image.
		vector<Point2f> right_image;        // stores 4 points that the user clicks(mouse left click) in the main image.

		// Image containers for main and logo image
		Mat imageMain;
		Mat imageLogo=input_Frame;
		left_image.push_back(points[0]);
		left_image.push_back(points[1]);
		left_image.push_back(points[2]);
		left_image.push_back(points[3]);

		right_image.push_back(Point2f(float(472),float(52)));
		right_image.push_back(Point2f(float(472),float(830)));
		right_image.push_back(Point2f(float(800),float(830)));
		right_image.push_back(Point2f(float(800),float(52)));

		Mat H = findHomography(         left_image,right_image,0 );
		Mat logoWarped;
		// Warp the logo image to change its perspective
		warpPerspective(imageLogo,logoWarped,H,Size(1280,875));
		//showFinal(imageMain,logoWarped);

		Mat gray,gray_inv,src1final,src2final;
		if(colour) cvtColor(logoWarped,gray,COLOR_BGR2GRAY); //IN COLOUR
		else gray=logoWarped.clone(); //IN GRAY_SCALE
		threshold(gray,gray,0,255,THRESH_BINARY);
		//adaptiveThreshold(gray,gray,255,ADAPTIVE_THRESH_MEAN_C,THRESH_BINARY,5,4);
		bitwise_not ( gray, gray_inv );
				//imageMain.copyTo(src1final,gray_inv);
		logoWarped.copyTo(src2final,gray);
		Mat finalImage = src2final;
		float p1x=points2[0].x;
		float p1y=points2[0].y;
		float p2x=points2[1].x;
		float p2y=points2[1].y;
		float p3x=points2[2].x;
		float p3y=points2[2].y;
		float p4x=points2[3].x;
		float p4y=points2[3].y;
		float a1x=(p1x+p2x)/2;
		float a1y=(p1y+p4y)/2;
		float a2x=(p3x+p4x)/2;
		float a2y=(p2y+p3y)/2;
		Mat retImage = finalImage(Rect(a1x,a1y,a2x-a1x,a2y-a1y));
		//img[900:56, 389:831]
   return retImage;
   }

void find_line(Mat img,int xfit, int yfit, bool up,double ylimTop, double tempBlobRatio){ //note that img has to be grayscale
	//ylimtop is the ratio from top which is ylimit as upper box ;
	int ylimTopPix = (1.0-ylimTop)*img.rows;
	calx = img.cols/xfit ;
	caly = img.rows/yfit ;
	int rows = img.rows ;
	int cols = img.cols ;
	bool done=false ;
	int bw[rows-ylimTopPix][cols];
	for(int i=ylimTopPix;i<rows;i++)
		for(int j=0;j<cols;j++)
			bw[i-ylimTopPix][j]=0;
	// Mat img2 = img.clone() ;

	if((int)img.at<uchar>(ylimTopPix,0)>150)
		bw[0][0]= 1 ;

	for(int i=1;i<cols;i++)
		if((int)img.at<uchar>(0,i)>150)
			bw[0][i]=bw[0][i-1]+1 ;
		else 
			bw[0][i]=bw[0][i-1] ;

	for(int j=ylimTopPix+1;j<rows;j++)
		if((int)img.at<uchar>(j,0)>150)
			bw[j-ylimTopPix][0]=bw[j-1-ylimTopPix][0]+1 ;
		else 
			bw[j-ylimTopPix][0]=bw[j-1-ylimTopPix][0] ;                

	if(up){
		int y=1+ylimTopPix ; int x=1 ;
		for(;y<rows;y++){
			for(x=1;x<cols;x++){
				if((int)(img.at<uchar>(y,x))>150)
					bw[y-ylimTopPix][x]=(int)bw[y-ylimTopPix][x-1]+(int)bw[y-1-ylimTopPix][x]-(int)bw[y-1-ylimTopPix][x-1]+1;
				else
					bw[y-ylimTopPix][x]=(int)bw[y-ylimTopPix][x-1]+(int)bw[y-1-ylimTopPix][x]-(int)bw[y-1-ylimTopPix][x-1];
				
				if(y>=caly+ylimTopPix&&x>=calx && ((double)(((int)bw[y-ylimTopPix][x]+(int)bw[y-caly-ylimTopPix][x-calx]-(int)bw[y-caly-ylimTopPix][x]-(int)bw[y-ylimTopPix][x-calx])*1.0/(calx*caly))>tempBlobRatio))
					done=true ;
				if(done)
					break ;                
			}
			if(done)
				break ;
		}
		if(!done){
			X=0;Y=img.rows;}
		else{
			X=x-calx ;Y=y-caly ;}


// cv::Mat A(100,100,CV_64F);
			
			// Mat temp(rows, cols, CV_8UC3);
			// std::memcpy(temp.data, bw, cols*rows*sizeof(int));
			// imshow("bw",temp);
			// waitKey(0);
			// line(img2,Point(0,Y),Point(img2.cols,Y),Scalar(255),1) ;
			// line(img2,Point(X,0),Point(X,img2.rows),Scalar(255),1) ;
	}

	else{
		int y=1+ylimTopPix ; int x=1 ;
		for(;y<rows;y++)
			for(x=1;x<cols;x++)
				if((int)(img.at<uchar>(y,x))>150)
					bw[y-ylimTopPix][x]=(int)bw[y-ylimTopPix][x-1]+(int)bw[y-1-ylimTopPix][x]-(int)bw[y-1-ylimTopPix][x-1]+1;
				else
					bw[y-ylimTopPix][x]=(int)bw[y-ylimTopPix][x-1]+(int)bw[y-1-ylimTopPix][x]-(int)bw[y-1-ylimTopPix][x-1];

		for(y=rows-1;y>=caly+ylimTopPix;y--){
			for(x=calx;x<cols;x++){
				//cout<<"ratio:"<<(double)(((int)bw[y][x]+(int)bw[y-caly][x-calx]-(int)bw[y-caly][x]-(int)bw[y][x-calx])*1.0/(calx*caly))<<endl;
				if((double)(((int)bw[y-ylimTopPix][x]+(int)bw[y-caly-ylimTopPix][x-calx]-(int)bw[y-caly-ylimTopPix][x]-(int)bw[y-ylimTopPix][x-calx])*1.0/(calx*caly))>tempBlobRatio)
					done=true ;
				if(done)
					break ;
			}
		if(done)
			break ;
		}
		
		if(!done)
			{X=0;Y=ylimTopPix;}
		else
			{X=x-calx ;Y=y-caly ;}

			// line(img2,Point(0,Y),Point(img2.cols,Y),Scalar(255),1) ;
			// line(img2,Point(X,0),Point(X,img2.rows),Scalar(255),1) ;                      
	}
}

void find_line(Mat img,int xfit, int yfit, bool up){
	return find_line(img,xfit,yfit,up,1.00,BLOB_RATIO) ;
}

void find_line(Mat img){
	return find_line(img,XFIT,YFIT,true) ;
}

void find_line(Mat img,bool up){

		return find_line(img,XFIT,YFIT,up) ;

}

bool isRed_(Mat statMat,Mat dynMat){
	double ImgRatio = 0.4 ;
	find_line(statMat,XFIT,YFIT,false,ImgRatio,0.7) ;
	int sy=Y ;
	find_line(dynMat,XFIT,YFIT,false,ImgRatio,0.7) ;
	int dy=Y ;
	// cout<<"dy:"<<dy<<" sy:"<<sy<<" rows:"<<statMat.rows ;
	int ch1=(1.00-ImgRatio)*statMat.rows;
	bool bs=sy>ch1+2 && sy<statMat.rows-1 ;
	bool bd=dy>ch1+2 ;
	// cout<<" bs:"<<bs<<" bd:"<<bd<<endl ;
	// if(!(bs&&!bd))
		// cout<<"**********************************GREEN********************************************" ;
		
	return (bs&&!bd) ;
}

double white_area(Mat img){
	int rows = img.rows ;
	int cols = img.cols ;
	double countpos=0;
	int countall=0;
	for(int j=0;j<rows;j++){
		for(int i=0;i<cols;i++){
			// if((int)img.at<uchar>(j,i)>150)
				countpos+=((int)img.at<uchar>(j,i))/255.0;
			countall++;
		}
	}
	return countpos/countall;

}

double white_area_stop(Mat img){
	int rows = img.rows ;
	int cols = img.cols ;
	int countpos=0;
	int countall=0;
	for(int j=0;j<rows;j++){
		for(int i=0;i<cols;i++){
			// if((int)img.at<uchar>(j,i)>150)
				countpos+= (int)img.at<uchar>(j,i);
			countall++;
		}
	}
	return(double)((countpos*1.0)/(countall*1.0));

}

double white_area_dyn(Mat img){
	int rows = img.rows ;
	int cols = img.cols ;
	double countpos=0;
	int countall=0;
	for(int j=0;j<rows;j++){
		for(int i=0;i<cols;i++){
			// if(>150)
			int x=(int)img.at<uchar>(j,i);
			countpos+= (x/255.0);
			countall++;
		}
	}
	return(double)((countpos)/(countall*1.0));

}

Mat magn_norm;

int p1=0,p2=0,p3=0,p4=0;

int proj_rows, proj_cols;

void runDenseFlow(Mat frame){
    Mat frame2 = proj(frame,0);
    Mat dst,next;
    resize(frame2, frame2, Size(),resize_percent,resize_percent,INTER_CUBIC);

    if (frame2.empty())
        return;
    cvtColor(frame2, next, COLOR_BGR2GRAY);
    prvs= proj(prvs,0);
    resize(prvs, prvs, Size(),resize_percent,resize_percent,INTER_CUBIC);
    Mat flow(prvs.size(), CV_32FC2);

    // cout<<"next.size(): "<<next.size()<<" and prvs.size(): "<<prvs.size()<<"prvs.channels(): "<<prvs.channels()<<"next.channels(): "<<next.channels()<<endl;
    calcOpticalFlowFarneback(prvs, next, flow, 0.5, 3, 15, 5, 5, 1.2, 0);
    // cout<<"next.size(): "<<next.size()<<" and prvs.size(): "<<prvs.size()<<endl;
    // visualization

    Mat flow_parts[2];
    split(flow, flow_parts);
    Mat magnitude, angle;
    cartToPolar(flow_parts[0], flow_parts[1], magnitude, angle, true);
    
    normalize(magnitude, magn_norm, 0.0f, 1.0f, NORM_MINMAX);

    // magn_norm*=255.0;
    Mat dst1 = magn_norm.clone();
    magn_norm.setTo(2, magn_norm < 0.25);
    magn_norm.setTo(85,magn_norm<=0.4);
    magn_norm.setTo(255, magn_norm <=1);
    // magn_norm.setTo(255, magn_norm <=1);
    // cout<<"magn_norm: "<<endl<<(magn_norm)<<endl;
    // magn_norm.setTo(0,magn_norm==2);
    // magn_norm.setTo(125,magn_norm>=0.3 && magn_norm<0.5);
    
    // magn_norm = magn_norm*255;

}

string type2str(int type) {
  string r;

  uchar depth = type & CV_MAT_DEPTH_MASK;
  uchar chans = 1 + (type >> CV_CN_SHIFT);

  switch ( depth ) {
    case CV_8U:  r = "8U"; break;
    case CV_8S:  r = "8S"; break;
    case CV_16U: r = "16U"; break;
    case CV_16S: r = "16S"; break;
    case CV_32S: r = "32S"; break;
    case CV_32F: r = "32F"; break;
    case CV_64F: r = "64F"; break;
    default:     r = "User"; break;
  }

  r += "C";
  r += (chans+'0');

  return r;
}

Mat projDYN,projSTAT,trafficProj,blackwhiteframe,SS; 
double STATIC_FACTOR=0.0; //This is calculated as the number by which a frame will reduce to half its importance in STATIC_TIME
bool SS_ON = false ;
bool CAP_ON = true ;
int MAXFPS_LIM ;
int t1=0,t2=0,t3=0,t4=0,t5=0,t6=0,t7=0,t8=0,t9=0;
int FPUNIT = (int)FPS; //We consider a certain number of frames as new FPUNIT rather than FPS because it is more generic
char out_folder[10];

int init(VideoCapture cap){

	int argc = 6;
        const char* ccp0 = (const char*)"0";
        //const char* ccp1 = (const char*)"1";
	const char *argv[] = {ccp0, ip, ccp0, ccp0, ccp0, ccp0};
	// string results_folder=  "results", points_folder="proj_points", video_folder="videos", background_folder="backgrounds", traffic_folder="traffic_pics", file_seperator="/";
	string results_folder=  "results/", points_folder="proj_points/", video_folder="", background_folder="", traffic_folder="traffic/", file_seperator="";
	string video_format = ".mp4";

	// auto t;

	//auto startcode = chrono::high_resolution_clock::now();
	//argv[1] is video name
	//argv[2] is whether or not you have to set four values to configure the projection thing ;
	//argv[3] is whether or not GPU is on
	//argv[4] is if you want to limit the fps
	//argv[5] is whether you want data to acquire as soon as it is processed or wait for whole one second ;
	// STATIC_FACTOR = 0.9 ; //chnageeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee
	//cout<<"STATIC_FACTOR"<<STATIC_FACTOR<<endl;

	int i=0;
	double li[] = {0.998};
	//for(int i=0;i<1;i++){
	STATIC_FACTOR = li[i];
	cout<<"doing for STATIC_FACTOR: "<<STATIC_FACTOR<<endl;
	//if(argc<2) cout<<"Video name missing!"<<endl ;
	//else
	//{
	bool GPU_ON = false ;
	bool WAIT_ON = false ;
	string tempString="" ;
	string video_name = argv[1];  //e.g. a1b_day
	string points_file_location = points_folder+file_seperator+video_name+".txt";
	if(argc>2){
		tempString=argv[2] ;
		SS_ON = stoi(tempString)!=0;
		if(SS_ON)
			outfile.open(points_file_location);
		else
			infile.open(points_file_location);
	}
	else{
		infile.open(points_file_location);
	}
	if(!SS_ON){
		float fx,fy;
		for(int i=0;i<4;i++){
			infile>>fx;
			infile>>fy;
			points.push_back(Point2f(fx,fy));
		}
		for(int i=0;i<4;i++){
			infile>>fx;
			infile>>fy;
			points2.push_back(Point2f(fx,fy));
		}
	}
	if(argc>3){
		tempString=argv[3] ;
		// GPU_ON = stoi(tempString)!=0 ;
	}
	if(argc>5){
		tempString=argv[5] ;
		WAIT_ON = stoi(tempString)!=0 ;			
	}
	

	//OPen a out file to write
	string outname = string(out_folder)+"../"+results_folder+file_seperator+"results_"+video_name+".csv" ;
        output_fd = open(outname.c_str(), O_WRONLY | O_CREAT, 0644);

	//outfile.open(outname.c_str());
	#if 0
	//Video 
	string vid_name;
	if (video_name.rfind(172))
		vid_name = video_folder+file_seperator+ video_name+video_format ;
	else
		vid_name = "rtsp://admin:P5544pot@172.16.7.41:554/cam/realmonitor?channel=1&subtype=0&unicast=true&proto=Onvif";
	VideoCapture cap(vid_name) ;
	if(!cap.isOpened()){
		cout << "Error opening video stream or file" << endl;
		return -1;
	}
	#endif
	
	//Initialisation
	Vwidth = cap.get(CAP_PROP_FRAME_WIDTH) ;
	Vheight = cap.get(CAP_PROP_FRAME_HEIGHT);
	double fps = FPS;//round(cap.get(CAP_PROP_FPS)) ;

	 // namedWindow("Average",WINDOW_NORMAL) ;
	// resizeWindow("Average", Vwidth/3,Vheight/3) ;
	 // namedWindow("Original",WINDOW_NORMAL) ;
	// resizeWindow("Original", Vwidth/3,Vheight/3) ;	
	 // namedWindow("Difframe",WINDOW_NORMAL) ;
	// resizeWindow("Difframe", Vwidth/3,Vheight/3) ;	
	
	//MAXFPS_LIM is the max numebr of frames that will be used out of that FPUNIT, note that when gpu is on the MAXFPS_LIM will be on automatically because we work only in terms of perfect squares or their multiples
	MAXFPS_LIM = myFPS ;
	if(GPU_ON){
		CAP_ON=true ;
		// MAXFPS_LIM = FPUNIT ;
	}
	if(argc>4){
		CAP_ON=true;
		tempString = argv[4] ;
		if(stoi(tempString)>0&&stoi(tempString)<FPUNIT){
			MAXFPS_LIM = stoi(tempString) ;
		}
	}
	
	cout<<"Width: "<<Vwidth<<endl ;
	cout<<"Length: "<<Vheight<<endl ;
	cout<<"FPS: "<<fps<<endl ;
	cout<<"GPU_ON: "<<GPU_ON<<endl ;
	cout<<"CAP_ON: "<<CAP_ON<<endl ;
	cout<<"WAIT_ON: "<<WAIT_ON<<endl ;
	Mat frame(Size(Vwidth,Vheight),CV_8UC3,Scalar(0)),sumframe(frame.size(),CV_64FC3,Scalar(0)) ;
	Mat avgframe,ErodeDilateMat,MedianBlurMat,difframe,difframe_2,difframe_3,
	difdisplayframe,difdisplayframe_2,difframe_result,temp2frame,tempframe,
	projCOL,staticINIT;

	//INITIALISATION
	//int height = frame.rows ;
	//int width = frame.cols ;
	// imwrite("intialbkg.jpg",frame) ;
	// cout<<"done here 3"<<endl;
	cvtColor(frame, blackwhiteframe, COLOR_BGR2GRAY);
	cout<<"done here 4 @ video_name "<< video_name << endl;

	string background_file_name="bg/"+video_name+".jpg";
	staticAVG = imread(background_folder+file_seperator+background_file_name, IMREAD_UNCHANGED); ; //STATICAVG of b&w frame of original, no projection
	cvtColor(staticAVG, staticAVG, COLOR_BGR2GRAY);

	// dynAVG=blackwhiteframe.clone();		//DYNAVG of b&w frame of original, no projection
	// cout<<"done here 5"<<endl;

	//for projection selection///////////////////////////////////
	if(SS_ON){
		SS=staticAVG.clone();
		SS=proj(SS,true,SS);
	}

	string traffic_file_name = video_name+".jpg";
	trafficProj = imread(traffic_folder+file_seperator+traffic_file_name, IMREAD_UNCHANGED);
	trafficProj = proj(trafficProj,0);
	imwrite(traffic_folder+file_seperator+"proj_traffic_"+video_name+".jpg",trafficProj);


	Mat temp_f = proj(frame,0);
	proj_rows = temp_f.rows;
	proj_cols = temp_f.cols;

	cout<<"proj_cols: "<<proj_cols<<" and proj_rows: "<<proj_rows<<endl;

	cout<<"done here 1"<<endl;

	//read videos/skip videos for initial one second ;
	for(int i=0;i<FPUNIT;i++){
		if(!cap.read(frame)){
		cout<<"CANNOT READ FRAMES, TECHNICAL ERROR, PLEASE CHECK!";
		return -1 ;				
		}
		else{
			cvtColor(frame, prvs, COLOR_BGR2GRAY);

			// prvs = proj(prvs,0);
			// resize(prvs, prvs, Size(),resize_percent,resize_percent,INTER_CUBIC);
		}
	}

	// cout<<"done here 2"<<endl;
	
	



	//***********************INITIALISATION FOR GPU PART BEGINS*********************************
	//GPU_ONLY	
	//framefactor is just a number less than equal to root which can be used to send data or take average in multiple of root*root.
	int framefactor=1;
	int root=3;
	int square=root*root;
	int close = (int) FPUNIT ;

	// cout<<"done here 4"<<endl;
	
	if(GPU_ON){
		for(int r=1;r<5;r++)
			for(int f=1;f<r+1;f++)
				if(MAXFPS_LIM - r*r*f>=0&&MAXFPS_LIM - r*r*f<close){
					close = MAXFPS_LIM - r*r*f ;
					root=r ;
					square= r*r ;
					framefactor=f ;
				}
		MAXFPS_LIM = square*framefactor ;
	}
	cout<<"MAX_FPS: "<<MAXFPS_LIM<<endl ;
	cout<<"SQUARE: "<<square<<endl ;
	cout<<"FRAMEFACTOR: "<<framefactor<<endl ;
	//ALWAYS
	//tempframes are all the frames which are sent for their dynamic average to be considered
	//int loop=0;
	Mat tempframesCPUV2[MAXFPS_LIM] ;
	Mat tempframes[square*framefactor];
	// Mat tempframes_total[DYNAMIC_TIME];
	
	// for(int i=0;i<DYNAMIC_TIME;i++){
	// 	Mat local ;
	// 	(blackwhiteframe).convertTo(local,CV_64FC1) ;
	// 	tempframes_total[i]=local ;
	// }
	
	// Mat sum_tempframes;//sum of the DYNAMIC_TIME(default=6) number of average frames;
	//Mat avg_tempframes;//avg of the DYNAMIC_TIME(default=6) number of average frames;
	// for(int i=0;i<DYNAMIC_TIME;i++)
	// 	if(i==0)
	// 		sum_tempframes=tempframes_total[i]/DYNAMIC_TIME;
	// 	else
	// 		sum_tempframes+=tempframes_total[i]/DYNAMIC_TIME;
	
	// cout<<"Sumtempframes "<<sum_tempframes.size()<<" "<<sum_tempframes.channels()<<" " ;
	// sum_tempframes.convertTo(dynAVG,CV_8UC1,1.0/DYNAMIC_TIME);
	
	
	// imwrite("firstimg.jpeg",dynAVG) ;
	// char *converted_arr[framefactor];
	
	// //GPU_ONLY
	// for(int i=0;i<framefactor;i++)
	// 	converted_arr[i]=(char *)malloc(square*height*width) ;
	//***********************INITIALISATION FOR GPU PART ENDS*********************************

	return 0;
}




bool isRED=false ;
int myTimeOffset = 0;
bool bFirst = true;

int calc_density(Mat frame, chrono::system_clock::time_point mytimePt)
{
    cvtColor(frame, blackwhiteframe, COLOR_BGR2GRAY); //TO black and white
    if (bFirst) {
        prvs = blackwhiteframe.clone();
        bFirst = false;
        cout<<"Captured first prvs frame"<<endl ;
        return 0;
    }

    int ms = std::chrono::time_point_cast<std::chrono::milliseconds>(mytimePt).time_since_epoch().count() % 1000;
    if (ms == myTimeOffset) {
        runDenseFlow(frame);
    }

    prvs = blackwhiteframe.clone();

    //cout << staticAVG.size() << endl << STATIC_FACTOR << endl << blackwhiteframe.size() << endl;
    if(!isRED)
        staticAVG=STATIC_FACTOR*staticAVG+(1-STATIC_FACTOR)*blackwhiteframe;

    if (ms == myTimeOffset) {
        //cout<<"Dense Cal"<<endl ;
	resize(magn_norm, dynAVG, Size(trafficProj.cols,trafficProj.rows));

	Mat temp1 = cv::abs(blackwhiteframe-staticAVG);
	dynAVG.convertTo(projDYN, CV_8UC1);

	// cout<<"starting this 3"<<endl;
	threshold(temp1,staticAVG_dif,VARIANCE_FROM_AVG,255,THRESH_BINARY) ;

	//imwrite("dynavgdiff.jpeg",dynAVG_dif);

	// bool isRED = isRed(proj(staticAVG_dif,false,SS),proj(dynAVG_dif,false,SS));
	if(SS_ON){
		// projDYN = proj(dynAVG,0,SS);
		projSTAT = proj(staticAVG_dif,0,SS) ;
	}else{
		// projDYN = proj(dynAVG,0);
		projSTAT = proj(staticAVG_dif,0) ;
	}

	isRED = isRed_(projSTAT,projDYN);
	white_area_variable = white_area(projSTAT);
	white_area_variable1 = white_area(projDYN);
	Mat stop_dens = projSTAT - projDYN;
	stop_dens.setTo(0,stop_dens<0);

	stop_density = white_area(stop_dens);
	white_area_variable1 = (white_area_variable1>white_area_variable) ? white_area_variable : white_area_variable1;
	// stop_density = white_area_variable - white_area_variable1;
	
	find_line(projSTAT) ;
	line(projSTAT,Point(0,Y),Point(projSTAT.cols,Y),Scalar(255),1) ;
	sy=Y ;
	find_line(projDYN) ;
	dy=Y ;

	//writing part
        int sec = std::chrono::time_point_cast<std::chrono::seconds>(mytimePt).time_since_epoch().count();
	//string output_ans = to_string(sec)+","+to_string(white_area_variable)+","+to_string(stop_density);//+","+to_string(sy*1.0/projSTAT.rows);
	//outfile<<output_ans<<endl ;
	//cout<<output_ans<<endl;
        do{
            lock_guard<mutex> guard(g_mutex2);
	    outbuf_off += sprintf(outbuf+outbuf_off, "%d,%f,%f,%d\n", sec, white_area_variable, stop_density, isRED);
            //printf("%d,%f,%f,%d\n", sec, white_area_variable, stop_density, isRED);
            //outbuf[outbuf_off] = 0;
            //cout << "Write @ " << outbuf_off << " " << sizeof(outbuf) << " " << get_current_time_point();
        }while(0);
    }

    return 0;
}



bool keep = true;

#define MAX_LEN    200
//char img_folder[] = "./img";

chrono::system_clock::time_point startTimePt;

Mat frame;

//int FPS = 18;
//int read_scale = 1;// FPS/myFPS;
//int dump_ctr = dump_time*FPS;

/*
CV_CAP_PROP_POS_MSEC 0
CV_CAP_PROP_POS_FRAME 1
CV_CAP_PROP_POS_AVI_RATIO 2
CV_CAP_PROP_FRAME_WIDTH 3
CV_CAP_PROP_FRAME_HEIGHT 4
CV_CAP_PROP_FPS 5
CV_CAP_PROP_FOURCC 6
CV_CAP_PROP_FRAME_COUNT 7
*/

void reader(VideoCapture cap)
{
    auto tmoffst = std::chrono::milliseconds(1000/myFPS);
    chrono::system_clock::time_point mytimePt = startTimePt + tmoffst;
    int ctr=1;

    this_thread::sleep_until(startTimePt);
    while(keep && cap.isOpened())
    {
        cap.grab();
        if (chrono::system_clock::now() >= mytimePt) {
            do{
                lock_guard<mutex> guard(g_mutex);
                cap.retrieve(frame);
            }while(0);
            calc_density(frame, mytimePt);
            mytimePt += tmoffst;
            if (++ctr >= myFPS) {
                ctr = 0;
                mytimePt += std::chrono::milliseconds(myComp);
            }
        }
    }
    cout << "Exiting Grab" << endl;
}

#if 0
void processor(VideoCapture cap)
{
    chrono::system_clock::time_point mytimePt = startTimePt;
    auto tmoffst = std::chrono::milliseconds(1000/myFPS);

    while(keep && cap.isOpened())
    {
        mytimePt += tmoffst;
        this_thread::sleep_until(mytimePt);
	
        do{
            lock_guard<mutex> guard(g_mutex);
            cap.retrieve(frame);
        }while(0);
        calc_density(frame, mytimePt);
    }
    cout << "Exiting Process" << endl;
}
#endif

void dump(VideoCapture cap)
{
    chrono::system_clock::time_point mytimePt = startTimePt + std::chrono::milliseconds(500/myFPS);
    auto tmoffst = std::chrono::milliseconds(1000*dump_time);
    auto utc = std::chrono::minutes(5*60+30);

    long duration = std::chrono::time_point_cast<std::chrono::seconds>(mytimePt+utc).time_since_epoch().count();
    int ghr = (duration / 3600) % 24;

    char place[MAX_LEN+1];
    Mat frame2;
    for(int ctr=0 ; keep && cap.isOpened() ; ++ctr)
    {
        mytimePt += tmoffst;
        this_thread::sleep_until(mytimePt);

        long duration = std::chrono::time_point_cast<std::chrono::seconds>(mytimePt+utc).time_since_epoch().count();
        int hr = (duration / 3600) % 24;
        int min = (duration / 60) % 60;
        int sec = duration % 60;
        sprintf(place, "%s/%02d_%02d_%02d.jpg", out_folder, hr, min, sec);
#if 1
        if (dump_scale == 1){
            lock_guard<mutex> guard(g_mutex);
            imwrite(place, frame);
        }
        else {
            do{
                lock_guard<mutex> guard(g_mutex);
                resize(frame, frame2, Size(1920/dump_scale, 1080/dump_scale));
            }while(0);
            imwrite(place, frame2);
        }
#endif
        if ( (hr==22 && min>=5) /* || (rounds>0 && ctr>=rounds)*/ ) {
            keep = false;
            cout << "keep <= false" << endl;
            hr += 1;
        }
        if (ghr != hr )/*&& min/dump_time==stoi(ip))*/ {
            // Dump all density data
            //string outname = string(out_folder)+"../results/"+ip+"_"+to_string(hr)+".txt";
            //int output_fd = open(outname.c_str(), O_WRONLY | O_CREAT, 0644);

            do{
                lock_guard<mutex> guard(g_mutex2);
                ssize_t ret_out = write (output_fd, &outbuf, (ssize_t) outbuf_off);
                cout << "Writing to file " << " for " << ret_out << "/" << outbuf_off << " @ " << get_current_time_point() << endl;
                outbuf_off = 0;
                //close(output_fd);//outfile << outbuf; outfile.flush(); outfile.close();
            }while(0);
            ghr = hr;
        }
    }
    cout << "Exiting Dump" << endl;
}

void current_time_point(chrono::system_clock::time_point timePt)
{
    time_t timeStamp = chrono::system_clock::to_time_t(timePt);
    cout << std::ctime(&timeStamp) << endl;
}

void config(void)
{
    time_t tt = chrono::system_clock::to_time_t(chrono::system_clock::now());
    tm local_tm = *localtime(&tt);

    sprintf(out_folder, "out/%d/%s/", local_tm.tm_mday, ip);

    cout << "Date " << local_tm.tm_year + 1900 << ':' << local_tm.tm_mon + 1 << ':' << local_tm.tm_mday << '\n';
    cout << "out_folder :" << out_folder << endl;

}

chrono::system_clock::time_point get_wait(long waitm)
{
    cout<<"Current Time :: ";
    current_time_point(chrono::system_clock::now());
  
    chrono::system_clock::time_point timePt = chrono::system_clock::now();
    long duration = std::chrono::time_point_cast<std::chrono::milliseconds>(timePt).time_since_epoch().count();
    long int ms = duration % 1000;
    long int sec = (duration/1000) % waitm;
    //std::cout << ms << std::endl;

    timePt -= std::chrono::milliseconds(ms);
    timePt -= std::chrono::seconds(sec);
    timePt += std::chrono::seconds(waitm);
    timePt += std::chrono::milliseconds(myTimeOffset);

    return timePt;
}

void start_wait(chrono::system_clock::time_point timePt)
{
    cout<<"Current Time :: ";
    current_time_point(chrono::system_clock::now());

    long duration = std::chrono::time_point_cast<std::chrono::milliseconds>(timePt).time_since_epoch().count();
    long int ms = duration % 1000;
    long int sec = (duration/1000) % 60;
    //std::cout << ms << std::endl;

    timePt -= std::chrono::milliseconds(ms);
    timePt -= std::chrono::seconds(sec);

    cout << "Sleeping Until :: "; 
    current_time_point(timePt);

    this_thread::sleep_until(timePt);

    cout<<"Woke up...Current Time :: ";
    current_time_point(chrono::system_clock::now());
}

int main(int argc, char *argv[]){

    if (argc>1)
    {
        ip = argv[1];
        myTimeOffset = int(1000/(6*myFPS)) * (stoi(ip)-1);
        cout << "myTimeOffset for cam " << ip << " is " << myTimeOffset << endl;
    }

    char vid_name[MAX_LEN];
    sprintf(vid_name, "rtsp://admin:P5544pot@172.16.7.4%s:554/cam/realmonitor?channel=1&subtype=0&unicast=true&proto=Onvif", ip);
    VideoCapture cap(vid_name);
    if(!cap.isOpened()){
        cout << "Error opening video stream or file" << endl;
        return -1;
    }

    config();
    init(cap);
    
    //Initialisation
    double Vwidth = cap.get(CAP_PROP_FRAME_WIDTH) ;
    double Vheight = cap.get(CAP_PROP_FRAME_HEIGHT);
    double fps = round(cap.get(CAP_PROP_FPS)) ;
    cout<<"Width: "<<Vwidth<<endl ;
    cout<<"Length: "<<Vheight<<endl ;
    cout<<"FPS: "<<fps<<endl ;

    startTimePt = get_wait(60);

    thread hReader(reader, cap);
    //thread hProcess(processor, cap);
    thread hDump(dump, cap);
    cout<<"Started threads"<<endl ;

    //start_wait(startTimePt);
#if 0
    char place[100];
    int tcap=0,tresize=0,twrite=0,tset=0,tret=0;
    for (int ctr=1;;ctr+=read_scale)
    {
        auto t1 = chrono::high_resolution_clock::now();
        //cap >> frame;
        //cap.grab();
        //cout << cap.get(0) << " " << cap.get(1) << endl;
        this_thread::sleep_for(chrono::milliseconds(1000/FPS));

        auto t2 = chrono::high_resolution_clock::now();
        tcap += (chrono::duration_cast<chrono::milliseconds>(t2-t1).count());
        if (read_scale > 1)
        cap.set(0, 0.00555556*2*ctr);
        auto t3a = chrono::high_resolution_clock::now();
        tset += chrono::duration_cast<chrono::milliseconds>(t3a-t2).count();

        if (ctr%dump_ctr==0)
        {
            cap.retrieve(frame);
            auto t3 = chrono::high_resolution_clock::now();
            tret += (chrono::duration_cast<chrono::milliseconds>(t3-t3a).count());

        sprintf(place, "%s/%s/frame%d.jpg", img_folder, ip, ctr);
        resize(frame, frame2, Size(1920/dump_scale, 1080/dump_scale));
        auto t4 = chrono::high_resolution_clock::now();
        imwrite(place, frame2);
        auto t5 = chrono::high_resolution_clock::now();
        tresize += (chrono::duration_cast<chrono::milliseconds>(t4-t3).count());
        twrite += (chrono::duration_cast<chrono::milliseconds>(t5-t4).count());
        cout << ctr <<": "<< frame.rows << " " << frame.cols << " to " << frame2.rows << " " << frame2.cols << " " << tcap/(ctr/read_scale+1) << " " << tset/(ctr/read_scale+1) << " " << tret/((ctr/dump_ctr)+1)<< " " << tresize/((ctr/dump_ctr)+1) << " " << twrite/((ctr/dump_ctr)+1) << endl;
        }
        //resize(frame, frame2, Size(1920/4, 1080/4)); 
        //cout << ctr <<": "<< frame2.rows << " " << frame2.cols << endl;
        //imshow("live cam", frame2);
        //if ((waitKey(1) & 0xFF) == 'q')
        //break;
    }
#endif
    hDump.join();
    hReader.join();
    //hProcess.join();
    cap.release();
    close(output_fd);

    cout<<"TaTa :: ";
    current_time_point(chrono::system_clock::now());

    return 0 ;
}
