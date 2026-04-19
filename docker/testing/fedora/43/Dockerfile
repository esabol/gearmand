ARG version=43
FROM fedora:${version}

ARG version
ARG GEARMAN_REPO=https://github.com/gearman/gearmand
ARG GEARMAN_REPO_BRANCH=master

LABEL description="Gearman Job Server Image (Fedora ${version})"
LABEL maintainer="Gearmand Developers ${GEARMAN_REPO}"
LABEL version="${GEARMAN_REPO}/tree/${GEARMAN_REPO_BRANCH} Fedora ${version}"

# Install packages
RUN dnf makecache \
 && dnf update -y \
 && dnf install -y \
	rpm-build \
	git \
	libtool \
	autoconf \
	automake \
	bison \
	make \
	gettext-devel \
	boost-devel \
	boost-thread \
	chrpath \
	gcc-c++ \
	gperf \
	gperftools-devel \
	hiredis-devel \
	libevent-devel \
	libmemcached-devel \
	libpq-devel \
	libuuid-devel \
	mariadb-connector-c-devel \
	memcached \
	openssl-devel \
	sqlite-devel \
	tokyocabinet-devel \
	zlib-devel \
	python3-sphinx \
 && dnf clean all \
 && rm -rf /var/cache/yum

# Add user and set up rpmbuild directory
RUN adduser -m --shell /bin/bash gearman \
 && mkdir -p /home/gearman/rpmbuild/SOURCES \
 && chown -R gearman:gearman /home/gearman/rpmbuild

# Switch to gearman user
USER gearman

# Clone the GitHub repository master branch
RUN cd /home/gearman && git clone --depth 1 --branch ${GEARMAN_REPO_BRANCH} ${GEARMAN_REPO}.git

# Bootstrap, configure, make, and make test
WORKDIR /home/gearman/gearmand
RUN ./bootstrap.sh -a
RUN ./configure --enable-ssl 2>&1 | tee ./configure.log
RUN make 2>&1 | tee ./build.log
RUN make test 2>&1 | tee ./test.log
RUN make dist 2>&1 | tee ./dist.log
RUN make dist-rpm 2>&1 | tee ./rpmbuild.log
