/*
	oscpack -- Open Sound Control packet manipulation library
	http://www.audiomulch.com/~rossb/oscpack

	Copyright (c) 2004-2005 Ross Bencina <rossb@audiomulch.com>

	Permission is hereby granted, free of charge, to any person obtaining
	a copy of this software and associated documentation files
	(the "Software"), to deal in the Software without restriction,
	including without limitation the rights to use, copy, modify, merge,
	publish, distribute, sublicense, and/or sell copies of the Software,
	and to permit persons to whom the Software is furnished to do so,
	subject to the following conditions:

	The above copyright notice and this permission notice shall be
	included in all copies or substantial portions of the Software.

	Any person wishing to distribute modifications to the Software is
	requested to send the modifications to the original developer so that
	they can be incorporated into the canonical version.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
	EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
	MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
	IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
	ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
	CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
	WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#include "OscSendTests.h"

#include <iostream>
#include <string.h>
#include <cstring>
#include <cstdlib>

#include "osc/OscOutboundPacketStream.h"

#include "ip/UdpSocket.h"
#include "ip/IpEndpointName.h"

#define IP_MTU_SIZE 1536

namespace osc{
    
void RunSendTests( const IpEndpointName& host )
{
    char buffer[IP_MTU_SIZE];
    osc::OutboundPacketStream p( buffer, IP_MTU_SIZE );
	UdpTransmitSocket socket( host );

    p.Clear();
    p << osc::BeginMessage( "/test1" )
            << true << 23 << (float)3.1415 << "hello" << osc::EndMessage;
    socket.Send( p.Data(), p.Size() );

    // test1 message with too few arguments
    p.Clear();
    p << osc::BeginMessage( "/test1" )
            << true << osc::EndMessage;
    socket.Send( p.Data(), p.Size() );

    // test1 message with too many arguments
    p.Clear();
    p << osc::BeginMessage( "/test1" )
            << true << 23 << (float)3.1415 << "hello" << 42 << osc::EndMessage;
    socket.Send( p.Data(), p.Size() );

    // test1 message with wrong argument type
    p.Clear();
    p << osc::BeginMessage( "/test1" )
            << true << 1.0 << (float)3.1415 << "hello" << osc::EndMessage;
    socket.Send( p.Data(), p.Size() );

    p.Clear();
    p << osc::BeginMessage( "/test2" )
            << true << 23 << (float)3.1415 << "hello" << osc::EndMessage;
    socket.Send( p.Data(), p.Size() );

    // send four /test3 messages, each with a different type of argument
    p.Clear();
    p << osc::BeginMessage( "/test3" )
            << true << osc::EndMessage;
    socket.Send( p.Data(), p.Size() );

    p.Clear();
    p << osc::BeginMessage( "/test3" )
            << 23 << osc::EndMessage;
    socket.Send( p.Data(), p.Size() );

    p.Clear();
    p << osc::BeginMessage( "/test3" )
            << (float)3.1415 << osc::EndMessage;
    socket.Send( p.Data(), p.Size() );

    p.Clear();
    p << osc::BeginMessage( "/test3" )
           << "hello" << osc::EndMessage;
    socket.Send( p.Data(), p.Size() );
    

    // send a bundle
    p.Clear();
    p << osc::BeginBundle();

        p << osc::BeginMessage( "/no_arguments" )
            << osc::EndMessage;

        p << osc::BeginMessage( "/a_bool" )
            << true << osc::EndMessage;

        p << osc::BeginMessage( "/a_bool" )
            << false << osc::EndMessage;

        p << osc::BeginMessage( "/a_bool" )
            << (bool)1234 << osc::EndMessage;

        p << osc::BeginMessage( "/nil" )
            << osc::Nil << osc::EndMessage;

        p << osc::BeginMessage( "/inf" )
            << osc::Infinitum << osc::EndMessage;

        p << osc::BeginMessage( "/an_int" ) << 1234 << osc::EndMessage;

        p << osc::BeginMessage( "/a_float" )
            << 3.1415926f << osc::EndMessage;

        p << osc::BeginMessage( "/a_char" )
            << 'c' << osc::EndMessage;

        p << osc::BeginMessage( "/an_rgba_color" )
            << osc::RgbaColor(0x22334455) << osc::EndMessage;

        p << osc::BeginMessage( "/a_midi_message" )
            << MidiMessage(0x7F) << osc::EndMessage;

        p << osc::BeginMessage( "/an_int64" )
            << (int64)(0xFFFFFFF) << osc::EndMessage;

        p << osc::BeginMessage( "/a_time_tag" )
            << osc::TimeTag(0xFFFFFFFUL) << osc::EndMessage;

        p << osc::BeginMessage( "/a_double" )
            << (double)3.1415926 << osc::EndMessage;

        p << osc::BeginMessage( "/a_string" )
            << "hello world" << osc::EndMessage;

        p << osc::BeginMessage( "/a_symbol" )
            << osc::Symbol("foobar") << osc::EndMessage;

        // blob
        {
            char blobData[] = "abcd";

            p << osc::BeginMessage( "/a_blob" )
                << osc::Blob( blobData, 4 )
                << osc::EndMessage;
        }

    p << osc::EndBundle;
    socket.Send( p.Data(), p.Size() );



    // nested bundles, and multiple messages in bundles...
    p.Clear();
    p << osc::BeginBundle( 1234 )
        << osc::BeginMessage( "/an_int" ) << 1 << osc::EndMessage
        << osc::BeginMessage( "/an_int" ) << 2 << osc::EndMessage
        << osc::BeginMessage( "/an_int" ) << 3 << osc::EndMessage
        << osc::BeginMessage( "/an_int" ) << 4 << osc::EndMessage
        << osc::BeginBundle( 12345 )
            << osc::BeginMessage( "/an_int" ) << 5 << osc::EndMessage
            << osc::BeginMessage( "/an_int" ) << 6 << osc::EndMessage
        << osc::EndBundle
    << osc::EndBundle;

    socket.Send( p.Data(), p.Size() );
}

} // namespace osc

#ifndef NO_OSC_TEST_MAIN

int main(int argc, char* argv[])
{
    if( argc >= 2 && strcmp( argv[1], "-h" ) == 0 ){
        std::cout << "usage: OscSendTests [hostname [port]]\n";
        return 0;
    }

    char *hostName = "localhost";
    int port = 7000;
    
    if( argc >= 2 )
        hostName = argv[1];

    if( argc >= 3 )
        port = atoi( argv[2] );


	IpEndpointName host( hostName, port );

	char hostIpAddress[ IpEndpointName::ADDRESS_STRING_LENGTH ];
	host.AddressAsString( hostIpAddress );

    std::cout << "sending test messages to " << hostName 
		<< " (" << hostIpAddress << ") on port " << port << "...\n";

    osc::RunSendTests( host );
}

#endif /* NO_OSC_TEST_MAIN */
