# -*- mode: ruby -*-
# vi: set ft=ruby :

# All Vagrant configuration is done below. The "2" in Vagrant.configure
# configures the configuration version (we support older styles for
# backwards compatibility). Please don't change it unless you know what
# you're doing.
Vagrant.configure("2") do |config|
 	if Vagrant.has_plugin?("vagrant-timezone")
    	config.timezone.value =:host
    end
  config.vbguest.auto_update = false
  # The most common configuration options are documented and commented below.
  # For a complete reference, please see the online documentation at
  # https://docs.vagrantup.com.

  # VM principale del progetto (allineata al naming Docker: nebbieserver).
  # Jammy (22.04) per allineamento toolchain C++17.
  config.vm.box = "ubuntu/jammy64"
  config.vm.define  "nebbieserver"
  config.vm.hostname = "nebbieserver"

  # Disable automatic box update checking. If you disable this, then
  # boxes will only be checked for updates when the user runs
  # `vagrant box outdated`. This is not recommended.
  # config.vm.box_check_update = false

  # Create a forwarded port mapping which allows access to a specific port
  # within the machine from a port on the host machine. In the example below,
  # accessing "localhost:8080" will access port 80 on the guest machine.
  # NOTE: This will enable public access to the opened port
  # config.vm.network "forwarded_port", guest: 80, host: 8080

  # Create a forwarded port mapping which allows access to a specific port
  # within the machine from a port on the host machine and only allow access
  # via 127.0.0.1 to disable public access
  # config.vm.network "forwarded_port", guest: 80, host: 8080, host_ip: "127.0.0.1"
  config.vm.network "forwarded_port", guest: 4000, host: 4000, host_ip: "127.0.0.1"
  config.vm.network "forwarded_port", guest: 4001, host: 4001, host_ip: "127.0.0.1"
  config.vm.network "forwarded_port", guest: 4002, host: 4002, host_ip: "127.0.0.1"
  config.vm.network "forwarded_port", guest: 10000, host: 10001, host_ip: "127.0.0.1"

  # Create a private network, which allows host-only access to the machine
  # using a specific IP.
  # config.vm.network "private_network", ip: "192.168.33.10"

  # Create a public network, which generally matched to bridged network.
  # Bridged networks make the machine appear as another physical device on
  # your network.
  # config.vm.network "public_network"
  config.vm.network "public_network"

  # Share an additional folder to the guest VM. The first argument is
  # the path on the host to the actual folder. The second argument is
  # the path on the guest to mount the folder. And the optional third
  # argument is a set of non-required options.
  # config.vm.synced_folder "../data", "/vagrant_data"

  # Provider-specific configuration so you can fine-tune various
  # backing providers for Vagrant. These expose provider-specific options.
  # Example for VirtualBox:
  #
  config.vm.provider "virtualbox" do |vb|
  #   # Display the VirtualBox GUI when booting the machine
  #   vb.gui = true
  #
  # defaulting to a sane nic
    vb.default_nic_type = "82543GC"
  #   # Customize the amount of memory on the VM:
     vb.memory = 4096
  	  vb.cpus = 6
  	  vb.name = "nebbieserver"
	  vb.linked_clone = true if Gem::Version.new(Vagrant::VERSION) >= Gem::Version.new('1.8.0')
  end
  #
  # View the documentation for the provider you are using for more
  # information on available options.

  # Provisioning shell: installa toolchain + dipendenze e poi lancia build.sh.
  # config.vm.provision "shell", inline: <<-SHELL
  #   apt-get update
  #   apt-get install -y apache2
  # SHELL
  config.vm.provision "shell", inline: <<-SHELL
  set -e
  	echo "Updating apt cache"
    sudo apt-get -qq  update
    echo "Installing dev tools"
	apt-get -qq install git php8.1-cli g++ apache2 make cmake libconfig++-dev lnav libsqlite3-dev libcurlpp-dev gdb libcurl4-openssl-dev wget
	apt-get -qq install libboost-dev libboost-program-options-dev libboost-system-dev libboost-filesystem-dev liblog4cxx-dev
	apt-get -qq install libboost-date-time-dev gcc-12 g++-12 gcc-12-plugin-dev
  apt-get -qq install build-essential
  apt-get -qq  install librtmp-dev  libnghttp2-dev libkrb5-dev comerr-dev libpsl-dev libssh-dev libbrotli-dev
	echo "Installing mysql related packages"
	echo "mysql-server mysql-server/root_password password secret" | debconf-set-selections
	echo "mysql-server mysql-server/root_password_again password secret" | debconf-set-selections
	apt-get install -qq mysql-server mysql-client libmysqlclient-dev libmysqlcppconn-dev
  echo "Installing ODB 2.5 toolchain (build2 + bpkg)"
  BUILD2_DEB_BASE="https://download.build2.org/0.18.1/bindist/ubuntu/ubuntu22.04/x86_64"
  mkdir -p /tmp/build2-debs
  cd /tmp/build2-debs
  wget -q "${BUILD2_DEB_BASE}/build2-toolchain_0.18.1-0~ubuntu22.04_amd64.deb"
  apt-get -qq install -y ./build2-toolchain_0.18.1-0~ubuntu22.04_amd64.deb
  BPKG_STABLE_REPO="https://pkg.cppget.org/1/stable"
  mkdir -p /tmp/odb-build
  echo "Creating temporary swap for ODB build..."
  fallocate -l 4G /swapfile || dd if=/dev/zero of=/swapfile bs=1M count=4096
  chmod 600 /swapfile
  mkswap /swapfile
  swapon /swapfile
  bpkg create -d /tmp/odb-build/odb-gcc-12 cc \
    config.cxx=g++-12 \
    config.cc.coptions=-O2 \
    config.cc.jobs=1 \
    config.bin.rpath=/usr/local/lib \
    config.install.root=/usr/local
  printf 'y\n' | bpkg build --jobs 1 --trust-yes -d /tmp/odb-build/odb-gcc-12 "odb@${BPKG_STABLE_REPO}"
  bpkg install -d /tmp/odb-build/odb-gcc-12 odb
  bpkg create -d /tmp/odb-build/libodb-gcc-12 cc \
    config.cxx=g++-12 \
    config.cc.coptions=-O2 \
    config.cc.jobs=1 \
    config.install.root=/usr/local
  bpkg add --trust-yes -d /tmp/odb-build/libodb-gcc-12 "${BPKG_STABLE_REPO}"
  bpkg fetch -d /tmp/odb-build/libodb-gcc-12
  printf 'y\n' | bpkg build --jobs 1 --trust-yes -d /tmp/odb-build/libodb-gcc-12 libodb
  printf 'y\n' | bpkg build --jobs 1 --trust-yes -d /tmp/odb-build/libodb-gcc-12 "libodb-sqlite ?sys:libsqlite3"
  printf 'y\n' | bpkg build --jobs 1 --trust-yes -d /tmp/odb-build/libodb-gcc-12 "libodb-mysql ?sys:libmysqlclient"
  printf 'y\n' | bpkg build --jobs 1 --trust-yes -d /tmp/odb-build/libodb-gcc-12 libodb-boost
  bpkg install -d /tmp/odb-build/libodb-gcc-12 --all --recursive
  echo "/usr/local/lib" >/etc/ld.so.conf.d/odb-local.conf
  ldconfig
  cd /
  swapoff /swapfile || true
  rm -f /swapfile
  rm -rf /tmp/build2-debs /tmp/odb-build
  update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-12 120
  update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-12 120
	echo "Configuring git"
	git config --global user.email "nebbie@hexkeep.com"
 	git config --global user.name "Nebbie Server"
	echo "Mkdir"
	cd ~vagrant
 	mkdir -p Confs
 	echo 'MYSQL_USER="root" #db user' >Confs/vagrant.conf
 	echo 'MYSQL_PASSWORD="secret" # db password' >>Confs/vagrant.conf
 	echo 'MYSQL_HOST="localhost" #db host' >>Confs/vagrant.conf
 	echo 'MYSQL_DB="nebbie" #db name' >>Confs/vagrant.conf
 	echo 'SERVER_PORT=4000 #default server port' >>Confs/vagrant.conf
 	chown -R vagrant. Confs
 	echo "Building"
 	sudo -iu vagrant /vagrant/build.sh vagrant
 	exit 0
  SHELL
  config.ssh.forward_x11 = true
  config.ssh.forward_agent = true
end
