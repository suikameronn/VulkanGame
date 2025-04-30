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

    //ロード時などの別枠で一つスレッドを持っておく
    class SingleWorker
    {
    private:

        std::pair<inputTypes,std::function<void(inputTypes&)>> currentTask;
        bool isRequestedTermination{ false };
        bool isInitilized{ false };
        std::thread thread;
        std::mutex mutex;
        std::condition_variable cond;

        //スレッドを待機させる
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
                cond.notify_all();  // 何も処理していないことを通知する
                cond.wait(lock, [&]()
                    {
                        isInitilized = true;

                        // この述語内ではロックを取得しているはず
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
        //インスタンス作成段階でスレッドを作成し、待機させる
        SingleWorker() : thread([this]() { procWorker(); }) {}
        ~SingleWorker() {
            waitUntilIdle();
            requestTermination();
            if (thread.joinable()) {
                thread.join();
            }
        }

        //スレッドプールが作成された時点で、呼び出され、
        //スレッドプールへの参照とスレッドの番号を持つ
        void initialize()
        {
            std::unique_lock<std::mutex> lock(mutex);
            cond.notify_all();
        }

        void push(std::pair<inputTypes, std::function<void(inputTypes&)>>& function)
        {
            //初期化が別スレッドで終わるまで、メインスレッドを待機させる
            while (!isInitilized)
            {

            }

            currentTask = function;

            cond.notify_all();
        }

        //自身のスレッドの通知を送る
        void wakeup() {
            std::unique_lock<std::mutex> lock(mutex);
            cond.notify_all();
        }

        //通知が来るか、ローカルのキューが空になるまで、タスクの中断通知がされるまで待機する
        void waitUntilIdle() {
            std::unique_lock<std::mutex> lock(mutex);
            cond.wait(lock, [this]() { return !currentTask.second || isRequestedTermination; });
        }

        //スレッドに中断を通知する
        void requestTermination() {
            std::unique_lock<std::mutex> lock(mutex);
            isRequestedTermination = true;
            cond.notify_all();
        }

        //このスレッドのIDを返す
        std::thread::id getThreadId() const {
            return thread.get_id();
        }

        //現在このクラスが担うスレッドの処理が終わるまで、
        //メインスレッドを待機させる
        void stopMainThread()
        {
            while (currentTask.second)
            {
                //タスクが終わったら、抜け出す
            }
        }
    };

    //一つのスレッドのみ担当する
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

        //スレッドを待機させる
        void waitInitilize() {
            std::unique_lock<std::mutex> lock(mutex);
            cond.wait(lock, [this]() { return parent != nullptr && index >= 0; });
        }

        void procWorker() {
            waitInitilize();

            while (true) {
                bool is_assigned = false;
                {
                    // ローカルキュー末尾からの取り出しを優先する
                    std::unique_lock<std::mutex> lock(mutex);
                    if (!localQueue.empty()) {
                        currentTask = localQueue.back();
                        localQueue.pop_back();
                        is_assigned = true;
                    }
                }
                if (!is_assigned) {
                    //ローカルキューが空になったら、他のスレッドから持ってくる

                    //自身のスレッドを除いたスレッドからタスクを持ってくる
                    auto task = parent->stealOrPull(index);
                    if (!task.second) {

                        std::unique_lock<std::mutex> lock(mutex);
                        currentTask.first = {};
                        currentTask.second = {};
                        cond.notify_all();  // 何も処理していないことを通知する
                        cond.wait(lock, [&]() {
                            // この述語内ではロックを取得しているはず
                            if (isRequestedTermination) {
                                return true;
                            }
                            // steal_or_pullの時にロックを保持しているとデッドロックを起こす
                            lock.unlock();
                            auto task = parent->stealOrPull(index);
                            lock.lock();
                            // current_task_の更新時に再度ロックを取得する
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
        //インスタンス作成段階でスレッドを作成し、待機させる
        InnerWorker() : thread([this]() { procWorker(); }) {}
        ~InnerWorker() {
            waitUntilIdle();
            requestTermination();
            if (thread.joinable()) {
                thread.join();
            }
        }

        //スレッドプールが作成された時点で、呼び出され、
        //スレッドプールへの参照とスレッドの番号を持つ
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

        //タスクを送る
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

        //自身のスレッドの通知を送る
        void wakeup() {
            std::unique_lock<std::mutex> lock(mutex);
            cond.notify_all();
        }

        //ローカルのキューが空になるか、タスクの中断通知がされるまで待機する
        void waitUntilIdle() {
            std::unique_lock<std::mutex> lock(mutex);
            cond.wait(lock, [this]() { return (localQueue.empty() && !currentTask.second) || isRequestedTermination; });
        }

        //スレッドに中断を通知する
        void requestTermination() {
            std::unique_lock<std::mutex> lock(mutex);
            isRequestedTermination = true;
            cond.notify_all();
        }

        std::thread::id getThreadId() const {
            return thread.get_id();
        }

        //スレッドが待機状態がいなか
        bool isIdle()
        {
            bool idle = localQueue.empty();
            idle = !currentTask.second;

            //待機状態の場合trueを返す
            return idle;
        }
    };

    //スレッドプール内のスレッドの数(SingleWorkerは除く)
    int totalThreadSize;

    InnerWorker* innerWorkers;
    bool isRequestedTermination{ false };
    std::deque<std::pair<inputTypes, std::function<void(inputTypes&)>>> globalQueue;
    std::mutex mutex;
    std::condition_variable cond;

    //ロード時など用の別枠のスレッド一つ
    SingleWorker singleWorker;

    //ワーカーのスレッド番号を返す
    int getCurrentThreadIndex() {

        //現在のスレッドの識別番号を返す
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

    //すべてのスレッドに番号を持たせる
    ThreadPool()
    {
        //スレッド数は、cpuのスレッドの数から2減らした数だけ作成
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

    //関数をセットし、実行させる
    void run(std::pair<inputTypes, std::function<void(inputTypes&)>> task) {
        auto current_thread_index = getCurrentThreadIndex();
        {
            std::unique_lock<std::mutex> lock(mutex);
            // インデックス-1はワーカー外スレッドを指す
            if (current_thread_index == -1) {
                //ワーカー外のスレッドの場合、
                //ワーカー全体のキューにタスクを積む
                globalQueue.emplace_back(task);
            }
            else {
                innerWorkers[current_thread_index].push(task);
            }
        }
        wakeupAll(current_thread_index);
    }

    //すべてのスレッドの待機処理を行う
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

    //すべてのスレッドのタスクを中断する
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

    //タスクを 自身のスレッド以外のスレッド全体からとってくる
    std::pair<inputTypes, std::function<void(inputTypes&)>> stealOrPull(int index) 
    {
        for (int i = 0; i < totalThreadSize; ++i) {
            if (i == index) {
                //自身のスレッドは除く
                continue;
            }

            auto task = innerWorkers[i].steal();
            if (!!task.second) {
                //タスクが存在すれば、それを返す
                return task;
            }
        }

        //スレッド内のローカルキューにタスクが無ければ
        //グローバルキューからタスクをとってくる
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

    //引数で指定されたスレッド以外の、
    //スレッドの更新処理を行う
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

    //別枠の一つのスレッドに処理を任せる
    void runSingle(std::pair<inputTypes, std::function<void(inputTypes&)>> function)
    {
        singleWorker.push(function);
    }

    //SingleWokerの実行を中断
    void requestTerminationSingle()
    {
        singleWorker.requestTermination();
    }

    //現在このクラスが担うスレッドの処理が終わるまで、
    //メインスレッドを待機させる
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