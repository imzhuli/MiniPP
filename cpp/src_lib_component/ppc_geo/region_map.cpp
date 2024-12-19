#include "./region_map.hpp"

#include <rapidcsv.hpp>
#include <unordered_map>

static std::vector<xPP_RegionInfo> LoadCsvRegionInfoFromFile(const char * filename) {
	auto Collection = std::vector<xPP_RegionInfo>();

	auto Doc      = rapidcsv::Document(filename, rapidcsv::LabelParams(-1, -1));
	auto RowCount = Doc.GetRowCount();
	auto ColCount = Doc.GetColumnCount();

	RuntimeAssert(ColCount == 8, "Column count should be 8");

	for (size_t I = 0; I < RowCount; ++I) {
		auto Row  = Doc.GetRow<std::string>(I);
		auto Info = xPP_RegionInfo();
		Touch(Row[0]);  // Ignore Continent Id
		Touch(Row[1]);  // Ignore Country Name
		Info.CountryName        = Row[2];
		Info.RegionId.CountryId = CountryCodeToCountryId(Row[3].c_str());
		Info.StateName          = Row[4];
		Info.RegionId.StateId   = (uint32_t)atol(Row[5].c_str());
		Info.CityName           = Row[6];
		Info.RegionId.CityId    = (uint32_t)atol(Row[7].c_str());
		Collection.push_back(std::move(Info));
	}

	return Collection;
}

static std::vector<xPP_RegionInfo>                             RegionInfoList;
static std::unordered_map<std::string, const xPP_RegionInfo *> RegionMapByStateCityName;
static std::unordered_map<std::string, const xPP_RegionInfo *> DefaultRegionMapByCityName;

void BuildRegionInfoMapFromCSV(const char * filename) {
	RegionMapByStateCityName.clear();
	auto DupCount = size_t(0);

	auto RegionInfoList = LoadCsvRegionInfoFromFile(filename);
	for (auto & I : RegionInfoList) {
		if (I.CityName.empty()) {
			continue;
		}

		auto Key = I.CountryName + '|' + I.StateName + '|' + I.CityName;

		auto & R = RegionMapByStateCityName[Key];
		if (R) {
			++DupCount;
			X_PERROR("Duplicate full name: %s, OtherId=%u, MyId=%u", Key.c_str(), (unsigned)R->RegionId.CityId, I.RegionId.CityId);
		}
		R = &I;
	}
	X_DEBUG_PRINTF("Valid fullname count: %i, Dup=%zi", (int)RegionMapByStateCityName.size(), DupCount);

	DefaultRegionMapByCityName.clear();
	DupCount = 0;
	for (auto & I : RegionInfoList) {
		if (I.CityName.empty()) {
			continue;
		}

		auto Key = I.CityName;

		auto & R = DefaultRegionMapByCityName[Key];
		if (R) {
			++DupCount;
			X_DEBUG_PRINTF(
				"Duplicate city name: %s, Country=%s, OtherId=%u, MyId=%u", I.CityName.c_str(), I.CountryName.c_str(), (unsigned)R->RegionId.CityId,
				I.RegionId.CityId
			);
			continue;
		}
		R = &I;
	}
	X_DEBUG_PRINTF("Valid city count: %i, Dup=%zi", (int)DefaultRegionMapByCityName.size(), DupCount);
}

const xPP_RegionInfo * GetRegionInfoByFullName(const std::string & FullName) {
	auto Iter = RegionMapByStateCityName.find(FullName);
	if (Iter == RegionMapByStateCityName.end()) {
		return nullptr;
	}
	return Iter->second;
}

const xPP_RegionInfo * GetRegionInfoByCityNameOnly(const std::string & CityName) {
	auto Iter = DefaultRegionMapByCityName.find(CityName);
	if (Iter == DefaultRegionMapByCityName.end()) {
		return nullptr;
	}
	return Iter->second;
}
