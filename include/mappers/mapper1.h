#pragma once
#include "mapper.h"

class Mapper1 : public Mapper
{

  public:
    Mapper1( u16 prg_size, u16 chr_size );
    auto             TranslateCPUAddress( u16 address ) -> u16 override;
    auto             TranslatePPUAddress( u16 address ) -> u16 override;
    void             HandleCPUWrite( u16 address, u8 data ) override;
    [[nodiscard]] u8 GetMirrorMode() override;

  private:
    // Shifrt register variables
    u8 _shift_register = 0;
    u8 _bits_loaded = 0;

    // Control and bank registers
    u8 _control = 0x0C; // Default: PRG ROM bank mode 3, vertical mirroring
    u8 _chr_bank_0 = 0;
    u8 _chr_bank_1 = 0;
    u8 _prg_bank = 0;

    // Bank ofsets
    u32 _prg_bank_offset_0 = 0;
    u32 _prg_bank_offset_1 = 0;
    u32 _chr_bank_offset_0 = 0;
    u32 _chr_bank_offset_1 = 0;

    // Update bank offsets based on current register values
    void UpdateOffsets();
};
