#!/usr/bin/env python

# Dependency Generator for C/C++
#
# Language: Python (2.7 or 3.x)
#
# Version: 1.0.0
#
# Copyright (c) 2000-2024 Objexx Engineering, Inc. All Rights Reserved.

# Notes:
# . Dependency files generated are for use with GNU make
# . Only processes C/C++ with supported file name extensions
# . Leaves specified paths on prerequisites but does not add path where found
# . Adds a line with all prerequisites as targets to avoid make error if one is removed or renamed
# . Include search path defaults to environment variables:
#   . C/C++: CPATH then CPLUS_INCLUDE_PATH then INCLUDE

# Imports
import argparse, os, re, sys

# Globals
C_ext = re.compile( r'(c|cc|cpp|cxx|c\+\+|h|hh|hpp|hxx|h\+\+|ii|ipp|ixx|i\+\+)', re.I )
C_inc = re.compile( r'\s*#\s*include +<([^<>]+\.[^<>]+)>' ) # Exclude <name> C++ Standard Library headers
C_inq = re.compile( r'\s*#\s*include +"([^"]+)"' )
C_sys = re.compile( r'sys/' ) # Exclude <sys/header> C/C++ system headers
C_path = None

# Main
def main():

    # Get options and arguments
    parser = argparse.ArgumentParser( description = 'Generates C/C++ dependency files' )
    parser.add_argument( '--inc', help = 'include path or env var [CPATH|CPLUS_INCLUDE_PATH|INCLUDE]' )
    parser.add_argument( '--ext', help = 'object file extension [obj|o]', default = ( 'obj' if os.name == 'nt' and sys.platform != 'msys' and os.sep != '/' else 'o' ) )
    parser.add_argument( 'source', help = 'Source file' )
    arg = parser.parse_args()

    # Set up include search paths
    global C_path
    if not arg.inc:
        if 'CPATH' in os.environ:
            C_path = os.environ[ 'CPATH' ]
        elif 'CPLUS_INCLUDE_PATH' in os.environ:
            C_path = os.environ[ 'CPLUS_INCLUDE_PATH' ]
        elif 'INCLUDE' in os.environ:
            C_path = os.environ[ 'INCLUDE' ]
    elif arg.inc in os.environ: # Treat arg as env var name
        C_path = os.environ[ arg.inc ]
    else: # Treat arg as path
        C_path = arg.inc
    C_path = C_path.split( os.pathsep ) if C_path else None

    # Parse source file name
    if not os.path.isfile( arg.source ):
        raise IOError( 'Source file not found: ' + str( arg.source ) )
    src_name = os.path.basename( arg.source )
    src_base, src_ext = os.path.splitext( src_name )
    dep_name = src_base + '.d'
    obj_name = src_base + '.' + arg.ext
    if src_ext: src_ext = src_ext[ 1: ]
    if not ( src_base and src_ext and C_ext.match( src_ext ) ):
        raise ValueError( 'Not a recognized C/C++ file name extension: ' + str( src_ext ) )

    # Build dependencies
    tar = obj_name + ' ' + dep_name
    dep = [ src_name ]
    if C_ext.match( src_ext ):
        dep_list = list( C_deps( os.path.abspath( arg.source ), add = False ) )
    else:
        assert False, 'Not a recognized C/C++ file name extension: ' + str( src_ext )
    dep_list.sort()
    dep.extend( dep_list )
    dep_str = ' '.join( dep )

    # Write dependency file
    dep_file = open( dep_name, mode = 'w' )
    dep_file.write( tar + ' : ' + dep_str + '\n' )
    dep_file.write( dep_str + ' :\n' )
    dep_file.close()

def C_deps( fname, fdeps = None, par_dir = None, quoted = False, add = True ):
    '''C/C++ dependencies of a file: Recursive'''
    if fdeps is None: fdeps = set()
    if par_dir is None: par_dir = []
    try:
        gname = None
        if ( not os.path.isabs( fname ) ) and ( not os.path.isfile( fname ) ): # Search include path list
            if quoted: # Check relative to parent directories
                for adir in par_dir:
                    tname = os.path.join( adir, fname )
                    if os.path.isfile( tname ):
                        if ' ' not in tname: gname = tname # Skip dependencies in paths with spaces for GNU Make compatibility
                        break
            if not gname: # Use include search path
                for adir in C_path:
                    tname = os.path.join( adir, fname )
                    if os.path.isfile( tname ):
                        if ' ' not in tname: gname = tname # Skip dependencies in paths with spaces for GNU Make compatibility
                        break
        if gname is None: gname = fname
        dfile = open( gname, mode = 'r' if sys.version_info >= ( 3, 0 ) else 'rU' )
        if add: fdeps.add( fname ) # Only add dependency if it is found
        par_new = os.path.dirname( os.path.abspath( gname ) )
        if ( not par_dir ) or ( par_dir[ 0 ] != par_new ): # Push parent dir onto front of list
            par_dir.insert( 0, par_new )
        else: # Flag not to pop since didn't push
            par_new = None
        for line in dfile:
            m = C_inc.match( line ) # #include <header> form
            if m :
                dep = m.group( 1 )
                if dep and ( dep not in fdeps ) and ( not C_sys.match( dep ) ):
                    C_deps( dep, fdeps, par_dir ) # Recurse
            else:
                m = C_inq.match( line ) # #include "header" form
                if m:
                    dep = m.group( 1 )
                    if dep and ( dep not in fdeps ) and ( not C_sys.match( dep ) ):
                        C_deps( dep, fdeps, par_dir, quoted = True ) # Recurse
        dfile.close()
        if par_new: del par_dir[ 0 ] # Pop parent dir from front of list
    except:
        pass # Skip missing file
    return fdeps

# Runner
if __name__ == '__main__':
    main()
