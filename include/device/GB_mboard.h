#ifndef GB_BOARD_H_
# define GB_BOARD_H_

# include "memory/GB_bus.h"

# include "device/GB_interrupt.h"
# include "device/GB_wram.h"

namespace GB::device {

    class MBoard {
    public:
    private:
        memory::BusInterface*   __memoryBus;

        WRAM*                   __wram;
        InterruptController*    __interruptController;
    };

}

#endif
