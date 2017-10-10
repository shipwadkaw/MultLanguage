
import sys
import os
import os.path
import shutil

import convertString


def build(platform):
	infile = "./text.xls"
	outfile = os.path.abspath(os.path.join(os.path.basename(__file__)))
	if not os.path.exists(outfile):
		os.mkdir(outfile)

	tmpDir = "./lc"
        if os.path.exists(tmpDir):
		for f in os.listdir(tmpDir):
                        os.remove(tmpDir + "/" + f)
	os.mkdir(tmpDir)
	sheetFiles = convertString.ExportGameText(infile, tmpDir)

	
	return 0

if __name__ == "__main__":
	platform = "win32"
	if len(sys.argv) > 1:
		platform = sys.argv[1]
	ret = build(platform)
	sys.exit(ret)
