// OSCKinect.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "OSCKinect.h"

using namespace std;

OSCKinect* kinectapp;

BOOL WINAPI exitHandler(DWORD ctrltype) {
	kinectapp->Nui_UnInit();
	return true;
}

int main(int argc, char* argv[])
{
	int udp_port = PORT;
	cerr << argc << endl;
	if (argc > 2) {
		if (!strcmp((const char*)argv[1], "-p"))
			udp_port = atoi((const char*) argv[2]);
	}
	cerr << "port is " << udp_port << endl;
	kinectapp = new OSCKinect(udp_port);
	kinectapp->Nui_Init();
	SetConsoleCtrlHandler(exitHandler, true);
	while(!kinectapp->OSCKinectDone);
	delete kinectapp;
	return 0;
}

OSCKinect::OSCKinect(int port) : m_pNuiSensor(NULL), m_instanceId(NULL), p(msgbuffer, OUTPUT_BUFFER_SIZE) {
	m_TransmitSocket = new UdpTransmitSocket( IpEndpointName( ADDRESS, port ) );
	Nui_Zero();
	OSCKinectDone = false;
}

OSCKinect::~OSCKinect() {
	Nui_Zero();
    SysFreeString(m_instanceId);
	delete m_TransmitSocket;
}

