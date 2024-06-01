#include"FileManager.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include<tiny_obj_loader.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

FileManager* FileManager::fileManager = nullptr;

FileManager::FileManager()
{
    manager = FbxManager::Create();
    ios = FbxIOSettings::Create(manager, IOSROOT);
    manager->SetIOSettings(ios);
    importer = FbxImporter::Create(manager, "");
    scene = FbxScene::Create(manager, "");
    converter = new FbxGeometryConverter(manager);

    indexSize = 0;
}

std::string FileManager::getModelPath(OBJECT obj)
{
    switch (obj)
    {
    case OBJECT::MODELTEST:
        modelPath = "models/viking_room.obj";
        return modelPath;
    case OBJECT::FBXTEST:
        modelPath = "models/Buche_de_Noel_Cut.fbx";
        return modelPath;
    }
}

Meshes* FileManager::loadModelPoints(OBJECT obj)
{
    if (obj >= 1000)
    {
        return loadPointsFbx(obj);
    }
    else
    {
        return loadObj(obj);
    }
}

Meshes* FileManager::loadObj(OBJECT obj)
{
    if (Storage::GetInstance()->containMeshes(obj))
    {
        return Storage::GetInstance()->accessObj(obj);
    }

    Meshes* meshes = new Meshes();

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, getModelPath(obj).c_str()))
    {
        throw std::runtime_error(warn + err);
    }

    std::unordered_map<Vertex, uint32_t> uniqueVertices{};

    for (const auto& shape : shapes)//すべてのシェイプの数だけ繰り返す
    {
        for (const auto& index : shape.mesh.indices)//指定のシェイプを構成するメッシュの数だけ繰り返す
        {
            Vertex vertex{};

            vertex.pos =
            {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };

            vertex.texCoord =
            {
                attrib.texcoords[2 * index.texcoord_index + 0],
                1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
            };

            vertex.color = { 0.0f,0.0f,0.0f };

            vertex.normal =
            {
                attrib.normals[3 * index.normal_index + 0],
                attrib.normals[3 * index.normal_index + 1],
                attrib.normals[3 * index.normal_index + 2]
            };

            if (uniqueVertices.count(vertex) == 0)
            {
                uniqueVertices[vertex] = meshes->getVerticesSize();
                meshes->pushBackVertex(&vertex);
            }

            meshes->pushBackIndex(uniqueVertices[vertex]);
        }
    }

    Storage::GetInstance()->addObj(obj, meshes);

    return meshes;
}

void FileManager::loadPointsFbx(Meshes* meshes, FbxNode* node)
{
    if (node->GetNodeAttribute()->GetAttributeType() != FbxNodeAttribute::eMesh || node->GetNodeAttribute() == NULL)
    {
        return;
    }

    int i, j;

    FbxMesh* mesh = (FbxMesh*)node->GetNodeAttribute();

    for (i = 0; i < mesh->GetPolygonCount(); i++)
    {
        meshes->pushBackIndex(i * 3 + indexSize);
        meshes->pushBackIndex(i * 3 + 1 + indexSize);
        meshes->pushBackIndex(i * 3 + 2 + indexSize);
    }

    //頂点バッファの取得
    FbxVector4* verticesFbx = mesh->GetControlPoints();
    //インデックスバッファの取得
    int* indicesFbx = mesh->GetPolygonVertices();
    //頂点座標の数の取得
    int polygonVertexCount = mesh->GetPolygonVertexCount();
    
    //法線リストの取得
    FbxArray<FbxVector4> normals;
    //法線リスト取得
    mesh->GetPolygonVertexNormals(normals);

    //uvセットの名前保存
    FbxStringList uvSetName;
    //uvセットの名前リスト取得
    mesh->GetUVSetNames(uvSetName);
    FbxArray<FbxVector2> uvBuffer;
    //UVSetの名前からUVSetを取得する
    mesh->GetPolygonVertexUVs(uvSetName.GetStringAt(0), uvBuffer);

    //GetPolygonVertexCount:頂点数
    for (i = 0; i < polygonVertexCount; i++)
    {
        Vertex vertex{};
        //インデックスバッファから頂点番号を取得
        int index = indicesFbx[i];

        //頂点座標リストから座標を取得する
        if (indexSize != 0)
        {
            vertex.pos = glm::vec3(verticesFbx[index][0] + 0.1f, verticesFbx[index][1], verticesFbx[index][2]);
        }
        else
        {
            vertex.pos = glm::vec3(verticesFbx[index][0] + 0.0f, verticesFbx[index][1], verticesFbx[index][2]);
        }
        vertex.normal = glm::vec3(normals[index][0], normals[index][1], normals[index][2]);
        vertex.texCoord = glm::vec2(uvBuffer[index][0], uvBuffer[index][1]);

        meshes->pushBackVertex(&vertex);
    }

    indexSize += mesh->GetPolygonCount() * 3;
}

Meshes* FileManager::loadPointsFbx(OBJECT obj)
{
    if (Storage::GetInstance()->containMeshes(obj))
    {
        return Storage::GetInstance()->accessObj(obj);
    }

    Meshes* meshes = new Meshes();

    /*fbxファイルの読み込み*/
    importer->Initialize(getModelPath(obj).c_str(), -1, manager->GetIOSettings());
    importer->Import(scene);

    //三角ポリゴン化
    converter->Triangulate(scene, true);

    FbxNode* node = scene->GetRootNode();

    if (node)
    {
        for (int i = 0; i < node->GetChildCount(); i++)
        {
            loadPointsFbx(meshes,node->GetChild(i));
        }
    }

    return meshes;
}

std::string FileManager::getImagePath(IMAGE image)
{
    switch (image)
    {
    case IMAGE::IMAGETEST:
        imagePath = "textures\\viking_room.png";
        return imagePath;
        break;
    }
}

ImageData* FileManager::loadModelImage(IMAGE image)
{
    if (Storage::GetInstance()->containImageData(image))
    {
        return Storage::GetInstance()->accessImage(image);
    }

    int width, height, texChannels;
    std::vector<unsigned char> pixels;

    unsigned char* picture = stbi_load(getImagePath(image).c_str(), &width, &height, &texChannels, STBI_rgb_alpha);
    if (!picture)
    {
        throw std::runtime_error("faile image load");
    }

    int imageSize = width * height * 4;
    imageData = new ImageData(width,height,texChannels,picture);

    stbi_image_free(picture);

    Storage::GetInstance()->addImage(image, imageData);

    return Storage::GetInstance()->accessImage(image);
}