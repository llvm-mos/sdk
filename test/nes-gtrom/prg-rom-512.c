#include <mapper.h>
#include <peekpoke.h>
#include <stdlib.h>

asm(".weak __prg_rom_size \n __prg_rom_size = 512 \n");

volatile const char c[15000] = {1, [14999] = 2};

__attribute__((section(".prg_rom_0.rodata"))) volatile const char d[15000] = {
    3, [14999] = 4};

__attribute__((section(".prg_rom_15.rodata"))) volatile const char e[15000] = {
    5, [14999] = 6};

__attribute__((noinline, section(".prg_rom_0.text"))) char bank_0_fn(void) {
  asm volatile("");
  return 7;
}

__attribute__((noinline, section(".prg_rom_15.text"))) char bank_15_fn(void) {
  asm volatile("");
  return 8;
}

int main(void) {
  if (c[0] != 1 || c[14999] != 2)
    return EXIT_FAILURE;

  set_prg_bank(0);
  if (c[0] != 1 || c[14999] != 2)
    return EXIT_FAILURE;
  if (d[0] != 3 || d[14999] != 4)
    return EXIT_FAILURE;

  set_prg_bank(15);
  if (c[0] != 1 || c[14999] != 2)
    return EXIT_FAILURE;
  if (e[0] != 5 || e[14999] != 6)
    return EXIT_FAILURE;

  set_prg_bank(0);
  if ((unsigned)bank_0_fn >= 0xc000 || bank_0_fn() != 7)
    return EXIT_FAILURE;

  set_prg_bank(15);
  if ((unsigned)bank_15_fn >= 0xc000 || bank_15_fn() != 8)
    return EXIT_FAILURE;

  return EXIT_SUCCESS;
}
