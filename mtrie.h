
// Copyright (C) Thomas Gilray, 2018
// See the notice in LICENSE.md

#pragma once

#include "compat.h"
#include <new>
#include <iostream>



template<unsigned d, typename V>
class mtrienode
{
    static const u64 ptrmask = 0xfffffffffffffff8;

    struct KV
    {
        u64 key;
        V* val;
        KV(const u64 key, V* const val) : key(key), val(val) {}
    };
   
    union E
    {
        mtrienode<d+1,V>* c;
        KV* kv;
        u64 bits;

        E() : bits(0) { }
        E(const u64 bits) : bits(bits) { }
        E(const E& e) : bits(e.bits) { }

        inline void insert(const u64 k, const u64 path, V* const v, u64* const count)
        {
	    if (bits & 1)
	    {
	        KV* const _kv = (KV*)(bits & ptrmask);
	        if (_kv->key == k)
   		    _kv->val = v;
		else
		{
		    c = new mtrienode<d+1,V>();
                    c->insert(k, path >> 4, v, count);
		    c->insert(_kv->key, _kv->key >> (4*d+12), _kv->val, count);
		    delete _kv;
		    --*count;
		}
	    }
	    else if (bits)
	      c->insert(k, path >> 4, v, count);
	    else
	    {
	        kv = new KV(k,v);
		bits |= 1;
		++*count;
	    }
        }

      inline void remove(const u64 k, const u64 path, u64* const count)
        {
	    if (bits & 1)
	    {
	        KV* const _kv = (KV*)(bits & ptrmask);
	        if (_kv->key == k)
		{
		    delete _kv;
		    bits = 0;
		    --*count;
		}
	    }
	    else if (bits)
	      c->remove(k, path >> 4, count);
        }

        inline V* find(const u64 k, const u64 path)
        {
	    if (bits & 1)
	    {
	        KV* const _kv = (KV*)(bits & ptrmask);
	        if (_kv->key == k)
	    	    return _kv->val;
	    }
	    else if (bits)
	        return c->find(k, path >> 4);
	    return 0;
        }
    };
  
private:
    E buff[16];  

public:
    mtrienode()
      : buff{0}
    {
    }

    void insert(const u64 k, const u64 path, V* const v, u64* const count)
    {
      buff[path&15].insert(k,path,v,count);
    }

    void remove(const u64 k, const u64 path, u64* const count)
    {
        buff[path&15].remove(k,path,count);
    }

    V* find(const u64 k, const u64 path)
    {
        buff[path&15].find(k,path);
    }
};



template<typename V>
class mtrienode<13,V>
{   
private:
    V* buff[16];  

public:
    mtrienode()
        : buff{0}
    {
    }
    
    void insert(const u64 k, const u64 path, V* const v, u64* const count)
    {
        if (buff[path&15] == 0)
	    ++*count;
        buff[path] = v;
    }

    void remove(const u64 k, const u64 path, u64* const count)
    {
        if (buff[path&15])
	    --*count;
        buff[path] = 0;
    }

    V* find(const u64 k, const u64 path)
    {
        if (buff[path])
    	    return buff[path];
    }
};



template<typename V>
class mtrie
{
    static const u64 ptrmask = 0xfffffffffffffff8;

    struct KV
    {
        u64 key;
        V* val;
        KV(const u64 key, V* const val) : key(key), val(val) {}
    };
   
    union E
    {
        mtrienode<0,V>* c;
        KV* kv;
        u64 bits;

        E() : bits(0) { }
        E(const u64 bits) : bits(bits) { }
        E(const E& e) : bits(e.bits) { }

        inline void insert(const u64 k, const u64 path, V* const v, u64* const count)
        {
	    if (bits & 1)
	    {
	        KV* const _kv = (KV*)(bits & ptrmask);
	        if (_kv->key == k)
   		    _kv->val = v;
		else
		{
		    c = new mtrienode<0,V>();
                    c->insert(k, path >> 8, v, count);
		    c->insert(_kv->key, _kv->key >> 8, _kv->val, count);
		    delete _kv;
		    --*count;
		}
	    }
	    else if (bits)
	      c->insert(k, path >> 8, v, count);
	    else
	    {
	        kv = new KV(k,v);
		bits |= 1;
		++*count;
	    }
        }

        inline void remove(const u64 k, const u64 path, u64* const count)
        {
	    if (bits & 1)
	    {
	        KV* const _kv = (KV*)(bits & ptrmask);
	        if (_kv->key == k)
		{
		    delete _kv;
		    bits = 0;
		    --*count;
		}
	    }
	    else if (bits)
	      c->remove(k, path >> 8, count);
        }

        inline V* find(const u64 k, const u64 path)
        {
	    if (bits & 1)
	    {
	        KV* const _kv = (KV*)(bits & ptrmask);
	        if (_kv->key == k)
	    	    return _kv->val;
	    }
	    else if (bits)
	        return c->find(k, path >> 8);
	    return 0;
        }
    };
  
private:
    // Not cache aligned well, but this is only the root node
    u64 count; 
    E buff[256];  

public:
    mtrie()
      : count(0), buff{0}
    {        
    }

    u64 getCount()
    {
        return count;
    }
    
    void insert(const u64 k, V* const v)
    {
        if (v) 
            buff[k&255].insert(k,k,v,&count);
	else
	    buff[k&255].remove(k,k,&count);
    }

    void remove(const u64 k)
    {
        buff[k&255].remove(k,k,&count);
    }

    V* find(const u64 k)
    {
        buff[k&255].find(k,k);
    }
};



