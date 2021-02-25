import sys
import os
import paramiko
from intelhex import IntelHex


ssh = paramiko.SSHClient()
ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
ssh.connect('172.18.18.118', 22, 'pi', 'raspberry')

chan = ssh.get_transport().open_session()
chan.exec_command('sudo umount -f /mnt/test_pc')
exit_code = chan.recv_exit_status()
print ('umount nfs,'+str(exit_code))

chan = ssh.get_transport().open_session()
chan.exec_command('sudo mount -t nfs 172.18.18.10:/e/ /mnt/test_pc -o nfsvers=3')
exit_code = chan.recv_exit_status()
print ('mount nfs,'+str(exit_code))
assert exit_code == 0

path = os.getcwd()
path = '/'.join(path.split('\\'))
path = path[path.find('/'):]

chan = ssh.get_transport().open_session()
chan.exec_command('python3 /mnt/test_pc' + path +'/Trunk/tools/jenkins/RaspberryPi/reset_control.py BOOT_FROM_UART')
exit_code = chan.recv_exit_status()
print ('RESET,P16 HIGH,'+str(exit_code))
assert exit_code == 0

chan = ssh.get_transport().open_session()
chan.exec_command('python3 /mnt/test_pc' + path +'/Trunk/tools/jenkins/RaspberryPi/swd_sel.py '+ sys.argv[2])
exit_code = chan.recv_exit_status()
print ('SWD SEL ' + sys.argv[2] +','+str(exit_code))
assert exit_code == 0

if sys.argv[1] != "EMPTY_PROJECT":
    hex_path = './gccbin/output/'+sys.argv[1]+'/'+sys.argv[1]+'_with_bootloader.hex'
    original = IntelHex(hex_path)
    info_page = IntelHex('./Trunk/tools/jenkins/info_page_'+ sys.argv[2] +'.hex')
    original.merge(info_page)
    new_path = './gccbin/output/'+sys.argv[1]+'/'+sys.argv[1]+'_with_bootloader_info_page_' + sys.argv[2] + '.hex'
    original.write_hex_file(new_path)
else:
    new_path = './Trunk/tools/jenkins/info_page_'+ sys.argv[2] +'.hex'

jflash_program = os.popen('JFlash.exe -openprj./Trunk/tools/jenkins/apollo_00_1v8_chip_erase.jflash -open'+new_path+' -auto -exit')
exit_code = jflash_program.close()
print ('JFlash download,'+str(exit_code))
assert exit_code is None

chan = ssh.get_transport().open_session()
chan.exec_command('python3 /mnt/test_pc' + path +'/Trunk/tools/jenkins/RaspberryPi/reset_control.py BOOT_FROM_FLASH')
exit_code = chan.recv_exit_status()
print ('RESET,P16 LOW,'+str(exit_code))
assert exit_code == 0

ssh.close()

    

    
