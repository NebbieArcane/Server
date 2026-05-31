# -*- mode: ruby -*-
# vi: set ft=ruby :

# VM nebbieserver — allineata a Docker (Ubuntu 24.04 Noble, GCC 12, ODB 2.5, MySQL 8).
# Canonical non pubblica più box Vagrant da 24.04: usiamo Bento (vanilla Noble).
# Toolchain e pacchetti: scripts/vagrant-provision-noble.sh (stesso set del Dockerfile).

Vagrant.configure("2") do |config|
	if Vagrant.has_plugin?("vagrant-timezone")
		config.timezone.value = :host
	end
	config.vbguest.auto_update = false

	config.vm.box = "bento/ubuntu-24.04"
	config.vm.define "nebbieserver"
	config.vm.hostname = "nebbieserver"

	config.vm.network "forwarded_port", guest: 4000, host: 4000, host_ip: "127.0.0.1"
	config.vm.network "forwarded_port", guest: 4001, host: 4001, host_ip: "127.0.0.1"
	config.vm.network "forwarded_port", guest: 4002, host: 4002, host_ip: "127.0.0.1"
	config.vm.network "forwarded_port", guest: 10000, host: 10001, host_ip: "127.0.0.1"

	config.vm.network "public_network"

	# Provision lungo (ODB): timeout boot/SSH generosi
	config.vm.boot_timeout = 900
	config.ssh.keep_alive = true

	config.vm.provider "virtualbox" do |vb|
		vb.default_nic_type = "82543GC"
		vb.memory = 6144
		vb.cpus = 6
		vb.name = "nebbieserver"
		vb.linked_clone = true if Gem::Version.new(Vagrant::VERSION) >= Gem::Version.new("1.8.0")
		# DNS guest: usa il resolver del Mac (evita "Temporary failure resolving")
		vb.customize ["modifyvm", :id, "--natdnsproxy1", "on"]
		vb.customize ["modifyvm", :id, "--natdnshostresolver1", "on"]
	end

	config.vm.provision "shell", path: "scripts/vagrant-provision-noble.sh"

	config.ssh.forward_x11 = true
	config.ssh.forward_agent = true
end
