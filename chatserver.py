# Author: Daniel Yopp
# Date: 7/25/19
# Assignment: CS372 - ChatServer Project
# Description: A TCP server to connect to the chat client


from socket import *
import sys
import time


################################################
## Name: Tail functions
## Description: Adds, checks, or removes, a @@@ tail from a string object
################################################
def AddTail(buff):
	tempbuff = "@@@"
	buff+=tempbuff
	return buff

def CheckTail(buff):
	return buff.endswith("@@@")

def RemoveTail(buff):
	buff = buff[:-3]
	return buff


################################################
## Name: Meet Function
## Description: receive client username and send server username
################################################
def ServerClientMeet(connectionSocket, serverName):
	clientName = connectionSocket.recv(1024)
	connectionSocket.send(serverName)
	return clientName


################################################
## Name: Client Chat
## Description: Receives a message from the client the sends a message to the client in loop
################################################
def chatService(connectionSocket, clientName, serverName):
	sendingMSG = ""
	incomingMSG = ""
	while 1:    #send receive loop
	#client message
		incomingMSG = ""
		print "\n***Waiting for Client Message***"
		while not CheckTail(incomingMSG): #loop to receive all of client message until ending tail is detected
			incomingMSG = connectionSocket.recv(501)
			time.sleep(.1)
		#check if client requests quit
		if incomingMSG == "\\quit\n@@@":
			print "\n---Client Termination Request\n   Goodbye :)\n"
			sys.stdout.flush()
			break
		#print client message
		incomingMSG = RemoveTail(incomingMSG)
		print "\n", clientName,"> ", incomingMSG 
		sys.stdout.flush()

	#sending message to client
		sendingMSG = ""
		print serverName,
		sendingMSG = raw_input("> ")
		if sendingMSG == "\quit":  #if server requests quit
			print "---Server Termination Request\n"
			sys.stdout.flush()
			sendingMSG = AddTail(sendingMSG)
			sendingMSG = sendingMSG.encode('utf-8')
			connectionSocket.send(sendingMSG)
			break
		#regular server transmission
		sendingMSG = AddTail(sendingMSG)
		sendingMSG = sendingMSG.encode('utf-8')
		connectionSocket.send(sendingMSG)
	

#########################################################
## Main Function
#########################################################
if __name__ == "__main__":
	#check argument for port number to start on
	if len(sys.argv) != 2:
		print "ERROR: No port number argument included from command line"
		exit(1)
	elif (int(sys.argv[1]) > 65535) or (int(sys.argv[1]) < 100):
		print "port: ",sys.argv[1]
		print "ERROR: please select a port number between 100 and 65535"
		exit(2)
	#prompt for 10 char server name
	serverName = ""
	inputVer = 1
	while inputVer == 1:
		serverName = raw_input("Enter a Server chat name (max 10 char): ")
		if len(serverName) == 0 or len(serverName) > 10:
			print "ERROR: Servername must be between 1 and 10 chars"
		else:
			inputVer = 0 
	serverPort = int(sys.argv[1]) #get server port from command line
	serverSocket = socket(AF_INET,SOCK_STREAM) #create TCP socket
	serverSocket.bind(('',serverPort)) #bind the port to the socket 
	serverSocket.listen(1) #begin listening for connections
	print "Server Socket Setup Complete\nServer Ready to receive chat requests"
	while 1:
		#create private socket for incoming connection
		connectionSocket, addr = serverSocket.accept()
		print "Incoming connection from ", addr
		#exchange chat names
		clientName = ""
		clientName = ServerClientMeet(connectionSocket, serverName)
		print "ClientID: ", clientName
		#begin chat service
		chatService(connectionSocket, clientName, serverName)
		#close socket
		connectionSocket.close()
		print "Connection Closed\n"

