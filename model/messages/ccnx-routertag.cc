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
#include "ccnx-routertag.h"
#include "ns3/ccnx-perhopheaderentry.h"

#include "ns3/ccnx-tlv.h"
#include "ns3/ccnx-schema-v1.h"

using namespace ns3;
using namespace ns3::ccnx;

NS_LOG_COMPONENT_DEFINE ("CCNxRouterTags");

NS_OBJECT_ENSURE_REGISTERED(CCNxRouterTags);

const uint32_t CCNxRouterTags::m_routerTagTLVType = 0x0005;

TypeId
CCNxRouterTags::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::ccnx::CCNxRouterTags")
    .SetParent<CCNxPerHopHeaderEntry> ()
    .SetGroupName ("CCNx");
  return tid;
}

TypeId
CCNxRouterTags::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

CCNxRouterTags :: CCNxRouterTags ()
{
}

CCNxRouterTags::~CCNxRouterTags ()
{
  // empty
}

uint16_t
CCNxRouterTags :: GetTLVType(void)
{
  return m_routerTagTLVType;
}

uint16_t
CCNxRouterTags :: GetInstanceTLVType (void) const
{
  return GetTLVType ();
}

std::vector<int>
CCNxRouterTags :: GetTags () const
{
    return m_tags;
}

int
CCNxRouterTags :: NumberOfTags () const
{
    return m_tags.size();
}

void
CCNxRouterTags :: AppendTag (int tag)
{
    m_tags.push_back(tag);
}

void
CCNxRouterTags :: DropTag ()
{
    m_tags.erase(m_tags.begin());
}

bool
CCNxRouterTags::Equals (const Ptr<CCNxPerHopHeaderEntry> other) const
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
CCNxRouterTags::Equals (CCNxPerHopHeaderEntry const &other) const
{
  bool result = false;
  const CCNxRouterTags *ptr = dynamic_cast<const CCNxRouterTags *>(&other);
  if (m_tags == ptr->GetTags())
  {
    result = true;
  }
  return result;
}

std::ostream &
CCNxRouterTags::Print(std::ostream &os) const
{
  os << "{ RouterTags {";
  for (int i = 0; i < m_tags.size(); i++) {
      os << "[" << m_tags.at(i) << "]";
  }
  os << " }";
  return os;
}
