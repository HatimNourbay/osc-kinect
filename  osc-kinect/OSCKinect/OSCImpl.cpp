#include "stdafx.h"
#include "OSCKinect.h"

using namespace std;

static const char* jointNames[NUI_SKELETON_POSITION_COUNT] = {
	"hip_centre",
	"spine",
	"shoulder_centre",
	"head",
	"shoulder_left",
	"elbow_left",
	"wrist_left",
	"hand_left",
	"shoulder_right",
	"elbow_right",
	"wrist_right",
	"hand_right",
	"hip_left",
	"knee_left",
	"ankle_left",
	"foot_left",
	"hip_right",
	"knee_right",
	"ankle_right",
	"foot_right"
};

//typedef enum _NUI_SKELETON_POSITION_INDEX
//{
//    NUI_SKELETON_POSITION_HIP_CENTER = 0,
//    NUI_SKELETON_POSITION_SPINE,
//    NUI_SKELETON_POSITION_SHOULDER_CENTER,
//    NUI_SKELETON_POSITION_HEAD,
//    NUI_SKELETON_POSITION_SHOULDER_LEFT,
//    NUI_SKELETON_POSITION_ELBOW_LEFT,
//    NUI_SKELETON_POSITION_WRIST_LEFT,
//    NUI_SKELETON_POSITION_HAND_LEFT,
//    NUI_SKELETON_POSITION_SHOULDER_RIGHT,
//    NUI_SKELETON_POSITION_ELBOW_RIGHT,
//    NUI_SKELETON_POSITION_WRIST_RIGHT,
//    NUI_SKELETON_POSITION_HAND_RIGHT,
//    NUI_SKELETON_POSITION_HIP_LEFT,
//    NUI_SKELETON_POSITION_KNEE_LEFT,
//    NUI_SKELETON_POSITION_ANKLE_LEFT,
//    NUI_SKELETON_POSITION_FOOT_LEFT,
//    NUI_SKELETON_POSITION_HIP_RIGHT,
//    NUI_SKELETON_POSITION_KNEE_RIGHT,
//    NUI_SKELETON_POSITION_ANKLE_RIGHT,
//    NUI_SKELETON_POSITION_FOOT_RIGHT,
//    NUI_SKELETON_POSITION_COUNT
//} NUI_SKELETON_POSITION_INDEX;

void  OSCKinect::OSC_SendSkeletonData( NUI_SKELETON_DATA * pSkel, int pSkelid) {
	p.Clear();
	p << osc::BeginBundleImmediate;
	
	sprintf_s(messageaddr, "/position/%d", pSkelid);
	p << osc::BeginMessage(messageaddr)
		<< (float) pSkel->Position.x << (float) pSkel->Position.y << (float) pSkel->Position.z
		<< osc::EndMessage;

	int i;
    for (i = 0; i < NUI_SKELETON_POSITION_COUNT; i++) {
		if ( pSkel->eSkeletonPositionTrackingState[i] != NUI_SKELETON_POSITION_NOT_TRACKED ) {
			sprintf_s(messageaddr, "/joint/%s/%d", jointNames[i], pSkelid);
			 p << osc::BeginMessage( messageaddr )
			 << (float) pSkel->SkeletonPositions[i].x << (float) pSkel->SkeletonPositions[i].y <<  (float) pSkel->SkeletonPositions[i].z
			 << osc::EndMessage;       
		}
	}
	p << osc::EndBundle;
    m_TransmitSocket->Send( p.Data(), p.Size() );
}

void  OSCKinect::OSC_SendSkeletonPosData( NUI_SKELETON_DATA * pSkel, int pSkelid) {
	p.Clear();

	sprintf_s(messageaddr, "/position/%d", pSkelid);
	p << osc::BeginMessage(messageaddr)
		<< (float) pSkel->Position.x << (float) pSkel->Position.y << (float) pSkel->Position.z
		<< osc::EndMessage;

	m_TransmitSocket->Send( p.Data(), p.Size() );
} 

void  OSCKinect::OSC_SendUserLeftMessage(int index, int userid) {
	p.Clear();

	p << osc::BeginMessage("/lost_user") << index << osc::EndMessage;
	m_TransmitSocket->Send( p.Data(), p.Size() );
}

void  OSCKinect::OSC_SendNewUserMessage(int index, int userid) {
	p.Clear();

	p << osc::BeginMessage("/new_user") << index << osc::EndMessage;
	m_TransmitSocket->Send( p.Data(), p.Size() );
}

void  OSCKinect::OSC_SendReenterMessage(int index, int userid) {
	p.Clear();

	p << osc::BeginMessage("/user_reenter") << index << osc::EndMessage;
	m_TransmitSocket->Send( p.Data(), p.Size() );
}

void  OSCKinect::OSC_SendNoTrackMessage(int index, int userid) {
	p.Clear();

	p << osc::BeginMessage("/position_only") << index << osc::EndMessage;
	m_TransmitSocket->Send( p.Data(), p.Size() );
}

void  OSCKinect::OSC_SendNewTrackMessage(int index, int userid) {
	p.Clear();

	p << osc::BeginMessage("/new_skel") << index << osc::EndMessage;
	m_TransmitSocket->Send( p.Data(), p.Size() );
}