# -*- mode: ruby -*-
# vi: set ft=ruby :

# All Vagrant configuration is done below. The "2" in Vagrant.configure
# configures the configuration version (we support older styles for
# backwards compatibility). Please don't change it unless you know what
# you're doing.
Vagrant.configure("2") do |config|
 	if Vagrant.has_plugin?("vagrant-timezone")
    	config.timezone.value =":host"
    end

  # The most common configuration options are documented and commented below.
  # For a complete reference, please see the online documentation at
  # https://docs.vagrantup.com.

  # Every Vagrant development environment requires a box. You can search for
  # boxes at https://vagrantcloud.com/search.
  config.vm.box = "ubuntu/xenial64"
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

  # Create a private network, which allows host-only access to the machine
  # using a specific IP.
  # config.vm.network "private_network", ip: "192.168.33.10"

  # Create a public network, which generally matched to bridged network.
  # Bridged networks make the machine appear as another physical device on
  # your network.
  # config.vm.network "public_network"

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
  #   # Customize the amount of memory on the VM:
  #   vb.memory = "1024"
  	  vb.name = "nebbieserver"
  end
  #
  # View the documentation for the provider you are using for more
  # information on available options.

  # Enable provisioning with a shell script. Additional provisioners such as
  # Puppet, Chef, Ansible, Salt, and Docker are also available. Please see the
  # documentation for more information about their specific syntax and use.
  # config.vm.provision "shell", inline: <<-SHELL
  #   apt-get update
  #   apt-get install -y apache2
  # SHELL
  config.vm.provision "shell", inline: <<-SHELL
  	echo "Updating apt cache"
    sudo apt-get -qq  update	
    echo "Installing dev tools"
	apt-get -qq install git php7.0-cli g++ apache2 make cmake libconfig++-dev lnav libsqlite3-dev libcurlpp-dev gdb libcurl4-openssl-dev
	apt-get -qq install libboost-dev libboost-program-options-dev libboost-system-dev libboost-filesystem-dev liblog4cxx-dev
	echo "Installing mysql related packages"
	echo "mysql-server mysql-server/root_password password secret" | debconf-set-selections
	echo "mysql-server mysql-server/root_password_again password secret" | debconf-set-selections	
	apt-get install -qq mysql-server mysql-client libmysqld-dev libmysqlcppconn-dev
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
