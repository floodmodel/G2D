#include <stdio.h>
#include <iostream>
#include <filesystem>
#include <string>

#include "gentle.h"
#include "g2dLib.h"

using namespace std;
namespace fs = std::filesystem;

extern fs::path fpn_prj;
extern fs::path fpn_log;
extern fs::path fp_prj;
extern generalEnv genEnv;

int deleteAlloutputFiles()
{
	//모의 시작 할때, 다 지우고 새로 만든다
	vector<string> fpns;
	fs::path fn = fpn_prj.filename();
	string prjNameWithoutExt = fn.replace_extension().u8string();
	for (const auto & entry : fs::directory_iterator(fp_prj))
	{
		string afile = entry.path().u8string();
		if (afile.find(CONST_OUTPUT_ASCFILE_EXTENSION,0) != string::npos  ||
			afile.find(CONST_OUTPUT_IMGFILE_EXTENSION,0) != string::npos 	||
			afile.find(CONST_OUTPUT_PROJECTIONFILE_EXTENSION,0) != string::npos ||
			afile.find(CONST_OUTPUT_QMLFILE_EXTENSION_LCASE,0) != string::npos ||
			afile.find(CONST_OUTPUT_QMLFILE_EXTENSION_UCASE,0) != string::npos )
		{
			if (afile.find(prjNameWithoutExt + CONST_FILENAME_TAG_DISCHARGE,0) != string::npos ||
				afile.find(prjNameWithoutExt + CONST_FILENAME_TAG_DEPTH,0) != string::npos  ||
				afile.find(prjNameWithoutExt + CONST_FILENAME_TAG_HEIGHT, 0) != string::npos  ||
				afile.find(prjNameWithoutExt + CONST_FILENAME_TAG_VELOCITY, 0) != string::npos  ||
				afile.find(prjNameWithoutExt + CONST_FILENAME_TAG_FLOWDIRECTION, 0) != string::npos  ||
				afile.find(prjNameWithoutExt + CONST_FILENAME_TAG_RFGRID, 0) != string::npos  ||
				afile.find(prjNameWithoutExt + CONST_FILENAME_TAG_BCDATA, 0) != string::npos  ||
				afile.find(prjNameWithoutExt + CONST_FILENAME_TAG_SOURCEALL, 0) != string::npos ||
				afile.find(prjNameWithoutExt + CONST_FILENAME_TAG_SINKDATA, 0) != string::npos )
			{
				fpns.push_back(afile);
			}
		}

	}
	if (fpns.size() > 0)
	{
		appendTextAndCloseFile( fpn_log, "Delete all output files...",1,1);
		if (confirmDeleteFiles(fpns) == -1)
		{
			genEnv.modelSetupIsNormal = -1;
			appendTextAndCloseFile(fpn_log,"Some output files were not deleted. Initializing new output files was failed.\n", 1,1);
			return -1;
		}
		appendTextAndCloseFile(fpn_log," completed.\n", 1,1);
	}
	return 1;
}
