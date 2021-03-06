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

#ifndef CCNS3_CCNxPacketMAC_H
#define CCNS3_CCNxPacketMAC_H

#include "ns3/object.h"
#include "ns3/ccnx-perhopheaderentry.h"
#include "ns3/ccnx-time.h"
#include "ccnx-buffer.h"

namespace ns3  {
namespace ccnx {

class CCNxMAC : public Object {
public:
    CCNxMAC (int id, std::string buffer);
    virtual ~CCNxMAC ();

    std::string GetMAC(void) const;
    int GetID(void) const;
private:
    int m_id;
    std::string m_bytes;
};

class CCNxMACList : public Object {
public:
    CCNxMACList ();
    virtual ~CCNxMACList ();

    void AppendMAC(Ptr<CCNxMAC> mac);
    void DropMACAtIndex(int index);
    Ptr<CCNxMAC> GetMACAtIndex(int index) const;
    int Size() const;
private:
    std::vector< Ptr<CCNxMAC> > m_macs;
};


/**
 * @ingroup ccnx-messages
 *
 * Class representation of InterestLifetime Per Hop Header Entry.
 */
class CCNxPacketMAC : public CCNxPerHopHeaderEntry {
public:
  static TypeId GetTypeId (void);

  virtual TypeId GetInstanceTypeId (void) const;

  /**
   * Constructor for CCNxPacketMAC.
   *
   */
  CCNxPacketMAC (std::vector< Ptr<CCNxMACList> > macs);

  /**
   * Destructor for CCNxPacketMAC
   */
  virtual ~CCNxPacketMAC ();

  /**
   * Returns the Per Hop Header Type ( = 6) for Packet MAC
   */
  static uint16_t GetTLVType(void);

  virtual uint16_t GetInstanceTLVType (void) const;

  /**
   * Determines if the given InterestLifetime is equivalent to this InterestLifetime.
   *
   * Two InterestLifetimes are equivalent if the time values are exactly equal
   */
  bool Equals (const Ptr<CCNxPerHopHeaderEntry> other) const;

  /**
   * Determines if the given InterestLifetime is equivalent to this InterestLifetime.
   *
   * Two InterestLifetimes are equivalent if time values are exactly equal
   */
  bool Equals (CCNxPerHopHeaderEntry const &other) const;

  /**
   * Get the number of MACs in this PacketMAC
   */
  size_t GetMACCount () const;

  /**
   * Retrieve the i-th MAC list from this packet.
   */
  Ptr<CCNxMACList> GetMACList (size_t i) const;

  /**
   * Append a new MAC list to this packet.
   */
  void AppendMACList (Ptr<CCNxMACList> macList);

  /**
   * Drop the i-th MAC from this packet.
   */
  void DropMACList (size_t i);

  /**
   * Retrieve the vector of MACs in this entry.
   */
  std::vector<Ptr<CCNxMACList> > GetMACs (void) const;

  /**
   * Prints a string like this:
   *
   * { Interest Lifetime timeValue T }
   *
   * Where T is the uint64_t value of the CCNxTime.
   *
   * @param [in] os ostream object
   * @return ostream object
   */

   virtual std::ostream & Print(std::ostream &os) const;

protected:

  static const uint32_t m_packetMACTLVType;
  std::vector<Ptr<CCNxMACList> > m_packetMACs;
};

}
}

#endif // CCNS3_CCNxPacketMAC_H
