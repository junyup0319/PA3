#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <stdint.h>
#include <assert.h>

static struct
{
  unsigned int num_page_fault;
  unsigned int num_page_try;
  unsigned int num_tlb_miss;
  unsigned int num_tlb_try;
} perf_stat = {
  .num_page_fault = 0,
  .num_page_try = 0,
  .num_tlb_miss = 0,
  .num_tlb_try = 0
};

struct tlb_entry
{
  uint8_t page_num;
  uint8_t frame_num;
  int is_valid; // 1 is valid or not.
};

struct page_entry
{
  uint16_t frame_num;
  int is_valid; // 1 is valid or not.
};

uint8_t lookup_tlb(uint8_t page_num);
uint8_t lookup_page_table(uint8_t page_num);
uint8_t lookup_phy_mem(uint32_t phy_addr);

void page_fault_handler(uint8_t page_num);
uint32_t to_phy_addr(uint32_t virt_addr);

struct tlb_entry tlb[16];
int tlb_fifo_idx = 0;
struct page_entry page_table[256];
uint8_t phy_mem[256 * 256] = {0,};

int main(int argc, char** argv)
{
  // Clean tlb and page table.
  for (int it = 0; it < 16; ++it) {
    tlb[it].is_valid = 0;
  }
  for (int it = 0; it < 256; ++it) {
    page_table[it].is_valid = 0;
  }

  uint32_t virt_addr;
  while (scanf("%u", &virt_addr) != EOF) {
    uint32_t phy_addr = to_phy_addr(virt_addr);

    fprintf(stderr, "%d\n", lookup_phy_mem(phy_addr));
  }

  printf("pf: %lf\ntlb: %lf\n",
    (double)perf_stat.num_page_fault / perf_stat.num_page_try,
    (double)perf_stat.num_tlb_miss / perf_stat.num_tlb_try);

  return 0;
}

uint8_t lookup_tlb(uint8_t page_num)
{
  perf_stat.num_tlb_try++;

  for (struct tlb_entry* it = tlb;
       it < tlb + 16;
       it++) {
    if (it->is_valid && it->page_num == page_num) {
      return it->frame_num;
    }
  }

  perf_stat.num_tlb_miss++;

  uint8_t frame_num = lookup_page_table(page_num);

  struct tlb_entry* it = tlb + tlb_fifo_idx;
  tlb_fifo_idx = ++tlb_fifo_idx % 16;

  it->page_num = page_num;
  it->frame_num = frame_num;
  it->is_valid = 1;

  return it->frame_num;
}

uint8_t lookup_page_table(uint8_t page_num)
{
  if (!page_table[page_num].is_valid) {
    page_fault_handler(page_num);
    perf_stat.num_page_fault++;
  }

  assert(page_table[page_num].is_valid);

  perf_stat.num_page_try++;

  return page_table[page_num].frame_num;
}

void page_fault_handler(uint8_t page_num)
{
  // TODO: Fill this!
  FILE* fp = fopen("./input/BACKINGSTORE.bin", "r");

  // TODO 이렇게 읽어오는게 맞아?
  // 값 확인하기....
  fseek(fp, page_num * 256, SEEK_SET);
  char backingstore_page[256];
  fread(backingstore_page, 1, 256, fp);
  // fprintf(stderr, "buf: %s\n", backingstore_page);


  // phy_mem 에 저장
  // for 문을 돌면서 page_num*256 + i에 한 비트를 저장해야하나?
  int i;
  for(i = 0; i < 256; i++) {
    phy_mem[page_num * 256 + i] = backingstore_page[i];
  }

  // page_table 에 저장
  page_table[page_num].frame_num = page_num;
  page_table[page_num].is_valid = 1;
  fclose(fp);
}

uint32_t to_phy_addr(uint32_t virt_addr)
{
  // TODO: Make it work!
  uint8_t offset = 0x00ff & virt_addr;
  uint8_t page_number = (0xff00 & virt_addr) >> 8;

  // printf(">offset:%d\n", offset);
  // printf(">page number:%d\n", page_number);

  uint8_t frame_number = lookup_tlb(page_number);

  // printf(">frame number:%d\n", frame_number);


  // TODO
  // frame_number & offset으로 physical memeory를 찾아야함
  uint32_t phy_mem_addr = (frame_number << 8) | offset;
  // fprintf(stderr, "<<%x\n", phy_mem_addr);
  // return 0xdeadbeaf;
  return phy_mem_addr;
}

uint8_t lookup_phy_mem(uint32_t phy_addr)
{
  // TODO: Make it work!

  uint8_t offset = 0x00ff & phy_addr;
  uint8_t frame_number = (0xff00 & phy_addr) >> 8;
  
  // fprintf(stderr, "pppp>> %x/\n", phy_addr);
  return phy_mem[frame_number * 256 + offset];
  // return phy_mem[phy_addr];
  // return 0xbe; 
}
