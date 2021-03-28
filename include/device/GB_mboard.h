#ifndef GB_BOARD_H_
# define GB_BOARD_H_

# include "memory/GB_bus.h"

# include "device/GB_interrupt.h"
# include "device/GB_wram.h"

namespace GB {

    class MBoard {
    public:
    private:
        memory::UMBus*          __memoryBus;

        WRAM*                   __wram;
        InterruptController*    __interruptController;
    };

}

#endif
