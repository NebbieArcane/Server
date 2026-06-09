#!/bin/bash
# Installa ODB 2.5 + libodb via build2/bpkg (bindist ubuntu24.04, gcc-12).
# Usato da Dockerfile e vagrant-provision-noble.sh.
set -euo pipefail

export DEBIAN_FRONTEND=noninteractive

ODB_BUILD="${ODB_BUILD:-/var/cache/nebbie-odb-build}"
BUILD2_DEB_BASE="${BUILD2_DEB_BASE:-https://download.build2.org/0.18.1/bindist/ubuntu/ubuntu24.04/x86_64}"
BPKG_STABLE_REPO="${BPKG_STABLE_REPO:-https://pkg.cppget.org/1/stable}"
BPKG_RETRY_MAX="${BPKG_RETRY_MAX:-6}"
BPKG_RETRY_SLEEP="${BPKG_RETRY_SLEEP:-15}"

ensure_dns() {
	if getent hosts pkg.cppget.org >/dev/null 2>&1 \
		&& getent hosts archive.ubuntu.com >/dev/null 2>&1; then
		return 0
	fi
	echo "==> Riparo DNS (resolver pubblici)"
	if command -v systemctl >/dev/null 2>&1 && [ -d /run/systemd/system ]; then
		install -d -m 0755 /etc/systemd/resolved.conf.d
		cat >/etc/systemd/resolved.conf.d/10-nebbie-dns.conf <<'EOF'
[Resolve]
DNS=8.8.8.8 1.1.1.1
FallbackDNS=8.8.4.4
EOF
		systemctl restart systemd-resolved 2>/dev/null || true
	else
		# Docker build: embedded DNS (127.0.0.11) a volte fallisce verso pkg.cppget.org
		if [ -f /etc/resolv.conf ] && ! grep -qE '^nameserver (8\.8\.8\.8|1\.1\.1\.1)' /etc/resolv.conf; then
			printf 'nameserver 8.8.8.8\nnameserver 1.1.1.1\n' >>/etc/resolv.conf
		fi
	fi
	sleep 2
	getent hosts pkg.cppget.org >/dev/null 2>&1
}

wait_for_cppget() {
	local url="${BPKG_STABLE_REPO}/repositories.manifest"
	local attempt=1
	while ! curl -fsSL --connect-timeout 20 --max-time 120 "$url" -o /dev/null; do
		if [ "$attempt" -ge "$BPKG_RETRY_MAX" ]; then
			echo "ERRORE: pkg.cppget.org non raggiungibile dopo ${BPKG_RETRY_MAX} tentativi ($url)"
			return 1
		fi
		echo "==> Attendo pkg.cppget.org (tentativo ${attempt}/${BPKG_RETRY_MAX})..."
		ensure_dns || true
		sleep "$BPKG_RETRY_SLEEP"
		attempt=$((attempt + 1))
	done
}

run_retry() {
	local attempt=1
	while ! "$@"; do
		if [ "$attempt" -ge "$BPKG_RETRY_MAX" ]; then
			echo "ERRORE: comando fallito dopo ${BPKG_RETRY_MAX} tentativi: $*"
			return 1
		fi
		echo "==> Retry ${attempt}/${BPKG_RETRY_MAX}: $*"
		ensure_dns || true
		wait_for_cppget || true
		sleep "$BPKG_RETRY_SLEEP"
		attempt=$((attempt + 1))
	done
}

install_build2_deb() {
	mkdir -p /tmp/build2-debs
	cd /tmp/build2-debs
	if [ ! -f build2-toolchain_0.18.1-0~ubuntu24.04_amd64.deb ]; then
		wget -q "${BUILD2_DEB_BASE}/build2-toolchain_0.18.1-0~ubuntu24.04_amd64.deb"
	fi
	if ! dpkg -s build2-toolchain >/dev/null 2>&1; then
		apt-get -qq install -y ./build2-toolchain_0.18.1-0~ubuntu24.04_amd64.deb
	fi
}

install_odb_packages() {
	ensure_dns
	wait_for_cppget

	mkdir -p "$ODB_BUILD"

	if [ ! -d "${ODB_BUILD}/odb-gcc-12" ]; then
		bpkg create -d "${ODB_BUILD}/odb-gcc-12" cc \
			config.cxx=g++-12 \
			config.cc.coptions=-O2 \
			config.bin.rpath=/usr/local/lib \
			config.install.root=/usr/local
	fi
	if ! command -v odb >/dev/null 2>&1; then
		run_retry bash -c "printf 'y\n' | bpkg build --trust-yes -d '${ODB_BUILD}/odb-gcc-12' 'odb@${BPKG_STABLE_REPO}'"
		bpkg install -d "${ODB_BUILD}/odb-gcc-12" odb
	fi

	if [ ! -d "${ODB_BUILD}/libodb-gcc-12" ]; then
		bpkg create -d "${ODB_BUILD}/libodb-gcc-12" cc \
			config.cxx=g++-12 \
			config.cc.coptions=-O2 \
			config.install.root=/usr/local
	fi
	bpkg add --trust-yes -d "${ODB_BUILD}/libodb-gcc-12" "${BPKG_STABLE_REPO}" 2>/dev/null || true
	run_retry bpkg rep-fetch -d "${ODB_BUILD}/libodb-gcc-12"
	run_retry bpkg fetch -d "${ODB_BUILD}/libodb-gcc-12"
	run_retry bash -c "printf 'y\n' | bpkg build --trust-yes -d '${ODB_BUILD}/libodb-gcc-12' libodb"
	run_retry bash -c "printf 'y\n' | bpkg build --trust-yes -d '${ODB_BUILD}/libodb-gcc-12' 'libodb-sqlite ?sys:libsqlite3'"
	run_retry bash -c "printf 'y\n' | bpkg build --trust-yes -d '${ODB_BUILD}/libodb-gcc-12' 'libodb-mysql ?sys:libmysqlclient'"
	run_retry bash -c "printf 'y\n' | bpkg build --trust-yes -d '${ODB_BUILD}/libodb-gcc-12' libodb-boost"
	bpkg install -d "${ODB_BUILD}/libodb-gcc-12" --all --recursive

	echo "/usr/local/lib" >/etc/ld.so.conf.d/odb-local.conf
	ldconfig
}

install_build2_deb
install_odb_packages
rm -rf /tmp/build2-debs
