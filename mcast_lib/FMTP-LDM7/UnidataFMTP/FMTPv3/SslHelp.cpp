/**
 * SSL helper functions.
 *
 *        File: SslHelp.h
 *  Created on: Sep 8, 2020
 *      Author: Steven R. Emmerson
 */

#include "SslHelp.h"

#include <openssl/err.h>
#include <openssl/rand.h>

#include <cerrno>
#include <fcntl.h>
#include <system_error>
#include <unistd.h>

namespace SslHelp {

/**
 * Initializes the OpenSSL pseudo-random number generator (PRNG).
 *
 * @param[in] numBytes         Number of bytes from "/dev/random" to initialize
 *                             the PRNG with
 * @throws std::system_error   Couldn't open "/dev/random"
 * @throws std::system_error   `read(2)` failure
 * @throws std::runtime_error  `RAND_bytes()` failure
 */
void initRand(const int numBytes)
{
	int fd = ::open("/dev/random", O_RDONLY);
	if (fd < 0)
		throw std::system_error(errno, std::generic_category(),
				"open() failure");

	try {
		unsigned char bytes[numBytes];
		if (::read(fd, bytes, numBytes) != numBytes)
			throw std::system_error(errno, std::generic_category(),
					"read() failure");

		if (RAND_bytes(bytes, numBytes) == 0)
			throw std::runtime_error("RAND_bytes() failure. "
				"Code=" + std::to_string(ERR_get_error()));

		::close(fd);
	} // `fd` open
	catch (const std::exception& ex) {
		::close(fd);
		throw;
	}
}

} // Namespace