#pragma once
#include<iostream>
#include<algorithm>

#include"Model.h"

#define MAX_TEXTURE_COUNT 5//gltf���f�������ő�̃e�N�X�`����
#define CUBEMAP_FACE_COUNT 6//�L���[�u�}�b�s���O�p�̖ʂ̐�
#define CUBEMAP_FACE_RIGHT 0
#define CUBEMAP_FACE_LEFT 1
#define CUBEMAP_FACE_TOP 2
#define CUBEMAP_FACE_BOTTOM 3
#define CUBEMAP_FACE_FRONT 4
#define CUBEMAP_FACE_BACK 5

//IBL�p�̃e�N�X�`���̃T�C�Y
//�T�C�Y��512�ȏ�ɂ���ƃN���b�V������
#define IBL_MAP_SIZE 256

//IBL��specular��BRDF�ɂ��Ă̍\����
struct IBLSpecularBRDF
{
    //�摜�T�C�Y
    uint32_t size;

    //�I�t�X�N���[�������_�����O�p�̃f�[�^
    OffScreenPass passData;

    //specular��specular�p��descriptorSet
    VkDescriptorSet descriptorSet;

    //specular��specular�p�̃R���s���[�g�V�F�[�_�ւ̃p�X
    std::string vertShaderPath, fragShaderPath;

    //�o�b�t�@�̔z��
    std::vector<MappedBuffer> mappedBuffers;

    //�I�t�X�N���[�������_�����O�̃t���[������ݒ肷��
    void setFrameCount(int frameCount)
    {
        mappedBuffers.resize(frameCount);

        passData.setFrameCount(frameCount);
    }

    //�����_�����O�̏o�͐�̃T�C�Y��ݒ�
    void setRenderSize(uint32_t s)
    {
        size = s;

        //specular�e�N�X�`���̃T�C�Y
        passData.width = s;
        passData.height = s;
    }

    void destroy(VkDevice& device)
    {
        passData.destroy(device);

        for (auto mappedBuffer : mappedBuffers)
        {
            mappedBuffer.destroy(device);
        }
    }
};

//IBL��specular�̋��ʔ��˕����p�̍\����
struct IBLSpecularReflection
{
    //�摜�t�H�[�}�b�g
    VkFormat format;

    //�摜�̃T�C�Y
    uint32_t size;

    //�V�F�[�_�ւ̃p�X
    std::string vertShaderPath, fragShaderPath;

    //specular�p��6�̃��C���[�̃e�N�X�`���[�f�[�^
    TextureData* multiLayerTexture;

    //specular�p�̃~�b�v�}�b�v���x���̔z��
    uint32_t mipmapLevel;
    std::vector<uint32_t> mipmapLevelSize;

    //IBL��specular�p�̃����_�[�p�X
    std::vector<VkRenderPass> renderPass;

    std::vector<VkFramebuffer> frameBuffer;//�����_�����O�̏o�͐�̃o�b�t�@�[
    std::vector<FrameBufferAttachment> imageAttachment;//�����_�����O�̏o�͐����������

    VkSampler sampler;//�����_�����O���ʂւ̃T���v���[

    VkDescriptorSetLayout prePassLayout;//�e�N�X�`������Specular�p�̃}�b�v���쐬���邽�߂̃��C�A�E�g
    VkPipelineLayout pipelineLayout;
    std::vector<VkPipeline> pipeline;
    std::vector<VkDescriptorSet> descriptorSets;

    VkDescriptorSet descriptorSet;

    //�o�b�t�@�̔z��
    std::vector<MappedBuffer> mappedBuffers;

    IBLSpecularReflection()
    {
        multiLayerTexture = new TextureData();
    }

    //�I�t�X�N���[�������_�����O�̃t���[������ݒ肷��
    void setFrameCount(int frameCount)
    {
        mappedBuffers.resize(frameCount);
    }

    void setRenderSize(uint32_t size)
    {
        this->size = size;

        //specular�̃~�b�v�}�b�v���x����
        mipmapLevel = static_cast<uint32_t>(std::floor(std::log2(std::max(size, size))) + 1);
        multiLayerTexture->mipLevel = mipmapLevel;

        //�e�~�b�v�}�b�v���x���̉摜�̃T�C�Y���v�Z����
        mipmapLevelSize.resize(mipmapLevel);
        mipmapLevelSize[0] = size;
        for (int i = 1; i < mipmapLevelSize.size(); i++)
        {
            mipmapLevelSize[i] = static_cast<uint32_t>(std::max(1.0, std::floor(size / std::pow(2.0, i))));
        }

        frameBuffer.resize(mipmapLevel * CUBEMAP_FACE_COUNT);
        imageAttachment.resize(mipmapLevel * CUBEMAP_FACE_COUNT);

        renderPass.resize(mipmapLevel);
        pipeline.resize(mipmapLevel);

        descriptorSets.resize(CUBEMAP_FACE_COUNT);
    }

    void destroy(VkDevice& device)
    {
        multiLayerTexture->destroy(device);

        for (auto& p : pipeline)
        {
            vkDestroyPipeline(device, p, nullptr);
        }

        vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
        vkDestroyDescriptorSetLayout(device, prePassLayout, nullptr);

        for (auto& framebuffer : frameBuffer)
        {
            vkDestroyFramebuffer(device, framebuffer, nullptr);
        }

        for (auto& attachment : imageAttachment)
        {
            attachment.destory(device);
        }

        vkDestroySampler(device, sampler, nullptr);

        for (auto& pass : renderPass)
        {
            vkDestroyRenderPass(device, pass, nullptr);
        }


        for (auto mappedBuffer : mappedBuffers)
        {
            mappedBuffer.destroy(device);
        }
    }
};

//�C���[�W�x�[�X�h���C�e�B���O�p�\����
struct IBLDiffuse
{
    //�摜�t�H�[�}�b�g
    VkFormat format;

    //�摜�T�C�Y
    uint32_t size;

    //�~�b�v�}�b�v���x��
    uint32_t mipmapLevel;

    //�I�t�X�N���[�������_�����O�p�̃f�[�^
    OffScreenPass passData;

    //diffuse�ɂ��Ẵe�N�X�`��
    TextureData* multiLayerTexture;

    //diffuse��specular�p��descriptorSet
    VkDescriptorSet descriptorSet;

    //diffuse��specular�p�̃R���s���[�g�V�F�[�_�ւ̃p�X
    std::string vertShaderPath, fragShaderPath;

    //�o�b�t�@�̔z��
    std::vector<MappedBuffer> mappedBuffers;

    IBLDiffuse()
    {
        multiLayerTexture = new TextureData();
    }

    //�I�t�X�N���[�������_�����O�̃t���[������ݒ肷��
    void setFrameCount(int frameCount)
    {
        mappedBuffers.resize(frameCount);

        passData.setFrameCount(frameCount);
    }

    void setMipmapLevel(uint32_t level)
    {
        mipmapLevel = level;
        multiLayerTexture->mipLevel = level;
    }

    //�����_�����O�̏o�͐�̃T�C�Y��ݒ�
    void setRenderSize(uint32_t s)
    {
        size = s;

        //diffuse�e�N�X�`���̃T�C�Y
        passData.width = s;
        passData.height = s;
    }

    void destroy(VkDevice& device)
    {
        multiLayerTexture->destroy(device);

        passData.destroy(device);

        for (auto mappedBuffer : mappedBuffers)
        {
            mappedBuffer.destroy(device);
        }
    }
};

//�L���[�u�}�b�v�쐬�p�̍\����
struct BackGroundColor
{
    //�摜�t�H�[�}�b�g
    VkFormat format;

    //�L���[�u�}�b�v�쐬���̃r���[�s��
    glm::mat4 view;
    //�L���[�u�}�b�v�쐬���̃r���[�s�� ����p��x���@��90�x�ɐݒ�
    glm::mat4 proj;

    //�I�t�X�N���[�������_�����O�p�̃f�[�^
    OffScreenPass passData;
    //�s��̔z��
    std::vector<MatricesUBO> matUBOs;
    //�o�b�t�@�̔z��
    std::vector<MappedBuffer> mappedBuffers;

    TextureData* srcHdriTexture;//�L���[�u�}�b�s���O�̌��̃e�N�X�`��
    TextureData* multiTexture;//�L���[�u�}�b�s���O�p�̃e�N�X�`���f�[�^

    VkDescriptorSet descriptorSet;

    //�L���[�u�}�b�v�쐬�p�̃p�C�v���C�����C�A�E�g
    VkPipelineLayout pipelineLayout;
    //���ۂ̃p�C�v���C��
    VkPipeline pipeline;

    BackGroundColor()
    {
        //�L���[�u�}�b�v�̌��ƂȂ�HDRI�摜�̃e�N�X�`���f�[�^
        srcHdriTexture = new TextureData();
        //�L���[�u�}�b�v�ƂȂ�6���̉摜����̃e�N�X�`���f�[�^�ɂ܂Ƃ߂�����
        multiTexture = new TextureData();
    }

    //�L���[�u�}�b�v�̖ʂ̐������쐬
    void setFrameCount(int frameCount)
    {
        matUBOs.resize(frameCount);
        mappedBuffers.resize(frameCount);
        passData.setFrameCount(frameCount);
    }

    void destroy(VkDevice& device)
    {
        srcHdriTexture->destroy(device);
        multiTexture->destroy(device);
        passData.destroy(device);

        vkDestroyPipeline(device, pipeline, nullptr);
        vkDestroyPipelineLayout(device, pipelineLayout, nullptr);

        for (auto& buffer : mappedBuffers)
        {
            buffer.destroy(device);
        }
    }
};

class Cubemap : public Model
{
private:

    BackGroundColor backGroundColor;//�L���[�u�}�b�s���O�p�̃f�[�^
    IBLDiffuse iblDiffuse;//IBL��diffuse�ɂ��Ă̍\����
    IBLSpecularReflection iblSpecularReflection;//IBL��specular�̋��ʔ��˂ɂ��Ă̍\����
    IBLSpecularBRDF iblSpecularBRDF;//IBL��specular��BRDF�ɂ��Ă̍\����

    //�L���[�u�}�b�s���O�p�̉摜
    std::shared_ptr<ImageData> hdriMap;

public:

    Cubemap() {};

    //hdri�摜�̐ݒ�Ǝ擾
    void setHDRIMap(std::shared_ptr<ImageData> image)
    {
        hdriMap = image;
    }

    std::shared_ptr<ImageData> getHDRIMap()
    {
        return hdriMap;
    }

    //�w�i�F�p�̃f�[�^��Ԃ�
    BackGroundColor& getBackGroundColor()
    {
        return backGroundColor;
    }

    //Diffuse�̃f�[�^��Ԃ�
    IBLDiffuse& getDiffuse()
    {
        return iblDiffuse;
    }

    //���ʔ��˂̔��˗v�f�̃f�[�^��Ԃ�
    IBLSpecularReflection& getSpecularReflection()
    {
        return iblSpecularReflection;
    }

    //���ʔ��˂�BRDF�̃f�[�^��Ԃ�
    IBLSpecularBRDF& getSpecularBRDF()
    {
        return iblSpecularBRDF;
    }

    //���j�t�H�[���o�b�t�@�̍X�V
	void updateUniformBuffer() override;

    //�t���[���I�����Ɏ��s
    void frameEnd() override;

    void cleanupVulkan() override;
};