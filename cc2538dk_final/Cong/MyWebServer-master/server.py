from socket import *
from struct import *
from collections import namedtuple
import lib
import threading, queue
import pymysql as db
#import mysql.connector
receiving=False

maxsize = 100
host='ec2-54-149-225-8.us-west-2.compute.amazonaws.com'
host1='localhost'
port=56789
hostAddr=(host, port)
lastSynAddr=('localhost',23456)#random address
#addr that allowed to update data
lock= threading.Lock()

def webCmdHandler():
    
    lcSock=socket(AF_INET, SOCK_DGRAM)
    lcSock.bind(('localhost', 12345))
    msg=lib.rPkg.copy()
    while True:
        data, addr=lcSock.recvfrom(100)
        if(data==b'start'):
            #print('Command to start')
            msg['Command']=lib.cmdType['Command to start']
        elif(data==b'stop'):
            #print('Command to stop')
            msg['Command']=lib.cmdType['Command to stop']
        pkg=lib.prepPkg(msg)
        sock.sendto(pkg, lastSynAddr)
    lcSock.close()
def dataHandler():
    dCon=db.connect('localhost', 'root', '11235813', 'nodedata', autocommit=True)
    dCur=dCon.cursor()
    while True:
        data=dataQueue.get()

        insert=lib.getInsertSQLCmd(data['Temperature'], data['Humidity'],data['Lux'] )
        try:
            dCur.execute(insert)
        except:
            dCon.rollback()

    dCon.close()


def createNewTable():
    con=db.connect('localhost', 'root', '11235813', 'nodedata')
    cur=con.cursor()
    cur.execute("DROP TABLE IF EXISTS node")
    new=lib.newTable
    cur.execute(new)
    #print('Created new table')
    con.close()
    return
        
if __name__=='__main__':
    lastSynAddr=hostAddr
    sock = socket(AF_INET,SOCK_DGRAM)
    sock.bind(hostAddr)
    dataQueue=queue.Queue()

    dataThread=threading.Thread(target=dataHandler,
                                        args=[],
                                        daemon=True)
    webHandlerThread=threading.Thread(target=webCmdHandler,
                                        args=[],
                                        daemon=True)
    dataThread.start()
    webHandlerThread.start()
    while True:
        #print("Wait for data from node...")
        data, addr = sock.recvfrom(maxsize)
        #print('Receive from %s' %str(addr))
        msg=lib.recvPkgHandler(data)
        if(msg!=None):
            #lib.printCmd(msg['Command'])        
            if(msg['Command']==lib.cmdType['Command to syn']):
                lastSynAddr=addr
                createNewTable()
                msg['Command']=lib.cmdType['Reply syn']
                tem=lib.prepPkg(msg)
                sock.sendto(tem, lastSynAddr)
                receiving=True
            elif(msg['Command']==lib.cmdType['Reply data']):
                if(lastSynAddr==addr):
                    with lock:
                        dataQueue.put(msg)        
        
