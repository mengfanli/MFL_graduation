import math
import threading
import serial
#import BinAscii
import socket
import time
loca_X=0
loca_Y=0
def tcplink():
    global loca_X,loca_Y
    print ("in tcp")
    s = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
    s.bind(('127.0.0.1',7000))
    s.listen(5)
    sock,addr =s.accept()
    print 'Accept new connection from %s:%s...' % addr  
    while True:
        data=sock.recv(10)
        time.sleep(0.1)
        loca=[ord(c) for c in data]
        print(len(loca))
        if loca[0]!=ord('\n'):
            loca_X=loca[0]+loca[1]*256 if loca[3]<128 else (loca[0]-255+(loca[1]-255)*256)
            loca_Y=loca[4]+loca[5]*256 if loca[7]<128 else (loca[4]-255+(loca[5]-255)*256)
            print('loca_X=%d' %loca_X)
            print('loca_Y=%d' %loca_Y)

def uartlink():
    print("in serial")
    sum_x=0
    sum_y=0
#    trans_beg=0xfe
    ser = serial.Serial("/dev/ttyAMA0",115200)
    trans_beg=0xfe
#    angle_b=4.1783
#    ser.write(chr(trans_beg))
#    ser.write(chr((int)(angle_b*100)%256))
#    ser.write(chr((int)(angle_b*100)/256))
#    print("after send")
    while True:
        count =ser.inWaiting()
        if count!=0:
            print(count)
            recv = ser.read(count)
            recv_d=[ord(c) for c in recv]
#            print(recv_d)
#            print(recv_d[0])
#            print(recv_d[count-1])
#            print('%x' %recv_d[0])
            if (recv_d[0]==239) and (recv_d[count-1]==254):
                sum_x=0
                sum_y=0
                obsta = [0]*(count/2-1)
                rep_x=[0]*(count/2-1)
                rep_y=[0]*(count/2-1)
                print('------')
                for index in range(count/2-1):
                    obsta[index]=recv_d[index*2+1]+recv_d[index*2+2]*256
                    print('%d ' % obsta[index])
                    obsta_f=(float)(obsta[index])
                    rep_x[index]=0 if obsta[index]>300 or obsta[index]<100 else -100000*((1/obsta_f-1/300.0)*1/(obsta_f*obsta_f))*math.sin(0.174533*index)
                    rep_y[index]=0 if obsta[index]>300 or obsta[index]<100 else 100000*((1/obsta_f-1/300.0)*1/(obsta_f*obsta_f))*math.cos(0.174533*index)
                    sum_x-=rep_x[index]
                    sum_y-=rep_y[index]
#                att=[loca_X,loca_Y]
                sum_x-=loca_X
                sum_y-=loca_Y

                print('sum_X %f' %sum_x)
                print('sum_Y %f' %sum_y)
#                angle_b=math.atan2(sum_y,sum_x)+3.14159+1.5708 if sum_x*sum_y>0 else math.atan2(sum_y,sum_x)+1.5708
                angle_b=math.atan2(sum_y,sum_x)+3.14159+1.5708 if sum_x*sum_y>0 or sum_x>0 else math.atan2(sum_y,sum_x)-3.14159
                print('%f' % angle_b)
                ser.write(chr(trans_beg))
                ser.write(chr((int)(angle_b*100)%256))
                ser.write(chr((int)(angle_b*100)/256))
        ser.flushInput()
        time.sleep(0.1)
def main():
    t1 = threading.Thread(target=tcplink)
    t2 = threading.Thread(target=uartlink)
#    t1.setDaemon(True)
    t1.start()
#    t2.setDaemon(True)
    t2.start()

if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        if ser != None:
            ser.close()
