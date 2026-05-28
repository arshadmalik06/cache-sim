#ifndef CACHE_SIMULATOR_H
#define CACHE_SIMULATOR_H

#include <vector>
#include <unordered_map>
#include <queue>
#include <string>

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
