#!/bin/bash

gradle_version="7.6.3"
fast_dds_gen_version="3.3.0"

FAST_DDS_WORK_DIR=./dds_build
WORKSPACE=$PWD

if ! dpkg -l | grep -qw openjdk-11-jre || ! dpkg -l | grep -qw openjdk-11-jdk; then
    sudo apt update
    sudo apt purge -y openjdk-* default-jdk default-jre --autoremove
    sudo apt install -y openjdk-11-jre openjdk-11-jdk
fi

sudo rm -rf ${FAST_DDS_WORK_DIR}

mkdir ${FAST_DDS_WORK_DIR}

cd ${FAST_DDS_WORK_DIR}
WORKSPACE=$PWD

sudo rm -rf /opt/gradle/
sudo mkdir /opt/gradle
cd /opt/gradle
sudo wget https://services.gradle.org/distributions/gradle-${gradle_version}-bin.zip
sudo unzip gradle-${gradle_version}-bin.zip
sudo rm -f gradle-${gradle_version}-bin.zip

sed -i -e '/export PATH=$PATH:\/opt\/gradle\/gradle-${gradle_version}\/bin/d' ~/.bashrc
echo 'export PATH=$PATH:/opt/gradle/gradle-${gradle_version}/bin' >> ~/.bashrc

export JAVA_HOME=$(readlink -f /usr/bin/java | sed "s:bin/java::")
sed -i -e '/export JAVA_HOME=/d' ~/.bashrc
echo 'export JAVA_HOME=$(readlink -f /usr/bin/java | sed "s:bin/java::")' >> ~/.bashrc

export PATH=$PATH:/opt/gradle/gradle-${gradle_version}/bin

cd ${WORKSPACE}
# Install Fast-DDS-Gen
sudo rm -rf /opt/fast-dds-gen
sudo mkdir -p /opt/fast-dds-gen
git clone --recursive -b v$fast_dds_gen_version https://github.com/eProsima/Fast-DDS-Gen.git fast-dds-gen \
    && cd fast-dds-gen \
    && gradle assemble \
    && sudo /opt/gradle/gradle-${gradle_version}/bin/gradle install --install_path=/opt/fast-dds-gen

sed -i -e '/export PATH=$PATH:\/opt\/fast-dds-gen\/bin/d' ~/.bashrc
echo 'export PATH=$PATH:/opt/fast-dds-gen/bin' >> ~/.bashrc

sudo ldconfig
