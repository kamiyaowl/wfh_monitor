#ifndef TASKBASE_H
#define TASKBASE_H

#include <cstdint>

#include <Seeed_Arduino_FreeRTOS.h>

/**
 * @brief FreeRTOSのTaskをWrapした基底クラスです
 */
class TaskBase {
    public:
        /**
         * @brief Construct a new Task Base object
         */
        TaskBase(void): isRunning(false) {}

        /**
         * @brief Destroy the Task Base object
         */
        virtual ~TaskBase(void) { this->deleteTask(); }

        /**
         * @brief Create a Task object
         * 
         * @param stackSize FreeRTOSで割り当てるStackSize
         * @param priority Task優先度
         */
        void createTask(size_t stackSize, uint32_t priority);

        /**
         * @brief Delete Task
         */
        void deleteTask(void);

        /**
         * @brief Get the Task Handle object
         * @note このオブジェクトを使用してTaskの状態を変化させないでください
         * 
         * @return TaskHandle_t 
         */
        TaskHandle_t getTaskHandle(void);

        /**
         * @brief Get the Name object
         * 
         * @return const char* TaskName
         */
        virtual const char* getName(void) = 0;
    protected:
        TaskHandle_t taskHandle;
        bool isRunning;

        /**
         * @brief FreeRTOSから起動されるTask本体です
         */
        virtual void taskMain(void);

        /**
         * @brief Task起動後1回だけ呼び出されます
         * @note loop()処理前に実施したい内容は個々に実装します
         */
        virtual void setup(void) {};

        /**
         * @brief Taskで呼び出され続ける関数です。戻り値でTaskの継続可否を制御できます
         * 
         * @return true Taskを終了する
         * @return false Taskを継続する
         */
        virtual bool loop(void) = 0;

        /**
         * @brief Taskをloop()の戻り値制御でAbortした際に、1回だけ呼び出されます
         */
        virtual void abort(void) {};
};

#endif /* TASKBASE_H */