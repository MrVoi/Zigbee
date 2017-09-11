from socket import *
from struct import *
from collections import namedtuple
#packet format
pkgFmt=namedtuple( 'pkgFmt', 'Command  ErrorCode NodeID Padding Temperature Lux Humidity')


#command type list
cmdType={'Command to start':  0,
	    'Command to stop':  1,
	    'Command to update':2,
	    'Command to syn':   3,
	    'Reply syn':        4,
	    'Reply start':      5,
	    'Reply stop':       6,
	    'Reply update':     7,
            'Reply data':       8,
            'Unknow command':   9
}
#real packet
rPkg={	        'Command':      0,
		'NodeID':       0,
		'ErrorCode':    0,
		'Padding':	0,
		'Temperature':  0,
		'Lux':          0,
                'Humidity':     0
}
errCode={	'No error':     0,
		'Syn failed':   1,
                'Timeout':      3,	
	
}

newTable="""create table node( id INT(6) UNSIGNED AUTO_INCREMENT PRIMARY KEY,
            Temperature SMALLINT(5), Humidity SMALLINT(5), Lux SMALLINT(5))"""
def getInsertSQLCmd(x,y,z):
    return "INSERT INTO node(Temperature, Humidity, Lux)\
         VALUES ('%d', '%d', '%d' )" % (x, y, z)
#prepare packet to send
def prepPkg(package):
    pkg=pack('BBBBHHH',	package['Command'],
			package['ErrorCode'], 
             		package['NodeID'],
             		package['Padding'],	
                    	package['Temperature'],
                    	package['Lux'],
                        package['Humidity'])
    return pkg

def printCmd(cmd):    
    if(cmd==cmdType['Command to syn']):
        print('Command to syn')
    elif(cmd==cmdType['Command to stop']):
        print('Command to send')
    elif(cmd==cmdType['Command to update']):
        print('Command to update')
    elif(cmd==cmdType['Command to stop']):
        print('Command to to stop')
    elif(cmd==cmdType['Reply syn']):
        print ('Reply syn')
    elif(cmd==cmdType['Reply start']):
        print('Reply send')
    elif(cmd==cmdType['Reply stop']):
        print('Reply finish')
    elif(cmd==cmdType['Reply data']):
        print('Reply data')
    else:
        print('Unknow command')

#Unpack the received packet
def recvPkgHandler(data):
    try:
        p=pkgFmt._asdict(pkgFmt._make(unpack('BBBBHHH', data)))
        return p
    except AttributeError:        
        return None
