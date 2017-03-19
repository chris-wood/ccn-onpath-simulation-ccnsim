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
#include "ns3/assert.h"
#include "ccnx-forwarder.h"
#include "ns3/ccnx-l3-protocol.h"

#include <model/messages/ccnx-packetmac.h>
#include <model/messages/ccnx-routertag.h>
#include <model/messages/ccnx-buffer.h>
#include <model/packets/ccnx-packet.h>
#include <model/packets/standard/ccnx-schema-v1.h>

using namespace ns3;
using namespace ns3::ccnx;

NS_LOG_COMPONENT_DEFINE ("CCNxForwarder");
NS_OBJECT_ENSURE_REGISTERED (CCNxForwarder);

/**
 * Used as a default callback for m_routeCallback in case the user does not set it.
 */
static void
NullRouteCallback (Ptr<CCNxPacket>, Ptr<CCNxConnection>, enum CCNxRoutingError::RoutingErrno, Ptr<CCNxConnectionList>)
{
  NS_ASSERT_MSG (false, "You must set the Route Callback via SetRouteCallback()");
}

void
CCNxForwarder :: AppendRouterTag(Ptr<CCNxPacket> packet) const
{
    Ptr<CCNxPerHopHeader> header = packet->GetPerhopHeaders();

    // Try to append to the running list, if it's there...
    for (size_t i = 0; i < header->size(); i++) {
        Ptr<CCNxPerHopHeaderEntry> entry = header->GetHeader(i);
        if (entry->GetInstanceTLVType() == CCNxSchemaV1::T_ROUTER_TAG) { // GetTLVType
            Ptr<CCNxRouterTags> tagBag = entry->GetObject<CCNxRouterTags>();

            // Don't exceed the radii size
            if (tagBag->GetTags().size() == m_raddiSize) {
                tagBag->DropTag();
            }
            tagBag->AppendTag(m_id);

            return;
        }
    }

    // Otherwise, create a new list and add it to the packet
    Ptr<CCNxRouterTags> tagBag = Create<CCNxRouterTags>();
    tagBag->AppendTag(m_id);
    header->AddHeader(tagBag);
}

bool
CCNxForwarder :: PreProcessPacketMAC (Ptr<CCNxPacket> packet) const
{
    if (!m_integrityChecked) {
        return true;
    }

    Ptr<CCNxPerHopHeader> header = packet->GetPerhopHeaders();
    for (size_t i = 0; i < header->size(); i++) {
        Ptr<CCNxPerHopHeaderEntry> entry = header->GetHeader(i);
        if (entry->GetInstanceTLVType() == CCNxSchemaV1::T_PACKET_MAC) { // GetTLVType
            Ptr<CCNxPacketMAC> macEntry = entry->GetObject<CCNxPacketMAC>();

            // Try to verify each of the packet MACs
            for (size_t j = 0; j < macEntry->GetMACCount(); j++) {

                // Extract the MAC info from the head of the list
                Ptr<CCNxMACList> list = macEntry->GetMACList(j);
                Ptr<CCNxMAC> mac = list->GetMACAtIndex(0);
                int keyId = mac->GetID();
                std::string macBuffer = mac->GetMAC();

                // Attempt to verify it
                if (!this->VerifySinglePacketMAC(packet, keyId, macBuffer)) {
                    NS_LOG_DEBUG("failed verifying the MAC");
                    return false;
                }

                // If the MAC was valid, then drop the first MAC from that list
                list->DropMACAtIndex(0);
                if (list->Size() == 0) {
                    macEntry->DropMACList(j);
                }
            }
        }
    }

    return true;
}

void
CCNxForwarder :: PostProcessPacketMAC (Ptr<CCNxPacket> packet, std::vector<int> keyIds) const
{
    if (!m_integrityChecked) {
        return;
    }

    if (keyIds.size() == 0) {
        return; // there's nothing to do here!
    }

    // Create a new MAC list for each of the key IDs
    Ptr<CCNxMACList> macList = Create<CCNxMACList>();
    for (std::vector<int>::iterator itr = keyIds.begin(); itr != keyIds.end(); itr++) {
        std::string macBuffer = this->ComputePacketMAC(packet, *itr);
        Ptr<CCNxMAC> mac = Create<CCNxMAC>(*itr, macBuffer);
        macList->AppendMAC(mac);
    }

    Ptr<CCNxPerHopHeader> header = packet->GetPerhopHeaders();
    bool added = false;
    for (size_t i = 0; i < header->size(); i++) {
        Ptr<CCNxPerHopHeaderEntry> entry = header->GetHeader(i);
        if (entry->GetInstanceTLVType() == CCNxSchemaV1::T_PACKET_MAC) { // GetTLVType
            added = true;
            Ptr<CCNxPacketMAC> macEntry = entry->GetObject<CCNxPacketMAC>();
            macEntry->AppendMACList(macList);
        }
    }

    // If there was no matching entry, then don't add anything...
    if (!added) {
        std::vector< Ptr<CCNxMACList> > macListList;
        macListList.push_back(macList);
        Ptr<CCNxPacketMAC> macEntry = Create<CCNxPacketMAC>(macListList);
        header->AddHeader(macEntry);
    }
}

static std::string
packetToString(Ptr<CCNxPacket> packet)
{
    Ptr<CCNxMessage> message = packet->GetMessage();
    std::ostringstream stream;
    stream << *message;
    std::string plain = stream.str();
    return plain;
}

bool
CCNxForwarder :: VerifySinglePacketMAC (Ptr<CCNxPacket> packet, int keyId, std::string MAC) const
{
    if (m_integrityKeys.find(keyId) == m_integrityKeys.end()) {
        return false;
    }

    SecByteBlock key = m_integrityKeys.find(keyId)->second;
    try {
        HMAC<SHA256> hmac(key, key.size());
        const int flags = HashVerificationFilter::THROW_EXCEPTION | HashVerificationFilter::HASH_AT_END;
        std::string plain = packetToString(packet);

        // std::cout << m_id << " verifying: " << plain << std::endl;
        // std::cout << m_id << " mac = " << MAC << std::endl;

        StringSource(plain + MAC, true,
            new HashVerificationFilter(hmac, NULL, flags)
        );

        return true; // if we get here, it verified correctly
    } catch (const CryptoPP::Exception& e) {
        return false;
    }

    // won't get here
    return false;
}

std::string
CCNxForwarder :: ComputePacketMAC (Ptr<CCNxPacket> packet, int keyId) const
{
    if (m_integrityKeys.find(keyId) == m_integrityKeys.end()) {
        return NULL;
    }

    SecByteBlock key = m_integrityKeys.find(keyId)->second;
    std::string mac, encoded;

    try {
        HMAC<SHA256> hmac(key, key.size());
        std::string plain = packetToString(packet);

        // std::cout << m_id << " tagging: " << plain << std::endl;

        StringSource ss2(plain, true,
            new HashFilter(hmac,
                new StringSink(mac)
            )
        );
        // std::cout << m_id << " mac = " << mac << std::endl;
    } catch (const CryptoPP::Exception& e) {
        return NULL;
    }

    return mac;
}

void
CCNxForwarder::AddIntegrityKey(int id, SecByteBlock block)
{
    m_integrityKeys[id] = block;
}

void
CCNxForwarder::SetId(int id)
{
    m_id = id;
}

void
CCNxForwarder::EnableIntegrityCheck(bool enable)
{
    m_integrityChecked = enable;
}

void
CCNxForwarder::SetRadiiSize(int size)
{
    m_raddiSize = size;
}

TypeId
CCNxForwarder::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::ccnx::CCNxForwarder")
    .SetParent<Object> ()
    .SetGroupName ("CCNx");
  return tid;
}

CCNxForwarder::CCNxForwarder ()
  : m_routeCallback (MakeCallback (&NullRouteCallback))
{

}

CCNxForwarder::~CCNxForwarder ()
{

}
void
CCNxForwarder::SetNode (Ptr<Node> node)
{
  m_node = node;
  m_ccnx = node->GetObject<CCNxL3Protocol> ();
  NS_ASSERT_MSG (m_ccnx != NULL, "Got nul CCNxL3Protcol from node " << node);
}

void
CCNxForwarder::SetRouteCallback (RouteCallback callback)
{
  m_routeCallback = callback;
}
