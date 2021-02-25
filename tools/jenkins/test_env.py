import sys
import os
import paramiko

pc_test_case_path = 'Trunk/tools/jenkins/test_case/pc/'+sys.argv[1]+'.py'
pi_test_case_path = 'Trunk/tools/jenkins/test_case/pi/'+sys.argv[1]+'.py'

if os.path.exists(pc_test_case_path):
    exit_code = os.popen('python '+pc_test_case_path).close()
    print(pc_test_case_path+':'+str(exit_code))
    assert exit_code == 0
    
if os.path.exists(pi_test_case_path):
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
    print("=====START TEST=====")
    chan.exec_command('sudo python3 /mnt/test_pc' + path +'/'+pi_test_case_path)
    exit_code = chan.recv_exit_status()
    recv_data = chan.recv(1024)
    recv_err  = chan.recv_stderr(1024)
    print(recv_data.decode())
    print(recv_err.decode())
    print (pi_test_case_path+' exit_code='+str(exit_code)+'\r\n')
    assert exit_code == 0    