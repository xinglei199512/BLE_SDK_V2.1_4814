import os
import sys
from ftplib import FTP

file = open(sys.argv[3],'rb')

ftp = FTP('172.18.18.5')
ftp.login(sys.argv[1],sys.argv[2])
ftp.storbinary('STOR %s' % os.path.basename(sys.argv[3]) , file)
ftp.quit()

file.close()