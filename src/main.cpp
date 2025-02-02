#include "SDL2/SDL_error.h"
#include "SDL2/SDL_events.h"
#include "SDL2/SDL_pixels.h"
#include "SDL2/SDL_render.h"
#include "SDL2/SDL_video.h"
#include "bus.h"
#include "cartridge.h"
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <iostream>
#include <chrono>

using u32 = uint32_t;
using namespace std;

constexpr int SCREEN_WIDTH = 512;  // NOLINT
constexpr int SCREEN_HEIGHT = 480; // NOLINT
constexpr int NES_WIDTH = 256;     // NOLINT
constexpr int NES_HEIGHT = 240;    // NOLINT
constexpr int BUFFER_SIZE = 61440; // NOLINT

SDL_Texture *texture = nullptr; // NOLINT

void renderFrame( const u32 *frameBufferData )
{
    void *pixels = nullptr;
    int   pitch = 0;

    if ( SDL_LockTexture( texture, nullptr, &pixels, &pitch ) == 0 ) {
        memcpy( pixels, frameBufferData, BUFFER_SIZE * sizeof( u32 ) );
        SDL_UnlockTexture( texture );
    }
}

int main()
{
    if ( SDL_Init( SDL_INIT_VIDEO ) != 0 ) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << '\n';
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow( "NES Emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                           SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );

    if ( window == nullptr ) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << '\n';
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED );
    if ( renderer == nullptr ) {
        std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << '\n';
        SDL_DestroyWindow( window );
        SDL_Quit();
        return 1;
    }

    texture = SDL_CreateTexture( renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, NES_WIDTH,
                                 NES_HEIGHT );

    if ( texture == nullptr ) {
        std::cerr << "SDL_CreateTexture Error: " << SDL_GetError() << '\n';
        SDL_DestroyRenderer( renderer );
        SDL_DestroyWindow( window );
        SDL_Quit();
        return 1;
    }

    // Initiate the emulator
    Bus bus;
    CPU cpu = bus.cpu;
    PPU ppu = bus.ppu;

    // Load the ROM
    shared_ptr<Cartridge> cartridge = make_shared<Cartridge>( "tests/roms/mario.nes" );
    bus.LoadCartridge( cartridge );
    cpu.Reset();

    // PPU render callback
    ppu.onFrameReady = renderFrame;

    // SDL_Event event;

    // Benchmark stuff
    using Clock = chrono::high_resolution_clock;
    auto start = Clock::now();
    u64  totalCycles = 0;

    bool running = true;
    while ( running ) {
        // while ( SDL_PollEvent( &event ) != 0 ) {
        //     if ( event.type == SDL_QUIT ) {
        //         running = false;
        //     }
        // }

        // Execute the CPU
        cpu.DecodeExecute();
        totalCycles += cpu.GetLastInstrCycleCost();
        // cout << cpu.GetCycles() << "\n";

        if ( totalCycles > 1790000 ) {
            auto now = Clock::now();
            auto elapsed = chrono::duration_cast<chrono::milliseconds>( now - start ).count();
            cout << "Elapsed time: " << elapsed << "ms\n";
            cout << "Total cycles: " << totalCycles << "\n";
            running = false;
        }

        // Clear screen
        // SDL_SetRenderDrawColor( renderer, 0, 0, 0, 255 );
        // SDL_RenderClear( renderer );

        // Copy NES framebuffer texture
        // SDL_RenderCopy( renderer, texture, nullptr, nullptr );
        // SDL_RenderPresent( renderer );
    }

    // Cleanup
    SDL_DestroyTexture( texture );
    SDL_DestroyRenderer( renderer );
    SDL_DestroyWindow( window );
    SDL_Quit();

    return 0;
}
