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

#include "ns3/log.h"
#include "ccnx-packetmac.h"
#include "ns3/ccnx-perhopheaderentry.h"

#include "ns3/ccnx-tlv.h"
#include "ns3/ccnx-schema-v1.h"

using namespace ns3;
using namespace ns3::ccnx;

NS_LOG_COMPONENT_DEFINE ("CCNxPacketMAC");

NS_OBJECT_ENSURE_REGISTERED(CCNxPacketMAC);

const uint32_t CCNxPacketMAC::m_packetMACTLVType = 0x0004;

//// CCNxMAC

CCNxMAC::CCNxMAC (int id, Ptr<CCNxBuffer> buffer)
{
    m_id = id;
    m_bytes = buffer;
}

CCNxMAC::~CCNxMAC ()
{
    // delete (m_bytes);
}

int
CCNxMAC :: GetID(void) const
{
    return m_id;
}

Ptr<CCNxBuffer>
CCNxMAC :: GetMAC(void) const
{
    return m_bytes;
}

//// CCNxMACList

CCNxMACList::CCNxMACList()
{
}
CCNxMACList::~CCNxMACList()
{
}

void
CCNxMACList::AppendMAC(Ptr<CCNxMAC> mac)
{
    m_macs.push_back(mac);
}

void
CCNxMACList::DropMACAtIndex(int index)
{
    m_macs.erase(m_macs.begin() + index, m_macs.begin() + index + 1);
}

int
CCNxMACList::Size() const
{
    return m_macs.size();
}

Ptr<CCNxMAC>
CCNxMACList::GetMACAtIndex(int index) const
{
    return m_macs.at(index);
}

//// meat and potatoes

TypeId
CCNxPacketMAC::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::ccnx::CCNxPacketMAC")
    .SetParent<CCNxPerHopHeaderEntry> ()
    .SetGroupName ("CCNx");
  return tid;
}

TypeId
CCNxPacketMAC::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

CCNxPacketMAC :: CCNxPacketMAC (std::vector< Ptr<CCNxMACList> > macs)
{
    for (std::vector<Ptr<CCNxMACList> >::iterator itr = macs.begin(); itr != macs.end(); itr++) {
        m_packetMACs.push_back(*itr);
    }
}

CCNxPacketMAC::~CCNxPacketMAC ()
{
  // empty
}

uint16_t
CCNxPacketMAC :: GetTLVType(void)
{
  return m_packetMACTLVType;
}

uint16_t
CCNxPacketMAC :: GetInstanceTLVType (void) const
{
  return GetTLVType ();
}

size_t
CCNxPacketMAC :: GetMACCount (void) const
{
  return m_packetMACs.size();
}

Ptr<CCNxMACList>
CCNxPacketMAC :: GetMACList (size_t i) const
{
  return m_packetMACs.at(i);
}

void
CCNxPacketMAC :: AppendMACList (Ptr<CCNxMACList> macList)
{
    m_packetMACs.push_back(macList);
}

void
CCNxPacketMAC :: DropMACList (size_t i)
{
    m_packetMACs.erase(m_packetMACs.begin() + i, m_packetMACs.begin() + i + 1);
}

std::vector<Ptr<CCNxMACList> >
CCNxPacketMAC :: GetMACs(void) const
{
  return m_packetMACs;
}

bool
CCNxPacketMAC::Equals (const Ptr<CCNxPerHopHeaderEntry> other) const
{
  if (other)
    {
      return Equals (*other);
    }
  else
    {
      return false;
    }
}

bool
CCNxPacketMAC::Equals (CCNxPerHopHeaderEntry const &other) const
{
  bool result = false;
  const CCNxPacketMAC *ptr = dynamic_cast<const CCNxPacketMAC *>(&other);
  if (m_packetMACs == ptr->GetMACs())
  {
    result = true;
  }
  return result;
}

std::ostream &
CCNxPacketMAC::Print(std::ostream &os) const
{
  os << "{ PacketMACS { ";
  os << m_packetMACs.size();
  os << " } }";
  return os;
}
