#include <ppc_geo/region_map.hpp>

int main(int, char **) {
	static const char * filename = "./test_assets/city.csv";

	BuildRegionInfoMapFromCSV(filename);

	return 0;
}
