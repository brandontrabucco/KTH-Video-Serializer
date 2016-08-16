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

	FILE *fp;
	fp = fopen("/stash/tlab/datasets/KTH/sequences-list.txt", "r");

	trainingSequencePos.push_back(vector<int>(8, -1));

	string fileName;
	int t;
	while ((t = fscanf(fp, " %s\t\tframes\t%d-%d, %d-%d, %d-%d, %d-%d", &fileName[0],
			&trainingSequencePos[trainingVideoIndex][0], &trainingSequencePos[trainingVideoIndex][1],
			&trainingSequencePos[trainingVideoIndex][2], &trainingSequencePos[trainingVideoIndex][3],
			&trainingSequencePos[trainingVideoIndex][4], &trainingSequencePos[trainingVideoIndex][5],
			&trainingSequencePos[trainingVideoIndex][6], &trainingSequencePos[trainingVideoIndex][7])) != EOF) {
		if (trainingFiles[trainingVideoIndex].find(fileName.c_str()) != string::npos) {
			// names are the same
			cout << "Frames: " << trainingSequencePos[trainingVideoIndex][0] << "-" << trainingSequencePos[trainingVideoIndex][1] << " " <<
					 trainingSequencePos[trainingVideoIndex][2] << "-" << trainingSequencePos[trainingVideoIndex][3] << " " <<
					 trainingSequencePos[trainingVideoIndex][4] << "-" << trainingSequencePos[trainingVideoIndex][5] << " " <<
					 trainingSequencePos[trainingVideoIndex][6] << "-" << trainingSequencePos[trainingVideoIndex][7] <<endl;
			break;
		}
	}

	frameIndex = 0;
}

void VideoLoader::loadTestVideo() {
	if (testVideoIndex < 0) cout << "Out of bounds" << endl;
	capture.open(testFiles[testVideoIndex]);

	FILE *fp;
	fp = fopen("/stash/tlab/datasets/KTH/sequences-list.txt", "r");

	testSequencePos.push_back(vector<int>(8, -1));

	string fileName;
	while (fscanf(fp, " %s\t\tframes\t%d-%d, %d-%d, %d-%d, %d-%d", &fileName[0],
			&testSequencePos[testVideoIndex][0], &testSequencePos[testVideoIndex][1],
			&testSequencePos[testVideoIndex][2], &testSequencePos[testVideoIndex][3],
			&testSequencePos[testVideoIndex][4], &testSequencePos[testVideoIndex][5],
			&testSequencePos[testVideoIndex][6], &testSequencePos[testVideoIndex][7]) != EOF) {
		if (fileName.compare(testFiles[testVideoIndex]) == 0) {
			// names are the same
			break;
		}
	}

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

bool VideoLoader::isLastTrainingFrame() {
	// make logic based on sequence pos
	bool isLast = (frameIndex == (trainingSequencePos[trainingVideoIndex][1] - 1)) ||
			(frameIndex == (trainingSequencePos[trainingVideoIndex][3] - 1)) ||
			(frameIndex == (trainingSequencePos[trainingVideoIndex][5] - 1)) ||
			(frameIndex == (trainingSequencePos[trainingVideoIndex][7] - 1));
	return isLast;
}

bool VideoLoader::isLastTestFrame() {
	// make logic based on sequence pos
	bool isLast = (frameIndex == (testSequencePos[testVideoIndex][1] - 1)) ||
			(frameIndex == (testSequencePos[testVideoIndex][3] - 1)) ||
			(frameIndex == (testSequencePos[testVideoIndex][5] - 1)) ||
			(frameIndex == (testSequencePos[testVideoIndex][7] - 1));
	return isLast;
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

struct RandomEngine {
	const int r = rand();
	int operator()(int i) {
		return (r % i);
	}
};

void VideoLoader::shuffle() {
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
		for (int i = 0; i < trainingLabels.size(); i++)trainingLabelsFile << ((i == 0)?"":" ") << trainingLabels[i] << " " << trainingLabels[i] << " " << trainingLabels[i] << " " << trainingLabels[i];
		while (nextTrainingVideo()) {
			while (nextTrainingFrame()) {
				DatasetExample data = getTrainingData();
				for (int i = 0; i < data.frame.size(); i++)trainingDatasetFile << data.frame[i] << " ";
				if (isLastTrainingFrame()) trainingDatasetFile << videoEnd << " ";
				else trainingDatasetFile << frameEnd << " ";
			}
		}

		for (int i = 0; i < testLabels.size(); i++)testLabelsFile << ((i == 0)?"":" ") << testLabels[i] << " " << testLabels[i] << " " << testLabels[i] << " " << testLabels[i];
		while (nextTestVideo() &&  !(testVideoIndex % 10)) {
			while (nextTestFrame()) {
				DatasetExample data = getTestData();
				for (int i = 0; i < data.frame.size(); i++)testDatasetFile << data.frame[i] << " ";
				if (isLastTestFrame()) testDatasetFile << videoEnd << " ";
				else testDatasetFile << frameEnd << " ";
			}
		}
	}
}
