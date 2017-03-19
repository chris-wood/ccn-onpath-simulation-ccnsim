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
#include "ns3/ccnx-tlv.h"
#include "ns3/ccnx-schema-v1.h"
#include "ccnx-codec-registry.h"
#include "ccnx-codec-packetmac.h"

#include <iostream>
#include <string>
#include <sstream>
#include <vector>

using namespace ns3;
using namespace ns3::ccnx;


NS_LOG_COMPONENT_DEFINE ("CCNxCodecPacketMAC");

NS_OBJECT_ENSURE_REGISTERED (CCNxCodecPacketMAC);

static bool _registered = false;
static void
RegisterCodec()
{
    if (!_registered) {
	_registered = true;
	Ptr<CCNxCodecPacketMAC> codec = CreateObject<CCNxCodecPacketMAC>();
	CCNxCodecRegistry::PerHopRegisterCodec(CCNxPacketMAC::GetTLVType(), codec);
    }
}

TypeId
CCNxCodecPacketMAC::GetTypeId (void)
{
    RegisterCodec();
    static TypeId tid = TypeId ("ns3::ccnx::CCNxCodecPacketMAC")
    .SetParent<CCNxCodecPerHopHeaderEntry> ()
    .SetGroupName ("CCNx")
    .AddConstructor<CCNxCodecPacketMAC>();
    return tid;
}

TypeId
CCNxCodecPacketMAC::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

CCNxCodecPacketMAC::CCNxCodecPacketMAC ()
{
  // empty
}

CCNxCodecPacketMAC::~CCNxCodecPacketMAC ()
{
  // empty
}

Ptr<CCNxPerHopHeaderEntry>
CCNxCodecPacketMAC::Deserialize (Buffer::Iterator *inputIterator, size_t *bytesRead)
{
    NS_LOG_FUNCTION (this << &inputIterator);
    NS_ASSERT_MSG (inputIterator->GetSize () >= CCNxTlv::GetTLSize(), "Need to have at least 4 bytes to read");
    Buffer::Iterator *iterator = inputIterator;

    uint32_t numBytes = 0;
    uint16_t type = CCNxTlv::ReadType (*iterator);
    uint16_t length = CCNxTlv::ReadLength (*iterator);
    numBytes += CCNxTlv::GetTLSize();

    NS_LOG_DEBUG ("Type " << type << " length " << length << " bytesRead " << numBytes);

    std::vector< Ptr<CCNxMACList> > macLists;

    // Build up the list of MAC lists
    while (numBytes < length) {
        uint16_t nestedType = CCNxTlv::ReadType (*iterator);
        uint16_t nestedLength = CCNxTlv::ReadLength (*iterator);
        numBytes += CCNxTlv::GetTLSize ();

        NS_LOG_DEBUG ("Nested  type " << nestedType << " length " << nestedLength << " bytesRead " << bytesRead);

        NS_ASSERT_MSG (*bytesRead + nestedLength <= length, "length goes beyond end of messageLength");

        // Read in the entire list
        Ptr<CCNxMACList> macList = Create<CCNxMACList>();
        uint32_t listBytesRead = 0;
        while (listBytesRead < nestedLength) {
            uint16_t entryType = CCNxTlv::ReadType (*iterator);
            uint16_t entryLength = CCNxTlv::ReadLength (*iterator);
            listBytesRead += CCNxTlv::GetTLSize ();

            NS_LOG_DEBUG ("List entry type " << entryType << " length " << entryLength);

            uint32_t id = iterator->ReadNtohU32();

            // TODO(cawood): this code is somehow broken -- where's the bug?
            // Ptr<CCNxBuffer> payload = Create<CCNxBuffer> (0);
            // payload->AddAtStart (entryLength - 4);
            // iterator->Read (payload->Begin (), entryLength - 4);

            uint16_t payloadLength = entryLength - 4;
            std::ostringstream convert;
            for (int i = 0; i < payloadLength; i++) {
                convert << iterator->ReadU8();
            }
            std::string macBuffer = convert.str();

            Ptr<CCNxMAC> mac = Create<CCNxMAC>(id, macBuffer);
            macList->AppendMAC(mac);

            listBytesRead += entryLength;
        }

        numBytes += nestedLength;
        macLists.push_back(macList);
    }

    *bytesRead += numBytes;

    Ptr<CCNxPacketMAC> packetMac = Create<CCNxPacketMAC>(macLists);
    return packetMac;
}

uint32_t
CCNxCodecPacketMAC::GetSerializedSize (Ptr<CCNxPerHopHeaderEntry> perhopEntry)
{
    Ptr<CCNxPacketMAC> macBag = DynamicCast<CCNxPacketMAC, CCNxPerHopHeaderEntry> (perhopEntry);

    uint32_t length = CCNxTlv::GetTLSize (); // T_PACKET_MAC

    size_t numLists = macBag->GetMACCount();
    for (size_t i = 0; i < numLists; i++) {
        length += CCNxTlv::GetTLSize (); // T_PACKET_MAC_LIST
        Ptr<CCNxMACList> macList = macBag->GetMACList(i);

        uint32_t listSize = macList->Size();
        for (uint32_t j = 0; j < listSize; j++) {
            length += CCNxTlv::GetTLSize (); // T_PACKET_MAC_LIST_ENTRY
            Ptr<CCNxMAC> mac = macList->GetMACAtIndex(j);
            std::string buffer = mac->GetMAC();

            length += 4; // ID
            length += buffer.size();
        }
    }

    return length;
}

void
CCNxCodecPacketMAC::Serialize (Ptr<CCNxPerHopHeaderEntry> perhopEntry, Buffer::Iterator *outputIterator)
{
    NS_LOG_FUNCTION (this << &outputIterator);

    uint16_t bytes = (uint16_t) GetSerializedSize (perhopEntry);
    NS_ASSERT_MSG (bytes >= CCNxTlv::GetTLSize(), "Serialized size must be at least 4 bytes");

    // -4 because it includes the T_PACKET_MAC TLV.
    CCNxTlv::WriteTypeLength (*outputIterator, CCNxSchemaV1::T_PACKET_MAC, bytes - CCNxTlv::GetTLSize ());

    Ptr<CCNxPacketMAC> macBag = DynamicCast<CCNxPacketMAC, CCNxPerHopHeaderEntry> (perhopEntry);
    size_t numLists = macBag->GetMACCount();
    for (size_t i = 0; i < numLists; i++) {
        Ptr<CCNxMACList> macList = macBag->GetMACList(i);

        // Count the size of the macList
        uint32_t listSize = macList->Size();
        uint32_t listSerializedSize = 0;
        for (uint32_t j = 0; j < listSize; j++) {
            listSerializedSize += CCNxTlv::GetTLSize (); // T_PACKET_MAC_LIST_ENTRY
            Ptr<CCNxMAC> mac = macList->GetMACAtIndex(j);
            std::string buffer = mac->GetMAC();

            listSerializedSize += 4; // ID
            listSerializedSize += buffer.size();
        }

        // Write the TL container
        CCNxTlv::WriteTypeLength (*outputIterator, CCNxSchemaV1::T_PACKET_MAC_LIST, listSerializedSize);

        // And actually write each entry in the list
        for (uint32_t j = 0; j < listSize; j++) {
            Ptr<CCNxMAC> mac = macList->GetMACAtIndex(j);
            std::string buffer = mac->GetMAC();

            CCNxTlv::WriteTypeLength (*outputIterator, CCNxSchemaV1::T_PACKET_MAC_LIST_ENTRY, 4 + buffer.size());
            outputIterator->WriteHtonU32 (mac->GetID());
            for (int k = 0; k < buffer.size(); k++) {
                outputIterator->WriteU8(buffer[k]);
            }
        }
    }
}

void
CCNxCodecPacketMAC::Print (Ptr<CCNxPerHopHeaderEntry> perhopEntry, std::ostream &os) const
{
  if (perhopEntry)
  {
      Ptr<CCNxPacketMAC> mac = DynamicCast<CCNxPacketMAC, CCNxPerHopHeaderEntry> (perhopEntry);
      mac->Print(os);
  }
  else
  {
      os << "\nNo router tags header";
  }
}
