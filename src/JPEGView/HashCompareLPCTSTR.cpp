#include "StdAfx.h"
#include "HashCompareLPCTSTR.h"

// the hash function (string hash)
size_t CHashLPCTSTR::operator()(LPCTSTR key) const {
	size_t nHash = 0;
	int nCnt = 0;
	while (key[nCnt] != 0 && nCnt++ < 16) {
		nHash += key[nCnt];
		nHash = (nHash << 8) + nHash;
	}
	return nHash;
}
