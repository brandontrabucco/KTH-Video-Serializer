/**
 *
 * An application to load video frames from the KTH dataset
 * Author: Brandon Trabucco
 * Date: 2016/07/25
 *
 */

#include "VideoLoader.h"
#include "DatasetAdapter.h"
#include <sys/time.h>
#include <stdlib.h>
#include <fstream>
#include <string>
#include <sstream>

long long getMSec() {
	struct timeval tp;
	gettimeofday(&tp, NULL);
	return tp.tv_sec * 1000 + tp.tv_usec / 1000;
}

int main(int argc, char **argv) {
	long long startTime = 0;
	long long endTime = 0;

	startTime = getMSec();
	VideoLoader loader = VideoLoader("/stash/tlab/datasets/KTH/");
	loader.serialize();
	endTime = getMSec();

	cout << "Videos serialized in " << (endTime - startTime) << "msecs" << endl;

	startTime = getMSec();
	DatasetAdapter adapter = DatasetAdapter();
	endTime = getMSec();

	cout << "Videos deserialized in " << (endTime - startTime) << "msecs" << endl;

	while (adapter.nextTrainingVideo()) {
		while (adapter.nextTrainingFrame()) {
			DatasetExample example = adapter.getTrainingFrame();
		}
	}

	return 0;
}
