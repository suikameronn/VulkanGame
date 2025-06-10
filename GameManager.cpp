#include"GameManager.h"

GameManager* GameManager::gameManager = nullptr;

void GameManager::initGame()//ゲーム全体の初期化処理
{
    VulkanBase* vulkan = VulkanBase::GetInstance();

    frameDuration = (1.0f / (fps + 1)) * 1000.0f;//指定したfpsから、一フレームにかけられる時間を計算する

    glfwGetWindowSize(window, &window_width, &window_height);

    uiProjection = glm::ortho(0.0f, static_cast<float>(window_width), static_cast<float>(window_height), 0.0f, -1.0f, 1.0f);

    vulkan->initVulkan();//Vulkanの初期化

    FontManager* fontmanager = FontManager::GetInstance();

    //setLoadUI();//ロードUIを設定

    bool load = false;
    //drawLoading(load);

    load = createScene();//luaスクリプトを読み取り、シーンの作成

    //シーンの作成が終わるまで、待機する
    //ThreadPool::GetInstance()->stopMainThreadSingle();

    mainGameLoop();//ゲームのメインループを開始
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

    //gpu上にロードUIのテクスチャなどを展開する
    vulkan->setUI(ui);
}

//ロードUIを描画する
void GameManager::drawLoading(bool& loadFinish)
{
    //非同期で描画させる
    std::function<void()> function = [this,&loadFinish]()
        {
            ThreadPool* pool = ThreadPool::GetInstance();

            VulkanBase* vulkan = VulkanBase::GetInstance();
            Storage* storage = Storage::GetInstance();

            auto start = std::chrono::system_clock::now();//フレームの開始時間を計測

            Rotate2D rot2D;
            rot2D.z = 0.0f;

            std::shared_ptr<UI> loadUI = storage->getLoadUI();

            while (!loadFinish)
            {
                //UIを回転させる
                loadUI->setRotate(rot2D);
                loadUI->updateTransformMatrix();

                //UIを描画する
                //vulkan->drawLoading();

                //UIの角度を調整する
                rot2D.z++;

                loadUI->frameEnd();

                //fps制限を掛ける
                auto end = std::chrono::system_clock::now();//フレーム時間の終了時間を計測
                float elapsed = static_cast<float>(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());//フレーム間の時間を計算
                if (elapsed < this->frameDuration)
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(this->frameDuration - elapsed)));//fpsを制限する
                }
                start = std::chrono::system_clock::now();
            }

            //ロード画面の描画をやめる
            vulkan->stopLoading();
        };

    //ロードUIの開始
    ThreadPool::GetInstance()->runSingle(function);
}

bool GameManager::createScene()//シーンを作成する
{
    scene = Scene::GetInstance();
    scene->init("LuaScripts/scene.lua");//luaスクリプトを読み取りシーンを作成する

    return true;
}

void GameManager::mainGameLoop()//メインゲームループ
{
    start = std::chrono::system_clock::now();//フレーム時間を計測

    while (true)
    {

        exit = scene->UpdateScene();//シーン全体を更新する

        if (exit == GAME_FINISH || exit == GAME_RESTART || glfwWindowShouldClose(window))//シーンを終了するかどうか
        {
            break;
        }

        end = std::chrono::system_clock::now();//フレーム終了時間を計測
        elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();//フレーム間時間を計算する
        if (elapsed < frameDuration)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(frameDuration - elapsed)));//fpsを制限する
        }
        else
        {
            std::cout << "fps down" << std::endl;
        }
        start = std::chrono::system_clock::now();

        Controller::GetInstance()->initInput();//キー入力を初期化
        glfwPollEvents();//キー入力などを受け取る
    }

    exitScene();
}

//シーンを狩猟させる処理
void GameManager::exitScene()
{
    //シーンを破棄する
    if (scene)
    {
        scene->Destroy();
    }

    //遅延させて破棄する予定のバッファをすべて破棄する
    VulkanBase::GetInstance()->allCleanupDefferedBuffer();

    if (exit == GAME_FINISH || glfwWindowShouldClose(window))
    {
        //ゲームを終了させる
        FinishGame();
    }
}

//ゲーム全体の終了処理
void GameManager::FinishGame()
{
    FinishInstance();
}