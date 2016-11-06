##########################################
# ESP8266 IM MESSAGE DISPLAY
#
# SUPPORTING PYTHON SCRIPT
# CALLED WITH DATA FROM PIDGIN
#
# ANKIT BHATNAGAR
# ANKIT.BHATNAGARINDIA@GMAIL.COM
# NOVEMBER 3 2016
##########################################

#!/usr/bin/python

import sys
import getopt
import socket

def main(argv):
	target_ip = ''
	target_port = ''
	message_sender = ''
	
	try:
		opts, args = getopt.getopt(argv, "hi:p:s", ["ip=", "port=", "sender="])
	except getopt.GetoptError:
                print 'test.py -i <target ip> -p <port> -s <sender name>'
                sys.exit(2)
        for opt, arg in opts:
                if opt == '-h':
			print 'test.py -i <target ip> -p <port> -s <sender name>'
			sys.exit()
		elif opt in ("-i", "--ip"):
			target_ip = arg
		elif opt in ("-p", "--port"):
			target_port = arg
		elif opt in ("-s", "--sender"):
			message_sender = arg
        print 'ip = ', target_ip
        print 'port = ', target_port
        print 'sender = ', message_sender
        send_udp_to_hardware(target_ip, target_port, message_sender)
        

#function to take in ip address, sender name and
#message and to send the and sender name
#to the specified IP address as UDP packet
def send_udp_to_hardware(ip, port, sender):
	IPADDRESS = ip
	PORTNUM = int(port)
	
	packet_data = chr(len(sender))
	packet_data += sender
	
	print packet_data
	
	s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, 0)
	try:
		s.connect((IPADDRESS, PORTNUM))
		s.send(packet_data)
	except:
		print 'socket exception. data not sent !'
	print 'data sent :)'
	s.close()
	sys.exit()
	return
                 
if __name__ == "__main__":
	main(sys.argv[1:])