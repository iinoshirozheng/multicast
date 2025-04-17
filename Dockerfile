FROM centos:7

# Install development tools and SCL repository
RUN yum -y update && \
    yum -y install centos-release-scl epel-release && \
    yum -y install devtoolset-11 git make lcov cppcheck && \
    yum clean all

# Set up working directory
WORKDIR /app

# Set gcc 11 as default permanently
RUN echo "source /opt/rh/devtoolset-11/enable" >> /etc/bashrc

# Copy project files
COPY . /app/

# Create an entrypoint script that enables GCC 11
RUN echo '#!/bin/bash' > /entrypoint.sh && \
    echo 'source /opt/rh/devtoolset-11/enable' >> /entrypoint.sh && \
    echo 'exec "$@"' >> /entrypoint.sh && \
    chmod +x /entrypoint.sh

# Set entrypoint
ENTRYPOINT ["/entrypoint.sh"]

# Default command
CMD ["/bin/bash"] 