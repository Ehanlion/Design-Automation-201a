#!/usr/bin/env python3
"""
Simple HTTP server for previewing HTML plots
Serves files from the plotting directory on localhost:8000
"""

import http.server
import socketserver
import os
import sys

PORT = 8000
# Serve from parent directory (lab1) where HTML files are located
DIRECTORY = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

class MyHTTPRequestHandler(http.server.SimpleHTTPRequestHandler):
	def __init__(self, *args, **kwargs):
		# For Python 3.6 compatibility, we change directory before calling super()
		# The directory parameter was added in Python 3.7
		os.chdir(DIRECTORY)
		super().__init__(*args, **kwargs)
	
	def end_headers(self):
		# Add CORS headers to allow loading from CDN
		self.send_header('Access-Control-Allow-Origin', '*')
		self.send_header('Cache-Control', 'no-cache, no-store, must-revalidate')
		self.send_header('Pragma', 'no-cache')
		self.send_header('Expires', '0')
		super().end_headers()
	
	def log_message(self, format, *args):
		# Custom log format
		sys.stderr.write("%s - - [%s] %s\n" %
			(self.address_string(),
			self.log_date_time_string(),
			format%args))

def start_server():
	"""Start the HTTP server"""
	os.chdir(DIRECTORY)
	
	# Create a custom TCPServer class with allow_reuse_address set
	class ReusableTCPServer(socketserver.TCPServer):
		allow_reuse_address = True
	
	try:
		httpd = ReusableTCPServer(("127.0.0.1", PORT), MyHTTPRequestHandler)
		
		print(f"Server started at http://localhost:{PORT}", flush=True)
		print(f"Serving directory: {DIRECTORY}", flush=True)
		print(f"Open http://localhost:{PORT}/fanout_histogram.html in your browser", flush=True)
		print("Press Ctrl+C to stop the server", flush=True)
		
		try:
			httpd.serve_forever()
		except KeyboardInterrupt:
			print("\nServer stopped.", flush=True)
		finally:
			httpd.shutdown()
			httpd.server_close()
	except OSError as e:
		print(f"Error starting server: {e}", file=sys.stderr, flush=True)
		sys.exit(1)

if __name__ == "__main__":
	start_server()
