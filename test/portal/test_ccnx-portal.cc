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

#include "ns3/test.h"
#include "ns3/ccnx-portal.h"
#include "ns3/ccns3Sim-module.h"

#include "../TestMacros.h"

using namespace ns3;
using namespace ns3::ccnx;

namespace TestSuiteCCNxPortal {

BeginTest (Constructor)
{
  NodeContainer c;
  c.Create (1);

  CCNxStackHelper ccnx;
  ccnx.Install (c);

  // Create a CCNxPortal and have it register a Name
  TypeId tid = TypeId::LookupByName ("ns3::ccnx::CCNxMessagePortalFactory");
  Ptr<Node> node = c.Get (0);

  Ptr<CCNxPortal> portal = CCNxPortal::CreatePortal (node, tid);

  bool exists = (portal);
  NS_TEST_EXPECT_MSG_EQ (exists, true, "Gut null pointer");

  portal->Close ();
  portal = 0;
}
EndTest ()

static void
PortalReceive (Ptr<CCNxPortal> portal)
{
  Ptr<CCNxPacket> packet;
  while ((packet = portal->Recv ()))
    {
      Ptr<CCNxMessage> ccnxMessage = packet->GetMessage ();
      NS_ASSERT_MSG (ccnxMessage, "Got null message from packet");

      printf ("Packet received successfully \n");
    }
}

static void
PortalSent (Ptr<CCNxPortal> portal, Ptr<CCNxPacket> packet)
{
  printf ("PortalSent invoked \n");
}

BeginTest (SendRecv)
{
  NodeContainer c;
  c.Create (1);

  CCNxStackHelper ccnx;
  ccnx.Install (c);

  // Create a CCNxPortal and have it register a Name
  TypeId tid = TypeId::LookupByName ("ns3::ccnx::CCNxMessagePortalFactory");
  Ptr<Node> node = c.Get (0);

  Ptr<CCNxPortal> sendPortal = CCNxPortal::CreatePortal (node, tid);
  Ptr<CCNxPortal> recvPortal = CCNxPortal::CreatePortal (node, tid);
  recvPortal->SetRecvCallback (MakeCallback (&PortalReceive));
  Ptr<CCNxName> recvName = Create<CCNxName> ("ccnx:/name=foo/name=sink");
  recvPortal->RegisterPrefix (recvName);

  Ptr<CCNxBuffer> payload = Create<CCNxBuffer> (100, true);
  Ptr<CCNxInterest> interest = Create<CCNxInterest> (Create<CCNxName> ("ccnx:/name=foo/name=sink"), payload);
  Ptr<CCNxPacket> packet = CCNxPacket::CreateFromMessage (interest);

  sendPortal->SetDataSentCallback (MakeCallback (&PortalSent));

  if ((sendPortal->Send (packet)) == true)
    {
      printf ("Packet sent successfully \n");
    }

  recvPortal->UnregisterPrefix (recvName);
  sendPortal->Close ();
  recvPortal->Close ();
  sendPortal = 0;
  recvPortal = 0;
}
EndTest ()

/**
 * @ingroup ccnx-test
 *
 * Test Suite for CCNxPortal
 */
static class TestSuiteCCNxPortal : public TestSuite
{
public:
  TestSuiteCCNxPortal () : TestSuite ("ccnx-portal", UNIT)
  {
    AddTestCase (new Constructor (), TestCase::QUICK);
    AddTestCase (new SendRecv (), TestCase::QUICK);
  }
} g_TestSuiteCCNxPortal;

} // namespace TestSuiteCCNxPortal
