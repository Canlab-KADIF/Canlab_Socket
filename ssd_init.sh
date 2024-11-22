#!/bin/bash

# Installed Parted Checking
if which parted >/dev/null; then
	echo "parted cli already installed!!!"

	for i in a b
	do
	if [ -b /dev/sd$i  ]; then			# 장치가 있으면
		for j in 1
		do
		if [ -b /dev/sd$i$j  ]; then		# 파티션이 있으면
			echo "/dev/sd$i$j Partition exist"
			sudo mkdir -p /mnt/sd$i$j
		else
			# Disk Repartition
			sudo parted --script /dev/sd$i mklabel gpt mkpart primary ext4 1049kb 100%

			# Disk Format -- Force
			sudo mkfs.ext4 -F /dev/sd$i$j

			# Change Disk's Reserved Blocks Setting To 0
			sudo tune2fs -m 0 /dev/sd$i$j

			# Making Directories
			sudo mkdir -p /mnt/sd$i$j

			### Mounting Target_disk
			sudo mount  /dev/sd$i$j /mnt/sd$i$j
		fi
			sudo mount  /dev/sd$i$j /mnt/sd$i$j
		done
	else
		echo "/dev/sd$i No Init Device"
		exit 0
	fi
	done
else
	echo "install parted"
	apt -y install parted
fi
