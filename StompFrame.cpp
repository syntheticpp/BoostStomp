/*
 BoostStomp - a STOMP (Simple Text Oriented Messaging Protocol) client
----------------------------------------------------
Copyright (c) 2012 Elias Karakoulakis <elias.karakoulakis@gmail.com>

SOFTWARE NOTICE AND LICENSE

BoostStomp is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published
by the Free Software Foundation, either version 3 of the License,
or (at your option) any later version.

BoostStomp is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with BoostStomp.  If not, see <http://www.gnu.org/licenses/>.

for more information on the LGPL, see:
http://en.wikipedia.org/wiki/GNU_Lesser_General_Public_License
*/

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#include "BoostStomp.hpp"
#include "helpers.h"

namespace STOMP {

	using namespace boost;
	using namespace boost::assign;

  /*
   * Escaping is needed to allow header keys and values to contain those frame header
   * delimiting octets as values. The CONNECT and CONNECTED frames do not escape the
   * colon or newline octets in order to remain backward compatible with STOMP 1.0.
   * C style string literal escapes are used to encode any colons and newlines that
   * are found within the UTF-8 encoded headers. When decoding frame headers, the
   * following transformations MUST be applied:
   *
   * \n (octet 92 and 110) translates to newline (octet 10)
   * \c (octet 92 and 99) translates to : (octet 58)
   * \\ (octet 92 and 92) translates to \ (octet 92)
   */
  string& encode_header_token(string& str) {
	  boost::algorithm::replace_all(str, "\n", "\\n");
	  boost::algorithm::replace_all(str, ":", "\\c");
	  boost::algorithm::replace_all(str, "\\", "\\\\");
	  return(str);
  };

  string& decode_header_token(string& str) {
	  boost::algorithm::replace_all(str, "\\n", "\n");
	  boost::algorithm::replace_all(str, "\\c", ":");
	  boost::algorithm::replace_all(str, "\\\\", "\\");
	  return(str);
  };

  boost::asio::streambuf& Frame::encode()
  // -------------------------------------
  {
	// prepare an output stream
	ostream os(&m_request);
	// step 1. write the command
	if (m_command.length() > 0) {
	  os << m_command << "\n";
	} else {
	  throw("stomp_write: command not set!!");
	}
	// step 2. Write the headers (key-value pairs)
	if( m_headers.size() > 0 ) {
	  for ( hdrmap::iterator it = m_headers.begin() ; it != m_headers.end(); it++ ) {
		  string key = (*it).first;
		  string val = (*it).second;
		os << encode_header_token(key)
			<< ":"
			<< encode_header_token(val)
			<< "\n";
	  }
	}
	// special header: content-length
	if( m_body.v.size() > 0 ) {
	  os << "content-length:" << m_body.v.size() << "\n";
	}
	// write newline signifying end of headers
	os << "\n";
	// step 3. Write the body
	if( m_body.v.size() > 0 ) {
		m_request.sputn(m_body.v.data(), m_body.v.size());
	}
	// write terminating NULL char
	m_request.sputc('\0');
	return(m_request);
  };

}
