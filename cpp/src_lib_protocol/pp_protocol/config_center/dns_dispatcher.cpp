#include "./dns_dispatcher.hpp"

#include <crypto/md5.hpp>

std::string xEnableDnsDispatcher::GenerateChallenge(uint64_t UnixTimestamp, uint32_t Version, const std::string & ChallengeKey) {
	auto Hash    = "KeySource+" + ChallengeKey + std::to_string(UnixTimestamp) + std::to_string(Version);
	auto Md5Hash = xel::Md5(Hash.data(), Hash.size());
	return StrToHex(Md5Hash.Data(), Md5Hash.Size());
}
