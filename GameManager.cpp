#include"GameManager.h"

GameManager* GameManager::gameManager = nullptr;

void GameManager::initGame()//初期化設定
{
    frameDuration = (1.0f / fps) * 1000.0f;//設定されたフレームレートから、1フレームでの最低の処理時間を設定

    VulkanBase::GetInstance()->initVulkan(limitBoneDataSize);//Vulkanのデータの一部をあらかじめ用意しておく

    createScene();//ステージの読み込み
}

void GameManager::createScene()//luaからステージの様子を読み込む
{
    scene = Scene::GetInstance();//luaから読み取ったデータからステージの様子を持つクラス
    scene->init("LuaScripts/test.lua");//ステージの読み込み

    mainGameLoop();//ゲームループ
}

void GameManager::mainGameLoop()//メインゲームループ
{

    start = std::chrono::system_clock::now();//フレーム開始時間

    while (true)
    {

        exit = scene->UpdateScene();//ゲームを終了する場合、trueが返る

        VulkanBase::GetInstance()->render();//オブジェクトのレンダリング

        if (exit || glfwWindowShouldClose(window))//メインゲームループの終了処理
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

void GameManager::exitScene()
{
    scene->Destroy();

    FinishGame();
}

void GameManager::FinishGame()
{
    FinishInstance();
}