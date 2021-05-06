/**
 * @file GB_vaddr.h
 * @brief File have Serial I/O addresses enumeration
 */

#ifndef GB_VADDR_H_
# define GB_VADDR_H_

# include "common/GB_types.h"

namespace GB::memory {

    /**
     * @brief Enumeration of virtual addresses
     */
    enum VirtualAddress : Word {
        /* Joypad controller */
        P1_VADDR        = 0xFF00,

        /* CPU contoller */
        KEY1_VADDR      = 0xFF4D,

        /* Bank contoller registers */
        VBK_VADDR       = 0xFF4F,
        SVBK_VADDR      = 0xFF70,

        /* Infared communication port */
        RP_VADDR        = 0xFF56,

        /* Serial communication */
        SB_VADDR        = 0xFF01,
        SC_VADDR        = 0xFF02,

        /* Timer */
        DIV_VADDR       = 0xFF04,
        TIMA_VADDR      = 0xFF05,
        TMA_VADDR       = 0xFF06,
        TAC_VADDR       = 0xFF07,

        /* Interrupts */
        IF_VADDR        = 0xFF0F,
        IE_VADDR        = 0xFFFF,

        /* Interrupts JMP table */
        VBLANK_JMP_VADDR  = 0x0040,
        LCDSTAT_JMP_VADDR = 0x0048,
        TIMOVER_JMP_VADDR = 0x0050,
        SERIO_JMP_VADDR   = 0x0058,
        JOYPAD_JMP_VADDR  = 0x0060,

        /* Sound Mode 1 */
        NR10_VADDR      = 0xFF10,
        NR11_VADDR      = 0xFF11,
        NR12_VADDR      = 0xFF12,
        NR13_VADDR      = 0xFF13,
        NR14_VADDR      = 0xFF14,

        /* Sound Mode 2 */
        NR21_VADDR      = 0xFF16,
        NR22_VADDR      = 0xFF17,
        NR23_VADDR      = 0xFF18,
        NR24_VADDR      = 0xFF19,

        /* Sound Mode 3 */
        NR30_VADDR      = 0xFF1A,
        NR31_VADDR      = 0xFF1B,
        NR32_VADDR      = 0xFF1C,
        NR33_VADDR      = 0xFF1D,
        NR34_VADDR      = 0xFF1E,

        /* Sound Mode 4 */
        NR41_VADDR      = 0xFF20,
        NR42_VADDR      = 0xFF21,
        NR43_VADDR      = 0xFF22,
        NR44_VADDR      = 0xFF23,

        /* Sound controller */
        NR50_VADDR      = 0xFF24,
        NR51_VADDR      = 0xFF25,
        NR52_VADDR      = 0xFF26,

        /* Waveform RAM */
        WVF_RAM_BASE_VADDR = 0xFF30,
        WVF_RAM_LAST_VADDR = 0xFF3F,

        /* LCD contoller */
        LCDC_VADDR      = 0xFF40,
        STAT_VADDR      = 0xFF41,
        SCY_VADDR       = 0xFF42,
        SCX_VADDR       = 0xFF43,
        LY_VADDR        = 0xFF44,
        LYC_VADDR       = 0xFF45,
        DMA_VADDR       = 0xFF46,
        BGP_VADDR       = 0xFF47,
        OBP0_VADDR      = 0xFF48,
        OBP1_VADDR      = 0xFF49,
        WY_VADDR        = 0xFF4A,
        WX_VADDR        = 0xFF4B,

        /* HDMA controller */
        HDMA1_VADDR     = 0xFF51,
        HDMA2_VADDR     = 0xFF52,
        HDMA3_VADDR     = 0xFF53,
        HDMA4_VADDR     = 0xFF54,
        HDMA5_VADDR     = 0xFF55,

        /* LCD color palletes */
        BCPS_VADDR      = 0xFF68,
        BCPD_VADDR      = 0xFF69,
        OCPS_VADDR      = 0xFF6A,
        OCPD_VADDR      = 0xFF6B,

        /* OAM RAM */
        OAM_RAM_BASE_VADDR = 0xFE00,
        OAM_RAM_LAST_VADDR = 0xFE9F,
        OAM_ECHO_BASE_VADDR = 0xFEA0, //TODO: maybe we can map those address to our oam search result
        OAM_ECHO_LAST_VADDR = 0xFEFF,

        /* High stack RAM */
        HRAM_BASE_VADDR = 0xFF80,
        HRAM_LAST_VADDR = 0xFFEF,

        /* Cartrige ROM */
        ROM0_BASE_VADDR = 0x0000,
        ROM0_LAST_VADDR = 0x3FFF,
        ROMX_BASE_VADDR = 0x4000,
        ROMX_LAST_VADDR = 0x7FFF,

        /* Video RAM */
        VRAM_BASE_VADDR = 0x8000,
        VRAM_LAST_VADDR = 0x9FFF,

        /* External cartrige RAM */
        SRAM_BASE_VADDR = 0xA000,
        SRAM_LAST_VADDR = 0xBFFF,

        /* Work unit RAM */
        WRAM0_BASE_VADDR = 0xC000,
        WRAM0_LAST_VADDR = 0xCFFF,
        WRAMX_BASE_VADDR = 0xD000,
        WRAMX_LAST_VADDR = 0xDFFF,
        WRAM0_ECHO_BASE_VADDR = 0xE000,
        WRAM0_ECHO_LAST_VADDR = 0xEFFF,
        WRAMX_ECHO_BASE_VADDR = 0xF000,
        WRAMX_ECHO_LAST_VADDR = 0xFDFF,

        /* RST instruction JMP table */
        RST0_JMP_VADDR  = 0x0000,
        RST1_JMP_VADDR  = 0x0008,
        RST2_JMP_VADDR  = 0x0010,
        RST3_JMP_VADDR  = 0x0018,
        RST4_JMP_VADDR  = 0x0020,
        RST5_JMP_VADDR  = 0x0028,
        RST6_JMP_VADDR  = 0x0030,
        RST7_JMP_VADDR  = 0x0038,

    };

}

#endif
