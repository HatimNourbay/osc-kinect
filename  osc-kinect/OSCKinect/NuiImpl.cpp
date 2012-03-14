#include "stdafx.h"
#include "OSCKinect.h"

using namespace std;

//-------------------------------------------------------------------
// Nui_Init
//
// Initialize Kinect
//-------------------------------------------------------------------
HRESULT OSCKinect::Nui_Init( )
{
    HRESULT  hr;

    if ( !m_pNuiSensor )
    {
        HRESULT hr = NuiCreateSensorByIndex(0, &m_pNuiSensor);

        if ( FAILED(hr) )
        {
			cerr <<  "Error opening the sensor" << endl;
            return hr;
        }

        SysFreeString(m_instanceId);

        m_instanceId = m_pNuiSensor->NuiDeviceConnectionId();
    }

    m_hNextSkeletonEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

 
    
    DWORD nuiFlags = NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX | NUI_INITIALIZE_FLAG_USES_SKELETON | NUI_INITIALIZE_FLAG_USES_COLOR;
    hr = m_pNuiSensor->NuiInitialize( nuiFlags );
  
    if ( FAILED( hr ) )
    {
        if ( E_NUI_DEVICE_IN_USE == hr )
        {
            cerr << "Error: Kinect already in use" << endl;
        }
        else
        {
           cerr <<  "Kinect had trouble initializing" << endl;
        }
        return hr;
    }

    if ( HasSkeletalEngine( m_pNuiSensor ) )
    {
        hr = m_pNuiSensor->NuiSkeletonTrackingEnable( m_hNextSkeletonEvent, 0 );
        if( FAILED( hr ) )
        {
            cerr << "Could not enable skeleton tracking" << endl;
            return hr;
        }
    }

    
    // Start the Nui processing thread
    m_hEvNuiProcessStop = CreateEvent( NULL, FALSE, FALSE, NULL );
    m_hThNuiProcess = CreateThread( NULL, 0, Nui_ProcessThread, this, 0, NULL );

    return hr;
}

//-------------------------------------------------------------------
// Nui_UnInit
//
// Uninitialize Kinect
//-------------------------------------------------------------------
void OSCKinect::Nui_UnInit( )
{
    // Stop the Nui processing thread
    if ( NULL != m_hEvNuiProcessStop )
    {
        // Signal the thread
        SetEvent(m_hEvNuiProcessStop);

        // Wait for thread to stop
        if ( NULL != m_hThNuiProcess )
        {
            WaitForSingleObject( m_hThNuiProcess, INFINITE );
            CloseHandle( m_hThNuiProcess );
        }
        CloseHandle( m_hEvNuiProcessStop );
    }

    if ( m_pNuiSensor )
    {
		cerr << "shutting down" << endl;
        m_pNuiSensor->NuiShutdown( );
    }
    if ( m_hNextSkeletonEvent && ( m_hNextSkeletonEvent != INVALID_HANDLE_VALUE ) )
    {
        CloseHandle( m_hNextSkeletonEvent );
        m_hNextSkeletonEvent = NULL;
    }
    

    if ( m_pNuiSensor )
    {
        m_pNuiSensor->Release();
        m_pNuiSensor = NULL;
    }

	OSCKinectDone = true;
}

void OSCKinect::Nui_Zero() {
	if (m_pNuiSensor)
	{
        m_pNuiSensor->Release();
        m_pNuiSensor = NULL;
    }
  
    m_hNextSkeletonEvent = NULL;
    m_hThNuiProcess = NULL;
    m_hEvNuiProcessStop = NULL;

	ZeroMemory(m_SkeletonIds,sizeof(m_SkeletonIds));
}

DWORD WINAPI OSCKinect::Nui_ProcessThread(LPVOID pParam)
{
    OSCKinect *pthis = (OSCKinect *) pParam;
    return pthis->Nui_ProcessThread();
}

//-------------------------------------------------------------------
// Nui_ProcessThread
//
// Thread to handle Kinect processing
//-------------------------------------------------------------------
DWORD WINAPI OSCKinect::Nui_ProcessThread()
{
	const int numEvents = 2;
    HANDLE hEvents[numEvents] = { m_hEvNuiProcessStop, m_hNextSkeletonEvent };
    int    nEventIdx;

    // Main thread loop
    bool continueProcessing = true;
    while ( continueProcessing )
    {
        // Wait for any of the events to be signalled
        nEventIdx = WaitForMultipleObjects( numEvents, hEvents, FALSE, 100 );

        // Process signal events
        switch ( nEventIdx )
        {
            case WAIT_TIMEOUT:
                continue;

            // If the stop event, stop looping and exit
            case WAIT_OBJECT_0:
                continueProcessing = false;
                continue;

            case WAIT_OBJECT_0 + 1:
                Nui_GotSkeletonAlert( );
                break;
        }
    }

	return 0;
}

//-------------------------------------------------------------------
// Nui_GotSkeletonAlert
//
// Handle new skeleton data
//-------------------------------------------------------------------
void OSCKinect::Nui_GotSkeletonAlert( )
{
    NUI_SKELETON_FRAME SkeletonFrame = {0};

    if ( SUCCEEDED(m_pNuiSensor->NuiSkeletonGetNextFrame( 0, &SkeletonFrame )) )
    {
	    // smooth out the skeleton data
		HRESULT hr = m_pNuiSensor->NuiTransformSmooth(&SkeletonFrame,NULL);
		if ( FAILED(hr) ) {
			return;
		}

		bool userleft = false;
		bool newuser = false;
		bool reenter = false;
		bool notrack = false;
		bool newtrack = false;
		bool idchanged = false;
        for ( int i = 0 ; i < NUI_SKELETON_COUNT ; i++ )
        {
			userleft = false;
			newuser = false;
			reenter = false;
			notrack = false;
			newtrack = false;
			idchanged = false;
			if (SkeletonFrame.SkeletonData[i].dwTrackingID != m_SkeletonIds[i].userid) {
					idchanged = true;
					if (m_SkeletonIds[i].userid != 0 && m_SkeletonIds[i].trackState != NUI_SKELETON_NOT_TRACKED) {
						cerr << "User at index " << i << " and user id " << m_SkeletonIds[i].userid << " left" << endl;
						userleft = true;
					} 
					if (SkeletonFrame.SkeletonData[i].eTrackingState != NUI_SKELETON_NOT_TRACKED) {
						cerr << "Got a new user at index " << i << " user id: " << SkeletonFrame.SkeletonData[i].dwTrackingID << endl;
						newuser = true;
					}
			}  
			
			switch (SkeletonFrame.SkeletonData[i].eTrackingState) {
				//case NUI_SKELETON_NOT_TRACKED:
				//	if (m_SkeletonIds[i].trackState != NUI_SKELETON_NOT_TRACKED) {
				//		cerr << "User id " << m_SkeletonIds[i].userid << " at index " << i << " has just left" << endl;
				//		//send message that user has left
				//	}
				//	break;
			case NUI_SKELETON_POSITION_ONLY:
				if (m_SkeletonIds[i].trackState == NUI_SKELETON_NOT_TRACKED && !idchanged)  {
					// Re-enter
					reenter = true;
					cerr << "User id " << m_SkeletonIds[i].userid << " at index " << i << " has re-entered" << endl;
				} else if (m_SkeletonIds[i].trackState == NUI_SKELETON_TRACKED) {
					// The user is still there but skeleton not being tracked
					cerr << "User at index " << i << " user id " << m_SkeletonIds[i].userid << " no longer being tracked" << endl;
					notrack = true;
				}
				break;
			case NUI_SKELETON_TRACKED:		
				if (m_SkeletonIds[i].trackState == NUI_SKELETON_POSITION_ONLY) {
					newtrack = true;
					cerr << "User at index " << i << " user id " << m_SkeletonIds[i].userid << " is now being tracked " << endl;
				} else if (m_SkeletonIds[i].trackState == NUI_SKELETON_NOT_TRACKED) {
					reenter = true;
					newtrack = true;
					cerr << "User id " << m_SkeletonIds[i].userid << " at index " << i << " has re-entered and is being tracked" << endl;
				}
				break;
			}

			// Send the OSC messages
			if (userleft)
				OSC_SendUserLeftMessage(i,SkeletonFrame.SkeletonData[i].dwTrackingID);
			if (newuser && !newtrack)
				OSC_SendNewUserMessage(i,SkeletonFrame.SkeletonData[i].dwTrackingID);
			if (reenter)
				OSC_SendReenterMessage(i,SkeletonFrame.SkeletonData[i].dwTrackingID);
			if (notrack)
				OSC_SendNoTrackMessage(i, SkeletonFrame.SkeletonData[i].dwTrackingID);
			if (newtrack)
				OSC_SendNewTrackMessage(i, SkeletonFrame.SkeletonData[i].dwTrackingID);

			if (SkeletonFrame.SkeletonData[i].eTrackingState == NUI_SKELETON_TRACKED)
				OSC_SendSkeletonData(&SkeletonFrame.SkeletonData[i], i);
			else if (SkeletonFrame.SkeletonData[i].eTrackingState == NUI_SKELETON_POSITION_ONLY)
				OSC_SendSkeletonPosData(&SkeletonFrame.SkeletonData[i], i);

			if (SkeletonFrame.SkeletonData[i].eTrackingState != NUI_SKELETON_NOT_TRACKED)
				m_SkeletonIds[i].userid = SkeletonFrame.SkeletonData[i].dwTrackingID;
			m_SkeletonIds[i].trackState = SkeletonFrame.SkeletonData[i].eTrackingState;
		}
	}
}
