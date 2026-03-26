FROM gcc:latest

# Install tools CLion needs to talk to the container
RUN apt-get update && apt-get install -y \
    cmake \
    gdb \
    valgrind \
    && rm -rf /var/lib/apt/lists/*