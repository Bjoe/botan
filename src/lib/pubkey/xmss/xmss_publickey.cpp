/*
 * XMSS Public Key
 * An XMSS: Extended Hash-Based Siganture public key.
 * The XMSS public key does not support the X509 standard. Instead the
 * raw format described in [1] is used.
 *
 * [1] XMSS: Extended Hash-Based Signatures,
 *     Request for Comments: 8391
 *     Release: May 2018.
 *     https://datatracker.ietf.org/doc/rfc8391/
 *
 * (C) 2016,2017 Matthias Gierlings
 *
 * Botan is released under the Simplified BSD License (see license.txt)
 **/

#include <botan/internal/xmss_verification_operation.h>
#include <botan/xmss_publickey.h>

namespace Botan {

XMSS_PublicKey::XMSS_PublicKey(const std::vector<uint8_t>& raw_key)
   : m_xmss_params(XMSS_PublicKey::deserialize_xmss_oid(raw_key)),
     m_wots_params(m_xmss_params.ots_oid())
   {
   if(raw_key.size() < XMSS_PublicKey::size())
      {
      throw Decoding_Error("Invalid XMSS public key size detected.");
      }

   // extract & copy root from raw key.
   m_root.clear();
   m_root.reserve(m_xmss_params.element_size());
   auto begin = raw_key.begin() + sizeof(uint32_t);
   auto end = begin + m_xmss_params.element_size();
   std::copy(begin, end, std::back_inserter(m_root));

   // extract & copy public seed from raw key.
   begin = end;
   end = begin + m_xmss_params.element_size();
   m_public_seed.clear();
   m_public_seed.reserve(m_xmss_params.element_size());
   std::copy(begin, end, std::back_inserter(m_public_seed));
   }

XMSS_Parameters::xmss_algorithm_t
XMSS_PublicKey::deserialize_xmss_oid(const std::vector<uint8_t>& raw_key)
   {
   if(raw_key.size() < 4)
      {
      throw Decoding_Error("XMSS signature OID missing.");
      }

   // extract and convert algorithm id to enum type
   uint32_t raw_id = 0;
   for(size_t i = 0; i < 4; i++)
      { raw_id = ((raw_id << 8) | raw_key[i]); }

   return static_cast<XMSS_Parameters::xmss_algorithm_t>(raw_id);
   }

std::unique_ptr<PK_Ops::Verification>
XMSS_PublicKey::create_verification_op(const std::string&,
                                       const std::string& provider) const
   {
   if(provider == "base" || provider.empty())
      {
      return std::unique_ptr<PK_Ops::Verification>(
                new XMSS_Verification_Operation(*this));
      }
   throw Provider_Not_Found(algo_name(), provider);
   }

std::vector<uint8_t> XMSS_PublicKey::raw_public_key() const
   {
   std::vector<uint8_t> result
      {
      static_cast<uint8_t>(m_xmss_params.oid() >> 24),
      static_cast<uint8_t>(m_xmss_params.oid() >> 16),
      static_cast<uint8_t>(m_xmss_params.oid() >>  8),
      static_cast<uint8_t>(m_xmss_params.oid())
      };

   std::copy(m_root.begin(), m_root.end(), std::back_inserter(result));
   std::copy(m_public_seed.begin(),
             m_public_seed.end(),
             std::back_inserter(result));

   return result;
   }

}
