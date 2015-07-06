Simple program that sends skeleton data from Kinect for Windows (the new one) as OSC messages on a UDP socket.  It uses the Microsoft SDK.

Usage:
```
OSCKinect [-p <port>]
```

Skeleton messages are bundles of joint messages:
```
/joint/<jointid>/<userid>  posx posy poz

Where:
userid is 0-5 and
jointid is one of:
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
```

It also sends position messages for untracked people (people that don't have generated skeletons):
```
/position/<userid> posx posy posz
```

And sends information messages:
```
/new_skel <userid>
/new_user <userid>
/lost_user <userid>
/position_only <userid>
/user_reenter <userid>
```

Right now I can't get Kinect to ever recognize someone that goes out of the space and comes back so the user\_reenter is never triggered.  Everything else works as far as I can tell.