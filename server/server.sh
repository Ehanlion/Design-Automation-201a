#!/bin/bash

# Server management script for ee201a directory
# Usage: ./server.sh [start|stop|restart|status|url]

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
EE201A_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
SERVER_DIR="$EE201A_DIR/server"
SERVER_PY="$SERVER_DIR/server.py"
PID_FILE="$SERVER_DIR/server.pid"
LOG_FILE="$SERVER_DIR/server.log"
PORT=8000

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Function to check if server is running
is_server_running() {
	if [ -f "$PID_FILE" ]; then
		PID=$(cat "$PID_FILE")
		if ps -p "$PID" > /dev/null 2>&1; then
			return 0
		else
			# PID file exists but process is dead
			rm -f "$PID_FILE"
			return 1
		fi
	else
		return 1
	fi
}

# Function to get the server URL
get_url() {
	echo "http://localhost:$PORT/"
}

# Function to kill any process using the port
kill_port() {
	local port=$1
	local pids=""
	
	# Try to find PIDs using the port
	if command -v lsof >/dev/null 2>&1; then
		pids=$(lsof -ti :$port 2>/dev/null)
	elif command -v fuser >/dev/null 2>&1; then
		pids=$(fuser $port/tcp 2>/dev/null | awk '{print $1}')
	fi
	
	if [ -n "$pids" ]; then
		echo -e "${YELLOW}Found process(es) using port $port: $pids${NC}"
		for pid in $pids; do
			echo -e "${YELLOW}Killing process $pid...${NC}"
			kill "$pid" 2>/dev/null
			sleep 1
			if ps -p "$pid" > /dev/null 2>&1; then
				echo -e "${YELLOW}Force killing process $pid...${NC}"
				kill -9 "$pid" 2>/dev/null
			fi
		done
		sleep 1
		return 0
	else
		echo -e "${YELLOW}No process found using port $port${NC}"
		return 1
	fi
}

# Function to start the server
start_server() {
	if is_server_running; then
		echo -e "${YELLOW}Server is already running (PID: $(cat $PID_FILE))${NC}"
		echo "Use './server.sh restart' to restart it"
		return 1
	fi
	
	if [ ! -d "$SERVER_DIR" ]; then
		echo -e "${RED}Error: server directory not found at $SERVER_DIR${NC}"
		return 1
	fi
	
	if [ ! -f "$SERVER_PY" ]; then
		echo -e "${RED}Error: server.py not found at $SERVER_PY${NC}"
		return 1
	fi
	
	# Check if port is already in use and kill it if needed
	if command -v lsof >/dev/null 2>&1; then
		if lsof -Pi :$PORT -sTCP:LISTEN -t >/dev/null 2>&1 ; then
			echo -e "${YELLOW}Port $PORT is already in use${NC}"
			if kill_port $PORT; then
				echo -e "${GREEN}Port $PORT is now free${NC}"
			else
				echo -e "${RED}Failed to free port $PORT${NC}"
				echo "  You may need to manually stop the process using:"
				echo "  lsof -ti :$PORT | xargs kill -9"
				return 1
			fi
		fi
	fi
	
	echo -e "${GREEN}Starting HTTP server...${NC}"
	cd "$SERVER_DIR" || {
		echo -e "${RED}Error: Cannot change to directory $SERVER_DIR${NC}"
		return 1
	}
	
	# Start server in background and redirect output to log file
	python3 "$SERVER_PY" > "$LOG_FILE" 2>&1 &
	SERVER_PID=$!
	echo $SERVER_PID > "$PID_FILE"
	
	# Wait a bit longer for server to initialize
	sleep 2
	
	# Check if process is still running
	# Optionally verify port is listening if lsof is available
	PORT_OK=true
	if command -v lsof >/dev/null 2>&1; then
		if ! lsof -Pi :$PORT -sTCP:LISTEN -t >/dev/null 2>&1; then
			PORT_OK=false
		fi
	fi
	
	if is_server_running; then
		if [ "$PORT_OK" = false ]; then
			echo -e "${YELLOW}Warning: Server process is running but port $PORT may not be listening${NC}"
		fi
		echo -e "${GREEN}Server started successfully!${NC}"
		echo "  PID: $SERVER_PID"
		echo "  URL: $(get_url)"
		echo "  Log: $LOG_FILE"
		return 0
	else
		echo -e "${RED}Failed to start server${NC}"
		echo -e "${YELLOW}Check log file: $LOG_FILE${NC}"
		if [ -f "$LOG_FILE" ]; then
			echo -e "${YELLOW}Last few lines of log:${NC}"
			tail -5 "$LOG_FILE" | sed 's/^/  /'
		fi
		rm -f "$PID_FILE"
		return 1
	fi
}

# Function to stop the server
stop_server() {
	if ! is_server_running; then
		echo -e "${YELLOW}Server is not running${NC}"
		return 1
	fi
	
	PID=$(cat "$PID_FILE")
	echo -e "${YELLOW}Stopping server (PID: $PID)...${NC}"
	kill "$PID" 2>/dev/null
	
	# Wait a bit for graceful shutdown
	sleep 1
	
	if is_server_running; then
		echo -e "${RED}Server did not stop, forcing kill...${NC}"
		kill -9 "$PID" 2>/dev/null
		sleep 1
	fi
	
	if ! is_server_running; then
		rm -f "$PID_FILE"
		echo -e "${GREEN}Server stopped${NC}"
		return 0
	else
		echo -e "${RED}Failed to stop server${NC}"
		return 1
	fi
}

# Function to restart the server
restart_server() {
	echo -e "${YELLOW}Restarting server...${NC}"
	stop_server
	sleep 1
	start_server
}

# Function to show server status
show_status() {
	if is_server_running; then
		PID=$(cat "$PID_FILE")
		echo -e "${GREEN}Server is running${NC}"
		echo "  PID: $PID"
		echo "  URL: $(get_url)"
		echo "  Log: $LOG_FILE"
		return 0
	else
		echo -e "${YELLOW}Server is not running${NC}"
		return 1
	fi
}

# Main command handling
case "${1:-}" in
	start)
		start_server
		;;
	stop)
		stop_server
		;;
	restart|refresh)
		restart_server
		;;
	status)
		show_status
		;;
	url)
		if is_server_running; then
			get_url
			exit 0
		else
			echo -e "${YELLOW}Server is not running${NC}"
			echo "Start the server with: $0 start"
			exit 1
		fi
		;;
	kill-port)
		if kill_port $PORT; then
			echo -e "${GREEN}Port $PORT has been freed${NC}"
			exit 0
		else
			echo -e "${YELLOW}No process found using port $PORT${NC}"
			exit 1
		fi
		;;
	*)
		echo "Usage: $0 {start|stop|restart|status|url|kill-port}"
		echo ""
		echo "Commands:"
		echo "  start     - Start the HTTP server (automatically kills processes on port if needed)"
		echo "  stop      - Stop the HTTP server"
		echo "  restart   - Restart the HTTP server (same as refresh)"
		echo "  refresh   - Restart the HTTP server (same as restart)"
		echo "  status    - Show server status"
		echo "  url       - Print the server URL (only if server is running)"
		echo "  kill-port - Kill any process using port $PORT"
		exit 1
		;;
esac

exit $?
