#include "device/GB_wram.h"
#include "memory/GB_vaddr.h"

namespace GB::device {

// static byte_t read_SVBK_reg(WRAM* dev, word_t) {
//     return dev->get_SVBK_reg();
// }

// static void write_SVBK_reg(WRAM* dev, word_t, byte_t value) {
//     dev->set_SVBK_reg(value);
// }

// static byte_t read_laddr_gb_wram(WRAM* dev, word_t laddr_gb) {
//     const word_t laddr = laddr_gb - memory::WRAM0_BASE_VADDR;
//     return dev->read_inner_vaddr(laddr);
// }

// static void write_laddr_gb_wram(WRAM* dev, word_t laddr_gb, byte_t data) {
//     const word_t laddr = laddr_gb - memory::WRAM0_BASE_VADDR;
//     dev->write_inner_vaddr(laddr, data);
// }

// static byte_t read_laddr_gb_wram_echo(WRAM* dev, word_t laddr_gb) {
//     const word_t laddr = laddr_gb - memory::WRAM0_ECHO_BASE_VADDR;
//     return dev->read_inner_vaddr(laddr);
// }

// static void write_laddr_gb_wram_echo(WRAM* dev, word_t laddr_gb, byte_t data) {
//     const word_t laddr = laddr_gb - memory::WRAM0_ECHO_BASE_VADDR;
//     dev->write_inner_vaddr(laddr, data);
// }

// void WRAM::map_to_memory(memory::BusInterface& mem_bus) {
//     mem_bus.MapVAddr(memory::SVBK_VADDR
//                     , memory::BusInterface::ReadCmd(read_SVBK_reg)
//                     , memory::BusInterface::WriteCmd(write_SVBK_reg)
//                     , this);
//     mem_bus.MapVAddr(memory::WRAM0_BASE_VADDR, memory::WRAMX_LAST_VADDR
//                     , memory::BusInterface::ReadCmd(read_laddr_gb_wram)
//                     , memory::BusInterface::WriteCmd(write_laddr_gb_wram)
//                     , this);
//     mem_bus.MapVAddr(memory::WRAM0_ECHO_BASE_VADDR, memory::WRAMX_ECHO_LAST_VADDR
//                     , memory::BusInterface::ReadCmd(read_laddr_gb_wram_echo)
//                     , memory::BusInterface::WriteCmd(write_laddr_gb_wram_echo)
//                     , this);
// }

}  // namespace GB::device
