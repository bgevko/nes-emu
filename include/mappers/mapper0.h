#pragma once
#include "mapper.h"

class Mapper0 : public Mapper
{
  public:
    Mapper0( u16 prg_size, u16 chr_size ) : Mapper( prg_size, chr_size ) {}
    auto             TranslateCPUAddress( u16 address ) -> u16 override;
    auto             TranslatePPUAddress( u16 address ) -> u16 override;
    void             HandleCPUWrite( u16 address, u8 data ) override;
    [[nodiscard]] u8 GetMirrorMode() override { return 0; }
};
