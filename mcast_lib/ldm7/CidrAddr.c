/**
 * This file defines a Classless Inter-Domain Routing (CIDR) address.
 *
 * Copyright 2018, University Corporation for Atmospheric Research
 * All rights reserved. See file COPYRIGHT in the top-level source-directory for
 * copying and redistribution conditions.
 * 
 *        File: CidrAddr.c
 *  Created on: Mar 8, 2018
 *      Author: Steven R. Emmerson
 */

#include "config.h"

#include "CidrAddr.h"
#include "log.h"

#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>

bool
cidrAddr_init(
        CidrAddr*       cidrAddr,
        const in_addr_t addr,
        const SubnetLen subnetLen)
{
    if (subnetLen > 32) {
        log_add("Too many bits in network prefix: %u", subnetLen);
        return false;
    }
    cidrAddr->addr = addr;
    cidrAddr->prefixLen = subnetLen;
    return true;
}

void
cidrAddr_destroy(CidrAddr* const cidrAddr)
{}

CidrAddr*
cidrAddr_new(
        const in_addr_t addr,
        const SubnetLen subnetLen)
{
    CidrAddr* cidrAddr = log_malloc(sizeof(CidrAddr), "CIDR address");
    if (cidrAddr) {
        if (!cidrAddr_init(cidrAddr, addr, subnetLen)) {
            free(cidrAddr);
            cidrAddr = NULL;
        }
    }
    return cidrAddr;
}

void
cidrAddr_free(CidrAddr* cidrAddr)
{
    free(cidrAddr);
}

in_addr_t
cidrAddr_getAddr(const CidrAddr* cidrAddr)
{
    return cidrAddr->addr;
}

SubnetLen
cidrAddr_getPrefixLen(const CidrAddr* cidrAddr)
{
    return cidrAddr->prefixLen;
}

in_addr_t
cidrAddr_getSubnetMask(const CidrAddr* cidrAddr)
{
    return htonl(~((1 << (32 - cidrAddr->prefixLen)) - 1));
}

in_addr_t
cidrAddr_getSubnet(const CidrAddr* cidrAddr)
{
    return cidrAddr_getSubnetMask(cidrAddr) & cidrAddr->addr;
}

bool
cidrAddr_isMember(
        const CidrAddr* cidrAddr,
        const in_addr_t addr)
{
    in_addr_t mask = cidrAddr_getSubnetMask(cidrAddr);
    return (mask & addr) == (mask & cidrAddr->addr);
}

CidrAddr*
cidrAddr_copy(
        CidrAddr*       lhs,
        const CidrAddr* rhs)
{
    if (rhs->prefixLen > 32) {
        log_add("Invalid subnet prefix length: %u", rhs->prefixLen);
        return NULL;
    }

    *lhs = *rhs;
    return lhs;
}

uint32_t cidrAddr_getNumHostAddrs(const CidrAddr* cidrAddr)
{
    // Don't count network address or broadcast address
    long count = (1 << (32 - cidrAddr->prefixLen)) - 2;

    return count > 0 ? count : 0;
}

CidrAddr*
cidrAddr_parse(const char* const spec)
{
    CidrAddr*      cidrAddr = NULL;
    char           addrSpec[INET_ADDRSTRLEN];
    unsigned short subnetLen;
    if (sscanf(spec, "%15[0-9.]/%hu", addrSpec, &subnetLen) != 2) {
        log_add("Not a CIDR address: \"%s\"", spec);
    }
    else {
        in_addr_t addr;
        if (inet_pton(AF_INET, addrSpec, &addr) != 1) {
            log_add("Not an IPv4 address: \"%s\"", addrSpec);
        }
        else {
            cidrAddr = cidrAddr_new(addr, subnetLen);
        }
    }
    return cidrAddr;
}

int
cidrAddr_snprintf(
        const CidrAddr* const addr,
        char* const           buf,
        const size_t          size)
{
    char dottedQuad[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr->addr, dottedQuad, sizeof(dottedQuad));
    return snprintf(buf, size, "%s/%u", dottedQuad, addr->prefixLen);
}

char*
cidrAddr_format(const CidrAddr* addr)
{
    char dottedQuad[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr->addr, dottedQuad, sizeof(dottedQuad));
    int   bufSize = INET_ADDRSTRLEN + 1 + 2; // dottedQuad + "/nn"
    char* buf = log_malloc(bufSize, "formatted CIDR address");
    if (buf != NULL) {
        int nbytes = snprintf(buf, bufSize, "%s/%u", dottedQuad,
                addr->prefixLen);
        if (nbytes >= bufSize) {
            bufSize = nbytes + 1;
            buf = log_realloc(buf, bufSize, "formatted CIDR address");
            if (buf != NULL)
                snprintf(buf, bufSize, "%s/%u", dottedQuad, addr->prefixLen);
        }
    }
    return buf;
}
