#pragma once

#include <unordered_map>

// Hash and equality of LPCTSTR keys for std::unordered_map (compares string contents, not pointers)
struct CHashLPCTSTR
{
	size_t operator()(LPCTSTR key) const;
};

struct CEqualLPCTSTR
{
	bool operator()(LPCTSTR key1, LPCTSTR key2) const { return _tcscmp(key1, key2) == 0; }
};

template<typename V>
using LPCTSTRHashMap = std::unordered_map<LPCTSTR, V, CHashLPCTSTR, CEqualLPCTSTR>;
