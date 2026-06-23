# qdmq

Lock free single producer multi consumer message queue (`msgq`) & Shared-memory IPC library (`shmm`)

### Build

Download and build the package locally:
```sh
git clone https://github.com/quantumdiamonds/qdmq
cd qdmq
# Install the required dependencies and build tools
chmod +x install.sh
./install.sh
pip install -r requirements.txt
# Build
scons -Q
# Minimal Build (w/o tests)
scons -Q --minimal
```
