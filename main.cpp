#include "cache_simulator.h"
#include <iostream>
#include <iomanip>

/*
 * ============================================================================
 * DEMONSTRATION PROGRAM
 * ============================================================================
 * 
 * This program tests the cache simulator with different access patterns
 * to show how hit/miss rates vary based on:
 * 1. Cache configuration (size, associativity, block size)
 * 2. Access patterns (sequential, random, locality)
 * 3. Replacement policies (LRU, FIFO, Random)
 * ============================================================================
 */

void printSeparator() {
    std::cout << "\n" << std::string(70, '=') << "\n" << std::endl;
}

// Test 1: Sequential access (best case - high hit rate)
void testSequentialAccess() {
    printSeparator();
    std::cout << "TEST 1: SEQUENTIAL ACCESS PATTERN" << std::endl;
    std::cout << "Simulating reading from consecutive memory addresses" << std::endl;
    std::cout << "(This is the best case for cache - lots of locality)\n" << std::endl;
    
    CacheSimulator cache(1024, 64, 2, ReplacementPolicy::LRU);
    
    // Access 256 consecutive addresses
    // Since block size = 64 bytes, each block covers 64 addresses
    // So 256 addresses = 4 cache blocks needed
    // First 4 blocks = hits, rest will be misses as we fill cache
    
    for (int i = 0; i < 256; ++i) {
        cache.read(i);
    }
    
    cache.printStats();
}

// Test 2: Random access (worst case - low hit rate)
void testRandomAccess() {
    printSeparator();
    std::cout << "TEST 2: RANDOM ACCESS PATTERN" << std::endl;
    std::cout << "Simulating reading from random memory addresses" << std::endl;
    std::cout << "(No locality = poor cache performance)\n" << std::endl;
    
    CacheSimulator cache(1024, 64, 2, ReplacementPolicy::LRU);
    
    // Access 256 random addresses
    for (int i = 0; i < 256; ++i) {
        unsigned long addr = (i * 7919) % 16384; // Pseudo-random
        cache.read(addr);
    }
    
    cache.printStats();
}

// Test 3: Spatial locality (good - works block level)
void testSpatialLocality() {
    printSeparator();
    std::cout << "TEST 3: SPATIAL LOCALITY PATTERN" << std::endl;
    std::cout << "Accessing nearby addresses within same cache block" << std::endl;
    std::cout << "(One block loaded, multiple accesses to it = hits)\n" << std::endl;
    
    CacheSimulator cache(1024, 64, 2, ReplacementPolicy::LRU);
    
    // Access 4 blocks, but hit each block multiple times
    for (int block = 0; block < 4; ++block) {
        for (int offset = 0; offset < 64; offset += 8) {
            cache.read(block * 64 + offset);
        }
    }
    
    cache.printStats();
}

// Test 4: Compare different replacement policies
void testReplacementPolicies() {
    printSeparator();
    std::cout << "TEST 4: COMPARING REPLACEMENT POLICIES" << std::endl;
    std::cout << "Same access pattern, different eviction strategies\n" << std::endl;
    
    // Test pattern: repeated access to 5 different blocks
    auto runTest = [](ReplacementPolicy policy, const std::string& name) {
        CacheSimulator cache(512, 64, 1, policy); // Direct mapped cache
        
        for (int iteration = 0; iteration < 3; ++iteration) {
            for (int block = 0; block < 8; ++block) {
                cache.read(block * 64);
            }
        }
        
        std::cout << "\n" << name << " Policy:" << std::endl;
        std::cout << "  Hits: " << cache.getHits() << std::endl;
        std::cout << "  Misses: " << cache.getMisses() << std::endl;
        std::cout << "  Hit Rate: " << std::fixed << std::setprecision(2) 
                  << cache.getHitRate() * 100 << "%" << std::endl;
    };
    
    runTest(ReplacementPolicy::LRU, "LRU (Least Recently Used)");
    runTest(ReplacementPolicy::FIFO, "FIFO (First In First Out)");
    runTest(ReplacementPolicy::RANDOM, "RANDOM");
}

// Test 5: Associativity impact
void testAssociativity() {
    printSeparator();
    std::cout << "TEST 5: ASSOCIATIVITY IMPACT" << std::endl;
    std::cout << "Testing how many blocks per set affects performance\n" << std::endl;
    
    auto runTest = [](unsigned long assoc, const std::string& name) {
        CacheSimulator cache(1024, 64, assoc, ReplacementPolicy::LRU);
        
        // Access pattern that benefits from higher associativity
        for (int i = 0; i < 512; ++i) {
            cache.read((i % 16) * 64);
        }
        
        std::cout << "\n" << name << std::endl;
        std::cout << "  Hits: " << cache.getHits() << std::endl;
        std::cout << "  Misses: " << cache.getMisses() << std::endl;
        std::cout << "  Hit Rate: " << std::fixed << std::setprecision(2) 
                  << cache.getHitRate() * 100 << "%" << std::endl;
    };
    
    runTest(1, "1-way (Direct Mapped)");
    runTest(2, "2-way Associative");
    runTest(4, "4-way Associative");
    runTest(8, "8-way Associative (Fully Assoc)");
}

// Test 6: Cache size impact
void testCacheSize() {
    printSeparator();
    std::cout << "TEST 6: CACHE SIZE IMPACT" << std::endl;
    std::cout << "Larger caches generally have better hit rates\n" << std::endl;
    
    auto runTest = [](unsigned long size, const std::string& name) {
        CacheSimulator cache(size, 64, 2, ReplacementPolicy::LRU);
        
        // Access 1024 random addresses
        for (int i = 0; i < 1024; ++i) {
            unsigned long addr = (i * 7919) % 8192;
            cache.read(addr);
        }
        
        std::cout << "\n" << name << std::endl;
        std::cout << "  Hits: " << cache.getHits() << std::endl;
        std::cout << "  Misses: " << cache.getMisses() << std::endl;
        std::cout << "  Hit Rate: " << std::fixed << std::setprecision(2) 
                  << cache.getHitRate() * 100 << "%" << std::endl;
    };
    
    runTest(256, "256 Bytes (Very Small)");
    runTest(512, "512 Bytes");
    runTest(1024, "1024 Bytes (1 KB)");
    runTest(4096, "4096 Bytes (4 KB)");
    runTest(16384, "16384 Bytes (16 KB)");
}

// Test 7: Write operations
void testWriteOperations() {
    printSeparator();
    std::cout << "TEST 7: WRITE OPERATIONS" << std::endl;
    std::cout << "Testing read-write mixed workload\n" << std::endl;
    
    CacheSimulator cache(2048, 64, 2, ReplacementPolicy::LRU);
    
    // Simulating array modification
    for (int i = 0; i < 512; ++i) {
        cache.read(i * 8);   // Read
        cache.write(i * 8, i); // Write back
    }
    
    cache.printStats();
}

// Test 8: Working set analysis
void testWorkingSet() {
    printSeparator();
    std::cout << "TEST 8: WORKING SET ANALYSIS" << std::endl;
    std::cout << "When working set > cache size, many misses occur\n" << std::endl;
    
    CacheSimulator cache(1024, 64, 2, ReplacementPolicy::LRU);
    
    std::cout << "Cache size: 1024 bytes (16 blocks of 64 bytes)\n" << std::endl;
    
    // Working set 1: fits in cache
    std::cout << "Test A: Access 4 blocks repeatedly (fits in cache)" << std::endl;
    for (int iter = 0; iter < 5; ++iter) {
        for (int block = 0; block < 4; ++block) {
            cache.read(block * 64);
        }
    }
    std::cout << "  Hit Rate: " << std::fixed << std::setprecision(2) 
              << cache.getHitRate() * 100 << "%" << std::endl;
    
    cache.clear();
    
    // Working set 2: larger than cache
    std::cout << "\nTest B: Access 20 blocks repeatedly (exceeds cache)" << std::endl;
    for (int iter = 0; iter < 5; ++iter) {
        for (int block = 0; block < 20; ++block) {
            cache.read(block * 64);
        }
    }
    std::cout << "  Hit Rate: " << std::fixed << std::setprecision(2) 
              << cache.getHitRate() * 100 << "%" << std::endl;
}

int main() {
    std::cout << "\n╔════════════════════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║                    CACHE SIMULATOR - EDUCATIONAL                   ║" << std::endl;
    std::cout << "║                                                                    ║" << std::endl;
    std::cout << "║  A demonstration of how CPU caches work, with performance          ║" << std::endl;
    std::cout << "║  analysis under different conditions and configurations.           ║" << std::endl;
    std::cout << "╚════════════════════════════════════════════════════════════════════╝\n" << std::endl;
    
    // Run all tests
    testSequentialAccess();
    testRandomAccess();
    testSpatialLocality();
    testReplacementPolicies();
    testAssociativity();
    testCacheSize();
    testWriteOperations();
    testWorkingSet();
    
    printSeparator();
    std::cout << "ALL TESTS COMPLETED\n" << std::endl;
    
    return 0;
}
