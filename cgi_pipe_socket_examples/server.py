import os
import socket
import select
import time

# Create a socket pair
parent_sock, child_sock = socket.socketpair()

# Get the actual send and receive buffer sizes
rcvbuf = parent_sock.getsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF)
sndbuf = parent_sock.getsockopt(socket.SOL_SOCKET, socket.SO_SNDBUF)
print("Receive buffer size:", rcvbuf)
print("Send buffer size:", sndbuf)

# Fork a child process
pid = os.fork()

if pid:
	# This is the parent process
	child_sock.close()
	bytes_received = 0
	while True:
		# time.sleep(0.001)  # Simulate some processing time
		if parent_sock.fileno() == -1:
			print("Socket is closed")
			break
		# Use select to wait for data to be available on the socket
		ready_to_read, _, _ = select.select([parent_sock], [], [])
		for sock in ready_to_read:
			data = sock.recv(212992)  # Read data in chunks of 64 bytes
			bytes_received += len(data)
			if data:
				# print("Received:", data)
				pass
			else:
				print("Connection closed")
				parent_sock.close()
				print("Total bytes received:", bytes_received)
				break
else:
	# This is the child process
	bytes_sent = 0
	parent_sock.close()
	message = b"This is a message from the child process. " * 1024 * 1024 # A lot of data
	while message:
		# Use select to wait until the socket is writable
		_, ready_to_write, _ = select.select([], [child_sock], [])
		for sock in ready_to_write:
			chunk = message[:212992]  # Only take the first 64 bytes of data
			sent = sock.send(chunk)  # Send the chunk of data
			bytes_sent += sent
			print("Sent:", bytes_sent)
			message = message[sent:]  # Remove the sent data from the message
			if not message:
				print("Total bytes sent:", bytes_sent)
	child_sock.close()