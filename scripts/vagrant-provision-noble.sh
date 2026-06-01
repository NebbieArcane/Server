#!/bin/bash
# Provisioning Vagrant nebbieserver — allineato a Dockerfile (Ubuntu 24.04 Noble).
# Eseguito come root da Vagrantfile. Idempotente: sicuro con `vagrant provision`.
set -euo pipefail
exec > >(tee -a /var/log/nebbie-vagrant-provision.log) 2>&1

export DEBIAN_FRONTEND=noninteractive

MARKER_DIR=/var/lib/nebbie
ODB_MARKER="${MARKER_DIR}/odb-toolchain-installed"
ODB_BUILD=/var/cache/nebbie-odb-build
APT_MARKER="${MARKER_DIR}/apt-base-installed"

mkdir -p "$MARKER_DIR"

# VirtualBox su Mac: spesso il guest non risolve DNS finché non si configura resolved.
ensure_guest_dns() {
	if getent hosts pkg.cppget.org >/dev/null 2>&1 \
		&& getent hosts archive.ubuntu.com >/dev/null 2>&1; then
		echo "==> DNS guest OK"
		return 0
	fi
	echo "==> Riparo DNS guest (8.8.8.8 / 1.1.1.1)"
	install -d -m 0755 /etc/systemd/resolved.conf.d
	cat >/etc/systemd/resolved.conf.d/10-nebbie-dns.conf <<'EOF'
[Resolve]
DNS=8.8.8.8 1.1.1.1
FallbackDNS=8.8.4.4
EOF
	systemctl restart systemd-resolved 2>/dev/null || true
	sleep 2
	if getent hosts pkg.cppget.org >/dev/null 2>&1; then
		echo "==> DNS guest OK dopo fix"
		return 0
	fi
	echo "ERRORE: la VM non risolve nomi DNS. Esegui: vagrant halt && vagrant up && vagrant provision"
	return 1
}

odb_already_installed() {
	command -v odb >/dev/null 2>&1 \
		&& ldconfig -p 2>/dev/null | grep -q 'libodb-mysql.so' \
		&& [ -f /etc/ld.so.conf.d/odb-local.conf ]
}

install_odb_toolchain() {
	echo "==> ODB 2.5 (build2 + bpkg, script condiviso con Docker)"
	SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
	export ODB_BUILD

	# Swap temporaneo: build ODB in VM può richiedere molta RAM.
	if ! swapon --show | grep -q '/swapfile'; then
		echo "==> Swap temporaneo 8G per build ODB"
		fallocate -l 8G /swapfile 2>/dev/null || dd if=/dev/zero of=/swapfile bs=1M count=8192 status=progress
		chmod 600 /swapfile
		mkswap /swapfile
		swapon /swapfile
		SWAP_CREATED=1
	fi

	bash "${SCRIPT_DIR}/install-odb-toolchain.sh"
	touch "$ODB_MARKER"

	if [ "${SWAP_CREATED:-0}" = "1" ]; then
		swapoff /swapfile || true
		rm -f /swapfile
	fi
}

ensure_guest_dns

echo "==> apt update"
apt-get -qq update || true

if [ ! -f "$APT_MARKER" ]; then
	echo "==> MySQL debconf (password secret)"
	echo "mysql-server mysql-server/root_password password secret" | debconf-set-selections
	echo "mysql-server mysql-server/root_password_again password secret" | debconf-set-selections

	echo "==> Dipendenze (stesso set del Dockerfile)"
	apt-get -qq install -y \
		git php8.3-cli gcc-12 g++-12 gcc-12-plugin-dev apache2 make cmake \
		libconfig++-dev lnav libsqlite3-dev libcurlpp-dev gdb wget \
		libcurl4-openssl-dev \
		libboost-dev libboost-program-options-dev libboost-system-dev \
		libboost-filesystem-dev liblog4cxx-dev libboost-date-time-dev \
		librtmp-dev libnghttp2-dev libkrb5-dev comerr-dev libpsl-dev libssh-dev libbrotli-dev \
		mysql-server mysql-client libmysqlclient-dev libmysqlcppconn-dev \
		net-tools iproute2 vim less dos2unix build-essential
	touch "$APT_MARKER"
else
	echo "==> Dipendenze già installate, skip apt (usa rm ${APT_MARKER} per reinstallare)"
fi

if [ -f "$ODB_MARKER" ] && odb_already_installed; then
	echo "==> ODB già installato, skip build ODB (usa rm ${ODB_MARKER} per forzare rebuild)"
else
	install_odb_toolchain
fi

update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-12 90 2>/dev/null || true
update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-12 90 2>/dev/null || true

echo "==> Git config utente vagrant"
sudo -iu vagrant git config --global user.email "nebbie@hexkeep.com"
sudo -iu vagrant git config --global user.name "Nebbie Server"

echo "==> Confs/vagrant.conf (come Docker)"
install -d -o vagrant -g vagrant /home/vagrant/Confs
cat >/home/vagrant/Confs/vagrant.conf <<'EOF'
MYSQL_USER="root" #db user
MYSQL_PASSWORD="secret" # db password
MYSQL_HOST="localhost" #db host
MYSQL_DB="nebbie" #db name
SERVER_PORT=4000 #default server port
EOF
chown vagrant:vagrant /home/vagrant/Confs/vagrant.conf

echo "==> MySQL: avvio servizio e database nebbie"
systemctl enable mysql
systemctl start mysql

for i in $(seq 1 30); do
	if mysqladmin ping -h 127.0.0.1 -uroot -psecret --silent 2>/dev/null; then
		break
	fi
	if mysqladmin ping -h 127.0.0.1 -uroot --silent 2>/dev/null; then
		break
	fi
	sleep 1
done

mysql -h 127.0.0.1 -uroot -psecret -e "CREATE DATABASE IF NOT EXISTS nebbie;" 2>/dev/null \
	|| mysql -h 127.0.0.1 -uroot -e "CREATE DATABASE IF NOT EXISTS nebbie;"

MIGRATION_FLAGS=/vagrant/docs/schema-s1-toon-migration-flags.sql
if [ -f "$MIGRATION_FLAGS" ]; then
	HAS_TOON=$(mysql -h 127.0.0.1 -uroot -psecret -N -e \
		"SELECT COUNT(*) FROM information_schema.TABLES WHERE TABLE_SCHEMA='nebbie' AND TABLE_NAME='toon';" 2>/dev/null || echo "0")
	if [ "$HAS_TOON" = "0" ]; then
		echo "==> Skip DDL migrated_at: DB nebbie senza tabella toon (importa dump o avvia myst per schema ODB)"
	else
		HAS_COL=$(mysql -h 127.0.0.1 -uroot -psecret -N -e \
			"SELECT COUNT(*) FROM information_schema.COLUMNS WHERE TABLE_SCHEMA='nebbie' AND TABLE_NAME='toon' AND COLUMN_NAME='migrated_at';" 2>/dev/null || echo "0")
		if [ "$HAS_COL" = "0" ]; then
			echo "==> Applico DDL cutover toon (migrated_at)"
			mysql -h 127.0.0.1 -uroot -psecret nebbie <"$MIGRATION_FLAGS" \
				|| mysql -h 127.0.0.1 -uroot nebbie <"$MIGRATION_FLAGS"
		fi
	fi
fi

echo "==> Build myst (/vagrant/build.sh vagrant)"
sudo -iu vagrant /vagrant/build.sh vagrant

echo "==> Provisioning Noble completato"
