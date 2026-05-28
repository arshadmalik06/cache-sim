#ifndef CACHE_SIMULATOR_H
#define CACHE_SIMULATOR_H

#include <vector>
#include <unordered_map>
#include <queue>
#include <string>

/*
 * ============================================================================
 * CACHE SIMULATOR - Educational Implementation
 * ============================================================================
 * 
 * A cache is a small, fast memory that stores copies of frequently accessed
 * data. Instead of always fetching from slow main memory, the CPU checks the
 * cache first, saving time.
 * 
 * KEY CONCEPT: Cache blocks (cache lines)
 * - The cache doesn't store individual bytes; it stores fixed-size blocks
 * - Each block contains: tag, data, and metadata (valid bit, dirty bit)
 * - Block size: typically 64 bytes in real CPUs
 * 
 * KEY CONCEPT: Cache operations
 * - HIT: Data found in cache (fast - ~4 cycles)
 * - MISS: Data not in cache, must fetch from memory (slow - ~200 cycles)
 * 
 * ============================================================================
 */

// Represents one cache block (cache line)
struct CacheBlock {
    bool valid;           // Is this block valid/in-use?
    bool dirty;           // Has this block been modified? (needs writing back)
    unsigned long tag;    // Tag to identify which memory address this block holds
    std::vector<int> data; // Actual data stored in this block
    unsigned long lastAccessTime; // For LRU replacement policy
};

// Replacement policies - determines which block to evict when cache is full
enum class ReplacementPolicy {
    LRU,    // Least Recently Used - remove the block not used for longest time
    FIFO,   // First In First Out - remove the oldest block
    RANDOM  // Random - remove a random block
};

class CacheSimulator {
private:
    // ========== CACHE PARAMETERS ==========
    unsigned long cacheSize;          // Total cache size in bytes (e.g., 64KB)
    unsigned long blockSize;          // Size of each cache block/line (e.g., 64 bytes)
    unsigned long associativity;      // How many blocks can map to same location
                                      // 1 = direct mapped, n = n-way associative
                                      // Full associative = all blocks can go anywhere
    ReplacementPolicy replacementPolicy; // Which block to evict on miss
    
    // ========== DERIVED VALUES (computed from above) ==========
    unsigned long numBlocks;          // Total blocks = cacheSize / blockSize
    unsigned long numSets;            // Number of sets = numBlocks / associativity
    unsigned long offsetBits;         // Bits to select byte within block
    unsigned long indexBits;          // Bits to select which set
    
    // ========== CACHE STRUCTURE ==========
    // In real caches, memory is divided into sets.
    // Each set contains 'associativity' number of blocks.
    // For example, a 2-way 16KB cache with 64B blocks has:
    //   - 16KB / 64B = 256 blocks total
    //   - 256 blocks / 2-way = 128 sets
    //   - Each set has 2 blocks
    std::vector<std::vector<CacheBlock>> cacheSets;
    
    // ========== STATISTICS ==========
    unsigned long totalAccesses;      // Total memory accesses
    unsigned long cacheHits;          // Number of cache hits
    unsigned long cacheMisses;        // Number of cache misses
    unsigned long totalCycles;        // Accumulated cycles (for performance)
    
    // ========== HELPER FUNCTIONS ==========
    
    // Extract parts of memory address
    unsigned long getBlockOffset(unsigned long address);  // Last offsetBits
    unsigned long getIndex(unsigned long address);        // Next indexBits
    unsigned long getTag(unsigned long address);          // Remaining bits
    
    // Find or create space for new block
    int findVictim(unsigned long setIndex);     // Which block to replace?
    int findBlock(unsigned long setIndex, unsigned long tag); // Is block here?
    
    // Update LRU tracking
    void updateLRU(unsigned long setIndex, int blockIndex);
    
public:
    // ========== CONSTRUCTOR & SETUP ==========
    CacheSimulator(unsigned long cacheSize, unsigned long blockSize,
                   unsigned long associativity, ReplacementPolicy policy);
    
    // ========== MAIN OPERATIONS ==========
    
    // Simulate reading from an address
    // Returns: number of cycles needed for this access
    // (hit = 1 cycle, miss = 100+ cycles depending on memory latency)
    unsigned long read(unsigned long address);
    
    // Simulate writing to an address
    unsigned long write(unsigned long address, int data);
    
    // ========== STATISTICS & REPORTING ==========
    
    // Print performance metrics
    void printStats();
    
    // Get individual statistics
    double getHitRate();        // Returns 0.0 to 1.0
    unsigned long getTotalCycles();
    unsigned long getHits();
    unsigned long getMisses();
    
    // Reset statistics (keep cache state)
    void resetStats();
    
    // Clear cache contents
    void clear();
};

#endif // CACHE_SIMULATOR_H
