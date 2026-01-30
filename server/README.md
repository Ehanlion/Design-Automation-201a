# File Server

This directory contains a simple HTTP server to preview and browse all files in the ee201a directory.

## Files

- `server.py` - Python HTTP server script
- `server.sh` - Server management script (start/stop/restart)

## Usage

### Starting the Server

```bash
cd server
./server.sh start
```

The server will start on `http://localhost:8000`

### Viewing Files

Once the server is running, open your browser and navigate to:
- Root directory: `http://localhost:8000/`
- Browse through all lab directories and files
- View HTML plots: `http://localhost:8000/lab1/plotting/fanout_histogram.html`

**Note:** If you're SSH'd into a remote server, you'll need to set up port forwarding:
```bash
# In your local terminal:
ssh -L 8000:localhost:8000 palatics@eeapps02
```

### Server Commands

```bash
./server.sh start    # Start the server
./server.sh stop     # Stop the server
./server.sh restart  # Restart the server
./server.sh refresh  # Same as restart
./server.sh status   # Check if server is running
./server.sh url      # Print the server URL
```

## Server Details

- **Port:** 8000
- **Host:** 127.0.0.1 (localhost only)
- **Serves:** All files from the ee201a root directory
- **Log File:** `server.log`
- **PID File:** `server.pid`

The server automatically serves files from the ee201a directory and includes proper CORS headers for loading external resources (like Chart.js from CDN).
