#pragma once

#include"SkyDomeBuilder.h"

struct SkyDomeHash
{
	const uint64_t FNV_PRIME = 1099511628211LLU; // 2^40 + 2^8 + 0xB3 = 0x100000001b3
	const uint64_t FNV_OFFSET_BIAS = 14695981039346656037U; // 0xcbf29ce484222325

	void getHash(const uint32_t& u, size_t& hash) const
	{
		uint64_t u64 = u;

		hash ^= u64;
		hash *= FNV_PRIME;
	}

	void getHash(const float f, size_t& hash) const
	{
		uint32_t u;
		std::memcpy(&u, &f, sizeof(float));

		getHash(u, hash);
	}

	void getHash(const std::string str, size_t& hash) const
	{
		uint32_t u;
		u = static_cast<uint32_t>(std::hash<std::string>()(str));

		getHash(u, hash);
	}

	size_t operator()(const SkyDomeProperty& prop) const
	{
		size_t hash = FNV_OFFSET_BIAS;

		getHash(prop.srcImagePath, hash);

		return hash;
	}
};

class SkyDomeFactory
{
private:

	//�n�b�V�����\����
	SkyDomeHash hash;

	//�r���_�[
	std::shared_ptr<SkyDomeBuilder> builder;

	//�쐬����SkyDome�������ɋL�^����
	std::unordered_map<uint32_t, std::shared_ptr<SkyDome>> skydomeMap;

public:

	SkyDomeFactory(std::shared_ptr<SkyDomeBuilder> b);

	~SkyDomeFactory()
	{
#ifdef _DEBUG
		std::cout << "SkyDomeFactory :: �f�X�g���N�^" << std::endl;
#endif
	}

	//SkyDome���쐬���āA����ID��Ԃ�
	uint32_t Create(const SkyDomeProperty& property);

	//ID����SkyDome���擾����
	std::shared_ptr<SkyDome> GetSkyDome(const uint32_t id);
};