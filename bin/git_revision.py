#!/usr/bin/env python

# Update QSS Git Revision in QSS Source
#
# Project: QSS Solver
#
# Language: Python 3.x
#
# Developed by Objexx Engineering, Inc. (https://objexx.com) under contract to
# the National Renewable Energy Laboratory of the U.S. Department of Energy
#
# Copyright (c) 2017-2024 Objexx Engineering, Inc. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# (1) Redistributions of source code must retain the above copyright notice,
#     this list of conditions and the following disclaimer.
#
# (2) Redistributions in binary form must reproduce the above copyright notice,
#     this list of conditions and the following disclaimer in the documentation
#     and/or other materials provided with the distribution.
#
# (3) Neither the name of the copyright holder nor the names of its
#     contributors may be used to endorse or promote products derived from this
#     software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER, THE UNITED STATES
# GOVERNMENT, OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
# OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# Python imports
import os, re, subprocess, sys
from pathlib import Path

# Globals
version_git_re = re.compile( r'^\"([a-f0-9]{7})\"$' )

try: # Get current git revision
    version_git = subprocess.check_output( [ 'git', 'rev-parse', '--short', 'HEAD' ], shell=True ).decode('ascii').strip()
except:
    version_git = ''

# Get QSS environment variable
QSS = os.environ.get( 'QSS' )
if not QSS:
    print( 'Git revision update script (git_revision.py) failed: QSS environment variable is not set' )
    sys.exit( 1 )

try: # Update/create git revision file if needed
    version_git_hh = Path( os.path.join( QSS, 'src', 'QSS', 'version_git.hh' ) )
    if os.path.isfile( version_git_hh ): # Update version_git.hh if needed
        old_version_git = version_git_hh.read_text()
        m = version_git_re.match( old_version_git )
        old_version_git = m.group( 1 ) if m else ''
        if old_version_git != version_git: # Update version_git.hh
            with open( version_git_hh, 'w' ) as version_git_file:
                version_git_file.write( r'"' + version_git + r'"' )
    else: # Create version_git.hh
        with open( version_git_hh, 'w' ) as version_git_file:
            version_git_file.write( r'"' + version_git + r'"' )
except Exception as msg:
    print( 'Git revision update script (git_revision.py) failed: ' + str( msg ) )
    sys.exit( 1 )
