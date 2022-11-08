HDF5 version 1.13.3-1 currently under development

![HDF5 Logo](doxygen/img/HDF5.png)

[![Build](https://img.shields.io/github/workflow/status/HDFGroup/hdf5/hdf5%20dev%20CI/develop)](https://github.com/HDFGroup/hdf5/actions?query=branch%3Adevelop)
[![BSD](https://img.shields.io/badge/License-BSD-blue.svg)](https://github.com/HDFGroup/hdf5/blob/develop/COPYING)

*Please refer to the release_docs/INSTALL file for installation instructions.*

This repository contains a high-performance library's source code and a file format
specification that implement the HDF5® data model. The model has been adopted across
many industries and this implementation has become a de facto data management standard
in science, engineering, and research communities worldwide.

The HDF Group is the developer, maintainer, and steward of HDF5 software. Find more
information about The HDF Group, the HDF5 Community, and other HDF5 software projects,
tools, and services at The HDF Group's website.
    
   https://www.hdfgroup.org/


DOCUMENTATION
-------------
This release is fully functional for the API described in the documentation.
    
   https://portal.hdfgroup.org/display/HDF5/The+HDF5+API

Full Documentation and Programming Resources for this release can be found at

   https://portal.hdfgroup.org/display/HDF5

See the RELEASE.txt file in the release_docs/ directory for information specific
to the features and updates included in this release of the library.

Several more files are located within the release_docs/ directory with specific
details for several common platforms and configurations.

    INSTALL - Start Here. General instructions for compiling and installing the library
    INSTALL_CMAKE  - instructions for building with CMake (Kitware.com)
    INSTALL_parallel - instructions for building and configuring Parallel HDF5
    INSTALL_Windows and INSTALL_Cygwin - MS Windows installations.



HELP AND SUPPORT
----------------
Information regarding Help Desk and Support services is available at

   https://portal.hdfgroup.org/display/support/The+HDF+Help+Desk



FORUM and NEWS
--------------
The following public forums are provided for public announcements and discussions
of interest to the general HDF5 Community.

   - Homepage of the Forum
   https://forum.hdfgroup.org

   - News and Announcement
   https://forum.hdfgroup.org/c/news-and-announcements-from-the-hdf-group

   - HDF5 and HDF4 Topics
   https://forum.hdfgroup.org/c/hdf5

These forums are provided as an open and public service for searching and reading.
Posting requires completing a simple registration and allows one to join in the
conversation.  Please read the following instructions pertaining to the Forum's
use and configuration
    https://forum.hdfgroup.org/t/quickstart-guide-welcome-to-the-new-hdf-forum


RELEASE SCHEDULE
----------------

![HDF5 release schedule](doc/img/release-schedule.png) 

HDF5 does not release on a regular schedule. Instead, releases are driven by
new features and bug fixes, though we try to have at least one release of each
maintenance branch per year. Future HDF5 releases indicated on this schedule
are tentative. 

| Release | New Features |
| ------- | ------------ |
| 1.13.2 | Onion VFD, Selection I/O, Subfiling |
| 1.13.3 | Multi-Dataset I/O |
| TBD | VFD SWMR |

This list of feature release versions is also tentative, and the specific release
in which a feature is introduced may change.


SNAPSHOTS, PREVIOUS RELEASES AND SOURCE CODE
--------------------------------------------
Periodically development code snapshots are provided at the following URL:
    
   https://gamma.hdfgroup.org/ftp/pub/outgoing/hdf5/snapshots/

Source packages for current and previous releases are located at:
    
   https://portal.hdfgroup.org/display/support/Downloads

Development code is available at our Github location:
    
   https://github.com/HDFGroup/hdf5.git

This is an example script for executing an automated regression test on the
Thetagpu system at the ALCF.  The entire process is handled by the
"./gpu-qsub" script.  The script can be copied to a desired location where
the test may be run and submitted via "qsub ./gpu-qsub" on a
thetagpu service node.


To do it manually:
- Login to theta, then to a thetagpu service node
- You can request an interactive session on a single-gpu via:
    * qsub -A *project name* -q single-gpu -n 1 -t *time* -I

- If the node you are on doesn't have outbound network connectivity,
  do the following: (see https://www.alcf.anl.gov/support/user-guides/theta-gpu/getting-started/index.html)
    * export HTTP_PROXY=http://theta-proxy.tmi.alcf.anl.gov:3128
    * export HTTPS_PROXY=http://theta-proxy.tmi.alcf.anl.gov:3128
    * export http_proxy=http://theta-proxy.tmi.alcf.anl.gov:3128
    * export https_proxy=http://theta-proxy.tmi.alcf.anl.gov:3128

- Use the latest spack from the develop branch:
    * git clone https://github.com/spack/spack
    * source spack/share/spack/setup-env.sh

- Build mochi-margo with the latest from the main branch so that we can get a new enough margo and mercury that
  have GPU capabilities:
    * git clone https://github.com/mochi-hpc/mochi-spack-packages
    * git clone https://github.com/mochi-hpc-experiments/platform-configurations.git
    * spack repo add mochi-spack-packages
    * spack install mochi-margo@develop

- Use the system (preinstalled) MPI instead of letting spack build it:
    * spack external find mpi

- Build libfabric:
    * with fabrics=verbs,rxm
    * with the system's rdma-core instead of letting spack build its own rdma-core
        ```
        rdma-core:
            buildable: False
            externals:
            - spec: rdma-core@39.1
              prefix: /usr
        ```

- Build mochi-ssg:
    * Pass whatever MPI implementation ThetaGPU has as a dependency to this command.
    * spack install mochi-ssg@develop ^<mpich | openmpi | ...

- Build mochi-tests:
    * git clone -q https://github.com/mochi-hpc-experiments/mochi-tests.git
    * If you want to build the gpu margo test, make sure nvcc is on PATH
    * spack load mochi-margo, mochi-ssg, mercury, argobots
    * setup LD_LIBRARY_PATH
