/***********************************************************************************************************
 *  $Id$
 *  File:    CS2610_A6.cpp
 *
 *  Author:  Madhav Mittal    Kshitij Raj    Mandala Tejesh   
 *
 *  Roll No: CS19B029         CS19B061       CS19B062
 *
 *  Purpose: Designing a cache simulator
 * 
 *  Input: 
        From the command line:
            cache size in bytes
            block size in bytes
            associativity
            replacement policy
            name of the file containing memory traces
        From file:
            memory traces, each entry 8-digit hex 
 *
 *  Output: to STDOUT
        Cache size
        Block size
        Type of cache
        Replacement policy
        # of cache accesses
        # of read accesses
        # of write accesses
        # of cache misses
        # of compulsory misses
        # of capacity misses
        # of conflict misses
        # of read misses
        # of write misses
        # of dirty blocks evicted 
 *  
*************************************************************************************/
#include <bits/stdc++.h>
#include <fstream>
#include <iostream>
typedef unsigned int uint;
using namespace std;

typedef struct cacheBlock {
    uint tag;
    bool valid;
    bool dirty;
} cBlock;

struct inpt {
    uint blockAdd;
    uint tag;
    uint setIndex;
    bool write;
};

/* all counters initialised to 0 */
uint cacheAccess = 0,
     readAccess = 0,
     writeAccess = 0,
     cacheMiss = 0,
     compMiss = 0,
     capMiss = 0,
     confMiss = 0,
     readMiss = 0,
     writeMiss = 0,
     evictDirty = 0;

void RandomReplacement(vector<list<cBlock>> &cache, vector<inpt> &inputs) {
    map<uint, bool> accessed;
    for (uint j = 0; j < inputs.size(); j++) {
        bool totalFull = true;
        for (uint k = 0; k < cache.size(); k++) {
            for (auto i = cache[k].begin(); i != cache[k].end(); ++i) {
                totalFull &= i->valid;
            }
        }

        uint blockAdd = inputs[j].blockAdd;
        uint setIndex = inputs[j].setIndex;
        uint itag = inputs[j].tag;    // tag associated with memory address
        bool write = inputs[j].write; // read/write bit (0 for read, 1 for write)

        cacheAccess++;
        (write) ? writeAccess++ : readAccess++;

        // given set index, check if required block is present
        bool hit = false;
        list<cBlock>::iterator it = cache[setIndex].end();
        it--;
        for (auto i = cache[setIndex].begin(); i != cache[setIndex].end(); ++i) {
            if (!i->valid) {
                it = i;
            } else if (i->tag == itag) {
                /* cache hit */
                hit = true;
                i->dirty |= write;
                break;
            }
        }
        if (!hit) {
            /* cache miss */
            cacheMiss++;
            (write) ? writeMiss++ : readMiss++;
            if (!accessed[blockAdd]) { /* if it is the first time accessing a block, it is a compulsory miss */
                compMiss++;
                accessed[blockAdd] = true;
                evictDirty += it->dirty & it->valid;
                it->valid = true;
                it->tag = itag;
                it->dirty = write;
            } else { /* if set is full, then it is capacity miss */
                (totalFull) ? capMiss++ : confMiss++;
                /* evict a block and replace with new block using LRUrandom policy */
                auto itr = cache[setIndex].begin();
                for (int k = 0; k < rand() % cache[setIndex].size(); k++) {
                    ++itr;
                }
                evictDirty += itr->dirty & itr->valid;
                /* insert the new block into set */
                itr->tag = itag;
                itr->dirty = write;
            }
        }
    }
}

void LRUReplacement(vector<list<cBlock>> &cache, vector<inpt> &inputs) {
    map<uint, bool> accessed;
    for (uint j = 0; j < inputs.size(); j++) {
        bool totalFull = true;
        for (uint k = 0; k < cache.size(); k++) {
            for (auto i = cache[k].begin(); i != cache[k].end(); ++i) {
                totalFull &= i->valid;
            }
        }

        uint blockAdd = inputs[j].blockAdd;
        uint setIndex = inputs[j].setIndex;
        uint itag = inputs[j].tag;    // tag associated with memory address
        bool write = inputs[j].write; // read/write bit (0 for read, 1 for write)
        cacheAccess++;
        (write) ? writeAccess++ : readAccess++;
        // given set index, check if required block is present
        bool hit = false;
        bool full = true;

        list<cBlock>::iterator it = cache[setIndex].end();
        it--;
        for (auto i = cache[setIndex].begin(); i != cache[setIndex].end(); ++i) {
            if (!i->valid) {
                it = i;
            } else if (i->tag == itag) {
                /* cache hit */
                hit = true;
                i->dirty |= write;
                cBlock updated = *i;
                cache[setIndex].erase(i);
                cache[setIndex].insert(cache[setIndex].begin(), updated);
                break;
            }
        }
        if (!hit) {
            /* cache miss */
            cacheMiss++;
            (write) ? writeMiss++ : readMiss++;
            if (!accessed[blockAdd]) { /* if it is the first time accessing a block, it is a compulsory miss */
                compMiss++;
                accessed[blockAdd] = true;
                evictDirty += it->dirty & it->valid;
                it->valid = true;
                it->tag = itag;
                it->dirty = write;
                cBlock updated = *it;
                cache[setIndex].erase(it);
                cache[setIndex].insert(cache[setIndex].begin(), updated);
            } else { /* if set is full, then it is capacity miss */
                (totalFull) ? capMiss++ : confMiss++;
                /* evict a block and replace with new block using LRUrandom policy */
                evictDirty += it->dirty & it->valid;
                /* insert the new block into set */
                it->tag = itag;
                it->dirty = write;
                /* update the recency of the current block */
                cBlock updated = *it;
                cache[setIndex].erase(it);
                cache[setIndex].insert(cache[setIndex].begin(), updated);
            }
        }
    }
}

void PseudoLRUReplacement(uint nSets, uint associativity, vector<list<cBlock>> &cache, vector<inpt> &inputs) {
    map<uint, bool> accessed;

    bool pseudotree[nSets][associativity];

    /* initialise all bits in tree */
    for (uint i = 0; i < nSets; i++) {
        for (uint j = 0; j < associativity; j++) {
            pseudotree[i][j] = 0;
        }
    }

    for (uint j = 0; j < inputs.size(); j++) { /* process all the inputs in given order */

        bool totalFull = true; // boolean to check whether or not cache is full
        for (uint k = 0; k < cache.size(); k++) {
            for (auto i = cache[k].begin(); i != cache[k].end(); ++i) {
                totalFull &= i->valid;
            }
        }

        uint blockAdd = inputs[j].blockAdd; //blockAddress of memory address
        uint setIndex = inputs[j].setIndex; // setIndex of in the cache
        uint itag = inputs[j].tag;          // tag associated with memory address
        bool write = inputs[j].write;       // read/write bit (0 for read, 1 for write)

        cacheAccess++;
        (write) ? writeAccess++ : readAccess++;

        // given set index, check if required block is present
        bool hit = false;  // boolean to indicate cache miss or hit
        bool full = true;  // boolean to indicate a full set
        uint hitindex = 0; // index of block in the set
        for (auto i = cache[setIndex].begin(); i != cache[setIndex].end(); ++i) {
            hitindex++;
            if (i->valid && i->tag == itag) { /* cache hit */
                hit = true;
                i->dirty |= write;
                uint indexing = (hitindex + associativity) / 2; // parent bit of hitIndex position in the tree
                while (indexing > 0) {                          /* flip all bits that lead to current index */
                    pseudotree[setIndex][indexing] = 1 ^ (hitindex % 2);
                    hitindex /= 2;
                    indexing /= 2;
                }
                break;
            }
        }

        if (!hit) {
            /* cache miss */
            hitindex = 0;
            cacheMiss++;
            (write) ? writeMiss++ : readMiss++;

            if (!accessed[blockAdd]) { /* compulsory miss */
                accessed[blockAdd] = true;
                compMiss++;
            } else {
                (totalFull) ? capMiss++ : confMiss++;
            }
            int i;
            for (i = 1; i <= (associativity - 1) / 2; i = 2 * i + pseudotree[setIndex][i]) {
                hitindex = hitindex * 2 + pseudotree[setIndex][i];
            }
            auto itr = cache[setIndex].begin();
            for (int k = 0; k < hitindex; k++) {
                itr++;
            }
            evictDirty += itr->dirty & itr->valid;
            /*now insertion*/
            itr->valid = 1;
            itr->tag = itag;
            itr->dirty = write;
            while (i > 0) {
                pseudotree[setIndex][i] = 1 ^ pseudotree[setIndex][i];
                i /= 2;
            }
        }
    }
}

int main() {
    uint cacheSize,
        blockSize,
        associativity,
        replPolicy,
        nBlocks, //number of blocks in cache
        nSets;   //number of sets in cache

    string infilename;
    ifstream ifile; //for input file
    cout << "Enter cache size in bytes: ";
    cin >> cacheSize;
    cout << "Enter block size in bytes: ";
    cin >> blockSize;
    cout << "associativity of cache: ";
    cin >> associativity;
    cout << "Replacement policy :";
    cin >> replPolicy;
    cout << "Name of input file :";
    cin >> infilename;
    ifile.open(infilename);

    cout << cacheSize << "\t#Cache size\n"
         << blockSize << "\t#Cache line size" << endl;

    nBlocks = cacheSize / blockSize;

    if (associativity == 0) {
        cout << "Fully-associative cache" << endl;
        associativity = nBlocks;
    } else if (associativity == 1) {
        cout << "Direct-mapped cache" << endl;
    } else {
        cout << "Set-associative cache" << endl;
    }

    nSets = nBlocks / associativity;

    vector<inpt> inputs;
    uint address;
    char rw;
    ifile.setf(std::ios::hex, std::ios::basefield);
    ifile >> address;
    while (!ifile.eof()) {
        ifile >> rw;
        //tag:index:offset
        inpt temp = {(address / blockSize),
                     (address / blockSize) / nSets,
                     (address / blockSize) % nSets,
                     (rw == 'w') ? true : false};
        //cout << temp.tag << " " << temp.setIndex << " " << temp.write << endl;
        inputs.push_back(temp);
        ifile >> address;
    }
    ifile.close();

    vector<list<cBlock>> cache(nSets, list<cBlock>(associativity));

    if (replPolicy == 0) {
        cout << "Random Replacement" << endl;
        RandomReplacement(cache, inputs);
    } else if (replPolicy == 1) {
        cout << "LRU Replacement" << endl;
        LRUReplacement(cache, inputs);
    } else {
        cout << "Pseudo-LRU Replacement" << endl;
        PseudoLRUReplacement(nSets, associativity, cache, inputs);
    }

    cout << cacheAccess << "\t#Cache accesses\n"
         << readAccess << "\t#Read accesses\n"
         << writeAccess << "\t#Write accesses\n"
         << cacheMiss << "\t#Cache misses\n"
         << compMiss << "\t#Compulsory misses\n"
         << capMiss << "\t#Capacity misses\n"
         << confMiss << "\t#Conflict misses\n"
         << readMiss << "\t#Read misses\n"
         << writeMiss << "\t#Write misses\n"
         << evictDirty << "\t#Dirty blocks evicted"
         << endl;

    return 0;
}