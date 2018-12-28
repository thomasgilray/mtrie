
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
    static const u64 mask64 = 0xffffffffffffffff;

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
        
        ~E()
        {
			if (bits & 1)
			{
                KV* const _kv = (KV*)(bits & ptrmask);
				delete _kv;
				bits = 0;
			}
			else if (bits)
			{
				delete c;
				bits = 0;
			}
        }

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

        inline bool remove(const u64 k, const u64 path, u64* const count)
        {
			if (bits & 1)
			{
				KV* const _kv = (KV*)(bits & ptrmask);
				if (_kv->key == k)
				{
					delete _kv;
					bits = 0;
					--*count;
					return true;
				}
			}
			else if (bits && c->remove(k, path >> 4, count))
			{
				delete c;
				bits = 0;
				return true;
			}

			return false;
        }

        inline V* find(const u64 k, const u64 path) const
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

        inline u64 findnext(const u64 k, u64 path, V** const valptr) const
		{
            if (bits & 1)
			{
				KV const * const _kv = (KV*)(bits & ptrmask);
				u64 kvpath = _kv->key >> (12+d*4);
				path >>= 4;
				for (; path != kvpath; (path >>= 4) && (kvpath >>= 4))
				{
					if ((kvpath&15) < (path&15))
						return k;
					else if ((kvpath&15) > (path&15))
						break;
				}
				*valptr = _kv->val;
				return _kv->key;
			}
			else
				return c->findnext(k, path >> 4, valptr);
        }
    };
  
private:
    E buff[16];  

public:
    mtrienode()
	    : buff{0}
    {
    }

    ~mtrienode()
    {
        for (u16 i = 0; i < 16; ++i)
			(&buff[i])->~E();
    }

    inline void insert(const u64 k, const u64 path, V* const v, u64* const count)
    {
        buff[path&15].insert(k,path,v,count);
    }

    inline bool remove(const u64 k, const u64 path, u64* const count)
    {
        if (buff[path&15].remove(k,path,count))
		{
			for (u16 i = 0; i < 16; ++i)
				if (buff[i].bits)
					return false;
			return true;
		}
		else
            return false;
    }

    inline V* find(const u64 k, const u64 path)
    {
        return buff[path&15].find(k,path);
    }
    
    inline u64 findnext(u64 k, u64 path, V** const valptr) const
    {
        while(true)
		{
			if (buff[path&15].bits)
			{
				k = buff[path&15].findnext(k,path,valptr);
				if (*valptr)
					return k;
			}
			if (path&15 == 15)
				return k;
			else
			{
				path = (path&15) + 1;
				k = (k&(mask64>>(56-d*4))) | (path<<(d*4+8));
			}
		}
    }
};



template<typename V>
class mtrienode<13,V>
{
    static const u64 topmask = 0x0fffffffffffffff;
  
 private:
    V* buff[16];  

 public:
 mtrienode()
	 : buff{0}
    {
    }
    
    ~mtrienode()
    {
    }
    
    inline void insert(const u64 k, const u64 path, V* const v, u64* const count)
    {
        if (buff[path] == 0)
			++*count;
        buff[path] = v;
    }

    inline bool remove(const u64 k, const u64 path, u64* const count)
    {
        if (buff[path])
		{
			--*count;
            buff[path] = 0; 
			for (u16 i = 0; i < 16; ++i)
				if (buff[i])
					return false;
			return true;
		}
		else
			return false;
    }

    inline V* find(const u64 k, const u64 path) const
    {
        if (buff[path])
    	    return buff[path];
		else
			return 0;
    }

    inline u64 findnext(u64 k, u64 path, V** const valptr) const
    {
        while(true)
		{     
			if (buff[path])
			{
				*valptr = buff[path];
				return k;
			}
			if (path == 15)
				return k;
			else
			{
				++path;
				k = (k&topmask) | (path<<60);
			}
		}
    }
};



template<typename V>
class mtrie
{
    static const u64 ptrmask = 0xfffffffffffffff8;
    static const u64 mask64 = 0xffffffffffffffff;
	
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

        ~E()
        {
			if (bits & 1)
			{
                KV* const _kv = (KV*)(bits & ptrmask);
				delete _kv;
				bits = 0;
			}
			else if (bits)
			{
				delete c;
				bits = 0;
			}
        }

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
            else if (bits && c->remove(k, path >> 8, count))
            {
                delete c;
                bits = 0;
            }
        }

        inline V* find(const u64 k, const u64 path) const
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

        inline u64 findnext(const u64 k, u64 path, V** const valptr) const
        {
            if (bits & 1)
            {
				KV const * const _kv = (KV*)(bits & ptrmask);
                u64 kvpath = _kv->key >> 8;
				path >>= 8;
				for (; path != kvpath; (path >>= 4) && (kvpath >>= 4))
					if (kvpath&15 < path&15)
						return k;
					else if (kvpath&15 > path&15)
						break;
				*valptr = _kv->val;
				return _kv->key;
			}
            else
                return c->findnext(k, path >> 8, valptr);
        }
    };
  
 private:
    // Not cache aligned well, but this is only the root node
    u64 kcount; 
    E buff[256];  

 public:
 mtrie()
	 : kcount(0), buff{0}
    {        
    }
    
    ~mtrie()
    {
        for (u16 i = 0; i < 256; ++i)
            (&buff[i])->~E();
    }
    
    u64 count() const
    {
        return kcount;
    }
    
    void insert(const u64 k, V* const v)
    {
        if (v) 
            buff[k&255].insert(k,k,v,&kcount);
        else
            buff[k&255].remove(k,k,&kcount);
    }
    
    void remove(const u64 k)
    {
        buff[k&255].remove(k,k,&kcount);
    }
    
    V* find(const u64 k) const
    {
        return buff[k&255].find(k,k);
    }
    
    u64 findnext(u64 k, V** const valptr) const
    {
        *valptr = 0;
        while(true)
		{
			if (buff[k&255].bits)
			{
				k = buff[k&255].findnext(k,k,valptr);
				if (*valptr)
					return k;
			}
			if (k&255 == 255)
				return 0;
			else
				k = (k&255) + 1;
		}
    }

    class iter
    {
    private:
        mtrie<V>* root;
        u64 ckey;
		V* cvalue; 
	
    public:
	iter(mtrie<V>* const _root)
		: root(_root)
        {
			ckey = root->findnext(0, &cvalue);
			if (cvalue == 0)
				root = 0;
        }

        bool more() const
        {
			return root != 0;
        }

        void advance()
        {
			//std::cout << std::hex << "before: " << ckey << std::endl;
			if (root && ckey < mask64)
			{
				// Increment ckey, then findnext(..)
				// (increment is NOT the same as ckey+1, as the
				//  lowest-significant pseduobytes come first)
				u16 d = 0;
			    while (d < 14)
				{
					const u16 pos = (60-4*d);
					const u64 shifted = ckey>>pos;
					const u64 frbit = (1+(15&shifted))&15;
					ckey = (ckey&(mask64^(mask64<<pos)))|((frbit|(shifted&(0xfffffffffffffff0)))<<pos);
					if (frbit) break;
					++d;
				}
				if (d==14 && ckey&0x0000000000000f00==0)
					++ckey;
				//std::cout << std::hex << "after: " << ckey << std::dec << std::endl;
				// Then scan forward from position ckey
				ckey = root->findnext(ckey,&cvalue);
				if (cvalue == 0)
					root = 0;
			}
			else
				root = 0;
        }

        u64 getkey() const
        {
			return ckey;
        }

        V* getval() const
        {
			return cvalue;
        } 
    };
};







