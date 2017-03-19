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
#include "ccnx-codec-routertag.h"

using namespace ns3;
using namespace ns3::ccnx;


NS_LOG_COMPONENT_DEFINE ("CCNxCodecRouterTag");

NS_OBJECT_ENSURE_REGISTERED (CCNxCodecRouterTag);

static bool _registered = false;
static void
RegisterCodec()
{
    if (!_registered) {
	_registered = true;
	Ptr<CCNxCodecRouterTag> codec = CreateObject<CCNxCodecRouterTag>();
	CCNxCodecRegistry::PerHopRegisterCodec(CCNxRouterTags::GetTLVType(), codec);
    }
}

TypeId
CCNxCodecRouterTag::GetTypeId (void)
{
    RegisterCodec();
    static TypeId tid = TypeId ("ns3::ccnx::CCNxCodecRouterTag")
    .SetParent<CCNxCodecPerHopHeaderEntry> ()
    .SetGroupName ("CCNx")
    .AddConstructor<CCNxCodecRouterTag>();
    return tid;
}

TypeId
CCNxCodecRouterTag::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

CCNxCodecRouterTag::CCNxCodecRouterTag ()
{
  // empty
}

CCNxCodecRouterTag::~CCNxCodecRouterTag ()
{
  // empty
}

Ptr<CCNxPerHopHeaderEntry>
CCNxCodecRouterTag::Deserialize (Buffer::Iterator *inputIterator, size_t *bytesRead)
{
    NS_LOG_FUNCTION (this << &inputIterator);
    NS_ASSERT_MSG (inputIterator->GetSize () >= CCNxTlv::GetTLSize(), "Need to have at least 4 bytes to read");
    Buffer::Iterator *iterator = inputIterator;

    uint32_t numBytes = 0;
    uint16_t type = CCNxTlv::ReadType (*iterator);
    uint16_t length = CCNxTlv::ReadLength (*iterator);
    numBytes += CCNxTlv::GetTLSize();

    NS_LOG_DEBUG ("Type " << type << " length " << length << " bytesRead " << numBytes);

    Ptr<CCNxRouterTags> routerTags = Create<CCNxRouterTags>();
    for (int i = 0; i < length / 4; i++) {
        uint32_t tag = iterator->ReadNtohU32 ();
        routerTags->AppendTag(tag);
    }

    numBytes += length;
    *bytesRead = numBytes;

    return routerTags;
}

uint32_t
CCNxCodecRouterTag::GetSerializedSize (Ptr<CCNxPerHopHeaderEntry> perhopEntry)
{
    Ptr<CCNxRouterTags> tags = DynamicCast<CCNxRouterTags, CCNxPerHopHeaderEntry> (perhopEntry);
    int numTags = tags->NumberOfTags();
    uint32_t length = CCNxTlv::GetTLSize () + (4 * numTags); // each tag is 4 bytes
    return length;
}

void
CCNxCodecRouterTag::Serialize (Ptr<CCNxPerHopHeaderEntry> perhopEntry, Buffer::Iterator *outputIterator)
{
    NS_LOG_FUNCTION (this << &outputIterator);

    uint16_t bytes = (uint16_t) GetSerializedSize (perhopEntry);
    NS_ASSERT_MSG (bytes >= CCNxTlv::GetTLSize(), "Serialized size must be at least 4 bytes");

    // -4 because it includes the T_ROUTER_TAG TLV.
    CCNxTlv::WriteTypeLength (*outputIterator, CCNxSchemaV1::T_ROUTER_TAG, bytes - CCNxTlv::GetTLSize ());

    Ptr<CCNxRouterTags> tags = DynamicCast<CCNxRouterTags, CCNxPerHopHeaderEntry> (perhopEntry);
    std::vector<int> theTags = tags->GetTags();

    for (std::vector<int>::iterator itr = theTags.begin(); itr != theTags.end(); itr++) {
        outputIterator->WriteHtonU32 (*itr);
    }
}

void
CCNxCodecRouterTag::Print (Ptr<CCNxPerHopHeaderEntry> perhopEntry, std::ostream &os) const
{
  if (perhopEntry)
  {
      Ptr<CCNxRouterTags> tags = DynamicCast<CCNxRouterTags, CCNxPerHopHeaderEntry> (perhopEntry);
      tags->Print(os);
  }
  else
  {
      os << "\nNo router tags header";
  }
}
