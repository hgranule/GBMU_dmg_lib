/**
 * @file GB_clock.h
 * @brief File describes some functionallity to deal with clock cycles
 */

#ifndef GB_CLOCK_H_
# define GB_CLOCK_H_

# include <algorithm>
# include <type_traits>

using CLKCycle = int64_t; ///< type for representing clock cycles

constexpr static auto MCYCLE_TO_CLK_CYCLE = 0x4;

constexpr inline unsigned long long
operator "" _CLKCycles(unsigned long long cCycles) { return cCycles; }

constexpr inline unsigned long long
operator "" _MCycles(unsigned long long mCycles) { return mCycles * MCYCLE_TO_CLK_CYCLE; }

namespace sync {

    /**
     * @brief returns true when we have cycles to do something
     */
    inline bool Synced(CLKCycle clock) {
        return clock >= 0;
    }

    /**
     * @brief pay for action
     */
    inline void StepPay(CLKCycle& clock, CLKCycle ccls) {
        clock -= ccls;
    }

    /**
     * @brief give cycles to a clock
     */
    inline void StepDone(CLKCycle& clock, CLKCycle ccls) {
        clock = std::min(clock + ccls, CLKCycle(0));
    }

    /**
     * @brief Class decorator for synced device with clock counter
     * @tparam _Device class which represents a clocked device
     * @tparam _stepLong clock cycles num which represents a long of one device step
     * 
     * @note
     *     Devices must have an implementation of an Action() method. This method
     *     is called by SyncedDevice decorator every moment when clock counter
     *     have value greater or equal to zero (which means clock counter have quota to action).
     *     
     *     Every action have cycles amount to make something. There are two methods
     *     to make an action of the device using SyncedDevice decorator.
     *     
     *     1) `Step(cyclesPayment, ActionArguments...) -> calls Action(ActionArguments...);`  
     *        SyncedDevice "pays" clock cycles by itself using ::sync::StepPay(clockCounterReference, cyclesPayment) function.
     *     
     *     2) `Step(ActionArguments...) -> calls Action(clockReference, ActionArguments...);`  
     *        SyncedDevice provide a reference to a clock counter and the Action method must "pay"
     *        by itself using ::sync::StepPay(clockCounterReference, cyclesPayment);
     *        So cyclesPayment defined by devices Action method.
     * 
     *     "Pay clock cycle" - to take some clock cycles from clock counter, so it looses its quota to action.
     *
     */
    template < class _Device, CLKCycle _stepLong = 1_MCycles >
    class SyncedDevice {
    private:
        CLKCycle        __clock;
        _Device         __device;

    public:

        template <typename... _DeviceContructArgs>
        SyncedDevice(_DeviceContructArgs... dargs)
        : __clock(0)
        , __device(dargs...)
        {}

        SyncedDevice(const SyncedDevice& source)
        : __clock(source.__clock)
        , __device(source.__device)
        {}

        SyncedDevice(SyncedDevice&& source)
        : __clock(std::move(source.__clock))
        , __device(std::move(source.__device))
        {}

        SyncedDevice& operator=(const SyncedDevice& source) {
            __clock = source.__clock;
            __device = source.__device;
        }

        SyncedDevice& operator=(SyncedDevice&& source) {
            __clock = std::move(source.__clock);
            __device = std::move(source.__device);
        }

        ~SyncedDevice() { }

        /**
         * @brief Call and synchronise devices action method, and take some cycles from clock counter.
         *        Action(elipsedArgs...) method is called in that case.
         * 
         * @param[in] deviceStepPay cycles num to "pay" (take from clock counter)
         * @param[in] dCallArgs elipsed arguments to device action method.
         * 
         * @note The action method called only when clock counter have such cycle quota!
         */
        template <typename _Num, typename... _DeviceCallArgs>
        inline void Step(_Num deviceStepPay, _DeviceCallArgs... dCallArgs) {
            if (::sync::Synced(__clock)) {
                __device.Action(dCallArgs...);
                ::sync::StepPay(__clock, deviceStepPay);
            }
            ::sync::StepDone(__clock, _stepLong);
        }

        /**
         * @brief Call and synchronise devices action method, but the slave device takes cycles from clock counter
         *        by itself. Action(clock, elipsedArgs...) method is called in that case.
         * 
         * @param[in] dCallArgs elipsed arguments to device action method.
         * 
         * @note The action method called only when clock counter have such cycle quota!
         */
        template <typename... _DeviceCallArgs>
        inline void Step(_DeviceCallArgs... dCallArgs) {
            if (::sync::Synced(__clock))
                __device.Action(__clock, dCallArgs...);
            ::sync::StepDone(__clock, _stepLong);
        }

        /**
         * @brief returns the reference a to slave synced device
         */
        inline _Device& Dev() {
            return __device;
        }

        /**
         * @brief returns duration of device step in
         */
        inline CLKCycle StepLong() const {
            return _stepLong;
        }
    };

    /**
     * @brief synchronized to clock decorator for a synced action
     */
    template< CLKCycle _cyclesPrice >
    struct Action {

        /** Decorator for a not method function */
        template<typename _Func, typename... _CBArgs>
        inline auto operator()(CLKCycle& clock, _Func cbFunc, _CBArgs... cbArgs) {
            StepPay(clock, _cyclesPrice);
            return cbFunc(cbArgs...);
        }

        /** Decorator for a method function */
        template<typename _Ret, class _Class, typename... _CBArgs>
        inline _Ret operator()(CLKCycle& clock, _Ret (_Class::*cbFunc)(_CBArgs...), _Class* cbOwner, _CBArgs... cbArgs) {
            StepPay(clock, _cyclesPrice);
            return (cbOwner->*cbFunc)(cbArgs...);
        }

    };

}

#endif
