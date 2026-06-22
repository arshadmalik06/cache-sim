#include "cache_simulator.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <cstdlib>
#include <iomanip>

/*
 * MEMORY ADDRESS BREAKDOWN
 * A memory address is split into three parts:
 * 
 * [TAG | INDEX | OFFSET]
 * 
 * Example: 32-bit address in a cache with:
 *   - Block size = 64 bytes (need 6 bits for offset)
 *   - 256 sets (need 8 bits for index)
 *   - Tag = remaining 32 - 6 - 8 = 18 bits
 * 
 * Address: 0x12345ABC
 * - OFFSET: last 6 bits    (which byte within the block)
 * - INDEX:  next 8 bits    (which set this address maps to)
 * - TAG:    remaining bits (which block in that set, and from which address?)
 * 
 * This allows cache to quickly find data without searching the entire cache.
 */

CacheSimulator::CacheSimulator(unsigned long cacheSize, unsigned long blockSize,
                               unsigned long associativity, ReplacementPolicy policy)
    : cacheSize(cacheSize), blockSize(blockSize), associativity(associativity),
      replacementPolicy(policy), totalAccesses(0), cacheHits(0), cacheMisses(0),
      totalCycles(0) {
    
    // Calculate derived values
    numBlocks = cacheSize / blockSize;
    numSets = numBlocks / associativity;
    
    // Calculate bits needed for each part of address
    offsetBits = static_cast<unsigned long>(std::log2(blockSize));
    indexBits = static_cast<unsigned long>(std::log2(numSets));
    
    // Print configuration
    std::cout << "\n=== CACHE CONFIGURATION ===" << std::endl;
    std::cout << "Cache Size: " << cacheSize << " bytes" << std::endl;
    std::cout << "Block Size: " << blockSize << " bytes" << std::endl;
    std::cout << "Associativity: " << associativity << "-way" << std::endl;
    std::cout << "Total Blocks: " << numBlocks << std::endl;
    std::cout << "Total Sets: " << numSets << std::endl;
    std::cout << "Offset Bits: " << offsetBits << std::endl;
    std::cout << "Index Bits: " << indexBits << std::endl;
    std::cout << "===========================\n" << std::endl;
    
    // Initialize cache: create numSets, each with associativity blocks
    cacheSets.resize(numSets);
    for (auto& set : cacheSets) {
        set.resize(associativity);
        // Initialize each block as invalid
        for (auto& block : set) {
            block.valid = false;
            block.dirty = false;
            block.tag = 0;
            block.lastAccessTime = 0;
            block.data.resize(blockSize, 0);
        }
    }
}

// Extract the byte offset from address (last offsetBits bits)
// Example: if offsetBits=6, offset = address & 0x3F (gets bits 0-5)
unsigned long CacheSimulator::getBlockOffset(unsigned long address) {
    return address & ((1 << offsetBits) - 1);
}
unsigned long CacheSimulator::getIndex(unsigned long address) {
    return (address >> offsetBits) & ((1 << indexBits) - 1);
}

// Extract the tag from address
// Tag = remaining bits after offset and index are removed
// Tag = address >> (offsetBits + indexBits)
unsigned long CacheSimulator::getTag(unsigned long address) {
    return address >> (offsetBits + indexBits);
}

// Find if a block with this tag exists in the set
// Returns: index of block if found, -1 if not found
int CacheSimulator::findBlock(unsigned long setIndex, unsigned long tag) {
    const auto& set = cacheSets[setIndex];
    for (int i = 0; i < static_cast<int>(set.size()); ++i) {
        // Block must be valid AND tag must match
        if (set[i].valid && set[i].tag == tag) {
            return i;
        }
    }
    return -1; // Block not found (MISS)
}

// Find a victim block to replace (for when cache is full)
// Returns: index of the block to replace
int CacheSimulator::findVictim(unsigned long setIndex) {
    auto& set = cacheSets[setIndex];
    
    // First, look for an invalid (empty) block
    for (int i = 0; i < static_cast<int>(set.size()); ++i) {
        if (!set[i].valid) {
            return i; // Found empty slot
        }
    }
    
    // All blocks are valid, must replace one using replacement policy
    switch (replacementPolicy) {
        case ReplacementPolicy::LRU: {
            // Find the block with oldest lastAccessTime
            int victim = 0;
            for (int i = 1; i < static_cast<int>(set.size()); ++i) {
                if (set[i].lastAccessTime < set[victim].lastAccessTime) {
                    victim = i;
                }
            }
            return victim;
        }
        
        case ReplacementPolicy::FIFO: {
            // Simplistic FIFO: tag value represents insertion order
            // In production, you'd use a queue or timestamp
            int victim = 0;
            for (int i = 1; i < static_cast<int>(set.size()); ++i) {
                if (set[i].tag < set[victim].tag) {
                    victim = i;
                }
            }
            return victim;
        }
        
        case ReplacementPolicy::RANDOM: {
            // Return a random block index
            return std::rand() % set.size();
        }
    }
    return 0;
}

// Update LRU metadata for a block
void CacheSimulator::updateLRU(unsigned long setIndex, int blockIndex) {
    cacheSets[setIndex][blockIndex].lastAccessTime = totalAccesses;
}

// Simulate a READ operation
unsigned long CacheSimulator::read(unsigned long address) {
    totalAccesses++;
    
    unsigned long setIndex = getIndex(address);
    unsigned long tag = getTag(address);
    
    // Step 1: Check if block exists in this set
    int blockIndex = findBlock(setIndex, tag);
    
    if (blockIndex != -1) {
        // ===== CACHE HIT =====
        // Block found! Data is in cache, fast access
        cacheHits++;
        updateLRU(setIndex, blockIndex);
        totalCycles += 1; // Fast: 1 cycle (approximately L1 hit time)
        
        return 1;
    } else {
        // ===== CACHE MISS =====
        // Block not found, must fetch from memory
        cacheMisses++;
        
        // Find which block to replace
        int victimIndex = findVictim(setIndex);
        auto& victimBlock = cacheSets[setIndex][victimIndex];
        
        // If victim block is dirty, we need to write it back to memory first
        // (This is writeback policy - we'll assume write-through for simplicity)
        
        // Load new block from memory into victim slot
        victimBlock.valid = true;
        victimBlock.tag = tag;
        victimBlock.dirty = false;
        victimBlock.data.assign(blockSize, 0); // Initialize with zeros
        
        updateLRU(setIndex, victimIndex);
        
        // Memory access time: ~100-300 cycles depending on how far memory is
        // We'll use a formula: base cost + penalty for distance
        unsigned long memoryLatency = 100 + (tag % 50); // Simulate varying distances
        totalCycles += memoryLatency;
        
        return memoryLatency;
    }
}

// Simulate a WRITE operation
unsigned long CacheSimulator::write(unsigned long address, int data) {
    totalAccesses++;
    
    unsigned long setIndex = getIndex(address);
    unsigned long tag = getTag(address);
    unsigned long offset = getBlockOffset(address);
    
    int blockIndex = findBlock(setIndex, tag);
    
    if (blockIndex != -1) {
        // ===== WRITE HIT =====
        // Block in cache, update it directly
        cacheHits++;
        auto& block = cacheSets[setIndex][blockIndex];
        block.data[offset] = data;
        block.dirty = true; // Mark as modified
        updateLRU(setIndex, blockIndex);
        
        return 1;
    } else {
        // ===== WRITE MISS =====
        // Block not in cache, allocate it (write-allocate policy)
        cacheMisses++;
        
        int victimIndex = findVictim(setIndex);
        auto& victimBlock = cacheSets[setIndex][victimIndex];
        
        victimBlock.valid = true;
        victimBlock.tag = tag;
        victimBlock.dirty = true; // New block is dirty (has been written)
        victimBlock.data.assign(blockSize, 0);
        victimBlock.data[offset] = data;
        
        updateLRU(setIndex, victimIndex);
        
        unsigned long memoryLatency = 100 + (tag % 50);
        totalCycles += memoryLatency;
        
        return memoryLatency;
    }
}

// Calculate and display cache statistics
void CacheSimulator::printStats() {
    std::cout << "\n========== CACHE PERFORMANCE STATISTICS ==========" << std::endl;
    std::cout << "Total Accesses: " << totalAccesses << std::endl;
    std::cout << "Cache Hits: " << cacheHits << std::endl;
    std::cout << "Cache Misses: " << cacheMisses << std::endl;
    std::cout << "Hit Rate: " << std::fixed << std::setprecision(2) 
              << getHitRate() * 100 << "%" << std::endl;
    std::cout << "Miss Rate: " << std::fixed << std::setprecision(2) 
              << (1.0 - getHitRate()) * 100 << "%" << std::endl;
    std::cout << "Total Cycles: " << totalCycles << std::endl;
    std::cout << "Average Access Time: " << std::fixed << std::setprecision(2) 
              << static_cast<double>(totalCycles) / totalAccesses << " cycles" << std::endl;
    std::cout << "=================================================\n" << std::endl;
}

// Get hit rate (0.0 to 1.0)
double CacheSimulator::getHitRate() {
    if (totalAccesses == 0) return 0.0;
    return static_cast<double>(cacheHits) / totalAccesses;
}

unsigned long CacheSimulator::getTotalCycles() {
    return totalCycles;
}

unsigned long CacheSimulator::getHits() {
    return cacheHits;
}

unsigned long CacheSimulator::getMisses() {
    return cacheMisses;
}

void CacheSimulator::resetStats() {
    totalAccesses = 0;
    cacheHits = 0;
    cacheMisses = 0;
    totalCycles = 0;
}

void CacheSimulator::clear() {
    for (auto& set : cacheSets) {
        for (auto& block : set) {
            block.valid = false;
            block.dirty = false;
        }
    }
    resetStats();
}
