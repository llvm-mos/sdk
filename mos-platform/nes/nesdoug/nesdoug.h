#ifndef _NESDOUG_H_
#define _NESDOUG_H_

#ifdef __cplusplus
extern "C" {
#endif

// Written by Doug Fraker
// version 1.2, 1/1/2022

// Why am I doing so much with a vram_buffer? This is an automated system, which
// works when the screen is on. You can write to the buffer at any time.
// During NMI / v-blank, it automatically writes to the PPU.
// You don't have to worry about timing those writes. And, more importantly,
// you shouldn't have to turn the screen off...
// allowing us to make smooth scrolling games.

// VRAM_BUF operations updated by Matthew "Cogwheel" Orlando - 2023-09-27
//
// Instead of using the top two bits of the address to identify the operation,
// values above 0x2F (the largest high byte possible for a nametable address)
// are treated as opcodes. This allows the system to support up to 208 unique
// operations instead of the current 4 (of which only 3 are used).
//
// The following typedefs are representative only; they are implicit in the
// assembly implementation. Though you may be able to use them to build
// operations in ways that aren't currently supported by the API (e.g. adding
// them to a queue)

#define NAMETABLE_OP_SEQ_START 0x30

// Single-tile operations include just the address and one byte of data.
typedef struct {
  char address_hi;  // Always < NAMETABLE_OP_SEQ_START
  char address_lo;
  char data;
} nametable_op_one;

// TODO: check if this is redundant
typedef enum {
  nametable_dir_horz = 0x00,
  nametable_dir_vert = 0x80,
} nametable_dir;

// The kind of sequential operation to perform
// This will be ORed with nametable_dir for the full opcode
typedef enum : char {
  // copy nametable data directly into the buffer
  nametable_op_copy = NAMETABLE_OP_SEQ_START,

  // store a pointer to nametable data in the buffer
  nametable_op_ref,

  // fill nametable span with a single value
  nametable_op_fill,

  /* TODO these and more?:
  // copy RLE data
  rle_copy,

  // ref RLE data
  rle_ref,
  */
} nametable_opcode;

// Multi-tile ops include the opcode, address and size. The data follows the
// header in VRAM_BUF
typedef struct {
  nametable_opcode opcode;
  char address_hi;
  char address_lo;
  char size;
} nametable_op_seq_header;

// when display is enabled, vram access could only be done with this vram update
// system the function sets a pointer to the update buffer that contains data
// and addresses in a special format. It allows to write non-sequential bytes,
// as well as horizontal or vertical nametable sequences. buffer pointer could
// be changed during rendering, but it only takes effect on a new frame number
// of transferred bytes is limited by vblank time to disable updates, call this
// function with NULL pointer
//
// the update data format:
//  nametable_op_one - for a non-sequential write
//  nametable_op_seq_header, [bytes] - for a sequential write
//  NT_UPD_EOF to mark end of the buffer
//
// length of this data should be under 128 bytes
void set_nametable_update(const void *buf);

// sets the vram update to point to the vram_buffer. VRAM_BUF defined in crt0.s
// this can be undone by set_nametable_update(NULL)
// TODO: make a version that preserves the index so that the buffer can be
// swapped dynamically
void set_nametable_buffer(void);

// Write a single tile update to the buffer
__attribute__((leaf)) void nametable_buffer_one(char data, int ppu_address);

// Low level helper for copying data into the buffer
__attribute__((leaf)) void nametable_buffer_copy_op(nametable_opcode op,
                                                    const void *data,
                                                    int ppu_address, char len);
// Copy a span of raw data to the buffer
void nametable_buffer_copy(nametable_dir dir, const void *data, int ppu_address,
                           char len);
// Copy a horizontal span of data to the buffer
void nametable_buffer_copy_horz(const void *data, int ppu_address, char len);
// Copy a vertical span of data to the buffer
void nametable_buffer_copy_vert(const void *data, int ppu_address, char len);


// Low level helper for writing fill operations into buffer
__attribute__((leaf)) void nametable_buffer_fill_op(nametable_opcode op,
                                                    char data, int ppu_address,
                                                    char len);
// Set a single character to be filled for a span
void nametable_buffer_fill(nametable_dir dir, char data, int ppu_address, char len);
// Set a single character to be filled for a horizontal span
void nametable_buffer_fill_horz(char data, int ppu_address, char len);
// Set a single character to be filled for a vertical span
void nametable_buffer_fill_vert(char data, int ppu_address, char len);

// Low level helper for writing ref operations into buffer
__attribute__((leaf)) void nametable_buffer_ref_op(nametable_opcode op,
                                                   const void *data,
                                                   int ppu_address, char len);
// Set a pointer to be read from to fill a span
void nametable_buffer_ref(nametable_dir dir, const void *data, int ppu_address, char len);
// Set a pointer to be read from to fill a horizontal span
void nametable_buffer_ref_horz(const void *data, int ppu_address, char len);
// Set a pointer to be read from to fill a vertical span
void nametable_buffer_ref_vert(const void *data, int ppu_address, char len);

// BEGIN Legacy nesdoug APIs
//
// These are now aliases/wrappers for the updated APIs

// Use set_nametable_buffer instead
__attribute__((deprecated)) void set_vram_buffer(void);

// Use `nametable_buffer_one` instead
__attribute__((deprecated)) void one_vram_buffer(char data, int ppu_address);

// Use `nametable_buffer_copy` instead
__attribute__((deprecated, leaf)) void
multi_vram_buffer_horz(const void *data, char len, int ppu_address);

// Use `nametable_buffer_copy` instead
__attribute__((deprecated, leaf)) void
multi_vram_buffer_vert(const void *data, char len, int ppu_address);
// END Legacy nesdoug APIs

// pad 0 or 1, use AFTER pad_poll() to get the trigger / new button presses
// more efficient than pad_trigger, which runs the entire pad_poll code again
char get_pad_new(char pad);

// use this internal value to time events, this ticks up every frame
char get_frame_count(void);

// this will alter the tempo of music, range 1-12 are reasonable, low is faster
// default is 6
// music_play also sets the tempo, and any Fxx effect in the song will too
// you will probably have to repeatedly set_music_speed() every frame
// music_stop() and music_pause() also overwrite this value
void set_music_speed(char tempo);

// expects an object (struct) where the first 4 bytes are X, Y, width, height
// you will probably have to pass the address of the object like &object
// the struct can be bigger than 4 bytes, as long as the first 4 bytes are X, Y,
// width, height
__attribute__((leaf)) char check_collision(void *object1, void *object2);

// adapted from Shiru's "Chase" game code
// values must be 0-8, 0 = all black, 8 = all white, 4 = normal
__attribute__((leaf)) void pal_fade_to(char from, char to);

// x can be in the range 0-0x1ff, but any value would be fine, it discards
// higher bits
void set_scroll_x(unsigned x);

// y can be in the range 0-0x1ff, but any value would be fine, it discards
// higher bits NOTE - different system than neslib (which needs y in range
// 0-0x1df) the advantage here, is you can set Y scroll to 0xff (-1) to shift
// the screen down 1, which aligns it with sprites, which are shifted down 1
// pixel
void set_scroll_y(unsigned y);

// add a value to y scroll, keep the low byte in the 0-0xef range
// returns y scroll, which will have to be passed to set_scroll_y
__attribute__((leaf)) int add_scroll_y(char add, unsigned scroll);

// subtract a value from y scroll, keep the low byte in the 0-0xef range
// returns y scroll, which will have to be passed to set_scroll_y
__attribute__((leaf)) int sub_scroll_y(char sub, unsigned scroll);

// gets a ppu address from x and y coordinates (in pixels)
// x is screen pixels 0-255, y is screen pixels 0-239, nt is nametable 0-3
__attribute__((leaf)) int get_ppu_addr(char nt, char x, char y);

// gets a ppu address in the attribute table from x and y coordinates (in
// pixels) x is screen pixels 0-255, y is screen pixels 0-239, nt is nametable
// 0-3
__attribute__((leaf)) int get_at_addr(char nt, char x, char y);

// the next 4 functions are for my metatile system, as described in my tutorial
// nesdoug.com

// for the metatile system, pass it the addresses of the room data
// room data should be exactly 240 bytes (16x15)
// each byte represents a 16x16 px block of the screen
void set_data_pointer(const void *data);

// for the metatile system, pass it the addresses of the metatile data
// a metatile is a 16x16 px block
// metatiles is variable length, 5 bytes per metatile...
// TopL, TopR, BottomL, BottomR, then 1 byte of palette 0-3
// max metatiles = 51 (because 51 x 5 = 255)
void set_mt_pointer(const void *metatiles);

// will push 1 metatile and 0 attribute bytes to the vram_buffer
// make sure to set_vram_buffer(), and clear_vram_buffer(),
// and set_mt_pointer()
// "metatile" should be 0-50, like the metatile data
__attribute__((leaf)) void buffer_1_mt(int ppu_address, char metatile);

// will push 4 metatiles (2x2 box) and 1 attribute byte to the vram_buffer
// this affects a 32x32 px area of the screen, and pushes 17 bytes to the
// vram_buffer. make sure to set_vram_buffer(), and clear_vram_buffer(),
// set_data_pointer(), and set_mt_pointer()
// "index" is which starting byte in the room data, to convert to tiles.
// use index = (y & 0xf0) + (x >> 4); where x 0-255 and y 0-239;
// index should be 0-239, like the room data it represents
__attribute__((leaf)) void buffer_4_mt(int ppu_address, char index);

// same as flush_vram_update, but assumes that a pointer to the vram has been
// set already this is for when the screen is OFF, but you still want to write
// to the PPU with the vram_buffer system
__attribute__((leaf)) void flush_vram_update2(void);

// change the PPU's color emphasis bits
void color_emphasis(char color);

#define COL_EMP_BLUE 0x80
#define COL_EMP_GREEN 0x40
#define COL_EMP_RED 0x20
#define COL_EMP_NORMAL 0x00
#define COL_EMP_DARK 0xe0

// a version of split that actually changes the y scroll midscreen
// requires a sprite zero hit, or will crash
__attribute__((leaf)) void xy_split(unsigned x, unsigned y);

// For debugging. Insert at the end of the game loop, to see how much frame is
// left. Will print a gray line on the screen. Distance to the bottom = how much
// is left. No line, possibly means that you are in v-blank.
__attribute__((leaf)) void gray_line(void);

// for getting or modifying just 1 byte of an int

#define high_byte(a) *((char *)&a + 1)
#define low_byte(a) *((char *)&a)

// get from the frame count. You can use a button (start on title screen) to
// trigger
void seed_rng(void);

#ifdef __cplusplus
}
#endif

#endif // _NESDOUG_H_
