/*
 * VideoLoader.cpp
 *
 *  Created on: Jul 25, 2016
 *      Author: trabucco
 */

#include "VideoLoader.h"

VideoLoader::VideoLoader(string _p) {
	// TODO Auto-generated constructor stub
	path = _p;
	srand(time(0));
	getAllFiles();
	shuffle();
	trainingVideoIndex = -1;
	testVideoIndex = -1;
	frameIndex = 0;
}

VideoLoader::~VideoLoader() {
	// TODO Auto-generated destructor stub
	capture.release();
}

void VideoLoader::loadTrainingVideo() {
	if (trainingVideoIndex < 0) cout << "Out of bounds" << endl;
	capture.open(trainingFiles[trainingVideoIndex]);
	frameIndex = 0;
}

void VideoLoader::loadTestVideo() {
	if (testVideoIndex < 0) cout << "Out of bounds" << endl;
	capture.open(testFiles[testVideoIndex]);
	frameIndex = 0;
}

void VideoLoader::closeVideo() {
	capture.release();
}

bool VideoLoader::nextTrainingVideo() {
	closeVideo();
	if (++trainingVideoIndex < getTrainingVideoCount()) {
		loadTrainingVideo();
		return true;
	} else return false;
}


bool VideoLoader::nextTestVideo() {
	closeVideo();
	if (++testVideoIndex < getTestVideoCount()) {
		loadTestVideo();
		return true;
	} else return false;
}

bool VideoLoader::nextTrainingFrame() {
	if (frameIndex < getFrameCount()) {
		capture >> frame;
		int attempts = 0;
		while ((frame.cols == 0 || frame.rows == 0)) {
			if (attempts > 4) return false;
			cout << "Image empty, retrying frame " << frameIndex << " read on " << trainingVideoIndex << endl;
			capture.release();
			waitKey(2000);
			capture.open(trainingFiles[trainingVideoIndex]);
			cout << "Opening: " << trainingFiles[trainingVideoIndex] << endl;
			for (int i = 0; i < frameIndex; i++)
				capture >> frame;
			capture >> frame;
			attempts++;
		}
		frameIndex++;
		return true;
	} else return false;
}

bool VideoLoader::nextTestFrame() {
	if (frameIndex < getFrameCount()) {
		capture >> frame;
		int attempts = 0;
		while ((frame.cols == 0 || frame.rows == 0)) {
			if (attempts > 4) return false;
			cout << "Image empty, retrying frame " << frameIndex << " read on " << testVideoIndex << endl;
			capture.release();
			waitKey(2000);
			capture.open(testFiles[testVideoIndex]);
			cout << "Opening: " << testFiles[testVideoIndex] << endl;
			for (int i = 0; i < frameIndex; i++)
				capture >> frame;
			capture >> frame;
			attempts++;
		}
		frameIndex++;
		return true;
	} else return false;
}

bool VideoLoader::isLastFrame() {
	return (frameIndex == getFrameCount());
}

DatasetExample VideoLoader::getTrainingData() {
	DatasetExample data;
	data.frame = convertFrame(frame);
	data.label = trainingLabels[trainingVideoIndex];
	return data;
}

DatasetExample VideoLoader::getTestData() {
	DatasetExample data;
	data.frame = convertFrame(frame);
	data.label = testLabels[testVideoIndex];
	return data;
}

vector<double> VideoLoader::convertFrame(Mat image) {
	if (image.cols == 0 || image.rows == 0) cout << "Image is empty" << endl;
	else {
		Mat scaled;
		resize(image, scaled, Size(20, 15), 0, 0, INTER_CUBIC);
		vector<double> buffer;
		for (int i = 0; i < scaled.rows; i++) {
			for (int j = 0; j < scaled.cols; j++) {
				double filter = floor((bWeight * scaled.at<Vec3b>(i, j)[0]) +
						(gWeight * scaled.at<Vec3b>(i, j)[1]) +
						(rWeight * scaled.at<Vec3b>(i, j)[2]));
				buffer.push_back(filter);
			}
		} return buffer;
	}
}

int VideoLoader::getFrameCount() {
	return capture.get(CV_CAP_PROP_FRAME_COUNT);
}

int VideoLoader::getTrainingVideoCount() {
	return trainingFiles.size();
}

int VideoLoader::getTestVideoCount() {
	return testFiles.size();
}

int VideoLoader::getPixelCount() {
	return frame.cols * frame.rows;
}

void VideoLoader::getAllFiles() {
    readDirectory((path + walkingPath), 0);
    readDirectory((path + runningPath), 1);
    readDirectory((path + joggingPath), 2);
    readDirectory((path + handwavingPath), 3);
    readDirectory((path + handclappingPath), 4);
    readDirectory((path + boxingPath), 5);
}

void VideoLoader::readDirectory(string path, int label) {
	DIR *dpdf;
	struct dirent *epdf;

	dpdf = opendir(path.c_str());
	if (dpdf != NULL){
		int i = 0;
		while ((epdf = readdir(dpdf))){
			if (string(epdf->d_name).size() > 5) {
				if (i > 5) {
					trainingFiles.push_back(path + string(epdf->d_name));
					trainingLabels.push_back(label);
				} else {
					testFiles.push_back(path + string(epdf->d_name));
					testLabels.push_back(label);
				}
				i++;
			}
		}
	}
	closedir(dpdf);
}

void VideoLoader::shuffle() {
	struct RandomEngine {
		int operator()(int i) {
			return (rand() % i);
		}
	};

	RandomEngine e;

	random_shuffle(testFiles.begin(), testFiles.end(), e);
	random_shuffle(testLabels.begin(), testLabels.end(), e);
	random_shuffle(trainingFiles.begin(), trainingFiles.end(), e);
	random_shuffle(trainingLabels.begin(), trainingLabels.end(), e);
}

void VideoLoader::serialize() {
	ofstream trainingDatasetFile("/stash/tlab/datasets/KTH/binary/training_dataset.txt");
	ofstream testDatasetFile("/stash/tlab/datasets/KTH/binary/test_dataset.txt");
	ofstream trainingLabelsFile("/stash/tlab/datasets/KTH/binary/training_labels.txt");
	ofstream testLabelsFile("/stash/tlab/datasets/KTH/binary/test_labels.txt");

	cout << "Training size: " << trainingLabels.size() << endl;

	if (trainingDatasetFile.is_open() &&
				testDatasetFile.is_open() &&
				trainingLabelsFile.is_open() &&
				testLabelsFile.is_open()) {
		for (int i = 0; i < trainingLabels.size(); i++)trainingLabelsFile << ((i == 0)?"":" ") << trainingLabels[i];
		while (nextTrainingVideo()) {
			while (nextTrainingFrame()) {
				DatasetExample data = getTrainingData();
				if (isLastFrame()) cout << "Label: " << data.label << endl;
				for (int i = 0; i < data.frame.size(); i++)trainingDatasetFile << data.frame[i] << " ";
				if (isLastFrame()) trainingDatasetFile << videoEnd << " ";
				else trainingDatasetFile << frameEnd << " ";
			}
		}

		for (int i = 0; i < testLabels.size(); i++)testLabelsFile << ((i == 0)?"":" ") << testLabels[i];
		while (nextTestVideo() &&  !(testVideoIndex % 10)) {
			while (nextTestFrame()) {
				DatasetExample data = getTestData();
				for (int i = 0; i < data.frame.size(); i++)testDatasetFile << data.frame[i] << " ";
				if (isLastFrame()) testDatasetFile << videoEnd << " ";
				else testDatasetFile << frameEnd << " ";
			}
		}
	}
}
