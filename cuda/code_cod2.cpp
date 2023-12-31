#include <bits/stdc++.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include </home/nvidia/users/mini/CudaFunc.h>
 
//PLEASE NOTE: We can run this code without GPU, just comment Line 6,728
//Helpful command: `pkg-config --cflags --libs opencv`
using namespace cv;
using namespace std;

//usr/local/include/opencv4/opencv2/
//int ix=30,fx=200 ;
int caly ;
int calx ;
int X,Y ;
int sx,sy,dx,dy ;

//******************************************************_______________________________________________________________________________________*******************
////PARAMETERS THAT CAN BE ALTERED FOR BETTER ACUURACY : XFIT,YFIT,DYNAMIC_TIME,STATIC_TIME,VARIANCE_FROM_AVG and inside isRED_ arguments of find_line************
//******************************************************_______________________________________________________________________________________*******************

// MUST SET BELOW PARAMETERS FIRST

double BLOB_RATIO=0.3;
int XFIT=7 ; 				//Number of vehicles that would fit in horizontal, now lets us suppose we can fit 3 cars so we may want to have XFIT atleast 3 but since we can also have bikes we can make it 4 or 5 or 6 
int YFIT=7 ; 				//Similar explanation but since bikes and cars have similar length, this is not a problem
int DYNAMIC_TIME=10 ;   	//Time within which a frame would completely disappear from average, we may want to keep it small so that slow vehicles or stopped vehicles disappear quickly in seconds or FRAMEUNITS
int STATIC_TIME=30 ; 		//in seconds or FRAMEUNITS
int VARIANCE_FROM_AVG =35 ; //variance from avg pixel, the variance from average pixel when it is considered an anomaly, be careful, should be between 10 to 40
int VAR =125 ; 				//variance from avg pixel, useless for now

vector<Point2f> points;  
vector<Point2f> points2;
bool proj_set=false;
bool size_set=false;

Mat staticAVG,dynAVG ;
Mat staticAVG_dif,dynAVG_dif ;
double white_area_variable=0.0;
ifstream infile;
ofstream outfile;

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

			size_set=true;

			// Deactivate callback

			setMouseCallback("Display window2", NULL, NULL);

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

			proj_set=true;

			// Deactivate callback

			setMouseCallback("Display window", NULL, NULL);

		}


	}

}

void size_points(Mat src2final){

   

	imshow( "Display window2", src2final );

	setMouseCallback("Display window2",on_mouse2, NULL );

	while(1)

	{

		int key=cvWaitKey(0);

		if(key==27) break;

	}

	return;

}

void proj_points(Mat ss){

	imshow( "Display window", ss );

	setMouseCallback("Display window",on_mouse, NULL );

	while(1)

	{

		int key=cvWaitKey(0);

		if(key==27) break;

	}

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

		//if(colour) cvtColor(logoWarped,gray,CV_BGR2GRAY); //IN COLOUR

		gray=logoWarped.clone(); //IN GRAY_SCALE

		threshold(gray,gray,0,255,CV_THRESH_BINARY);

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

		if(colour) cvtColor(logoWarped,gray,CV_BGR2GRAY); //IN COLOUR

		else gray=logoWarped.clone(); //IN GRAY_SCALE

		threshold(gray,gray,0,255,CV_THRESH_BINARY);

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
	cout<<"dy:"<<dy<<" sy:"<<sy<<" rows:"<<statMat.rows ;
	int ch1=(1.00-ImgRatio)*statMat.rows;
	bool bs=sy>ch1+2 && sy<statMat.rows-1 ;
	bool bd=dy>ch1+2 ;
	cout<<" bs:"<<bs<<" bd:"<<bd<<endl ;
	if(!(bs&&!bd))
		cout<<"**********************************GREEN********************************************" ;
		
	return (bs&&!bd) ;
}

void white_area(Mat img){
	int rows = img.rows ;
	int cols = img.cols ;
	int countpos=0;
	int countall=0;
	for(int j=0;j<rows;j++){
		for(int i=0;i<cols;i++){
			if((int)img.at<uchar>(j,i)>150)
				countpos++;
			countall++;
		}
	}
	white_area_variable=(double)((countpos*1.0)/(countall*1.0));

}

int main(int argc, char *argv[]){
	auto startcode = chrono::high_resolution_clock::now();
	
	//argv[1] is video name
	//argv[2] is whether or not you have to set four values to configure the projection thing ;
	//argv[3] is whether or not GPU is on
	//argv[4] is if you want to limit the fps
	//argv[5] is whether you want data to acquire as soon as it is processed or wait for whole one second ;
	
	if(argc<2) cout<<"Video name missing!"<<endl ;
	else{
		bool GPU_ON = false ;
		bool SS_ON = false ;
		bool WAIT_ON = false ;
		bool CAP_ON = false ;
		int MAXFPS_LIM ;
		string tempString="" ;
		string io=argv[1];
		io+=".txt";
		if(argc>2){
			tempString=argv[2] ;
			SS_ON = stoi(tempString)!=0;
			if(SS_ON)
				outfile.open(io);
			else
				infile.open(io);
		}
		else{
			infile.open(io);
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
			GPU_ON = stoi(tempString)!=0 ;
		}
		if(argc>5){
			tempString=argv[5] ;
			WAIT_ON = stoi(tempString)!=0 ;			
		}
		
		//OPen a out file to write
		ofstream outfile;
		string inname = argv[1] ;
		string outname = "results_"+inname+"_.txt" ;
		outfile.open(outname.c_str());
		//Video 
		string vid_name = argv[1] ;
		VideoCapture Video_cap(vid_name) ;
		if(!Video_cap.isOpened()){
			cout << "Error opening video stream or file" << endl;
			return -1;
		}
		
		//Initialisation
		double Vwidth = Video_cap.get(CAP_PROP_FRAME_WIDTH) ;
		double Vheight = Video_cap.get(CAP_PROP_FRAME_HEIGHT);
		double fps = round(Video_cap.get(CAP_PROP_FPS)) ;
		int FPUNIT = (int) fps ;				//We consider a certain number of frames as new FPUNIT rather than FPS because it is more generic
		 // namedWindow("Average",WINDOW_NORMAL) ;
		// resizeWindow("Average", Vwidth/3,Vheight/3) ;
		 // namedWindow("Original",WINDOW_NORMAL) ;
		// resizeWindow("Original", Vwidth/3,Vheight/3) ;	
		 // namedWindow("Difframe",WINDOW_NORMAL) ;
		// resizeWindow("Difframe", Vwidth/3,Vheight/3) ;	
		
		//MAXFPS_LIM is the max numebr of frames that will be used out of that FPUNIT, note that when gpu is on the MAXFPS_LIM will be on automatically because we work only in terms of perfect squares or their multiples
		MAXFPS_LIM = FPUNIT ;
		if(GPU_ON){
			CAP_ON=true ;
			MAXFPS_LIM = FPUNIT ;
		}
		if(argc>4){
			tempString = argv[4] ;
			if(stoi(tempString)>0&&stoi(tempString)<FPUNIT){
				CAP_ON=true ;
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
		difdisplayframe,difdisplayframe_2,difframe_result,temp2frame,blackwhiteframe,tempframe,
		projDYN,projSTAT,projCOL,staticINIT,SS ;

		//read videos/skip videos for initial one second ;
		for(int i=0;i<FPUNIT;i++){
			if(!Video_cap.read(frame)){
			cout<<"CANNOT READ FRAMES, TECHNICAL ERROR, PLEASE CHECK!";
			return -1 ;				
			}
		}
		
		//INITIALISATION
		int height = frame.rows ;
		int width = frame.cols ;
		imwrite("intialbkg.jpg",frame) ;
		cvtColor(frame, blackwhiteframe, CV_BGR2GRAY);
		staticAVG = blackwhiteframe.clone() ; //STATICAVG of b&w frame of original, no projection
		dynAVG=blackwhiteframe.clone();		//DYNAVG of b&w frame of original, no projection

		double STATIC_FACTOR=0.0 ;			//This is calculated as the number by which a frame will reduce to half its importance in STATIC_TIME
		STATIC_FACTOR = pow(2,-1/(MAXFPS_LIM*STATIC_TIME)) ; //chnageeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee
		cout<<"STATIC_FACTOR"<<STATIC_FACTOR<<endl ;
		//for projection selection///////////////////////////////////
		if(SS_ON){
			SS=blackwhiteframe.clone();
			SS=proj(SS,true,SS);
		}

		//***********************INITIALISATION FOR GPU PART BEGINS*********************************
		//GPU_ONLY	
		//framefactor is just a number less than equal to root which can be used to send data or take average in multiple of root*root.
		int framefactor=1;
		int root=3;
		int square=root*root;
		int close = (int) FPUNIT ;
		
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
		int loop=0;
		Mat tempframesCPUV2[MAXFPS_LIM] ;
		Mat tempframes[square*framefactor];
		Mat tempframes_total[DYNAMIC_TIME];
		
		for(int i=0;i<DYNAMIC_TIME;i++){
			Mat local ;
			(blackwhiteframe).convertTo(local,CV_64FC1) ;
			tempframes_total[i]=local ;
		}
		
		Mat sum_tempframes;//sum of the DYNAMIC_TIME(default=6) number of average frames;
		//Mat avg_tempframes;//avg of the DYNAMIC_TIME(default=6) number of average frames;
		for(int i=0;i<DYNAMIC_TIME;i++)
			if(i==0)
				sum_tempframes=tempframes_total[i]/DYNAMIC_TIME;
			else
				sum_tempframes+=tempframes_total[i]/DYNAMIC_TIME;
		
		cout<<"Sumtempframes "<<sum_tempframes.size()<<" "<<sum_tempframes.channels()<<" " ;
		sum_tempframes.convertTo(dynAVG,CV_8UC1,1.0/DYNAMIC_TIME);
		
		
		imwrite("firstimg.jpeg",dynAVG) ;
		char *converted_arr[framefactor];
		
		//GPU_ONLY
		for(int i=0;i<framefactor;i++)
			converted_arr[i]=(char *)malloc(square*height*width) ;
		//***********************INITIALISATION FOR GPU PART ENDS*********************************

		
		int loop_counter=0;
		bool isRED=false ;		
		bool isEND= false ;
		double dropratio = FPUNIT*1.0/MAXFPS_LIM ;
		
		while(!isEND){
			auto startloop = chrono::high_resolution_clock::now() ;
			int counter=0;			//counter goes upto fps
			int success_counter=1 ; //successcounter goes upto MAXFPS_LIM
			while(counter<FPUNIT){
				bool success = Video_cap.read(frame) ;
				if(!success) {
					isEND=true ;
					cout<<"Cannot read frame anymore"<<endl;
					break ;
				}
				counter++ ;
				
				if(CAP_ON&&counter!=floor(success_counter*dropratio))
					continue ;
				cvtColor(frame, blackwhiteframe, CV_BGR2GRAY); //TO black and white	
				if(GPU_ON)
					tempframes[success_counter-1]=blackwhiteframe.clone();
				else
					tempframesCPUV2[success_counter-1]=blackwhiteframe.clone();
				success_counter++;
				
				if(success_counter>MAXFPS_LIM+1)
					cout<<"ERERERERERERERERERERERERERERERRERERERERERERERERERERERERERERERERERERERERERERERERERERERER ....... CONTACT THE ADMIN! "<<endl ;
				
				if(!isRED) {
					staticAVG=STATIC_FACTOR*staticAVG+(1-STATIC_FACTOR)*blackwhiteframe;
					cout<<"------------------static_changes---------------------------"<<endl;
				}				
			}
			cout<<"SUCCESSCOUNTER = "<<success_counter<<endl ;
				
			cout<<"Loop going "<<endl ;
			char sum_arr[height*width] ;
			char sum_array[height][width];			
			
			//We want to receive sum_array from this part, Either GPU or CPU
			if(GPU_ON){			
				for(int f=0;f<framefactor;f++)
					for(int r=0;r<square;r++)
						for(int y=0;y<height;y++)
							for(int x=0;x<width;x++)
								converted_arr[f][r*height*width+y*width+x]=((char)(tempframes[f*square+r].at<uchar>(y,x)));
							
				auto st = chrono::high_resolution_clock::now() ;
				cout<<"Gpu function called . . ." ;
				AvgCalGpu(height,width,3,framefactor,converted_arr,sum_arr) ;	
				cout<<" . . . Gpu function has returned"<<endl ;
				cout<<"Time taken by gpu function "<<chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now()-st).count()/1000.0<<endl;
				
				for(int i=0;i<height;i++)
					for(int j=0;j<width;j++)
						sum_array[i][j]=sum_arr[(i*width)+j];
			}
			else{
				int tempsum=0 ;
				for(int y=0;y<height;y++)
					for(int x=0;x<width;x++){
						tempsum=0;
						
						for(int i=0;i<MAXFPS_LIM;i++)
								tempsum+=((int)(tempframesCPUV2[i].at<uchar>(y,x))) ;
						
						tempsum/=MAXFPS_LIM ;
						sum_array[y][x] = (char)tempsum ;
					}
			}
			
			Mat A(height,width,CV_8UC1,sum_array);
			std::memcpy(A.data,sum_array,height*width*sizeof(char));
			//imwrite("Avgphoto.jpeg",A) ;
			Mat Aframe ;
			A.convertTo(Aframe,CV_64FC1) ;
			Mat Bframe=Aframe.clone();

			sum_tempframes-=tempframes_total[loop_counter%DYNAMIC_TIME]/DYNAMIC_TIME;
			tempframes_total[loop_counter%DYNAMIC_TIME]=Bframe;
			sum_tempframes+=Bframe/DYNAMIC_TIME;
			//imwrite("sumframes.jpeg",sum_tempframes);
			sum_tempframes.convertTo(dynAVG,CV_8UC1);	
			//imwrite("avg.jpeg",dynAVG) ;
	
			///DYNAVG CODE OVER.... dynAVG is not correct
	
			threshold(cv::abs(blackwhiteframe-staticAVG),staticAVG_dif,VARIANCE_FROM_AVG,255,THRESH_BINARY) ;
			threshold(cv::abs(blackwhiteframe-dynAVG),dynAVG_dif,VARIANCE_FROM_AVG,255,THRESH_BINARY) ;
			//imwrite("dynavgdiff.jpeg",dynAVG_dif);

			// bool isRED = isRed(proj(staticAVG_dif,false,SS),proj(dynAVG_dif,false,SS));
			if(SS_ON){
				projDYN = proj(dynAVG_dif,0,SS) ;
				projSTAT = proj(staticAVG_dif,0,SS) ;
			}else{
				projDYN = proj(dynAVG_dif,0) ;
				projSTAT = proj(staticAVG_dif,0) ;
			}
			isRED = isRed_(projSTAT,projDYN);
			white_area(projSTAT);
			
			find_line(projSTAT) ;
			line(projSTAT,Point(0,Y),Point(projSTAT.cols,Y),Scalar(255),1) ;
			sy=Y ;
			find_line(projDYN) ;
			dy=Y ;
			
			//imshow("Original",blackwhiteframe) ;
			//imshow("Average",dynAVG) ;
			//imshow("Difframe",dynAVG_dif) ;
			
			//****************************************   U N C O M M E N T   T H E   F O L L O W I N G   F O R    D E  B U G G I N G   *********************************************
			// imshow("Original_Proj",proj(frame,1)) ;
			// imshow("DYNAMIC",projDYN) ;
			// imshow("STATIC",projSTAT) ;  // ANSWER FRAME
			
			cout<<"Static_Y: "<<sy<<","<<"Dynamic_Y: "<<dy<<endl ;
			
			//writing part
			string output_ans = to_string(loop)+":"+to_string(white_area_variable)+","+to_string(sy*1.0/projSTAT.rows) ;
			outfile<<output_ans<<endl ;
			cout<<loop++<<endl;
			
			while(WAIT_ON){
				if(chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now()-startloop).count()>998)
					break ;
			}
			
			if(waitKey(1)==27) break ;
			cout<<"total time taken: "<<chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now()-startloop).count()/1000.0<<endl; 
			loop_counter++;	
		}

		//imwrite("Size_det.jpg",frame) ;
		cout<<"I am out of the loop,Press Esc to exit!"<<endl ;
		cout<<"Total time to run code:"<<chrono::duration_cast<chrono::seconds>(chrono::high_resolution_clock::now()-startcode).count()<<endl; 

		if(waitKey(10000)==27) {
			infile.close();
			outfile.close();
			return 0 ;
		}
    }
	return 0 ;
}
