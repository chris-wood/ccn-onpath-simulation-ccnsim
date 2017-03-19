/*
 * Copyright (c) 2016, Xerox Corporation (Xerox) and Palo Alto Research Center, Inc (PARC)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL XEROX OR PARC BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/* ################################################################################
 * #
 * # PATENT NOTICE
 * #
 * # This software is distributed under the BSD 2-clause License (see LICENSE
 * # file).  This BSD License does not make any patent claims and as such, does
 * # not act as a patent grant.  The purpose of this section is for each contributor
 * # to define their intentions with respect to intellectual property.
 * #
 * # Each contributor to this source code is encouraged to state their patent
 * # claims and licensing mechanisms for any contributions made. At the end of
 * # this section contributors may each make their own statements.  Contributor's
 * # claims and grants only apply to the pieces (source code, programs, text,
 * # media, etc) that they have contributed directly to this software.
 * #
 * # There is no guarantee that this section is complete, up to date or accurate. It
 * # is up to the contributors to maintain their portion of this section and up to
 * # the user of the software to verify any claims herein.
 * #
 * # Do not remove this header notification.  The contents of this section must be
 * # present in all distributions of the software.  You may only modify your own
 * # intellectual property statements.  Please provide contact information.
 *
 * - Palo Alto Research Center, Inc
 * This software distribution does not grant any rights to patents owned by Palo
 * Alto Research Center, Inc (PARC). Rights to these patents are available via
 * various mechanisms. As of January 2016 PARC has committed to FRAND licensing any
 * intellectual property used by its contributions to this software. You may
 * contact PARC at cipo@parc.com for more information or visit http://www.ccnx.org
 */

/*
 * Uses a point-to-poit topology.  Sink will RegisterPrefix() on n0 which creates a route from n0 to the sink
 * Portal.  We add a static route on node n1 -> n0 for ccnx:/name=foo/name=sink.
 *
 * sink      source
 *  |         |
 * n0 ------ n1
 *     5Mbps
 *     2ms
 */

#include <iostream>
#include <fstream>
#include <string>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ccns3Sim-module.h"
#include "ns3/ccnx-message.h"

#include <cryptopp/asn.h>
#include <cryptopp/base64.h>
#include <cryptopp/des.h>
#include <cryptopp/files.h>
#include <cryptopp/filters.h>
#include <cryptopp/hex.h>
#include <cryptopp/modes.h>
#include <cryptopp/osrng.h>
#include <cryptopp/pssr.h>
#include <cryptopp/pwdbased.h>
#include <cryptopp/rsa.h>
#include <cryptopp/sha.h>
#include <cryptopp/eccrypto.h>
#include <cryptopp/oids.h>
#include <cryptopp/dsa.h>

using CryptoPP::Exception;
using CryptoPP::HMAC;
using CryptoPP::SHA256;
using CryptoPP::HexEncoder;
using CryptoPP::HexDecoder;
using CryptoPP::StringSink;
using CryptoPP::StringSource;
using CryptoPP::HashFilter;
using CryptoPP::HashVerificationFilter;
using CryptoPP::SecByteBlock;

using namespace ns3;
using namespace ns3::ccnx;

NS_LOG_COMPONENT_DEFINE ("ccnx-2node");

static const char * prefixString = "ccnx:/name=foo/name=sink";
static const char * contentString = "ccnx:/name=foo/name=sink/name=kitchen";

static Ptr<CCNxPacket>
CreatePacket (uint32_t size, Ptr<CCNxName> name, CCNxMessage::MessageType msgType)
{
  Ptr<CCNxBuffer> payload = Create<CCNxBuffer> (size, true);
  Ptr<CCNxPacket> packet;

  switch (msgType)
    {
    case CCNxMessage::ContentObject:
      {
        Ptr<CCNxContentObject> content = Create<CCNxContentObject> (name, payload);
        packet = CCNxPacket::CreateFromMessage (content);
        break;
      }
    case  CCNxMessage::Interest:
      {
        Ptr<CCNxInterest> interest = Create<CCNxInterest> (name, payload);
        packet = CCNxPacket::CreateFromMessage (interest);
        break;
      }

    default:
      {
        NS_ASSERT_MSG (false, "Unsupported msg type: " << msgType);
        Ptr<CCNxInterest> wtf = Create<CCNxInterest> (name, payload);
        packet = CCNxPacket::CreateFromMessage (wtf);
        break;
      }
    }
  return packet;


}


static void
GenerateTraffic (Ptr<CCNxPortal> source, uint32_t size, CCNxMessage::MessageType msgType,int pktCnt = 1)
{
  std::cout << "Packet Send    at=" << Simulator::Now().GetMicroSeconds() << "us, tx bytes=" << size << std::endl;

  //uniquify content name in case there is a content store
  std::ostringstream itoa; itoa << pktCnt;
  std::string nameString = contentString + itoa.str();
  Ptr<CCNxName> newname = Create<CCNxName> (nameString);
  Ptr<CCNxPacket> packet = CreatePacket (size, newname, msgType);
  source->Send (packet);


  if (pktCnt > 1)
    {
      // Every 1/2 second, send a packet by calling GenerateTraffic.
      Simulator::Schedule (Seconds (0.5), &GenerateTraffic, source, size, msgType, pktCnt - 1);
    }
  else
    {
      source->Close ();
    }
}

static void
PortalPrinter (Ptr<CCNxPortal> portal)
{
  Ptr<CCNxPacket> packet;
  while ((packet = portal->Recv ()))
    {
      std::cout << "Client Rx at=" << Simulator::Now ().GetMicroSeconds () << "us, name=" << *packet->GetMessage ()->GetName () << ", pkt type=" << (packet->GetMessage ()->GetMessageType () ? "Content" : "Interest") << std::endl;
    }
}

static void
RunSimulation (int numNodes, int k, bool enableCheck)
{
  Time::SetResolution (Time::MS);

  NodeContainer nodes;
  nodes.Create (numNodes);

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  std::vector<NodeContainer> containers;
  std::vector<NetDeviceContainer> deviceContainers;
  for (int i = 0; i < numNodes - 1; i++) {
      NodeContainer container = NodeContainer (nodes.Get (i), nodes.Get (i + 1));
      containers.push_back(container);

      NetDeviceContainer dc = pointToPoint.Install (container);
      deviceContainers.push_back(dc);
  }

  CCNxStackHelper ccnx;
  // Setup a CCNxL3Protocol on all the nodes

  ccnx.Install (nodes);
  for (int i = 0; i < numNodes - 1; i++) {
    ccnx.AddInterfaces(deviceContainers.at(i));
  }

  // Connect each router on the path
  Ptr<CCNxName> prefixName = Create<CCNxName> (prefixString);

  for (int i = 0; i < numNodes - 1; i++) {
      Ptr<Node> base = nodes.Get (i);
      Ptr<NetDevice> baseFace = base->GetDevice(0);
      Ptr<CCNxL3Protocol> ccnxl3source = base->GetObject<CCNxL3Protocol>();

      Ptr<Node> dest = nodes.Get (i + 1);
      Ptr<NetDevice> destFace = dest->GetDevice(0);
      Ptr<CCNxL3Protocol> ccnxl3dest = dest->GetObject<CCNxL3Protocol>();

      Ptr<CCNxConnectionDevice> connection = ccnxl3dest->AddNeighbor (baseFace->GetAddress (), destFace);
      ccnxl3dest->GetForwarder ()->AddRoute (connection, prefixName);
  }

  //////
  // key establishment

  // Generate the shared keys [XXX: we're just using the same fixed key here to make things easy, to start]
  CryptoPP::AutoSeededRandomPool prng;
  CryptoPP::SecByteBlock key(16);
  prng.GenerateBlock(key, key.size());

  // Extract the forwarders
  for (int i = 0; i < numNodes; i++) {
      Ptr<Node> node = nodes.Get (i);
      Ptr<NetDevice> face = node->GetDevice(0);
      Ptr<CCNxL3Protocol> ccnxl3 = node->GetObject<CCNxL3Protocol>();
      Ptr<CCNxForwarder> fwd = ccnxl3->GetForwarder();

      fwd->EnableIntegrityCheck(enableCheck);
      fwd->SetId(i);
      fwd->SetRadiiSize(k);
      for (int j = 0; j < numNodes; j++) {
          fwd->m_integrityKeys[j] = key;
      }
  }

  //// done

  // Now send packets and run simulation

  // Create a CCNxPortal on the sink and have it register a Name
  TypeId tid = TypeId::LookupByName ("ns3::ccnx::CCNxMessagePortalFactory");

  Ptr<CCNxPortal> node0Portal = CCNxPortal::CreatePortal (nodes.Get(0), tid);
  // the order of these does not matter.  Nothing is going to call the callback until we start
  // the simulator with Simulator::Run().
  node0Portal->SetRecvCallback (MakeCallback (&PortalPrinter));
  node0Portal->RegisterPrefix (prefixName);

  // Create a CCNxPortal on the source and have it send Interests
  Ptr<CCNxPortal> node3Portal = CCNxPortal::CreatePortal (nodes.Get(numNodes - 1), tid);
  node3Portal->SetRecvCallback (MakeCallback (&PortalPrinter));

  int pktCnt = 100;

  // Run some traffic
  GenerateTraffic (node3Portal, 100,  CCNxMessage::Interest,pktCnt);

  // Return content
  Simulator::Schedule (Seconds (0.1), &GenerateTraffic, node0Portal, 1500,  CCNxMessage::ContentObject,pktCnt);

  // Run the simulator and execute all the events
  Simulator::Run ();
  Simulator::Destroy ();

}

int
main (int argc, char *argv[])
{
    bool enable = true;
    int numNodes = 4;
    int k = 2;
    CommandLine cmd;
    cmd.AddValue ("enableCheck", "Enable integrity checks", enable);
    cmd.AddValue ("numNodes", "Number of nodes", numNodes);
    cmd.AddValue ("k", "Radii size", k);
    cmd.Parse (argc, argv);

    RunSimulation (numNodes, k, enable);
    return 0;
}
