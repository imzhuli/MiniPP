#pragma once
#include <pp_common/base.hpp>

struct xRegionInfo {

	static constexpr const xCountryId UNSPEC_COUNTRY_ID = 0;
	static constexpr const xCityId    UNSPEC_CITY_ID    = 0;

	/*** 说明:
	 * 1. 业务中, 使用CountryId/CityId, 不使用字符串名称.
	 * 2. 如果使用多个不同的地址库. 需要对其进行统一
	 * 3. 国家码使用IOS两位的表示法, 暂时高2字节为0, 后二字节为两位ASCII对应的数值
	 * 4. 国家名/城市名, 可以依赖地址库里的名字, 用于后期处理. 或透传给后台由后台自行处理
	 */

	xCountryId ContryId = UNSPEC_COUNTRY_ID;
	xCityId    CityId   = UNSPEC_CITY_ID;

	std::string countryName;
	std::string cityName;

	//
};
