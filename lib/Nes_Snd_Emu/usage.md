## Using the APU in an emulator

---

Two code skeletons are shown below. The first shows how to add basic APU support to an
emulator that doesn't keep track of overall CPU time. The second shows how to add full
APU support (including IRQs) to a framework which keeps track of overall CPU time.

## Basic APU support

---

```cpp
#include "Nes_Apu.h"

Blip_Buffer buf;
Nes_Apu apu;

void output_samples( const blip_sample_t*, size_t count );
const size_t out_size = 4096;
blip_sample_t out_buf [out_size];

int total_cycles;
int cycles_remain;

int elapsed()
{
 return total_cycles - cycles_remain;
}

const int apu_addr = 0x4000;

void cpu_write_memory( cpu_addr_t addr, int data )
{
 // ...
 if ( addr >= apu.start_addr && addr <= apu.end_addr )
  apu.write_register( elapsed(), addr, data );
}

int cpu_read_memory( cpu_addr_t addr )
{
 // ...
 if ( addr == apu.status_addr )
  return apu.read_status( elapsed() );
}

int dmc_read( void*, cpu_addr_t addr )
{
 return cpu_read_memory( addr );
}

void emulate_cpu( int cycle_count )
{
 total_cycles += cycle_count;
 cycles_remain += cycle_count;

 while ( cycles_remain > 0 )
 {
  // emulate opcode
  // ...
  cycles_remain -= cycle_table [opcode];
 }
}

void end_time_frame( int length )
{
 apu.end_frame( length );
 buf.end_frame( length );
 total_cycles -= length;

 // Read some samples out of Blip_Buffer if there are enough to
 // fill our output buffer
 if ( buf.samples_avail() >= out_size )
 {
  size_t count = buf.read_samples( out_buf, out_size );
  output_samples( out_buf, count );
 }
}

void render_frame()
{
 // ...
 end_time_frame( elapsed() );
}

void init()
{
 blargg_err_t error = buf.sample_rate( 44100 );
 if ( error )
  report_error( error );
 buf.clock_rate( 1789773 );
 apu.output( &buf );

 apu.dmc_reader( dmc_read );
}
```

## Full APU support

```cpp

#include "Nes_Apu.h"

Blip_Buffer buf;
Nes_Apu apu;

void output_samples( const blip_sample_t*, size_t count );
const size_t out_size = 4096;
blip_sample_t out_buf [out_size];

cpu_time_t cpu_end_time; // Time for CPU to stop at
cpu_time_t cpu_time;     // Current CPU time relative to current time frame

unsigned apu_addr = 0x4000;

void cpu_write_memory( cpu_addr_t addr, int data )
{
 // ...
 if ( addr >= apu.start_addr && addr <= apu.end_addr )
  apu.write_register( cpu_time, addr, data );
}

int cpu_read_memory( cpu_addr_t addr )
{
 // ...
 if ( addr == apu.status_addr )
  return apu.read_status( cpu_time );
}

int dmc_read( void*, cpu_addr_t addr )
{
 return cpu_read_memory( addr );
}

void emulate_cpu()
{
 while ( cpu_time < cpu_end_time )
 {
  // Decode instruction
  // ...
  cpu_time += cycle_table [opcode];
  switch ( opcode )
  {
   // ...
   case 0x58: // CLI
    if ( cpu_status & i_flag )
    {
     cpu_status &= ~i_flag;
     return; // I flag cleared; stop CPU immediately
    }
  }
 }
}

// Time of next IRQ if before end_time, otherwise end_time
cpu_time_t earliest_irq_before( cpu_time_t end_time )
{
 if ( !(cpu_status & i_flag) )
 {
  cpu_time_t irq_time = apu.earliest_irq();
  if ( irq_time < end_time )
   end_time = irq_time;
 }
 return end_time;
}

// IRQ time may have changed, so update CPU end time
void irq_changed( void* )
{
 cpu_end_time = earliest_irq_before( cpu_end_time );
}

// Run CPU to 'end_time' (possibly a few cycles over depending on instruction)
void run_cpu_until( cpu_time_t end_time )
{
 while ( cpu_time < end_time )
 {
  cpu_end_time = earliest_irq_before( end_time );
  if ( cpu_end_time <= cpu_time )
  {
   // Save PC and status, load IRQ vector, set I flag, etc.
   cpu_trigger_irq();

   // I flag is now set, so CPU can be run for full time
   cpu_end_time = end_time;
  }

  emulate_cpu();
 }
}

// Run CPU for at least 'cycle_count'
void run_cpu( int cycle_count )
{
 run_cpu_until( cpu_time + cycle_count );
}

// End a time frame and make its samples available for reading
void end_time_frame( cpu_time_t length )
{
 apu.end_frame( length );
 buf.end_frame( length );
 cpu_time -= length;

 // Read some samples out of Blip_Buffer if there are enough to
 // fill our output buffer
 if ( buf.samples_avail() >= out_size )
 {
  size_t count = buf.read_samples( out_buf, out_size );
  output_samples( out_buf, count );
 }
}

// Emulator probably has a function which renders a video frame
void render_video_frame()
{
 for ( int n = scanline_count; n--; )
 {
  run_cpu( 113 ); // or whatever would be done here
  // ...
 }
 // ...
 end_time_frame( cpu_time );
}

void init()
{
 blargg_err_t error = buf.sample_rate( 44100 );
 if ( error )
  report_error( error );
 buf.clock_rate( 1789773 );
 apu.output( &buf );

 apu.dmc_reader( dmc_read );
 apu.irq_notifier( irq_changed );
}
```
