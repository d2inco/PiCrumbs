#include "predict.h"
#include <fstream>
#include <sstream>
#include <cstdlib>
#include "beacon.h"

// GLOBAL VARS
extern float pos_lat;						// current latitude
extern float pos_lon;						// current longitude
extern unsigned short int pos_alt;			// current altitude in meters
extern string mycall;						// callsign we're operating under, excluding ssid

// VARS
string predict_path;					// path to PREDICT program

bool is_visible(string sat, int min_ele) {		// use PREDICT to figure out if this sat is around to try
	if (predict_path.compare("") == 0) return false;	// short circuit if predict not defined
	ofstream qthfile;
	qthfile.open("/tmp/picrumbs.qth", ios::trunc);		// PREDICT doesn't seem to be able to take a location via command line, so we need to build a "qth file" based on the current position.
	qthfile.precision(6);
	qthfile << fixed;
	qthfile << mycall << '\n';
	qthfile << ' ' << pos_lat << '\n';
	qthfile << ' ' << -pos_lon << '\n';	// predict expects negative values for east
	qthfile << ' ' << pos_alt << '\n';
	qthfile.close();

	stringstream predict_cmd;
	predict_cmd << predict_path;
	predict_cmd << " -f " << '"' << sat << "\" -q /tmp/picrumbs.qth";	// build the command line
	try {
		FILE *predict = popen(predict_cmd.str().c_str(), "r");
		if (!predict) {
			fprintf(stderr, "Could not execute %s\n", predict_cmd.str().c_str());
			return false;
		}

		char buff[256];
		fgets(buff, sizeof(buff), predict);
		pclose(predict);
		string buff_s = buff;

		return (atoi(buff_s.substr(32, 4).c_str()) > min_ele);
	} catch (exception& e) {
		fprintf(stderr, "Error while executing PREDICT (%s): %s\n", predict_cmd.str().c_str(), e.what());
		return false;
	}
	return false;
}	// END OF 'is_visible'