#include"GameManager.h"

GameManager* GameManager::gameManager = nullptr;

void GameManager::initGame()//初期化設定
{
    VulkanBase* vulkan = VulkanBase::GetInstance();

    frameDuration = (1.0f / (fps + 1)) * 1000.0f;//設定されたフレームレートから、1フレームでの最低の処理時間を設定

    vulkan->initVulkan();//Vulkanのデータの一部をあらかじめ用意しておく

    setLoadUI();//ロードUIの設定

    bool load = false;
    std::thread loadThread(&GameManager::drawLoading, this,std::ref(load));

    load = createScene();//ステージの読み込み

    if (loadThread.joinable())
    {
        loadThread.join();
    }

    mainGameLoop();//ゲームループ
}

//ロードUIの設定
void GameManager::setLoadUI()
{
    VulkanBase* vulkan = VulkanBase::GetInstance();

    std::shared_ptr<UI> ui = std::shared_ptr<UI>(new UI(FileManager::GetInstance()->loadImage("textures/loadUI.png")));

    ui->setScale(50.0f);
    ui->setPosition(glm::vec2(window_width - ui->getTexWidth(), window_height - ui->getTexHeight()));

    ui->updateTransformMatrix();
    Storage::GetInstance()->setLoadUI(ui);

    //gpu上に頂点バッファなどを作成
    vulkan->setUI(ui);
}

//ロードUIの表示
void GameManager::drawLoading(bool& loadFinish)
{
    VulkanBase* vulkan = VulkanBase::GetInstance();

    start = std::chrono::system_clock::now();//フレーム開始時間
    while (!loadFinish)
    {
        //vulkan->drawLoading();

        end = std::chrono::system_clock::now();//フレーム終了時間
        elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();//フレーム間時間計測
        if (elapsed < frameDuration)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(frameDuration - elapsed)));//処理を停止する
        }
        start = std::chrono::system_clock::now();
    }

    //ロード画面レンダリングの後始末をする
    vulkan->stopLoading();
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

        VulkanBase::GetInstance()->render();//オブジェクトのレンダリング

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