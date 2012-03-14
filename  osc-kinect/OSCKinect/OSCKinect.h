#include "stdafx.h"

#include "ip/UdpSocket.h"
#include "osc/OscOutboundPacketStream.h"
#define ADDRESS "127.0.0.1"
#define PORT 7000

#define OUTPUT_BUFFER_SIZE 2048

struct skelinfo {
	int userid;
	int trackState;
};

class OSCKinect {
private:
	INuiSensor*             m_pNuiSensor;
	BSTR                    m_instanceId;

	UdpTransmitSocket*		m_TransmitSocket;

	char msgbuffer[OUTPUT_BUFFER_SIZE];
	osc::OutboundPacketStream p;
	char messageaddr[100];

	 // thread handling
    HANDLE        m_hThNuiProcess;
    HANDLE        m_hEvNuiProcessStop;
    HANDLE        m_hNextSkeletonEvent;

	bool          m_bPosTracking;
	skelinfo      m_SkeletonIds[NUI_SKELETON_COUNT];
	
	static DWORD WINAPI     Nui_ProcessThread(LPVOID pParam);
    DWORD WINAPI            Nui_ProcessThread( );

public:
	OSCKinect(int port);
	~OSCKinect();

	bool OSCKinectDone;

	HRESULT                 Nui_Init( );
    void                    Nui_UnInit( );
	void					Nui_Zero( );

	void                    Nui_GotSkeletonAlert( );
	void                    OSC_SendSkeletonData( NUI_SKELETON_DATA * pSkel, int pSkelid);
	void					OSC_SendSkeletonPosData( NUI_SKELETON_DATA * pSkel, int pSkelid);
    void					OSC_SendUserLeftMessage(int index, int userid);
	void					OSC_SendNewUserMessage(int index, int userid);
	void					OSC_SendReenterMessage(int index, int userid);
	void					OSC_SendNoTrackMessage(int index, int userid);
	void					OSC_SendNewTrackMessage(int index, int userid);
};