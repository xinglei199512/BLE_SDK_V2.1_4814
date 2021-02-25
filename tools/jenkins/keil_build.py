# -*- coding: UTF-8 -*-
# import logging
# import logging.handlers
import os
import sys
import time

# logger = logging.getLogger("BlueX")
# logger.setLevel(level=logging.DEBUG)

#日志打印格式
# log_fmt = '%(asctime)s - %(filename)s[line:%(lineno)d] - %(levelname)s - %(message)s'
# formatter = logging.Formatter(log_fmt)

#文件
# rq = time.strftime('%Y-%m-%d_%H-%M-%S', time.localtime(time.time()))
# logfile = rq +'_ERR'+'.log'
# fh = logging.FileHandler(logfile, mode='w')
# fh.setLevel(logging.ERROR)
# fh.setFormatter(formatter)
#控制台
# console = logging.StreamHandler()
# console.setLevel(logging.INFO)
# console.setFormatter(formatter)

#日志时间滚动
# th = logging.handlers.TimedRotatingFileHandler(filename="bluex", when="H", interval=2, backupCount=0)
# th.suffix = "%Y-%m-%d_%H-%M-%S.log"
# th.extMatch = re.compile(r"^\d{4}-\d{2}-\d{2}_\d{2}-\d{2}-\d{2}.log$")
#
# th.setLevel(logging.DEBUG)
# th.setFormatter(formatter)

# logger.addHandler(th)
# logger.addHandler(fh)
# logger.addHandler(console)
# logger.debug("!!!KeilBuild run start!!!")

Apoll0_NAME = "apollo_00_keil5"

class KeilBuild(object):
    """
    keil build
    """
    __ERRORLEVEL__ = {0:'No Errors or Warnings',
                      1:'Warnings Only',
                      2:'Errors',
                      3:'Fatal Errors',
                      11:'Cannot open project file for writing',
                      12:'Device with given name in not found in database',
                      13:'Error writing project file',
                      15:'Error reading import XML file',
                      20:'Error converting project',
                      }
    _HDR_FILE_NAME_ = 'inc_dirs.txt'#头文件路径
    _PRJ_FILE_NAME_ = 'project_path.txt'#工程文件路径
    _SRC_FILE_NAME_ = 'srcs.txt'#源文件
    def __init__(self,argvs,uv_path =r'C:\Keil_v5\UV4\UV4.exe',Test=False):
        try:
            self.RESULT = None
            self.ERR_LEVEL_SET = 1#'No Errors or Warnings'
            self.__uv = uv_path
            self._build_type = '.uvprojx'
            self.__uvprojx = ''
            self.__target_path=''
            self.build_time=''
            self.__objs = (self._PRJ_FILE_NAME_, self._SRC_FILE_NAME_, self._HDR_FILE_NAME_)
            if self.__get_project_files(argvs, Test) == True:
               self.__parse_prj(self.__prj_file)

        except:
            self.RESULT = False
            # logger.exception("!!!!!!!!!!!!!!!!##--ERROR--##!!!!!!!!!!!!!!!!")
    def __check_obj_list_path(self,path):
        """
        检查输入路径是否合法
        :param path:
        :return: True 合法
        """
        try:
            result = True

            if os.path.exists(path) and os.path.isdir(path):
                for f in self.__objs:
                    if f not in os.listdir(path):
                        result = False
                        break
                    else:
                        f_p = os.path.normpath(os.path.join(path,f)).replace('\\','/')
                        if self.__files_check(f_p, f) ==False :
                            result = False
                            break
            else:
                result = False

            return  result
        except:
            self.RESULT = False
            # logger.exception("!!!!!!!!!!!!!!!!##--ERROR--##!!!!!!!!!!!!!!!!")

    def __get_project_files(self,argvs,cmd=False):
        try:
            if len(argvs) == 2:
                select_path = sys.argv[1]
                # logger.debug(select_path)
                # select_path = select_paths.split('\\')[-1]
                # print(select_path)
            elif len(argvs) == 3:
                select_path = sys.argv[1]
                # logger.debug(select_path)
                self.ERR_LEVEL_SET = int(sys.argv[2])
                # logger.debug(self.ERR_LEVEL_SET)
            elif len(argvs) == 4:
                select_path = sys.argv[1]
                # logger.debug(select_path)
                self.ERR_LEVEL_SET = int(sys.argv[2])
                # logger.debug(self.ERR_LEVEL_SET)
                self.__uv = sys.argv[3]
                # logger.debug(self.__uv)
            else:
                select_path = None

            if select_path == None:
                while True:
                    if cmd == False:
                        select_path = input("请输入目标工程的list文件路径：")
                    else:
                        # select_path = r"F:\SVN\BX2400_A5\armbin\output\osapp_dis_server_lists"
                        select_path = r"F:\SVN\BX2400_A5\armbin\output\MESH_simple_pts_ctl_lists"
                    if self.__check_obj_list_path(select_path) == True:
                        break
            else:
                assert self.__check_obj_list_path(select_path) ==True

            self.__prj_file = os.path.normpath(os.path.join(select_path,self._PRJ_FILE_NAME_)).replace('\\','/')
            # logger.debug(self.__prj_file )

            return True
        except:
            self.RESULT = False
            # logger.exception("!!!!!!!!!!!!!!!!##--ERROR--##!!!!!!!!!!!!!!!!")

    def __files_check(self,path,type):
        try:
            result = True
            if os.path.exists(path) and os.path.isfile(path) and os.access(path, os.R_OK) :
                result = True
            else:
                result = False

            if result == True:
                [dirname, filename] = os.path.split(path)
                # logger.debug("dir->[{0}] file->[{1}]".format(dirname, filename))
                if filename != type:
                    result = False
                    # logger.error("文件 {0} 的名称必须是 {1} ".format(filename,type))
            # else:
                # logger.error("请检查输入，输入文件的错误!")
            return result
        except:
            self.RESULT = False
            # logger.exception("!!!!!!!!!!!!!!!!##--ERROR--##!!!!!!!!!!!!!!!!")

    def __parse_prj(self,file):
        """
        设置工程名称，工程路径，建立工程文件夹
        :param file:
        :return:
        """
        try:
            # logger.debug("__parse_prj -> {0}".format(file))
            line=""
            with open(file,'r') as f:
                line = f.readline().rstrip('\n')

            assert len(line)>0
            # logger.debug("path -> {0}".format(line))
            assert os.path.exists(line)
            path = os.path.join(line,Apoll0_NAME)
            assert os.path.exists(path)
            self.__target_path = path.replace('\\','/')
            # logger.debug("target_path -> {0}".format(self.__target_path ))
            name = line.replace('\\','/').split('/')[-1]+self._build_type
            # logger.debug("name -> {0}".format(name))
            self.__uvprojx = os.path.join(path,name).replace('\\','/')
            # logger.debug("[uvprojx] -> {0}".format(self.__uvprojx))
            assert os.path.exists(self.__uvprojx)

        except:
            self.RESULT = False
            # logger.exception("!!!!!!!!!!!!!!!!##--ERROR--##!!!!!!!!!!!!!!!!")
    def build(self):
        try:
            assert self.RESULT != False

            temp_log = os.path.join(self.__target_path,'build_log.txt')
            with open(temp_log,'w') as f:
                f.seek(0,0)
                f.truncate()

            start_time = time.time()
            print('Init building ...')

            cmd = self.__uv.replace('\\','/')+' '+'-j0 -r'+' '+self.__uvprojx.replace('\\','/')+' '+'-o'+' '+temp_log.replace('\\','/')
            result = os.system(cmd)

            with open(temp_log, 'r') as f:
                print(f.read())
            print('Done ...')
            stop_time = time.time()
            self.build_time = "[{:.3f} 秒]".format((start_time - stop_time))
            os.remove(temp_log)
            
            if result in self.__ERRORLEVEL__.keys():
                print(self.__ERRORLEVEL__[result])
                assert result <= self.ERR_LEVEL_SET
            else:
                assert len('Build Error -> {0}'.format(result)) ==0
                
            self.RESULT = True
        except:
            self.RESULT = False
            # logger.exception("!!!!!!!!!!!!!!!!##--ERROR--##!!!!!!!!!!!!!!!!")


def ver_info():
    print("<Keil5 Build Tool ver 0.1 >")
    # print("请把工具放在与SDK代码的 mesh 目录下与examples目录同一级")


def main():
    ver_info()

    build = KeilBuild(sys.argv,Test=False)
    build.build()
    assert build.RESULT == True
    # os.system('pause')

if __name__ == '__main__':
    main()