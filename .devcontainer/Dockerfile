FROM mcr.microsoft.com/devcontainers/universal

RUN apt-get update && \
	apt-get install -y qemu-system-aarch64 qemu-kvm make clang llvm lld && \
	apt-get clean && \
	rm -rf /var/lib/apt/lists/*
