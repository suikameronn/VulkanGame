#include"GameManager.h"

GameManager* GameManager::gameManager = nullptr;

void GameManager::initGame()//初期化設定
{
    VulkanBase* vulkan = VulkanBase::GetInstance();

    frameDuration = (1.0f / (fps + 1)) * 1000.0f;//設定されたフレームレートから、1フレームでの最低の処理時間を設定

    glfwGetWindowSize(window, &window_width, &window_height);

    uiProjection = glm::ortho(0.0f, static_cast<float>(window_width), static_cast<float>(window_height), 0.0f, -1.0f, 1.0f);

    vulkan->initVulkan();//Vulkanのデータの一部をあらかじめ用意しておく

    setLoadUI();//ロードUIの設定

    bool load = false;
    drawLoading(load);

    load = createScene();//ステージの読み込み

    //ロード画面描画の終了を待つ
    ThreadPool::GetInstance()->stopMainThreadSingle();

    mainGameLoop();//ゲームループ
}

//ロードUIの設定
void GameManager::setLoadUI()
{
    VulkanBase* vulkan = VulkanBase::GetInstance();

    std::shared_ptr<UI> ui = std::shared_ptr<UI>(new UI(FileManager::GetInstance()->loadImage("textures/loadUI.png")));

    ui->setScale(100.0f);
    ui->setPosition(glm::vec2(window_width - ui->getTexWidth(), window_height - ui->getTexHeight()));

    ui->updateTransformMatrix();
    Storage::GetInstance()->setLoadUI(ui);

    //gpu上に頂点バッファなどを作成
    vulkan->setUI(ui);
}

//ロードUIの表示
void GameManager::drawLoading(bool& loadFinish)
{
    //別スレッドに渡す、引数用の変数
    inputTypes settings(2);
    settings[0] = &loadFinish;
    settings[1] = frameDuration;

    //ロード画面の描画(別スレッドで実行)
    std::function<void(inputTypes&)> function = [](std::vector<std::any>& loadSettings)
        {
            //配列から設定を取得する
            bool* loadFinish = nullptr;
            float frameDuration = 0.0f;
            ThreadPool* pool = ThreadPool::GetInstance();
            pool->safeAnyCast(loadFinish, 0, loadSettings);
            pool->safeAnyCast(frameDuration, 1, loadSettings);

            VulkanBase* vulkan = VulkanBase::GetInstance();
            Storage* storage = Storage::GetInstance();

            auto start = std::chrono::system_clock::now();//フレーム開始時間

            Rotate2D rot2D;
            rot2D.z = 0.0f;

            std::shared_ptr<UI> loadUI = storage->getLoadUI();

            while (!*loadFinish)
            {
                //UIの回転行列を計算
                loadUI->setRotate(rot2D);
                loadUI->updateTransformMatrix();

                //UIの描画
                //vulkan->drawLoading();

                //UIの回転
                rot2D.z++;

                loadUI->frameEnd();

                //fps調整
                auto end = std::chrono::system_clock::now();//フレーム終了時間
                float elapsed = static_cast<float>(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());//フレーム間時間計測
                if (elapsed < frameDuration)
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(frameDuration - elapsed)));//処理を停止する
                }
                start = std::chrono::system_clock::now();
            }

            //ロード画面レンダリングの後始末をする
            vulkan->stopLoading();
        };

    //別スレッドで実行開始
    std::pair<inputTypes, std::function<void(inputTypes&)>> pair = { settings,function };
    ThreadPool::GetInstance()->runSingle(pair);
}

bool GameManager::createScene()//luaからステージの様子を読み込む
{
    scene = Scene::GetInstance();//luaから読み取ったデータからステージの様子を持つクラス
    scene->init("LuaScripts/scene.lua");//ステージの読み込み

    return true;
}

void GameManager::mainGameLoop()//メインゲームループ
{
    start = std::chrono::system_clock::now();//フレーム開始時間

    while (true)
    {

        exit = scene->UpdateScene();//ゲームを終了する場合、trueが返る

        if (exit == GAME_FINISH || exit == GAME_RESTART || glfwWindowShouldClose(window))//メインゲームループの終了処理
        {
            break;
        }

        end = std::chrono::system_clock::now();//フレーム終了時間
        elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();//フレーム間時間計測
        if (elapsed < frameDuration)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(frameDuration - elapsed)));//処理を停止する
        }
        else
        {
            std::cout << "fps down" << std::endl;
        }
        start = std::chrono::system_clock::now();

        Controller::GetInstance()->initInput();//キーボードから入力を初期化
        glfwPollEvents();//キーボードから入力を取得
    }

    exitScene();
}

//ステージを出る
void GameManager::exitScene()
{
    //ステージを破棄
    if (scene)
    {
        scene->Destroy();
    }

    if (exit == GAME_FINISH || glfwWindowShouldClose(window))
    {
        //ゲーム全体を終了
        FinishGame();
    }
    else if(exit == GAME_RESTART)
    {
        RestartGame();
    }
}

//ゲーム全体の終了処理
void GameManager::FinishGame()
{
    FinishInstance();
}

//ゲームのリスタート処理 読み取ったリソースは解放しない
void GameManager::RestartGame()
{
    VulkanBase::GetInstance()->FinishVulkanBase();

    initGame();
}