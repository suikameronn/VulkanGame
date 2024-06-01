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

void FileManager::loadMesh(Meshes* meshes,FbxMesh* mesh)
{
    int i, j;
    for (i = 0; i < mesh->GetPolygonCount(); i++)
    {
        meshes->pushBackIndex(i * 3 + indexSize);
        meshes->pushBackIndex(i * 3 + 1 + indexSize);
        meshes->pushBackIndex(i * 3 + 2 + indexSize);
    }

    //���_�o�b�t�@�̎擾
    FbxVector4* verticesFbx = mesh->GetControlPoints();
    //�C���f�b�N�X�o�b�t�@�̎擾
    int* indicesFbx = mesh->GetPolygonVertices();
    //���_���W�̐��̎擾
    int polygonVertexCount = mesh->GetPolygonVertexCount();

    //�@�����X�g�̎擾
    FbxArray<FbxVector4> normals;
    //�@�����X�g�擾
    mesh->GetPolygonVertexNormals(normals);

    //uv�Z�b�g�̖��O�ۑ�
    FbxStringList uvSetName;
    //uv�Z�b�g�̖��O���X�g�擾
    mesh->GetUVSetNames(uvSetName);
    FbxArray<FbxVector2> uvBuffer;
    //UVSet�̖��O����UVSet���擾����
    mesh->GetPolygonVertexUVs(uvSetName.GetStringAt(0), uvBuffer);

    //GetPolygonVertexCount:���_��
    for (i = 0; i < polygonVertexCount; i++)
    {
        Vertex vertex{};
        //�C���f�b�N�X�o�b�t�@���璸�_�ԍ����擾
        int index = indicesFbx[i];

        //���_���W���X�g������W���擾����
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

FbxModel* FileManager::loadModel(OBJECT obj)
{
    if (Storage::GetInstance()->containFbxModel(obj))
    {
        return Storage::GetInstance()->accessFbxModel(obj);
    }

    FbxModel* model = new FbxModel();

    indexSize = 0;

    converter = new FbxGeometryConverter(manager);
    converter->Triangulate(scene,true);

    if (importer->Initialize(getModelPath(obj).c_str()))
    {
        throw std::runtime_error("load model error");
    }

    if (importer->Import(scene))
    {
        throw std::runtime_error("load scene error");
    }

    FbxNode* rootNode = scene->GetRootNode();
    if (rootNode == nullptr)
    {
        throw std::runtime_error("rootNode error");
    }

    for (int i = 0; i < rootNode->GetChildCount(); i++)
    {
        FbxNode* child = rootNode->GetChild(i);
        FbxNodeAttribute::EType type = child->GetNodeAttribute()->GetAttributeType();

        switch (type)
        {
        case FbxNodeAttribute::EType::eMesh:
            FbxMesh* mesh = (FbxMesh*)child->GetNodeAttribute();
            loadMesh(model->getMeshes(),mesh);
            break;
        }
    }
}

/*
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

    for (const auto& shape : shapes)//���ׂẴV�F�C�v�̐������J��Ԃ�
    {
        for (const auto& index : shape.mesh.indices)//�w��̃V�F�C�v���\�����郁�b�V���̐������J��Ԃ�
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
*/

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

/*
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
*/