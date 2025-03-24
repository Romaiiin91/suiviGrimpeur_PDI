# Installation of suiviGrimpeur

**WARNING: WIP on this file, need to be test following all the instruction a new clean device (especially for the permission)**

This guide describes the installation and configuration of the suiviGrimpeur software on a Raspberry Pi.

## Prerequisites

Before starting, make sure your system is up to date:

```sh
sudo apt update && sudo apt upgrade -y
```

Then install the necessary packages:

```sh
sudo apt-get install -y git build-essential apache2 libcurl4-openssl-dev libopencv-dev libjansson-dev autoconf automake cmake libass-dev libfdk-aac-dev libtool libv4l-dev libx264-dev nasm pkg-config texinfo yasm
```


## Compiling and Installing FFmpeg for a raspberry PI4

On a raspberry pi 4, we can use the materiel acceleration to encode video that's why we enable manually v4l2. For a x86 architecture you can use libx264 for encoding or a special lib depending of your graphic card.

```sh
cd ~
git clone https://github.com/FFmpeg/FFmpeg.git ffmpeg
cd ffmpeg
./configure --enable-gpl --enable-nonfree --enable-libx264 --enable-v4l2-m2m --enable-libv4l2 --disable-doc --enable-shared --disable-static
make -j$(nproc)
sudo make install
```
Warning: Can be very long, depend of your machine 

Add the library to the path:

```sh
echo "export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH" >> ~/.bashrc
source ~/.bashrc
```

Verify that FFmpeg is installed:

```sh
ffmpeg -codecs | grep v4l2
```

## Installing and Configuring suiviGrimpeur

Clone the repository and configure permissions:

```sh
cd ~
git clone https://github.com/Romaiiin91/suiviGrimpeur_PDI
cd suiviGrimpeur
```
Add the necessary permissions:

```sh
sudo groupadd suiviGrimpeur
sudo usermod -aG suiviGrimpeur www-data
sudo usermod -aG suiviGrimpeur $(USER)
sudo usermod -aG suiviGrimpeur root
```

### Apache Configuration

Add the site configuration:

```sh
udo apt-get install apache2
sudo ln -s $(pwd)/server/010-suiviGrimpeur.conf /etc/apache2/sites-available/
sudo a2enmod cgi
```

Modify the file [010-suiviGrimpeur.conf](server/010-suiviGrimpeur.conf) with the right directory for files.

```sh
sudo a2ensite 010-suiviGrimpeur.conf
```
Restart Apache:

```sh
sudo systemctl restart apache2
```

### Configure the program
Modify the file [chemin.h](include/chemin.h) to define the right path for file used or executed. 
Configure the IP for the cameras in file [camera.json](data/cameras.json)

### Build the program
If you want to build the project for encoding with x264, add ```X264=1```. If you want to build in debug mode add ```DEBUG=1``` in this command. If you're using this program on a computer with a desktop environment you can add ```PC=1``` to see detection in real time. 
```sh
make build -j$(nproc)
```


## Uninstalling suivi grimpeur:
```sh
make uninstall
```
