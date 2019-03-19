#include "pch.h"
#include "EngineUtil.h"

namespace engine
{
	std::random_device EngineUtil::s_RandDevice;

	std::mt19937_64 EngineUtil::s_RandNumGenerator(s_RandDevice());

	std::wstring_convert<std::codecvt<wchar_t, char, std::mbstate_t>> EngineUtil::s_StringConvertor;

	std::string EngineUtil::ToByteString(const std::wstring& WideString)
	{
		return s_StringConvertor.to_bytes(WideString);
	}

	std::wstring EngineUtil::ToWideString(const std::string& ByteString)
	{
		return s_StringConvertor.from_bytes(ByteString);
	}

	void EngineUtil::ResetSeed()
	{
		s_RandNumGenerator.seed(s_RandNumGenerator.default_seed);
	}

	double EngineUtil::GetRandomValue()
	{
		return std::generate_canonical<double, 10>(s_RandNumGenerator);
	}

	double EngineUtil::GetRandomValue(unsigned int x, unsigned int y, unsigned long long SeedValue)
	{
		s_RandNumGenerator.seed(x * 65655 + y * 43434 + SeedValue);
		return std::generate_canonical<double, 10>(s_RandNumGenerator);
	}
}