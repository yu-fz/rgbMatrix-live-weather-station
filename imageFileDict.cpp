#include "./imageFileDictHeader.h"

//dict to look match weather codes to image file names 
vector<string> lookUpImageToDraw(vector<int>& weatherID)
{
	std::unordered_map<int, string> weatherCodeDict;
	weatherCodeDict.insert(pair<int, string>(200, "tStorm-1"));
	weatherCodeDict.insert(pair<int, string>(201, "tStorm-7"));
	weatherCodeDict.insert(pair<int, string>(202, "tStorm-8"));
	weatherCodeDict.insert(pair<int, string>(210, "tStorm-1"));
	weatherCodeDict.insert(pair<int, string>(211, "tStorm-2"));
	weatherCodeDict.insert(pair<int, string>(212, "tStorm-8"));
	weatherCodeDict.insert(pair<int, string>(221, "tStorm-9"));
	weatherCodeDict.insert(pair<int, string>(230, "tStorm-1"));
	weatherCodeDict.insert(pair<int, string>(231, "tStorm-7"));
	weatherCodeDict.insert(pair<int, string>(232, "tStorm-8"));
	weatherCodeDict.insert(pair<int, string>(300, "rainy-1"));
	weatherCodeDict.insert(pair<int, string>(301, "rainy-2"));
	weatherCodeDict.insert(pair<int, string>(302, "rainy-3"));
	weatherCodeDict.insert(pair<int, string>(310, "rainy-1"));
	weatherCodeDict.insert(pair<int, string>(311, "rainy-2"));
	weatherCodeDict.insert(pair<int, string>(312, "rainy-3"));
	weatherCodeDict.insert(pair<int, string>(313, "rainy-2"));
	weatherCodeDict.insert(pair<int, string>(314, "rainy-3"));
	weatherCodeDict.insert(pair<int, string>(321, "rainy-2"));
	weatherCodeDict.insert(pair<int, string>(500, "rainy-7"));
	weatherCodeDict.insert(pair<int, string>(501, "rainy-4"));
	weatherCodeDict.insert(pair<int, string>(502, "rainy-5"));
	weatherCodeDict.insert(pair<int, string>(503, "rainy-6"));
	weatherCodeDict.insert(pair<int, string>(504, "rainy-6"));
	weatherCodeDict.insert(pair<int, string>(511, "hail-1"));
	weatherCodeDict.insert(pair<int, string>(520, "rainy-7"));
	weatherCodeDict.insert(pair<int, string>(521, "rainy-4"));
	weatherCodeDict.insert(pair<int, string>(522, "rainy-5"));
	weatherCodeDict.insert(pair<int, string>(531, "rainy-5"));
	weatherCodeDict.insert(pair<int, string>(600, "snow-1"));
	weatherCodeDict.insert(pair<int, string>(601, "snow-2"));
	weatherCodeDict.insert(pair<int, string>(602, "snow-4"));
	weatherCodeDict.insert(pair<int, string>(611, "sleet-1"));
	weatherCodeDict.insert(pair<int, string>(612, "sleet-1"));
	weatherCodeDict.insert(pair<int, string>(613, "sleet-1"));
	weatherCodeDict.insert(pair<int, string>(615, "sleet-1"));
	weatherCodeDict.insert(pair<int, string>(616, "sleet-1"));
	weatherCodeDict.insert(pair<int, string>(620, "sleet-1"));
	weatherCodeDict.insert(pair<int, string>(621, "sleet-1"));
	weatherCodeDict.insert(pair<int, string>(622, "sleet-1"));
	weatherCodeDict.insert(pair<int, string>(701, "foggy-1"));
	weatherCodeDict.insert(pair<int, string>(711, "foggy-1"));
	weatherCodeDict.insert(pair<int, string>(721, "foggy-2"));
	weatherCodeDict.insert(pair<int, string>(731, "foggy-2"));
	weatherCodeDict.insert(pair<int, string>(741, "foggy-2"));
	weatherCodeDict.insert(pair<int, string>(751, "foggy-2"));
	weatherCodeDict.insert(pair<int, string>(761, "foggy-2"));
	weatherCodeDict.insert(pair<int, string>(762, "foggy-2"));
	weatherCodeDict.insert(pair<int, string>(771, "foggy-2"));
	weatherCodeDict.insert(pair<int, string>(781, "tornado"));
	weatherCodeDict.insert(pair<int, string>(800, "clear"));
	weatherCodeDict.insert(pair<int, string>(801, "cloudy-2"));
	weatherCodeDict.insert(pair<int, string>(802, "cloudy-2"));
	weatherCodeDict.insert(pair<int, string>(803, "cloudy-3"));
	weatherCodeDict.insert(pair<int, string>(804, "cloudy-3"));

	vector<string> fileNames;
	fileNames.clear();

	for (auto i : weatherID)
	{
		auto it = weatherCodeDict.find(i);
		fileNames.push_back(it->second); //retrieve array of image file names to be displayed
	}

	return fileNames;
}
