from socket import *
import threading, queue
import select
import lib
import sys
MAX_SIZE = 4096
cloudPort=56789
localPort=12345
localPort6=12346
cloudHost="ec2-54-149-225-8.us-west-2.compute.amazonaws.com"
host1='localhost'
cloudAddr=(cloudHost, cloudPort)
lock=threading.Lock()
sending=False

coojaNodeID=(0 ,'aaaa::212:7401:1:101',
		      'aaaa::212:7402:2:202',
                'aaaa::212:7403:3:303',
                'aaaa::212:7404:4:404',                                              
		'aaaa::212:7405:5:505',
                'aaaa::212:7406:6:606',
                'aaaa::212:7407:7:707',
		'aaaa::212:7408:8:808',
                'aaaa::212:7409:9:909', 
                    )

def getLocalHost():
    s = socket(AF_INET, SOCK_DGRAM)
    s.connect(("8.8.8.8", 80))
    lHost=s.getsockname()[0]
    print('LocalHost: %s' %lHost)
    s.close()
    return lHost
def synToCloud(sock):
    print("Sychronizing to Web cloud...")
    for x in range (1, 5):
        temPkg=lib.rPkg.copy()
        temPkg['Command']=lib.cmdType['Command to syn']
        send=lib.prepPkg(temPkg)
        sock.sendto(send,cloudAddr)
        ready = select.select([sock], [], [], 2) #2s
        if ready[0] == []: # Timeout
            print('Time out %d' %x)
            continue
        data, addr = sock.recvfrom(1024)
        p=lib.recvPkgHandler(data)
        if(p!=None):
            if(p['Command']==lib.cmdType['Reply syn']):
                print("Syn ok")
                return 1
            else:
                print("error no. %d" %x)
                return -1
def sendCmdToNode(data, NodeID):
    #use default node 2
    if(NodeID<2):
        NodeID=2
    print('Sending cmd to node %d' %NodeID)
    nodeAddr=(coojaNodeID[NodeID], 3000)
    print(nodeAddr)
    sock6.sendto(data, nodeAddr)
    ready= select.select([sock6],[],[], 1) #1s time out
    if ready[0] == []: # Timeout
        print('Send Cmd to node time out %d' %NodeID)
        rep=lib.rPkg.copy()
        rep['NodeId']=NodeID
        rep['ErrorCode']=lib.errCode['Timeout']
        pkg=lib.prepPkg(rep)
        sock4.sendto(pkg, cloudAddr)

def sendToCloud():
    while True:
        data, addr=sock6.recvfrom(100)
        #check if there is cmd to stop sending data
        if(sending==True):
            sock4.sendto(data, cloudAddr)


if __name__ == '__main__':
    print('Hello world')
    #ipv4 socket to cloud
    host=getLocalHost()
    sock4 = socket(AF_INET, SOCK_DGRAM)
    sock4.bind((host, localPort))
    #ipv6 socket to WSNs
    sock6=socket(AF_INET6, SOCK_DGRAM)
    #sock6.bind(('', localPort6)) 
    #Synchronize to cloud
    status=synToCloud(sock4)
    if(status==1):
        switchDataThread=threading.Thread(target=sendToCloud,
                                            args=[],
                                            daemon=True)
        switchDataThread.start()
    else:
        print('Failed to connect to cloud, try again.')
        sys.exit()
    
    while True:
        print('Start receiving')
        data, addr=sock4.recvfrom(100)
        pkg=lib.recvPkgHandler(data)
        pkg['NodeID']=2
        if(pkg!=None):
            if(pkg['Command']==lib.cmdType['Command to start']):
                if(pkg['NodeID']==0):
                    for node in range(2, len(coojaNodeID)):
                        sendCmdToNode(data, node)
                elif(pkg['NodeID']==1):
                    print('This is border router')
                else:
                    sendCmdToNode(data, pkg['NodeID'])

                #start to update data to cloud if system is not updating
                sending=True
                       
            elif(pkg['Command']==lib.cmdType['Command to stop']):
                #Command to stop sendToCloudThread
                if(pkg['NodeID']==0):
                    sending=False
                    for node in range(2, len(coojaNodeID)):
                        sendCmdToNode(data, node)
                elif(pkg['NodeID']==1):
                    print('This is border router')
                else:
                    sendCmdToNode(data, pkg['NodeID'])
            else: print('Unknow command from cloud')