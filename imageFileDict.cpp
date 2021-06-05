#include "./imageFileDictHeader.h"
/* TODO:
 *  implement weatherCodeDict as a global variable so the dict does not get recreated every API call 
 *	
 *	map each second element of the pair to the file ext. 
 *
 */
//dict to look match weather codes to image file names 
vector<string> lookUpImageToDraw(vector<int>& weatherID)
{
	std::unordered_multimap<int, string> weatherCodeDict;
	weatherCodeDict.insert(pair<int, string>(200, "tstorm-1"));
	weatherCodeDict.insert(pair<int, string>(201, "tstorm-7"));
	weatherCodeDict.insert(pair<int, string>(202, "tstorm-8"));
	weatherCodeDict.insert(pair<int, string>(210, "tstorm-1"));
	weatherCodeDict.insert(pair<int, string>(211, "tstorm-2"));
	weatherCodeDict.insert(pair<int, string>(212, "tstorm-8"));
	weatherCodeDict.insert(pair<int, string>(221, "tstorm-9"));
	weatherCodeDict.insert(pair<int, string>(230, "tstorm-1"));
	weatherCodeDict.insert(pair<int, string>(231, "tstorm-7"));
	weatherCodeDict.insert(pair<int, string>(232, "tstorm-8"));
	weatherCodeDict.insert(pair<int, string>(300, "precipitation")); // cloud-precip
	weatherCodeDict.insert(pair<int, string>(301, "precipitation"));
	weatherCodeDict.insert(pair<int, string>(302, "precipitation"));
	weatherCodeDict.insert(pair<int, string>(310, "precipitation"));
	weatherCodeDict.insert(pair<int, string>(311, "precipitation"));
	weatherCodeDict.insert(pair<int, string>(312, "precipitation"));
	weatherCodeDict.insert(pair<int, string>(313, "precipitation"));
	weatherCodeDict.insert(pair<int, string>(314, "precipitation"));
	weatherCodeDict.insert(pair<int, string>(314, "precipitation"));
	weatherCodeDict.insert(pair<int, string>(321, "precipitation"));
	weatherCodeDict.insert(pair<int, string>(500, "precipitation"));
	weatherCodeDict.insert(pair<int, string>(500, "precipitation"));
	weatherCodeDict.insert(pair<int, string>(501, "precipitation"));
	weatherCodeDict.insert(pair<int, string>(501, "precipitation"));
	weatherCodeDict.insert(pair<int, string>(502, "precipitation"));
	weatherCodeDict.insert(pair<int, string>(503, "precipitation"));
	weatherCodeDict.insert(pair<int, string>(503, "precipitation"));
	weatherCodeDict.insert(pair<int, string>(504, "precipitation"));
	weatherCodeDict.insert(pair<int, string>(504, "precipitation"));
	weatherCodeDict.insert(pair<int, string>(511, "precipitation"));
	weatherCodeDict.insert(pair<int, string>(520, "precipitation"));
	weatherCodeDict.insert(pair<int, string>(521, "precipitation"));
	weatherCodeDict.insert(pair<int, string>(522, "precipitation"));
	weatherCodeDict.insert(pair<int, string>(531, "precipitation"));
	weatherCodeDict.insert(pair<int, string>(600, "precipitation"));
	weatherCodeDict.insert(pair<int, string>(601, "precipitation"));
	weatherCodeDict.insert(pair<int, string>(602, "precipitation"));
	weatherCodeDict.insert(pair<int, string>(602, "precipitation"));
	weatherCodeDict.insert(pair<int, string>(611, "precipitation"));
	weatherCodeDict.insert(pair<int, string>(612, "precipitation"));
	weatherCodeDict.insert(pair<int, string>(613, "precipitation"));
	weatherCodeDict.insert(pair<int, string>(615, "precipitation"));
	weatherCodeDict.insert(pair<int, string>(616, "precipitation"));
	weatherCodeDict.insert(pair<int, string>(620, "precipitation"));
	weatherCodeDict.insert(pair<int, string>(621, "precipitation"));
	weatherCodeDict.insert(pair<int, string>(622, "precipitation"));
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
	weatherCodeDict.insert(pair<int, string>(800, "clear-1"));
	weatherCodeDict.insert(pair<int, string>(800, "clear-2"));
	weatherCodeDict.insert(pair<int, string>(801, "cloudy-2"));
	weatherCodeDict.insert(pair<int, string>(802, "cloudy-2"));
	weatherCodeDict.insert(pair<int, string>(803, "cloudy-3"));
	weatherCodeDict.insert(pair<int, string>(804, "cloudy-3"));

	vector<string> fileNames;
	fileNames.clear();

	for (auto i : weatherID)
	{

		//I can append file ext here 
		//
		auto it = weatherCodeDict.find(i);
		fileNames.push_back(it->second); //retrieve array of image file names to be displayed
		auto range = weatherCodeDict.equal_range(i);
		for (auto it = range.first; it != range.second; ++it)
		{
			fileNames.push_back(it->second);
		}

	}

	return fileNames;
}
