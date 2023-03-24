/** \file

Simple minded example of homing & moving a motor.
*/

// Comment this out to use EtherCAT
//#define USE_CAN

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string> 
#include "CML.h"
#include <sstream>

#if defined( USE_CAN )
#include "can/can_copley.h"
#elif defined( WIN32 )
#include "ecat/ecat_winudp.h"
#else
#include "ecat/ecat_linux.h"
#endif

using std::ofstream;
using std::ifstream;
using std::string;
using std::stringstream;
using std::cout;
using std::cin;
using std::to_string;

// If a namespace has been defined in CML_Settings.h, this
// macros starts using it. 
CML_NAMESPACE_USE();

/* local functions */
static void showerr(const Error* err, const char* str);
double milliToCounts(double millimeters);
/* local data */
int32 canBPS = 1000000;             // CAN network bit rate
int16 canNodeID = -1;                // CANopen node ID, for ECAT it is negative node ID
int axisNum{ 3 };

/**************************************************
* Just home the motor and do a bunch of random
* moves.
**************************************************/

// load the PVT data from the passed excel file into the PvtConstAccelTrj class.
// Time between PVT Points
uint8 timeBetweenPoints = 10; // 10ms between points

void loadPvtPointsFromFile(PvtConstAccelTrj& pvtConstTrjObj, const char* inputExcelFile) {
	try {
		ifstream ifstrObj;
		string xPos{ NULL };
		string yPos{ NULL };
		string zPos{ NULL };

		const Error* err{ 0 };
		vector<double> tempVec = { 0.0, 0.0, 0.0 };
		vector<double>* tempVecPntr = &tempVec;

		vector<double> xPosVec;
		vector<double> yPosVec;
		vector<double> zPosVec;

		vector<uint8> constantTimeVec;
		constantTimeVec.push_back(timeBetweenPoints);

		ifstrObj.open(inputExcelFile);

		if (ifstrObj.is_open()) {
			int count{ 0 };
			string lineOfData{ NULL };
			while (!ifstrObj.eof()) {

				// get a line of data from the excel file.
				getline(ifstrObj, lineOfData);

				// ignore first row, which is just the title row. (X Coordinate, Y Coordinate, Z Coordinate, Time).
				if (count != 0) {

					vector<string> allNumsVec;
					string tempStr;
					stringstream strStream(lineOfData);

					if (lineOfData != "") {
						// seperate the line using the commas. The string looks like "1034,120345,123"
						while (getline(strStream, tempStr, ',')) {
							allNumsVec.push_back(tempStr);
						}

						// use the stod method to convert string to double type.
						tempVec[0] = std::stod(allNumsVec[0]);
						tempVec[1] = std::stod(allNumsVec[1]);
						tempVec[2] = std::stod(allNumsVec[2]);

						// add the PVT point to the PvtConstAccelTrj object.
						err = pvtConstTrjObj.addPvtPoint(tempVecPntr, &constantTimeVec[0]);
						showerr(err, "adding points to the PVT object");
					}
				}

				// used to ignore first row. 
				else {
					count = 1;
				}
			}
		}

		ifstrObj.close();
	}
	// catch any exception here
	catch (...) {
		cout << "\nException occured in loadPvtPointsFromFile.\n";
		throw;
	}
}

void exportPositionPointsToExcel(PvtConstAccelTrj& pvtConstTrjObj, const char* outputExcelFile) {
	ofstream excelFile;
	excelFile.open(outputExcelFile);

	vector<list<double>>* posPntr = pvtConstTrjObj.getPositionsPntr();
	int numPvtPoints = pvtConstTrjObj.getNumberOfPvtPoints();

	std::list<double>::iterator iter1 = (*posPntr)[0].begin();
	std::list<double>::iterator iter2 = (*posPntr)[1].begin();
	std::list<double>::iterator iter3 = (*posPntr)[2].begin();

	for (int i = 0; i < numPvtPoints; i++) {
		excelFile << to_string(*iter1) << ',' << to_string(*iter2) << ',' << to_string(*iter3) << '\n';

		if (i < numPvtPoints - 1) {
			iter1++;
			iter2++;
			iter3++;
		}
	}

	excelFile.close();
}

void exportVelocityPointsToExcel(PvtConstAccelTrj& pvtConstTrjObj, const char* outputExcelFile) {
	ofstream excelFile;
	excelFile.open(outputExcelFile);

	vector<list<double>>* velPntr = pvtConstTrjObj.getVelocitiesPntr();
	int numPvtPoints = pvtConstTrjObj.getNumberOfPvtPoints();

	std::list<double>::iterator iter1 = (*velPntr)[0].begin();
	std::list<double>::iterator iter2 = (*velPntr)[1].begin();
	std::list<double>::iterator iter3 = (*velPntr)[2].begin();

	for (int i = 0; i < numPvtPoints; i++) {
		excelFile << to_string(*iter1) << ',' << to_string(*iter2) << ',' << to_string(*iter3) << '\n';

		if (i < numPvtPoints - 1) {
			iter1++;
			iter2++;
			iter3++;
		}
	}

	excelFile.close();
}

void loadPvtPoints(PvtConstAccelTrj& pvtConstTrjObj) {
	try {
		const Error* err{ 0 };
		vector<double> tempVec = { 0.0, 0.0, 0.0 };
		vector<double>* tempVecPntr = &tempVec;

		// Working fast set
		//vector<double> xPosVec = { 0,2000,3000,5000,12000,13000,16000,22000,28000,30000,30400,30700,30700,25000,21000,18000,15000,11000,4000,3000,2500,2000,0,0,2000,3000,5000,12000,13000,16000,22000,28000,30000,30400,30700,30700,25000,21000,18000,15000,11000,4000,3000,2500,2000,0 };
		//vector<double> yPosVec = { 0,2000,3000,5000,12000,13000,16000,22000,28000,30000,30400,30700,30700,25000,21000,18000,15000,11000,4000,3000,2500,2000,0,0,2000,3000,5000,12000,13000,16000,22000,28000,30000,30400,30700,30700,25000,21000,18000,15000,11000,4000,3000,2500,2000,0 };
		//vector<double> zPosVec = { 0,2000,3000,5000,12000,13000,16000,22000,28000,30000,30400,30700,30700,25000,21000,18000,15000,11000,4000,3000,2500,2000,0,0,2000,3000,5000,12000,13000,16000,22000,28000,30000,30400,30700,30700,25000,21000,18000,15000,11000,4000,3000,2500,2000,0 };

		// Fast with delays
		vector<double> xPosVec = { 0,2000,12000,22000,29050,30200,30170,30150,29050,22000,12000,3000,1000,0 };
		vector<double> yPosVec = { 0,2000,12000,22000,29050,30200,30500,30150,29050,22000,12000,3000,1000,0 };
		vector<double> zPosVec = { 0,2000,12000,22000,29050,30850,30800,30550,29050,22000,12000,3000,1000,0 };


		vector<uint8> constantTimeVec;
		constantTimeVec.push_back(timeBetweenPoints);

		for (int i = 0; i < xPosVec.size(); i++) {

			tempVec[0] = xPosVec[i];
			tempVec[1] = yPosVec[i];
			tempVec[2] = zPosVec[i];

			// add the PVT point to the PvtConstAccelTrj object.
			err = pvtConstTrjObj.addPvtPoint(tempVecPntr, &constantTimeVec[0]);
			showerr(err, "adding points to the PVT object");
		}
	}
	// catch any exception here
	catch (...) {
		cout << "\nException occured in loadPvtPoints.\n";
		throw;
	}
}

int main(void) {
	// The libraries define one global object of type
	// CopleyMotionLibraries named cml.
	//
	// This object has a couple handy member functions
	// including this one which enables the generation of
	// a log file for debugging
	cml.SetLogFile("cml.log");
	cml.SetDebugLevel(LOG_EVERYTHING);

	// Create an object used to access the low level CAN network.
	// This examples assumes that we're using the Copley PCI CAN card.
#if defined( USE_CAN )
	CopleyCAN hw("CAN0");
	hw.SetBaud(canBPS);
#elif defined( WIN32 )
	WinUdpEcatHardware hw("192.168.0.100");
#else
	LinuxEcatHardware hw("eth0");
#endif

	// Open the network object
#if defined( USE_CAN )
	CanOpen net;
#else
	EtherCAT net;
#endif
	const Error* err = net.Open(hw);
	showerr(err, "Opening network");

	AmpSettings ampSettingsObj;
	ampSettingsObj.synchPeriod = 2000;

	// Initialize the amplifier using default settings
	Amp amp[3];
	printf("Doing init\n");
	err = amp[0].Init(net, canNodeID, ampSettingsObj);
	showerr(err, "Initting amp");

	printf("Doing init axis b\n");
	err = amp[1].InitSubAxis(amp[0], 2);
	showerr(err, "Initting amp");

	printf("Doing init axis c\n");
	err = amp[2].InitSubAxis(amp[0], 3);
	showerr(err, "Initting amp");

	//TPDO
	printf("initalizing TPDOs\n");
	TPDO tpdo;
	err = tpdo.Init(0);
	showerr(err, "Initing TPDO");

	CML::Pmap16 analoginputArray[3];
	err = analoginputArray[0].Init(0x2200,0);
	showerr(err, "analoginputArray[0].Init(0x2200,0)");
	err = analoginputArray[1].Init(0x2A00, 0);
	showerr(err, "analoginputArray[1].Init(0x2A00,0)");
	err = analoginputArray[2].Init(0x3200, 0);
	showerr(err, "analoginputArray[2].Init(0x3200,0)");

	err = tpdo.AddVar(analoginputArray[0]);
	showerr(err, "tpdo.AddVar(analoginputArray[0])");
	err = tpdo.AddVar(analoginputArray[1]);
	showerr(err, "tpdo.AddVar(analoginputArray[1])");
	err = tpdo.AddVar(analoginputArray[2]);
	showerr(err, "tpdo.AddVar(analoginputArray[2])");

	err = amp[0].PreOpNode();
	showerr(err, "amp[0].PreOpNode()");
	err = amp[0].PdoSet(2,tpdo);
	showerr(err, "amp[0].PdoSet(2,tpdo)");
	err = amp[0].StartNode();
	showerr(err, "amp[0].StartNode();");

	err = amp[0].Enable();
	showerr(err, "Enable");

	//Homing
	err = amp[0].GoHome();
	showerr(err, "Going home");
	err = amp[0].WaitMoveDone(-1);
	showerr(err, "waiting for home axis a");

	err = amp[1].GoHome();
	showerr(err, "Going home");
	err = amp[1].WaitMoveDone(-1);
	showerr(err, "waiting for home axis b");

	err = amp[2].GoHome();
	showerr(err, "Going home");
	err = amp[2].WaitMoveDone(-1);
	showerr(err, "waiting for home axis c");

	Linkage link;
	err = link.Init(3, amp);
	showerr(err, "initalizing linkage");

	struct Drawer { double x; double y; double z; };

	// Create a new path object
	double pathMaxVel{ 1000000 };
	double pathMaxAccel{ 50000000 };
	double pathMaxDecel{ 50000000 };
	double pathMaxJerk{ 20000000 };

	// set the limits for the linkage object
	err = link.SetMoveLimits(pathMaxVel, pathMaxAccel, pathMaxDecel, pathMaxJerk); showerr(err, "Setting Linkage Move Limits");

	// create an instance of the PvtConstAccelTrj class.
	PvtConstAccelTrj pvtConstTrjObj;

	// initialize the object with the number of dimensions in the trajectory.
	err = pvtConstTrjObj.Init(axisNum);
	showerr(err, "initializing the PvtConstAccelTrj object");

	pvtConstTrjObj.deletePointsAfterExecution = true;
	printf("Loading points for move.\n");
	loadPvtPoints(pvtConstTrjObj);
	//loadPvtPointsFromFile(pvtConstTrjObj, "XyzPoints.csv");

	int16 index{ 0x2000 };
	int16 subIndex{ 0 };
	// const int outConfigSize{ 13 };
	const int outConfigONSize{ 9 };

	// range of OUT1 ON: -1000 to 22,000

	// 0x0d = serial-binary op-code for setting a parameter.
	// 0x0070 = ASCII parameter ID of OUT1 config
	// 0x0004 = output is ON in a position window.
	// 0xfffffc18 = lower threshhold = -1,000 counts
	// 0x000055f0 = upper threshhold = 22,000 counts

	// 20,000 decimal = 0x4e20 in hexadecimal. = { 0x0d, 0x70, 0x00, 0x04, 0x00, 0xff, 0xff, 0x18, 0xfc, 0x00, 0x00, 0x20, 0x4e }; 

	/*
	int timeDelayMs{10};
	byte byte1Time = (timeDelayMs & 0x00ff0000) >> 16;
	byte byte2Time = (timeDelayMs & 0xff000000) >> 24;
	byte byte3Time = (timeDelayMs & 0x000000ff);
	byte byte4Time = (timeDelayMs & 0x0000ff00) >> 8;

	int lowerThresholdPos{ 22000 };
	byte byte1Pos = (lowerThresholdPos & 0x00ff0000) >> 16;
	byte byte2Pos = (lowerThresholdPos & 0xff000000) >> 24;
	byte byte3Pos = (lowerThresholdPos & 0x000000ff);
	byte byte4Pos = (lowerThresholdPos & 0x0000ff00) >> 8;

	byte outConfigData22_a[outConfigSize] = { 0x0d, 0x70, 0x00, 0x05, 0x01, byte1Pos, byte2Pos, byte3Pos, byte4Pos, byte1Time, byte2Time, byte3Time, byte4Time };
	byte outConfigData22_b[outConfigSize] = { 0x0d, 0x71, 0x00, 0x05, 0x01, byte1Pos, byte2Pos, byte3Pos, byte4Pos, byte1Time, byte2Time, byte3Time, byte4Time };
	byte outConfigData22_c[outConfigSize] = { 0x0d, 0x72, 0x00, 0x05, 0x01, byte1Pos, byte2Pos, byte3Pos, byte4Pos, byte1Time, byte2Time, byte3Time, byte4Time };
	*/

	const int startCVMSize{ 5 };
	byte startCVM[startCVMSize] = { 0x14, 0x09, 0x00, 0x00, 0x00 };

	const int stopCVMSize{ 3 };
	byte stopCVM[stopCVMSize] = { 0x14, 0x0a, 0x00 };

	// set output config to program control active ON
	byte outConfigON_1[outConfigONSize] = { 0x0d, 0x70, 0x00, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00 };
	byte outConfigON_2[outConfigONSize] = { 0x0d, 0x71, 0x00, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00 };
	byte outConfigON_3[outConfigONSize] = { 0x0d, 0x72, 0x00, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00 };

	short outputPinStates{ 0x2194 };
	uint16 pinStatesData{ 7 };

	double xAxisPos{ 0 };
	double xAxisVel{ 0 };

	// 1600 = nothing in it
	// 6000 = picked up the part

	int pickedUpVal{ 6000 };

	int count{ 0 };
	int dropCount{ 0 };

	int numberOfCycles{ 1 };
	while (numberOfCycles != 0) {

		// if even, make outputs HI
		if (count % 2 == 0) {
			err = amp[0].Download(index, subIndex, stopCVMSize, stopCVM);
			showerr(err, "stopping CVM");

			// set output config to program control active ON
			err = amp[0].Download(index, subIndex, outConfigONSize, outConfigON_1);
			showerr(err, "configuring OUT1 to program control active HI");

			err = amp[0].Download(index, subIndex, outConfigONSize, outConfigON_2);
			showerr(err, "configuring OUT2 to program control active HI");

			err = amp[0].Download(index, subIndex, outConfigONSize, outConfigON_3);
			showerr(err, "configuring OUT3 to program control active HI");

			err = amp[0].sdo.Dnld16(outputPinStates, subIndex, pinStatesData);
			showerr(err, "setting output pins hi");
		}

		// it's odd. configure upper threshold positions.
		else {

			int upperThresholdOutput1{ 20000 };//Axis A, distance at which the vaccum turns off
			err = amp[0].sdo.Dnld32(0x2600, 4, upperThresholdOutput1);
			showerr(err, "setting CVM register 3");

			int upperThresholdOutput2{ 25000 };//Axis B, distance at which the vaccum turns off
			err = amp[0].sdo.Dnld32(0x2600, 5, upperThresholdOutput2);
			showerr(err, "setting CVM register 4");

			int upperThresholdOutput3{ 25000 };//Axis C, distance at which the vaccum turns off
			err = amp[0].sdo.Dnld32(0x2600, 6, upperThresholdOutput3);
			showerr(err, "setting CVM register 5");

			// Start CVM program
			// The CVM program will do the following:
			// 1) Configure OUT1 as ON in axis A's position window [-200,000 to R3]. 
			// 2) Configure OUT2 as ON in axis B's position window [-200,000 to R4].
			// 3) Configure OUT3 as ON in axis C's position window [-200,000 to R5]. 
			// 4) Wait for output pin states parameter (0xab) to be 0. (all 3 outputs are OFF)
			// 5) Configure OUT1 as active on, state OFF. (keep it OFF)
			// 6) Configure OUT2 as active on, state OFF. (keep it OFF)
			// 7) Configure OUT3 as active on, state OFF. (keep it OFF)

			err = amp[0].Download(index, subIndex, startCVMSize, startCVM);
			showerr(err, "starting CVM");

			count = -1;
		}

		// export the points to a csv file.
		//exportVelocityPointsToExcel(pvtConstTrjObj, "Velocities.csv");
		//exportPositionPointsToExcel(pvtConstTrjObj, "Positions.csv");

		err = link.SendTrajectory(pvtConstTrjObj);
		while ((err == &AmpError::NodeState) || (err == &LinkError::StartMoveTO)) {
			if (err == &AmpError::NodeState) {
				printf("Node Err, Loading points for move. Attempting again.\n");
			}
			if (err == &LinkError::StartMoveTO) {
				printf("Link Err, Loading points for move. Attempting again.\n");
			}
			err = link.SendTrajectory(pvtConstTrjObj);
		}
		showerr(err, "Sending trajectory");

		// Set to -1 to wait indefinitely.
		err = link.WaitMoveDone(-1);
		showerr(err, "waiting for the linkage move to finish");

		loadPvtPoints(pvtConstTrjObj);

		// OUT is ON. (object picked up)
		if (count % 2 == 0) {
			Thread::sleep(250);
			if ((analoginputArray[0].Read() < pickedUpVal)) {
				dropCount = dropCount + 1;
				printf("Dropped A# %d\n", dropCount);
				timeBetweenPoints = 50;
			}
			else if ((analoginputArray[1].Read() < pickedUpVal)) {
				dropCount = dropCount + 1;
				printf("Dropped B# %d\n", dropCount);
				timeBetweenPoints = 50;
			}
			else if ((analoginputArray[2].Read() < pickedUpVal)) {
				dropCount = dropCount + 1;
				printf("Dropped C# %d\n", dropCount);
				timeBetweenPoints = 50;
			}
			else {
				// nothing
			}
		}
		// OUT is OFF. (object not picket up).
		else {
			timeBetweenPoints = 10;
			Thread::sleep(250);
		}

		// end the program
		// numberOfCycles = 0;

		count = count + 1;

		/*

				for (int i = 0; i < numberOfCycles; i++) {

			// set output config to fire in position window -1000 to 60,000 counts.
			err = amp[0].Download(index, subIndex, outConfigSize, outConfigDataLargeThresh_a);
			showerr(err, "configuring OUT1 to large thresh");

			err = amp[0].Download(index, subIndex, outConfigSize, outConfigDataLargeThresh_b);
			showerr(err, "configuring OUT2 to large thresh");

			err = amp[0].Download(index, subIndex, outConfigSize, outConfigDataLargeThresh_c);
			showerr(err, "configuring OUT3 to large thresh");

			// if first move, send the trajectory to the trajectory generator.
			if (i == 0) {

				printf("Sending trajectory to drives\n");

				// send the trajectory to the linkage.
				err = link.SendTrajectory(pvtConstTrjObj);
				showerr(err, "sending trajectory");
			}

			// wait for velocity to become negative
			amp[0].GetVelocityActual(xAxisVel);
			while (xAxisVel > 0) {
				amp[0].GetVelocityActual(xAxisVel);
			}

			amp[0].GetPositionActual(xAxisPos);
			amp[0].GetVelocityActual(xAxisVel);

			// wait for position to be less than 1,000 counts traveling in the negative direction
			while ((xAxisPos >= 20000) && (xAxisVel < 0)) {
				amp[0].GetPositionActual(xAxisPos);
				amp[0].GetVelocityActual(xAxisVel);
			}

			// set output config to fire in position window -1000 to 22,000 counts.
			err = amp[0].Download(index, subIndex, outConfigSize, outConfigData22_a);
			showerr(err, "configuring OUT1 to 22,000 thresh");

			err = amp[0].Download(index, subIndex, outConfigSize, outConfigData22_b);
			showerr(err, "configuring OUT2 to 22,000 thresh");

			err = amp[0].Download(index, subIndex, outConfigSize, outConfigData22_c);
			showerr(err, "configuring OUT3 to 22,000 thresh");

			// wait for position to become greater than 25,000 counts
			amp[0].GetPositionActual(xAxisPos);
			while (xAxisPos < 25000) {
				amp[0].GetPositionActual(xAxisPos);
			}

			// wait for position to become less than 100 counts
			amp[0].GetPositionActual(xAxisPos);
			while (xAxisPos > 100) {
				amp[0].GetPositionActual(xAxisPos);
			}
		}

		if (numberOfCycles != 0) {
			// Set to -1 to wait indefinitely.
			err = link.WaitMoveDone(-1);
		}

		*/
	}

	printf("Program finished. Hit any key to quit\n");
	getchar();

	return 0;
}

// convert millimeters to counts.
double milliToCounts(double millimeters) {
	return(millimeters * 1000);// 1000 counts = 1mm
}
/**************************************************/

static void showerr(const Error* err, const char* str)
{
	if (err)
	{
		printf("Error %s: %s\n", str, err->toString());
		getchar();
		exit(1);
	}
}