/*
 * VideoLoader.h
 *
 *  Created on: Jul 25, 2016
 *      Author: trabucco
 */

#ifndef VIDEOLOADER_H_
#define VIDEOLOADER_H_

#include "DatasetAdapter.h"
#include <opencv2/opencv.hpp>
#include <sys/time.h>
#include <cstdlib>
#include <dirent.h>
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <math.h>
#include <fstream>
using namespace std;
using namespace cv;

class VideoLoader {
private:
	int trainingVideoIndex;
	int testVideoIndex;
	int frameIndex;
	const int frameEnd = -1;
	const int videoEnd = -2;
	const double range = 255;
	const double rWeight = 0.2126;
	const double gWeight = 0.7152;
	const double bWeight = 0.0722;
	const string walkingPath = "walking/";
	const string runningPath = "running/";
	const string joggingPath = "jogging/";
	const string handclappingPath = "handclapping/";
	const string handwavingPath = "handwaving/";
	const string boxingPath = "boxing/";
	string path;
	Mat frame;
	VideoCapture capture;
	vector<string> trainingFiles, testFiles;
	vector<int> trainingLabels, testLabels;
	void getAllFiles();
	void readDirectory(string path, int label);
	void shuffle();
	void loadTrainingVideo();
	void loadTestVideo();
	void closeVideo();
	bool nextTrainingVideo();
	bool nextTestVideo();
	bool nextTrainingFrame();
	bool nextTestFrame();
	bool isLastFrame();
	DatasetExample getTrainingData();
	DatasetExample getTestData();
	vector<double> convertFrame(Mat image);
public:
	VideoLoader(string _p);
	virtual ~VideoLoader();
	void serialize();
	int getFrameCount();
	int getTrainingVideoCount();
	int getTestVideoCount();
	int getPixelCount();
};

#endif /* VIDEOLOADER_H_ */
