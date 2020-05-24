#ifndef UITASK_H
#define UITASK_H

#include <LovyanGFX.hpp>
#include "TaskBase.h"

/**
 * @brief UserInterfaceの表示を行うタスクです
 */
class UiTask : public TaskBase {
    public:
        UiTask(Serial_& serial, LGFX& lcd, LGFX_Sprite& sprite): counter(0), serial(serial), lcd(lcd), sprite(sprite) {}
        virtual ~UiTask(void) {}
        const char* getName(void) override { return "UiTask"; }
    private:
        uint32_t counter; /**< for debug*/
        Serial_& serial; /**< for debug */
        LGFX& lcd;
        LGFX_Sprite& sprite;
        void setup(void) override;
        bool loop(void) override;
};

#endif /* UITASK_H */