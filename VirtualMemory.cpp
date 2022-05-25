#include "MemoryConstants.h"
#include "PhysicalMemory.h"
#include <cmath>
#define FAILURE 0
#define SUCCESS 1

void VMinitialize() {
  for (int i = 0; i < PAGE_SIZE; i++) {
	PMwrite(i, 0);
  }
}

uint64_t get_offset(uint64_t virtualAddress) {
  return virtualAddress & (((uint64_t)1 << OFFSET_WIDTH) - 1);
}

int get_root_size() {
  return VIRTUAL_ADDRESS_WIDTH % OFFSET_WIDTH ?
		 VIRTUAL_ADDRESS_WIDTH % OFFSET_WIDTH : OFFSET_WIDTH;
}

uint64_t get_next_level(uint64_t virtualAddress, int level, int root_size) {
  uint64_t mask = ~0 << ((uint64_t)(VIRTUAL_ADDRESS_WIDTH - root_size)
	  - level * OFFSET_WIDTH);
  uint64_t result =
	  (virtualAddress & mask) >> ((TABLES_DEPTH - level) * OFFSET_WIDTH);
  return result & (((uint64_t)1 << (OFFSET_WIDTH)) - 1);
}

int get_cyclic_distance(uint64_t page_num, uint64_t parent) {
  return  (NUM_PAGES - abs(page_num - (long double)parent)) >
	  abs(page_num - (long double)parent) ? abs(page_num - (long double)parent) :
		  (NUM_PAGES - abs(page_num - (long double)parent));
}

void concatenate(uint64_t* left, int right, int level) {
  uint64_t shift = level == 0 ? get_root_size() : OFFSET_WIDTH;
  (*left) <<= shift;
  *left |= right;
}
void DFS(word_t root, int level, word_t *next_frame, word_t *max_frame_idx,
		 bool *next_frame_is_empty, word_t *parent, word_t *prev_parent,
		 word_t *prev_parent_row, uint64_t page_num, uint64_t*
max_cyclic_distance, word_t* evict_frame, uint64_t final_page_num,
		 word_t *evicted_parent, word_t *evicted_parent_row,
		 word_t* empty_parent, word_t* empty_parent_row, uint64_t* evicted_page_index) {
  if (level >= TABLES_DEPTH) {
	int cyclic_distance = get_cyclic_distance(page_num, final_page_num);
	if ((uint64_t)cyclic_distance > *max_cyclic_distance){
	  *max_cyclic_distance = cyclic_distance;
	  *evict_frame = root;
	  *evicted_parent = *prev_parent;
	  *evicted_parent_row = *prev_parent_row;
	  *evicted_page_index = final_page_num;
	}
	return;
  }
  bool root_is_empty = true;
  word_t root_val;
  uint64_t tmp_num_page = final_page_num;
  for (int row = 0; row < PAGE_SIZE; ++row) {
	PMread(root * PAGE_SIZE + row, &root_val);
	if (root_val) {
	  root_is_empty = false;
	  if (root_val > *max_frame_idx) {
		*max_frame_idx = root_val;
	  }
	  concatenate(&tmp_num_page, row, level);
	  DFS(root_val, level + 1, next_frame, max_frame_idx, next_frame_is_empty, parent,
		  &root, &row, page_num, max_cyclic_distance, evict_frame, tmp_num_page,
		  evicted_parent, evicted_parent_row, empty_parent, empty_parent_row, evicted_page_index);
	  tmp_num_page = final_page_num;
	}
  }
  if (root_is_empty && root != *parent) {
	*next_frame_is_empty = true;
	*next_frame = root;
	*empty_parent = *prev_parent;
	*empty_parent_row = *prev_parent_row;
	return;
  }
}

word_t get_next_frame(uint64_t page_num, word_t parent) {
  word_t next_frame, max_frame_idx = 0, prev_parent,
	  prev_parent_row, evict_frame, evicted_parent, evicted_parent_row, empty_parent, empty_parent_row;
  uint64_t max_cyclic_distance = 0, final_page_num = 0, evicted_page_index = 0;
  bool next_frame_is_empty = false;
  DFS(0, 0, &next_frame, &max_frame_idx, &next_frame_is_empty,
	  &parent,&prev_parent, &prev_parent_row, page_num, &max_cyclic_distance,
	  &evict_frame,  final_page_num, &evicted_parent,&evicted_parent_row,&empty_parent, &empty_parent_row, &evicted_page_index);

  if (next_frame_is_empty && next_frame != parent) {// found an empty frame
	PMwrite(empty_parent * PAGE_SIZE + empty_parent_row, 0);
	return next_frame;
  } else if (max_frame_idx + 1 < NUM_FRAMES) {// found an unused frame
	return max_frame_idx + 1;
  }
  // evict
  PMwrite(evicted_parent * PAGE_SIZE + evicted_parent_row, 0);
  PMevict(evict_frame, evicted_page_index);

  return evict_frame;
}

word_t get_final_frame(uint64_t virtualAddress) {
  uint64_t tmp_address, page_num = virtualAddress >> OFFSET_WIDTH;
  word_t parent, next_frame = 0;
  int root_size = get_root_size();
  for (int level = 0; level < TABLES_DEPTH; ++level) {
	tmp_address = get_next_level(virtualAddress, level, root_size);
	parent = next_frame;
	PMread(next_frame * PAGE_SIZE + tmp_address, &next_frame);
	if (next_frame == 0) {
//	a. find next frame (unused/empty/evict)
	  next_frame = get_next_frame(page_num, parent);
//	b.
//	  if level == TABLES_DEPTH - 1, restore next_frame in page_num
//    else, initialize the next frame
	  if (level == TABLES_DEPTH - 1) {
		PMrestore(next_frame, page_num);
	  } else {
		for (int row = 0; row < PAGE_SIZE; row++) {
		  PMwrite(next_frame * PAGE_SIZE + row, 0);
		}
	  }
//	c. PMwrite
	  PMwrite(parent * PAGE_SIZE + tmp_address, next_frame);
	}
  }
  return next_frame;
}

int VMwrite(uint64_t virtualAddress, word_t value) {
  if (virtualAddress >= VIRTUAL_MEMORY_SIZE) {
	return FAILURE;
  }
  uint64_t offset = get_offset(virtualAddress);
// get final frame (by DFS traversal)
  word_t final_frame = get_final_frame(virtualAddress);
//  write value to final frame in physical memory
  PMwrite(final_frame * PAGE_SIZE + offset, value);
  return SUCCESS;
}

int VMread(uint64_t virtualAddress, word_t *value) {
  if (!value || virtualAddress >= VIRTUAL_MEMORY_SIZE) {
	return FAILURE;
  }
  uint64_t offset = get_offset(virtualAddress);
  word_t final_frame = get_final_frame(virtualAddress);
  PMread(final_frame * PAGE_SIZE + offset, value);
  return SUCCESS;
}