import sys
import shutil

shutil.make_archive(sys.argv[1],'zip',sys.argv[1])