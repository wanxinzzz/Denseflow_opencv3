#include <iostream>
#include <fstream>
#include "opencv2/core.hpp"
#include <opencv2/core/utility.hpp>
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/cudaoptflow.hpp"
#include "opencv2/cudaarithm.hpp"


using namespace std;
using namespace cv;
using namespace cv::cuda;

static void convertFlowToImage(const Mat &flow_x, const Mat &flow_y, Mat &img_x, Mat &img_y,
       double lowerBound, double higherBound) {
	#define CAST(v, L, H) ((v) > (H) ? 255 : (v) < (L) ? 0 : cvRound(255*((v) - (L))/((H)-(L))))
	for (int i = 0; i < flow_x.rows; ++i) {
		for (int j = 0; j < flow_y.cols; ++j) {
			float x = flow_x.at<float>(i,j);
			float y = flow_y.at<float>(i,j);
			img_x.at<uchar>(i,j) = CAST(x, lowerBound, higherBound);
			img_y.at<uchar>(i,j) = CAST(y, lowerBound, higherBound);
		}
	}
	#undef CAST
}

int main(int argc, const char* argv[])
{
    const char* keys =
    {
        "{ f  | ex.avi  | filename of video }"
        "{ x  | img/x   | filename prefix of flow x component }"
        "{ y  | img/y   | filename prefix of flow x component }"
        "{ i  | img/i   | filename prefix of image }"
        "{ b  | 20      | specify the maximum (px) of optical flow }"
        "{ t  | 1       | specify the optical flow algorithm }"
        "{ d  | 0       | specify gpu id }"
        "{ s  | 1       | specify the step for frame sampling }"
        "{ h  | 0       | specify the height of saved flows, 0: keep original height }"
        "{ w  | 0       | specify the width of saved flows,  0: keep original width }"    
    };

	CommandLineParser cmd(argc, argv, keys);
	string vidFile = cmd.get<string>("f");
	string xFlowFile = cmd.get<string>("x");
	string yFlowFile = cmd.get<string>("y");
	string imgFile = cmd.get<string>("i");
	int bound = cmd.get<int>("b");
    int type  = cmd.get<int>("t");
    int device_id = cmd.get<int>("d");
    int step = cmd.get<int>("s");
    int height = cmd.get<int>("h");
    int width = cmd.get<int>("w");

    VideoCapture capture(vidFile);
	if(!capture.isOpened()) {
		printf("Could not initialize capturing..\n");
		return -1;
	}
    long total_frame = capture.get(CV_CAP_PROP_FRAME_COUNT);
    cout << "Processing video :" << vidFile << endl << "Total Frames :" << total_frame << endl;
    int frame_num = 0;
    Mat image, prev_image, prev_grey, grey, frame;

    setDevice(device_id);
    Ptr<cuda::BroxOpticalFlow> brox = cuda::BroxOpticalFlow::create(0.197f, 50.0f, 0.8f, 10, 77, 10);
    Ptr<cuda::DensePyrLKOpticalFlow> lk = cuda::DensePyrLKOpticalFlow::create(Size(7, 7));
    Ptr<cuda::FarnebackOpticalFlow> farn = cuda::FarnebackOpticalFlow::create();
    Ptr<cuda::OpticalFlowDual_TVL1> tvl1 = cuda::OpticalFlowDual_TVL1::create();


	while(true) {
		capture >> frame;
		if(frame.empty())
			break;
		if(frame_num == 0) {
            image.create(frame.size(), CV_8UC3);
			grey.create(frame.size(), CV_8UC1);
			prev_image.create(frame.size(), CV_8UC3);
			prev_grey.create(frame.size(), CV_8UC1);

			frame.copyTo(prev_image);
			cvtColor(prev_image, prev_grey, CV_BGR2GRAY);

			frame_num++;
			int step_t = step;
			while (step_t > 1){
				capture >> frame;
				step_t--;
			}
			continue;
		}
        frame.copyTo(image);
        cvtColor(image, grey, CV_BGR2GRAY);

        GpuMat d_frame0(prev_grey);
        GpuMat d_frame1(grey);
        GpuMat d_flow(image.size(), CV_32FC2);

        const int64 start = getTickCount();

        // GPU optical flow
		switch(type){
		case 0:
			farn->calc(d_frame0, d_frame1, d_flow);
			break;
		case 1:
			tvl1->calc(d_frame0, d_frame1, d_flow);
			break;
		case 2:
			{
				GpuMat d_frame0f;
				GpuMat d_frame1f;
				d_frame0.convertTo(d_frame0f, CV_32F, 1.0 / 255.0);
				d_frame1.convertTo(d_frame1f, CV_32F, 1.0 / 255.0);
				brox->calc(d_frame0f, d_frame1f, d_flow);
			}
			break;
        case 3:
            lk->calc(d_frame0, d_frame1, d_flow);
            break;
		}

        const double timeSec = (getTickCount() - start) / getTickFrequency();
        cout << "Get optical flow : " << timeSec << " sec" << endl;


		// Output optical flow
        GpuMat planes[2];
        cuda::split(d_flow, planes);
        Mat flow_x(planes[0]);
        Mat flow_y(planes[1]);

		Mat imgX(flow_x.size(),CV_8UC1);
		Mat imgY(flow_y.size(),CV_8UC1);
		convertFlowToImage(flow_x,flow_y, imgX, imgY, -bound, bound);
		char tmp[20];
		sprintf(tmp,"_%06d.jpg",int(frame_num));

		Mat imgX_, imgY_, image_;
		height = (height > 0)? height : imgX.rows;
		width  = (width  > 0)? width  : imgX.cols;
		cv::resize(imgX,imgX_,cv::Size(width,height));
		cv::resize(imgY,imgY_,cv::Size(width,height));
		cv::resize(image,image_,cv::Size(width,height));

		imwrite(xFlowFile + tmp,imgX_);
		imwrite(yFlowFile + tmp,imgY_);
		imwrite(imgFile + tmp, image_);

		image.copyTo(prev_image);
		grey.copyTo(prev_grey);
		frame_num = frame_num + 1;

		int step_t = step;
		while (step_t > 1){
			capture >> frame;
			step_t--;
		}
	}

    return 0;
}
