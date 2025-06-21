
/**
 * @file sparse-mmap.2.cpp
 * @brief Improved version keeping track of pages so that they can be removed if physical space is dwelling.
 * @date 2025-06-21
 * 
 * @copyright Copyright (c) 2025
 * 
 */

// huge_pager.cpp
// g++ -std=c++17 -O2 huge_pager.cpp -o huge_pager

#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <atomic>
#include <unistd.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/syscall.h>

// --- Configuration ------------------------------------------------------

// Virtual region size: here 1 GiB
static constexpr size_t REGION_SIZE = size_t(1) << 30;

// Maximum number of pages we’ll ever track (must cover REGION_SIZE/page_size)
static constexpr size_t MAX_PAGES = 1 << 20;  // e.g. 1M pages

// -------------------------------------------------------------------------

static size_t        page_size;
static void*         base_addr   = nullptr;
static std::atomic<size_t> pages_count{0};

// A simple circular buffer of committed pages:
static std::atomic<void*>  pages_buf[MAX_PAGES];

// A tiny async-safe LCG for random eviction index:
static std::atomic<uint32_t> lcg_state{123456789};

// Align an address down to page boundary
static inline void* page_align(void* p) {
    return (void*)( (uintptr_t)p & ~(page_size - 1) );
}

// Async-safe random: x = a*x + c
static uint32_t lcg_rand() {
    uint32_t x = lcg_state.load(std::memory_order_relaxed);
    // constants from glibc LCG
    x = x * 1103515245u + 12345u;
    lcg_state.store(x, std::memory_order_relaxed);
    return x;
}

// Try to commit one page. On success return 0.
// On ENOMEM: evict one random page, retry once.
// On other errors or second failure: return errno.
static int commit_or_evict(void* pg) {
    if (mprotect(pg, page_size, PROT_READ | PROT_WRITE) == 0) {
        // record it: push into circular buffer
        size_t idx = pages_count.fetch_add(1, std::memory_order_acq_rel);
        pages_buf[idx % MAX_PAGES].store(pg, std::memory_order_release);
        return 0;
    }
    if (errno != ENOMEM) {
        return errno;
    }

    // Evict one existing page at random
    size_t cnt = pages_count.load(std::memory_order_acquire);
    if (cnt > 0) {
        size_t victim = lcg_rand() % cnt;
        void* vp = pages_buf[victim % MAX_PAGES].load(std::memory_order_acquire);
        if (vp) {
            // revoke RW, drop physical memory
            mprotect(vp, page_size, PROT_NONE);
            madvise(vp, page_size, MADV_DONTNEED);
            // replace victim slot with our new page
            pages_buf[victim % MAX_PAGES].store(pg, std::memory_order_release);
        }
    }

    // retry
    if (mprotect(pg, page_size, PROT_READ | PROT_WRITE) == 0) {
        // record slot if count < MAX_PAGES
        size_t idx = pages_count.fetch_add(1, std::memory_order_acq_rel);
        if (idx < MAX_PAGES) {
            pages_buf[idx].store(pg, std::memory_order_release);
        }
        return 0;
    }
    return errno;
}

// SIGSEGV handler: commit-on-demand with random eviction
static void sigsegv_handler(int sig, siginfo_t* si, void* /*ctx*/) {
    void* fault = si->si_addr;
    if (fault >= base_addr && fault < (void*)((char*)base_addr + REGION_SIZE)) {
        void* pg = page_align(fault);
        if (commit_or_evict(pg) == 0) {
            // success: retry faulting instruction
            return;
        }
        // else fall-through to genuine segfault
    }

    // not ours or permanent failure: restore default and re-raise
    struct sigaction sa = {};
    sa.sa_handler = SIG_DFL;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGSEGV, &sa, nullptr);
    // re-raise
    syscall(SYS_tgkill, getpid(), syscall(SYS_gettid), SIGSEGV);
}

// Unmap region and restore default handler
static void cleanup() {
    if (base_addr) {
        munmap(base_addr, REGION_SIZE);
        base_addr = nullptr;
    }
    struct sigaction sa = {};
    sa.sa_handler = SIG_DFL;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGSEGV, &sa, nullptr);
}

int main() {
    page_size = sysconf(_SC_PAGESIZE);

    // Reserve a huge region with no physical commitment
    base_addr = mmap(nullptr,
                     REGION_SIZE,
                     PROT_NONE,
                     MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE,
                     -1, 0);
    if (base_addr == MAP_FAILED) {
        perror("mmap reserve");
        return 1;
    }
    printf("Reserved %zu MiB at %p\n",
           (size_t)(REGION_SIZE >> 20), base_addr);

    // Install SIGSEGV handler
    struct sigaction sa = {};
    sa.sa_sigaction = sigsegv_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO | SA_NODEFER;
    if (sigaction(SIGSEGV, &sa, nullptr) < 0) {
        perror("sigaction");
        cleanup();
        return 1;
    }

    // Example: touch some pages
    for (size_t i = 0; i < 1000; ++i) {
        // scatter touches across the region
        uintptr_t off = ((uintptr_t)lcg_rand() << 10 |
                         (uintptr_t)lcg_rand()) % REGION_SIZE;
        char* p = (char*)base_addr + off;
        p[0] = char(i & 0xFF);  // on-demand commit or eviction
    }

    printf("Touched pages: %zu\n", pages_count.load());
    cleanup();
    return 0;
}
