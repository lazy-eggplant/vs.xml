
/**
 * @file sparse-mmap.cpp
 * @brief Small experiment to see how to handle sparse chunks of binary data, and lazy alloc more as needed.
 * @date 2025-06-21
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include <bits/stdc++.h>
#include <sys/mman.h>
#include <unistd.h>
#include <signal.h>
#include <ucontext.h>

static size_t page_size;
static void*  base_addr;
static size_t region_size;

// Align addr down to page boundary
static constexpr inline void* page_align(void* addr) {
    return (void*)((uintptr_t)addr & ~(page_size - 1));
}

static void cleanup();

// SIGSEGV handler: commit pages on‐demand
static void segv_handler(int sig, siginfo_t* si, void* /*unused*/) {
    void* fault_addr = si->si_addr;

    if (fault_addr >= base_addr &&
        fault_addr < (void*)((char*)base_addr + region_size)) 
    {
        void* page = page_align(fault_addr);

        if (mprotect(page, page_size, PROT_READ | PROT_WRITE) == 0) {
            return;  // retry instruction successfully
        }
        // fall through on mprotect error
    }

    // Not ours or mprotect failed: restore default & re‐raise
    struct sigaction sa_def;
    sa_def.sa_handler = SIG_DFL;
    sigemptyset(&sa_def.sa_mask);
    sa_def.sa_flags = 0;
    sigaction(SIGSEGV, &sa_def, nullptr);
    kill(getpid(), SIGSEGV);
}

// Restore default handler and unmap the region
static void cleanup() {
    // Restore default SIGSEGV
    struct sigaction sa_def;
    sa_def.sa_handler = SIG_DFL;
    sigemptyset(&sa_def.sa_mask);
    sa_def.sa_flags = 0;
    sigaction(SIGSEGV, &sa_def, nullptr);

    // Unmap the reserved region
    if (base_addr && region_size > 0) {
        if (munmap(base_addr, region_size) == 0) {
            std::cerr << "Region successfully unmapped\n";
        }
        else {
            perror("munmap");
        }
        base_addr = nullptr;
        region_size = 0;
    }
}

int main() {
    // Determine page size
    page_size = sysconf(_SC_PAGESIZE);

    // Reserve 1 TB virtually
    region_size = size_t(1ULL << 40);
    base_addr = mmap(nullptr,
                     region_size,
                     PROT_NONE,
                     MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE,
                     -1,
                     0);
    if (base_addr == MAP_FAILED) {
        perror("mmap reserve");
        return 1;
    }
    std::cout << "Reserved " << (region_size >> 20)
              << " MiB at " << base_addr << "\n";

    // Install our SIGSEGV handler
    struct sigaction sa;
    sa.sa_sigaction = segv_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO | SA_NODEFER;
    if (sigaction(SIGSEGV, &sa, nullptr) < 0) {
        perror("sigaction");
        cleanup();
        return 1;
    }

    // Access some offsets
    std::vector<size_t> offsets = {
        0,
        page_size * 10,
        page_size * 1000000,          // ~4 GiB
        region_size - 2 * page_size   // near the end
    };

    int i = 0;
    for (auto off : offsets) {
        char* ptr = (char*)base_addr + off;
        std::cout << "Writing to offset " << off
                  << " (" << static_cast<void*>(ptr) << ")\n";
        ptr[0] = 42 + i;   // triggers on‐demand page commit
        std::cout << " ptr[0] = " << int(ptr[0]) << "\n";
        i++;
    }

    std::cout << "Done accessing pages.\n";
    // Clean up before exit
    cleanup();
    return 0;
}
