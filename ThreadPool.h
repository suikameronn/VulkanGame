#pragma once
#include <condition_variable>
#include <deque>
#include <functional>
#include <mutex>
#include <thread>
#include<any>

using inputTypes = std::vector<std::any>;

class ThreadPool {
private:

    static inline ThreadPool* instance = nullptr;

    //���[�h���Ȃǂ̕ʘg�ň�X���b�h�������Ă���
    class SingleWorker
    {
    private:

        std::pair<inputTypes,std::function<void(inputTypes&)>> currentTask;
        bool isRequestedTermination{ false };
        bool isInitilized{ false };
        std::thread thread;
        std::mutex mutex;
        std::condition_variable cond;

        //�X���b�h��ҋ@������
        void waitInitilize() {
            std::unique_lock<std::mutex> lock(mutex);
            cond.wait(lock, [this]() { return !isRequestedTermination || !currentTask.second; });
        }

        void procWorker() {
            waitInitilize();

            while (true)
            {
                std::unique_lock<std::mutex> lock(mutex);
                currentTask.first = {};
                currentTask.second = {};
                cond.notify_all();  // �����������Ă��Ȃ����Ƃ�ʒm����
                cond.wait(lock, [&]()
                    {
                        isInitilized = true;

                        // ���̏q����ł̓��b�N���擾���Ă���͂�
                        if (isRequestedTermination) 
                        {
                            return true;
                        }

                        if (currentTask.second)
                        {
                            return true;
                        }

                        return false;
                    });
                if (isRequestedTermination) {
                    break;
                }

                currentTask.second(currentTask.first);
            }
        }

    public:
        //�C���X�^���X�쐬�i�K�ŃX���b�h���쐬���A�ҋ@������
        SingleWorker() : thread([this]() { procWorker(); }) {}
        ~SingleWorker() {
            waitUntilIdle();
            requestTermination();
            if (thread.joinable()) {
                thread.join();
            }
        }

        //�X���b�h�v�[�����쐬���ꂽ���_�ŁA�Ăяo����A
        //�X���b�h�v�[���ւ̎Q�ƂƃX���b�h�̔ԍ�������
        void initialize()
        {
            std::unique_lock<std::mutex> lock(mutex);
            cond.notify_all();
        }

        void push(std::pair<inputTypes, std::function<void(inputTypes&)>>& function)
        {
            //���������ʃX���b�h�ŏI���܂ŁA���C���X���b�h��ҋ@������
            while (!isInitilized)
            {

            }

            currentTask = function;

            cond.notify_all();
        }

        //���g�̃X���b�h�̒ʒm�𑗂�
        void wakeup() {
            std::unique_lock<std::mutex> lock(mutex);
            cond.notify_all();
        }

        //�ʒm�����邩�A���[�J���̃L���[����ɂȂ�܂ŁA�^�X�N�̒��f�ʒm�������܂őҋ@����
        void waitUntilIdle() {
            std::unique_lock<std::mutex> lock(mutex);
            cond.wait(lock, [this]() { return !currentTask.second || isRequestedTermination; });
        }

        //�X���b�h�ɒ��f��ʒm����
        void requestTermination() {
            std::unique_lock<std::mutex> lock(mutex);
            isRequestedTermination = true;
            cond.notify_all();
        }

        //���̃X���b�h��ID��Ԃ�
        std::thread::id getThreadId() const {
            return thread.get_id();
        }

        //���݂��̃N���X���S���X���b�h�̏������I���܂ŁA
        //���C���X���b�h��ҋ@������
        void stopMainThread()
        {
            while (currentTask.second)
            {
                //�^�X�N���I�������A�����o��
            }
        }
    };

    //��̃X���b�h�̂ݒS������
    class InnerWorker
    {
    private:

        ThreadPool* parent;
        int index{ -1 };
        std::pair<inputTypes, std::function<void(inputTypes&)>> currentTask;
        bool isRequestedTermination{ false };
        std::thread thread;
        std::deque<std::pair<inputTypes,std::function<void(inputTypes&)>>> localQueue;
        std::mutex mutex;
        std::condition_variable cond;

        //�X���b�h��ҋ@������
        void waitInitilize() {
            std::unique_lock<std::mutex> lock(mutex);
            cond.wait(lock, [this]() { return parent != nullptr && index >= 0; });
        }

        void procWorker() {
            waitInitilize();

            while (true) {
                bool is_assigned = false;
                {
                    // ���[�J���L���[��������̎��o����D�悷��
                    std::unique_lock<std::mutex> lock(mutex);
                    if (!localQueue.empty()) {
                        currentTask = localQueue.back();
                        localQueue.pop_back();
                        is_assigned = true;
                    }
                }
                if (!is_assigned) {
                    //���[�J���L���[����ɂȂ�����A���̃X���b�h���玝���Ă���

                    //���g�̃X���b�h���������X���b�h����^�X�N�������Ă���
                    auto task = parent->stealOrPull(index);
                    if (!task.second) {

                        std::unique_lock<std::mutex> lock(mutex);
                        currentTask.first = {};
                        currentTask.second = {};
                        cond.notify_all();  // �����������Ă��Ȃ����Ƃ�ʒm����
                        cond.wait(lock, [&]() {
                            // ���̏q����ł̓��b�N���擾���Ă���͂�
                            if (isRequestedTermination) {
                                return true;
                            }
                            // steal_or_pull�̎��Ƀ��b�N��ێ����Ă���ƃf�b�h���b�N���N����
                            lock.unlock();
                            auto task = parent->stealOrPull(index);
                            lock.lock();
                            // current_task_�̍X�V���ɍēx���b�N���擾����
                            currentTask = std::move(task);
                            return !!currentTask.second;
                            });
                        if (isRequestedTermination) {
                            break;
                        }
                    }
                    else {
                        std::unique_lock<std::mutex> lock(mutex);
                        currentTask = std::move(task);
                    }
                }
                currentTask.second(currentTask.first);
            }
        }

    public:
        //�C���X�^���X�쐬�i�K�ŃX���b�h���쐬���A�ҋ@������
        InnerWorker() : thread([this]() { procWorker(); }) {}
        ~InnerWorker() {
            waitUntilIdle();
            requestTermination();
            if (thread.joinable()) {
                thread.join();
            }
        }

        //�X���b�h�v�[�����쐬���ꂽ���_�ŁA�Ăяo����A
        //�X���b�h�v�[���ւ̎Q�ƂƃX���b�h�̔ԍ�������
        void initialize(ThreadPool* parent, int index) {
            std::unique_lock<std::mutex> lock(mutex);
            this->parent = parent;
            this->index = index;
            cond.notify_all();
        }

        void push(std::pair<inputTypes, std::function<void(inputTypes&)>> task) {
            std::unique_lock<std::mutex> lock(mutex);
            localQueue.emplace_back(task);
        }

        //�^�X�N�𑗂�
        std::pair<inputTypes,std::function<void(inputTypes&)>> steal() 
        {
            std::unique_lock<std::mutex> lock(mutex);
            if (localQueue.empty()) {
                std::pair<inputTypes, std::function<void(inputTypes&)>> t;
                t.first = {};
                t.second = {};
                return t;
            }

            auto task = localQueue.front();
            localQueue.pop_front();
            cond.notify_all();
            return std::move(task);
        }

        //���g�̃X���b�h�̒ʒm�𑗂�
        void wakeup() {
            std::unique_lock<std::mutex> lock(mutex);
            cond.notify_all();
        }

        //���[�J���̃L���[����ɂȂ邩�A�^�X�N�̒��f�ʒm�������܂őҋ@����
        void waitUntilIdle() {
            std::unique_lock<std::mutex> lock(mutex);
            cond.wait(lock, [this]() { return (localQueue.empty() && !currentTask.second) || isRequestedTermination; });
        }

        //�X���b�h�ɒ��f��ʒm����
        void requestTermination() {
            std::unique_lock<std::mutex> lock(mutex);
            isRequestedTermination = true;
            cond.notify_all();
        }

        std::thread::id getThreadId() const {
            return thread.get_id();
        }

        //�X���b�h���ҋ@��Ԃ����Ȃ�
        bool isIdle()
        {
            bool idle = localQueue.empty();
            idle = !currentTask.second;

            //�ҋ@��Ԃ̏ꍇtrue��Ԃ�
            return idle;
        }
    };

    //�X���b�h�v�[�����̃X���b�h�̐�(SingleWorker�͏���)
    int totalThreadSize;

    InnerWorker* innerWorkers;
    bool isRequestedTermination{ false };
    std::deque<std::pair<inputTypes, std::function<void(inputTypes&)>>> globalQueue;
    std::mutex mutex;
    std::condition_variable cond;

    //���[�h���ȂǗp�̕ʘg�̃X���b�h���
    SingleWorker singleWorker;

    //���[�J�[�̃X���b�h�ԍ���Ԃ�
    int getCurrentThreadIndex() {

        //���݂̃X���b�h�̎��ʔԍ���Ԃ�
        auto current_id = std::this_thread::get_id();
        int index = 0;
        for (int i = 0; i < totalThreadSize; i++) 
        {
            if (current_id == innerWorkers[i].getThreadId()) {
                return index;
            }
            ++index;
        }
        return -1;
    }

    //���ׂẴX���b�h�ɔԍ�����������
    ThreadPool()
    {
        //�X���b�h���́Acpu�̃X���b�h�̐�����2���炵���������쐬
        totalThreadSize = std::thread::hardware_concurrency() - 2;
        innerWorkers = new InnerWorker[totalThreadSize];

        int index = 0;
        for (int i = 0; i < totalThreadSize; i++) {
            innerWorkers[i].initialize(this, index++);
        }

        singleWorker.initialize();
    }
    ~ThreadPool() {
        waitUntilIdle();
        requestTermination();

        delete[] innerWorkers;
    };

public:

    static ThreadPool* GetInstance()
    {
        if (!instance)
        {
            instance = new ThreadPool();
        }

        return instance;
    }

    static void FinishThreadPool()
    {
        delete instance;
        instance = nullptr;
    }

    //�֐����Z�b�g���A���s������
    void run(std::pair<inputTypes, std::function<void(inputTypes&)>> task) {
        auto current_thread_index = getCurrentThreadIndex();
        {
            std::unique_lock<std::mutex> lock(mutex);
            // �C���f�b�N�X-1�̓��[�J�[�O�X���b�h���w��
            if (current_thread_index == -1) {
                //���[�J�[�O�̃X���b�h�̏ꍇ�A
                //���[�J�[�S�̂̃L���[�Ƀ^�X�N��ς�
                globalQueue.emplace_back(task);
            }
            else {
                innerWorkers[current_thread_index].push(task);
            }
        }
        wakeupAll(current_thread_index);
    }

    //���ׂẴX���b�h�̑ҋ@�������s��
    void waitUntilIdle() {
        {
            std::unique_lock<std::mutex> lock(mutex);
            cond.wait(lock, [this]() { return globalQueue.empty() || isRequestedTermination; });
        }
        for (int i = 0; i < totalThreadSize; i++) 
        {
            innerWorkers[i].waitUntilIdle();
        }
    }

    //���ׂẴX���b�h�̃^�X�N�𒆒f����
    void requestTermination() {
        {
            std::unique_lock<std::mutex> lock(mutex);
            isRequestedTermination = true;
            cond.notify_all();
        }
        for (int i = 0; i < totalThreadSize; i++) {
            innerWorkers[i].requestTermination();
        }
    }

    //�^�X�N�� ���g�̃X���b�h�ȊO�̃X���b�h�S�̂���Ƃ��Ă���
    std::pair<inputTypes, std::function<void(inputTypes&)>> stealOrPull(int index) 
    {
        for (int i = 0; i < totalThreadSize; ++i) {
            if (i == index) {
                //���g�̃X���b�h�͏���
                continue;
            }

            auto task = innerWorkers[i].steal();
            if (!!task.second) {
                //�^�X�N�����݂���΁A�����Ԃ�
                return task;
            }
        }

        //�X���b�h���̃��[�J���L���[�Ƀ^�X�N���������
        //�O���[�o���L���[����^�X�N���Ƃ��Ă���
        std::unique_lock<std::mutex> lock(mutex);
        if (globalQueue.empty()) 
        {
            std::pair<inputTypes, std::function<void(inputTypes)>> t;
            t.first = {};
            t.second = {};
            return t;
        }

        auto task = globalQueue.front();
        globalQueue.pop_front();
        cond.notify_all();
        return std::move(task);
    }

    //�����Ŏw�肳�ꂽ�X���b�h�ȊO�́A
    //�X���b�h�̍X�V�������s��
    void wakeupAll(int index) {
        int i = 0;
        for (int i = 0; i < totalThreadSize; i++) 
        {
            if (i != index) {
                innerWorkers[i].wakeup();
            }
            ++i;
        }
    }

    //�ʘg�̈�̃X���b�h�ɏ�����C����
    void runSingle(std::pair<inputTypes, std::function<void(inputTypes&)>> function)
    {
        singleWorker.push(function);
    }

    //SingleWoker�̎��s�𒆒f
    void requestTerminationSingle()
    {
        singleWorker.requestTermination();
    }

    //���݂��̃N���X���S���X���b�h�̏������I���܂ŁA
    //���C���X���b�h��ҋ@������
    void stopMainThreadSingle()
    {
        singleWorker.stopMainThread();
    }

    template <typename T>
    void safeAnyCast(T& value,int index,std::vector<std::any>& params)
    {
        try
        {
            value = std::any_cast<T>(params[index]);
        }
        catch (const std::bad_any_cast& e)
        {
            std::cerr << "failed cast" << e.what() << std::endl;
        }
    }
};