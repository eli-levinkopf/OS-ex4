#include "VirtualMemory.h"
#include "PhysicalMemory.h"
#include <cstdio>
#include <cassert>

int main(int argc, char **argv) {
    VMinitialize();
	int a = NUM_PAGES;
    for (uint64_t i = 0; i < (10 * NUM_FRAMES); ++i) {
        printf( "writing to %llu\n", (long long int) i);
        if(!VMwrite(5 * i * PAGE_SIZE, i)){break;}
    }

    for (uint64_t i = 0; i < (10 * NUM_FRAMES); ++i) {
        word_t value;
        if(!VMread(5 * i * PAGE_SIZE, &value)){break;}
        printf("reading from %llu %d\n", (long long int) i, value);
        assert(uint64_t(value) == i);
    }
    printf("success\n");
}



//int main () {
//  VMinitialize();
//  VMwrite (13,3);
//  word_t value;
//  VMread(13, &value);
//  VMread(6, &value);
//  VMread(31, &value);
//  int value0, value1, value2, value3, value4, value5, value6;
//
//  for (int i = 0; i < NUM_FRAMES; ++i) {
//	PMread(i*PAGE_SIZE, &value0);
//	PMread(i*PAGE_SIZE + 1, &value1);
//	int x = 0;
//  }
//}

//int main() {
//  uint64_t virtualAddress = 397896; //110 0001 0010 0100 1000 => 6 1 2 4 8
//  int root_size = get_root_size();
//  for (int i = 0; i < 5; ++i) {
//	std::cout << get_next_level(virtualAddress, i, root_size) << std::endl;
//  }
//  return 0;
//}
