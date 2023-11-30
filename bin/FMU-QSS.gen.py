#!/usr/bin/env python

# FMU-QSS Generator
#
# Project: QSS Solver
#
# Language: Python 2.7 and 3.x
#
# Developed by Objexx Engineering, Inc. (https://objexx.com) under contract to
# the National Renewable Energy Laboratory of the U.S. Department of Energy
#
# Copyright (c) 2017-2023 Objexx Engineering, Inc. All rights reserved.
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

# Notes
#  Generates an FMU-QSS from an FMU-ME with specified QSS options
#  ElementTree reorders attributes lexicographically and omits comments so we are using lxml
#  lxml should be faster than ElementTree other than initial parsing
#  lxml is not included with most Python distributions but can be installed with pip or, on Linux, from a package
#  Discrete variables that aren't output variables are elided from the FMU-QSS XML
#  Zero-crossing variables (__zc_*) are output variables in our FMU-ME for now but are elided from the FMU-QSS XML

# Do
#  Add more QSS options->annotations as needed

# Imports
import argparse, errno, os, platform, shutil, subprocess, sys
from zipfile import ZipFile
from collections import OrderedDict
from lxml import etree

def fmu_qss_gen():
    '''Generate an FMU-QSS from an FMU-ME'''

    # Process arguments
    parser = argparse.ArgumentParser()
    parser.add_argument( 'ME', help = 'FMU-ME fmu or xml file', default = 'modelDescription.xml' )
    parser.add_argument( '--qss', help = 'QSS method  [n][i][r][f][LI]QSS(1|2|3)  [QSS2]', default = 'QSS2' )
    parser.add_argument( '--rTol', help = 'relative tolerance  [FMU]', type = float )
    parser.add_argument( '--aTol', help = 'absolute tolerance  [1e-6]', type = float, default = 1.0e-6 )
    parser.add_argument( '--tEnd', help = 'simulation end time  [FMU]', type = float )
    args = parser.parse_args()
    args.qss = args.qss.upper()
    solvers = (
     'QSS1',
     'QSS2',
     'QSS3',
     'LIQSS1',
     'LIQSS2',
     'LIQSS3',
     'fQSS1',
     'fQSS2',
     'fQSS3',
     'fLIQSS1',
     'fLIQSS2',
     'fLIQSS3',
     'iLIQSS1',
     'iLIQSS2',
     'iLIQSS3',
     'ifLIQSS1',
     'ifLIQSS2',
     'ifLIQSS3',
     'rQSS2',
     'rQSS3',
     'rfQSS2',
     'rfQSS3',
     'nQSS2',
     'nQSS3',
     'nLIQSS2',
     'nLIQSS3',
     'nfQSS2',
     'nfQSS3',
     'nfLIQSS2',
     'nfLIQSS3',
     'niLIQSS2',
     'niLIQSS3',
     'nifLIQSS2',
     'nifLIQSS3',
     'nrQSS2',
     'nrQSS3',
     'nrfQSS2',
     'nrfQSS3',
    )
    if args.qss not in solvers:
        print( '\nUnsupported QSS method: ' + args.qss + ': Must be one of:', ', '.join( solvers ) )
        sys.exit( 1 )
    if ( args.rTol is not None ) and ( args.rTol < 0.0 ):
        print( '\nNegative rTol: ' + "{:.16f}".format( args.rTol ) )
        sys.exit( 1 )
    if args.aTol <= 0.0:
        print( '\nNonpositive aTol: ' +"{:.16f}".format( args.aTol ) )
        sys.exit( 1 )
    if ( args.tEnd is not None ) and ( args.tEnd < 0.0 ):
        print( '\nNegative tEnd: ' +"{:.16f}".format( args.tEnd ) )
        sys.exit( 1 )
    ME_lower = args.ME.lower()
    if ME_lower.endswith( '.xml' ): # XML input
        me_fmu_name = me_name = None
        me_xml_name = args.ME
    elif ME_lower.endswith( '.fmu' ): # FMU input
        me_fmu_name = args.ME
        me_name = os.path.splitext( os.path.basename( me_fmu_name ) )[ 0 ]
        me_xml_name = 'modelDescription.xml'
    else:
        print( '\nFMU-ME input is not a .fmu or .xml file: ' + args.ME )
        sys.exit( 1 )

    # Extract modelDescription.xml from FMU-ME
    if me_fmu_name:
        try:
            zip_file = ZipFile( me_fmu_name )
            zip_file.extract( 'modelDescription.xml' )
            zip_file.close()
        except:
            print( '\nExtracting modelDescription.xml from FMU-ME fmu failed' )
            sys.exit( 1 )

    # Read FMU-ME xml file
    try:
        parser = etree.XMLParser( remove_blank_text = True )
        tree = etree.parse( me_xml_name, parser )
        root = tree.getroot()
    except:
        print( '\nFMU-ME XML open and parse failed: ' + me_xml_name )
        sys.exit( 1 )

    # fmiModelDescription changes
    if root.tag != "fmiModelDescription":
        print( '\nRoot is not fmiModelDescription in FMU-ME XML: ' + me_xml_name )
        sys.exit( 1 )
    fmiModelDescription = root
    if 'modelName' in fmiModelDescription.attrib:
        fmiModelDescription.attrib[ 'modelName' ] = fmiModelDescription.attrib[ 'modelName' ] + '_QSS' # FMU-QSS model name gets _QSS suffix
    if 'numberOfEventIndicators' in fmiModelDescription.attrib:
        fmiModelDescription.attrib[ 'numberOfEventIndicators' ] = '0' # FMU-QSS has no event indicators
    guid_placeholder = '@FMU-QSS_GUID@'
    fmiModelDescription.attrib[ 'guid' ] = guid_placeholder # Placeholder for running fmu-uuid

    # ModelExchange changes
    ModelExchange = root.find( 'ModelExchange' )
    if ModelExchange is None:
        print( '\nModelExchange not found in ' + me_xml_name )
        sys.exit( 1 )
    if 'modelIdentifier' in ModelExchange.attrib:
        ModelExchange.attrib[ 'modelIdentifier' ] = ModelExchange.attrib[ 'modelIdentifier' ] + '_QSS' # FMU-QSS model identifier gets _QSS suffix

    # Find ModelVariables
    ModelVariables = root.find( 'ModelVariables' )
    if ModelVariables is None:
        print( '\nModelVariables not found in ' + me_xml_name )
        sys.exit( 1 )

    # Add QSS annotations
    VendorAnnotations = root.find( 'VendorAnnotations' )
    if VendorAnnotations is None:
        VendorAnnotations = etree.Element( 'VendorAnnotations' )
        ModelVariables.addprevious( VendorAnnotations )
    QSS = etree.SubElement( VendorAnnotations, 'Tool', attrib = { 'name': 'QSS' } )
    Annotations = etree.SubElement( QSS, 'Annotations' )
    etree.SubElement( Annotations, 'Annotation', attrib = OrderedDict( [ ( 'name', 'qss' ), ( 'value', args.qss ) ] ) )
    if args.rTol is not None: etree.SubElement( Annotations, 'Annotation', attrib = OrderedDict( [ ( 'name', 'rTol' ), ( 'value', "{:.16f}".format( args.rTol ) ) ] ) )
    etree.SubElement( Annotations, 'Annotation', attrib = OrderedDict( [ ( 'name', 'aTol' ), ( 'value', "{:.16f}".format( args.aTol ) ) ] ) )
    if args.tEnd is not None: etree.SubElement( Annotations, 'Annotation', attrib = OrderedDict( [ ( 'name', 'tEnd' ), ( 'value', "{:.16f}".format( args.tEnd ) ) ] ) )
    #Do Add other annotations as needed

    # Generate model-specific QSS header
    try: # Create QSS options header
        QSS_option_name = 'FMU_QSS_options.hh'
        if sys.version_info >= ( 3, 0 ):
            QSS_option_file = open( QSS_option_name, 'w', newline = '\n' )
        else:
            QSS_option_file = open( QSS_option_name, 'wb' )
    except:
        print( '\nQSS options header open failed: ' + QSS_option_name )
        sys.exit( 1 )
    try: # Write QSS_option header
        QSS_option_file.write( '#ifndef FMU_QSS_options_hh_INCLUDED\n' )
        QSS_option_file.write( '#define FMU_QSS_options_hh_INCLUDED\n' )
        QSS_option_file.write( 'QSS::options::QSS const fmu_qss_qss( QSS::options::QSS::' + args.qss + ' );\n' )
        QSS_option_file.write( 'int const fmu_qss_order( ' + args.qss[ -1 ] + ' );\n' )
        if args.rTol is not None:
            QSS_option_file.write( 'double const fmu_qss_rTol( ' + "{:.16f}".format( args.rTol ) + ' );\n' )
        else:
            QSS_option_file.write( 'double const fmu_qss_rTol( -1.0 ); // Negative => Unspecified\n' )
        QSS_option_file.write( 'double const fmu_qss_aTol( ' + "{:.16f}".format( args.aTol ) + ' );\n' )
        if args.tEnd is not None:
            QSS_option_file.write( 'double const fmu_qss_tEnd( ' + "{:.16f}".format( args.tEnd ) + ' );\n' )
        else:
            QSS_option_file.write( 'double const fmu_qss_tEnd( -1.0 ); // Negative => Unspecified\n' )
        QSS_option_file.write( '#endif\n' )
        QSS_option_file.close()
    except Exception as err:
        print( '\nQSS options header write failed: ' + QSS_option_name + ': ' + str( err ) )
        sys.exit( 1 )

    # Find ScalarVariables
    ScalarVariables = ModelVariables.findall( 'ScalarVariable' ) # List of ScalarVariable

    # Identify state variables
    try:
        ModelStructure = root.find( 'ModelStructure' )
        Derivatives = ModelStructure.find( 'Derivatives' )
        Unknowns = Derivatives.findall( 'Unknown' )
    except:
        Unknowns = [] # No state variable derivatives
    derivatives_indexes = set() # State variable derivative indexes
    for Unknown in Unknowns:
        try:
            derivatives_indexes.add( int( Unknown.attrib[ 'index' ] ) )
        except:
            pass
    is_state = { i: False for i in range( 1, len( ScalarVariables ) + 1 ) }
    for i in range( len( ScalarVariables ) ):
        v = ScalarVariables[ i ]
        Real = v.find( 'Real' )
        if Real is not None:
            derivative_of = Real.attrib[ 'derivative' ] if 'derivative' in Real.attrib else None
            if derivative_of is not None:
                try:
                    derivative_of_index = int( derivative_of )
                    if i + 1 in derivatives_indexes: is_state[ derivative_of_index ] = True # Otherwise could be a non-state (internal or input) variable with a derivative
                except Exception as err:
                    name = v.attrib[ 'name' ] if 'name' in v.attrib else ''
                    print( 'Non-integer derivative in ' + name + ': ' + str( derivative_of ) )

    # Migrate variables and map indexes
    io = {} # FMU-ME to FMU-QSS variable index map
    # oi = {} # FMU-QSS to FMU-ME variable index map
    i = o = 0
    outputs = []
    n_real = n_integer = n_boolean = n_string = 0
    n_input_real = n_output_real = 0
    try:
        n_input_real_max_order = n_output_real_max_order = int( args.qss[ -1 ] )
    except Exception as err:
        print( '\nFMU-QSS XML generation failed: QSS method order not identified from last character of qss argument: ' + str( args.qss ) )
        sys.exit( 1 )
    for v in ScalarVariables:
        i += 1 # FMU-ME variable index
        a = v.attrib
        name = a[ 'name' ] if 'name' in a else ''
        causality = a[ 'causality' ] if 'causality' in a else 'local'
        variability = a[ 'variability' ] if 'variability' in a else 'continuous'
        previous = v.getprevious()
        comment = previous if ( previous is not None ) and ( previous.tag is etree.Comment ) and str( previous ).startswith( ( '<!-- Variable with index #', '<!-- Index for next variable = ' ) ) else None
        if causality in ( 'input', 'output' ) and not ( ( causality == 'output' ) and name.startswith( '__zc_' ) ): # Keep (except zero-crossing output variables)
            o += 1 # FMU-QSS variable index
            io[ i ] = o
            # oi[ o ] = i
            Real = v.find( 'Real' )
            Integer = v.find( 'Integer' )
            Boolean = v.find( 'Boolean' )
            String = v.find( 'String' )
            if Real is not None:
                n_real += 1
            elif Integer is not None:
                n_integer += 1
            elif Boolean is not None:
                n_boolean += 1
            elif String is not None:
                n_string += 1
            if causality == 'output':
                outputs.append( o )
                if Real is not None:
                    n_output_real += 1
            else: # Input
                if Real is not None:
                    n_input_real += 1
            set_comment = True
        elif ( causality == 'local' ) and ( variability == 'continuous' ) and is_state[ i ]: # State to output variable
            a[ 'causality' ] = 'output'
            o += 1 # FMU-QSS variable index
            io[ i ] = o
            # oi[ o ] = i
            outputs.append( o )
            if 'initial' in a: del a[ 'initial' ] # Drop initial spec
            set_comment = True
            Real = v.find( 'Real' )
            if Real is not None:
                n_real += 1
                n_output_real += 1
                if 'start' in Real.attrib: del Real.attrib[ 'start' ] # Remove start spec
            else:
                print( '\nFMU-ME (continuous) state variable is not Real: ' + name )
                sys.exit( 1 )
        else: # Remove
            ModelVariables.remove( v )
            if comment is not None: ModelVariables.remove( comment )
            set_comment = False
        if set_comment:
            if comment is not None: # Adjust variable index in comment
#               comment.text = ' Index for next variable = ' + str( o ) + ' (' + str( i ) + ') ' # Dymola format
                comment.text = ' Variable with index #' + str( o ) + ' (' + str( i ) + ') ' # OCT format
            else: # Insert comment
#               v.addprevious( etree.Comment( ' Index for next variable = ' + str( o ) + ' (' + str( i ) + ') ' ) ) # Dymola format
                v.addprevious( etree.Comment( ' Variable with index #' + str( o ) + ' (' + str( i ) + ') ' ) ) # OCT format

    # Re-index derivatives
    ScalarVariables = ModelVariables.findall( 'ScalarVariable' ) # List of ScalarVariable after above pruning
    for v in ScalarVariables:
        Real = v.find( 'Real' )
        if Real is not None:
            derivative = Real.attrib[ 'derivative' ] if 'derivative' in Real.attrib else None
            if derivative is not None:
                try:
                    derivative_index = int( derivative )
                    try:
                        Real.attrib[ 'derivative' ] = str( io[ derivative_index ] )
                    except Exception as err:
                        print( 'Derivative re-indexing failed for ' + ( v.attrib[ 'name' ] if 'name' in v.attrib else '' ) + ': ' + str( err ) )
                except:
                    pass # Already reported this above

    # Migrate ModelStructure
    ModelStructure = root.find( 'ModelStructure' )
    if ModelStructure is None:
        ModelStructure = etree.Element( 'ModelStructure' )
        ModelVariables.addnext( ModelStructure )
    for g in ( 'Derivatives', 'DiscreteStates', 'InitialUnknowns' ): # Remove these sections
        e = ModelStructure.find( g )
        if e is not None:
            ModelStructure.remove( e )
    Outputs = ModelStructure.find( 'Outputs' )
    if Outputs is None:
        Outputs = etree.SubElement( ModelStructure, 'Outputs' )
    Unknowns = Outputs.findall( 'Unknown' )
    for u in Unknowns: # Remove previous entries
        Outputs.remove( u )
    for o in outputs:
        etree.SubElement( Outputs, 'Unknown', attrib = OrderedDict( [ ( 'index', str( o ) ), ( 'dependencies', '' ) ] ) )

    # Write FMU-QSS xml file
    #print( etree.tostring( root, pretty_print=True, encoding='unicode' ) ) #Debug#####
    try:
        qss_xml_name = 'FMU-QSS_' + os.path.basename( me_xml_name )
        tree.write( qss_xml_name, encoding = 'UTF-8', xml_declaration = True, pretty_print = True )
    except Exception as err:
        print( '\nFMU-QSS XML write failed: ' + qss_xml_name + ': ' + str( err ) )
        sys.exit( 1 )

    # Add GUID to FMU-QSS xml file and generate GUID header
    try:
        subprocess.call( [ 'fmu-uuid', qss_xml_name, guid_placeholder, qss_xml_name, 'FMU_QSS_GUID.hh', 'FMU_QSS_GUID' ] )
    except OSError as e:
        if e.errno == errno.ENOENT:
            print( '\nFMU-QSS XML GUID computation failed: fmu-uuid program not in PATH' )
        else:
            print( '\nFMU-QSS XML GUID computation failed: ' + str( e ) )
        print( 'Generic no-check GUID header generated' )
        try:
            guid_name = 'FMU_QSS_GUID.hh'
            if sys.version_info >= ( 3, 0 ):
                guid_file = open( guid_name, 'w', newline = '\n' )
            else:
                guid_file = open( guid_name, 'wb' )
        except:
            print( '\nGUID header open failed: ' + guid_name )
            sys.exit( 1 )
        try:
            guid_file.write( '#ifndef FMU_QSS_GUID\n' )
            guid_file.write( '#define FMU_QSS_GUID "FMU-QSS_GUID" // No-check value\n' )
            guid_file.write( '#endif\n' )
            guid_file.close()
        except:
            print( '\nGUID header write failed: ' + guid_name )
            sys.exit( 1 )

    # Generate model-specific size definitions header
    try: # Create sizing header
        sizing_name = 'FMU_QSS_defines.hh'
        if sys.version_info >= ( 3, 0 ):
            sizing_file = open( sizing_name, 'w', newline = '\n' )
        else:
            sizing_file = open( sizing_name, 'wb' )
    except:
        print( '\nSizing header open failed: ' + sizing_name )
        sys.exit( 1 )
    try: # Write sizing header: Sizes >=1 to avoid illegal 0-sized arrays
        sizing_file.write( '#ifndef FMU_QSS_defines_hh_INCLUDED\n' )
        sizing_file.write( '#define FMU_QSS_defines_hh_INCLUDED\n' )
        sizing_file.write( '// Note: Sizes are >=1 to avoid illegal 0-sized arrays\n' )
        sizing_file.write( '#define BUFFER 1024\n' )
        sizing_file.write( '#define N_REAL ' + str( max( n_real, 1 ) ) + '\n' )
        sizing_file.write( '#define N_INTEGER ' + str( max( n_integer, 1 ) ) + '\n' )
        sizing_file.write( '#define N_BOOLEAN ' + str( max( n_boolean, 1 ) ) + '\n' )
        sizing_file.write( '#define N_STRING ' + str( max( n_string, 1 ) ) + '\n' )
        sizing_file.write( '#define N_INPUT_REAL ' + str( max( n_input_real, 1 ) ) + '\n' )
        sizing_file.write( '#define N_INPUT_REAL_MAX_ORDER ' + str( max( n_input_real_max_order, 1 ) ) + '\n' )
        sizing_file.write( '#define N_OUTPUT_REAL ' + str( max( n_output_real, 1 ) ) + '\n' )
        sizing_file.write( '#define N_OUTPUT_REAL_MAX_ORDER ' + str( max( n_output_real_max_order, 1 ) ) + '\n' )
        sizing_file.write( '#endif\n' )
        sizing_file.close()
    except Exception as err:
        print( '\nSizing header write failed: ' + sizing_name + ': ' + str( err ) )
        sys.exit( 1 )

    # Generate FMU-QSS
    if me_fmu_name:
        try: # Directory tree setup
            qss_name = me_name + '_QSS'
            if os.path.exists( qss_name ):
                if os.path.isdir( qss_name ):
                    shutil.rmtree( qss_name )
                elif os.path.isfile( qss_name ):
                    os.remove( qss_name )
            os.mkdir( qss_name )
            os.mkdir( os.path.join( qss_name, 'binaries' ) )
            if not platform.machine().endswith( '64' ):
                print( '\nFMU-QSS generation only supports 64-bit OS at this time' )
                sys.exit( 1 )
            if sys.platform.startswith( 'linux' ):
                binaries_dir = qss_name + '/binaries/linux64'
            elif sys.platform.startswith( 'win' ):
                binaries_dir = qss_name + '\\binaries\\win64'
            else:
                print( '\nPlatform is not supported for FMU-QSS generation' )
                sys.exit( 1 )
            os.mkdir( binaries_dir )
            os.mkdir( qss_name + os.sep + 'resources' )
            #os.mkdir( qss_name + os.sep + 'sources' )
        except Exception as err:
            print( '\nFMU-QSS directory tree setup failed: ' + str( err ) )
            sys.exit( 1 )
        try: # Build FMU-QSS library
            QSS = os.environ.get( 'QSS' )
            QSS_bin = os.environ.get( 'QSS_bin' )
            PlatformOSCompiler = os.environ.get( 'PlatformOSCompiler' )
            if QSS and QSS_bin and PlatformOSCompiler: # Configured for building
                QSS_src = os.path.join( QSS, 'src', 'QSS' )
                if os.path.exists( 'src' ):
                    if os.path.isdir( 'src' ):
                        shutil.rmtree( 'src' )
                    elif os.path.isfile( 'src' ):
                        os.remove( 'src' )
                src_bld = 'src'
                fmu_bld = os.path.join( 'src', 'QSS' )
                os.makedirs( fmu_bld )
                shutil.copy( 'FMU_QSS_defines.hh', fmu_bld )
                shutil.copy( 'FMU_QSS_GUID.hh', fmu_bld )
                shutil.copy( os.path.join( QSS_src, PlatformOSCompiler, 'GNUmakefile' ), src_bld )
                cwd = os.getcwd()
                os.chdir( src_bld )
                with open( 'GNUmakefile', 'r' ) as sources:
                    lines = sources.readlines()
                with open( 'GNUmakefile', 'w' ) as sources:
                    for line in lines:
                        if line.startswith( 'DLB := $(BIN_PATH)' + os.sep ):
                            sources.write( line.replace( '$(BIN_PATH)' + os.sep, '' ) )
                        else:
                            sources.write( line )
                try:
                    import psutil
                    n_processors = psutil.cpu_count()
                except:
                    print( '\nNon-parallel make used: psutil processor count lookup failed' )
                    n_processors = 1
                try:
                    subprocess.call( [ 'make', '-j', str( n_processors ) ] )
                    try:
                        if sys.platform.startswith( 'linux' ):
                            qss_lib = os.path.join( cwd, binaries_dir, qss_name + '.so' )
                            if os.path.isfile( qss_lib ): os.remove( qss_lib )
                            os.rename( 'libFMU-QSS.so', qss_lib )
                        elif sys.platform.startswith( 'win' ):
                            qss_lib = os.path.join( cwd, binaries_dir, qss_name + '.dll' )
                            if os.path.isfile( qss_lib ): os.remove( qss_lib )
                            os.rename( 'libFMU-QSS.dll', qss_lib )
                    except Exception as err:
                        print( '\nFMU-QSS library move into staging directory failed: ' + str( err ) )
                except Exception as err:
                    print( '\nFMU-QSS library make failed: ' + str( err ) )
                os.chdir( cwd )
                shutil.rmtree( src_bld )
            else:
                print( '\nFMU-QSS library can\'t be built: QSS and QSS_bin environment variables are not set' )
        except Exception as err:
            print( '\nFMU-QSS library build failed: ' + str( err ) )
        try: # File setup
            shutil.copyfile( qss_xml_name, qss_name + os.sep + 'modelDescription.xml' )
            shutil.copy( me_fmu_name, qss_name + os.sep + 'resources' )
        except Exception as err:
            print( '\nFMU-QSS file setup failed: ' + str( err ) )
            sys.exit( 1 )
        try: # Zip FMU-QSS
            qss_fmu_name = qss_name + '.fmu'
            if os.path.exists( qss_fmu_name ):
                if os.path.isfile( qss_fmu_name ):
                    os.remove( qss_fmu_name )
                elif os.path.isdir( qss_fmu_name ):
                    shutil.rmtree( qss_fmu_name )
            zip_file = ZipFile( qss_fmu_name, mode = 'w' )
            os.chdir( qss_name )
            for root, dirs, files in os.walk( '.' ):
                dirs.sort()
                for dir in dirs:
                    zip_file.write( os.path.join( root, dir ) )
                files.sort()
                for file in files:
                    zip_file.write( os.path.join( root, file ) )
            os.chdir( '..' )
            zip_file.close()
        except Exception as err:
            print( '\nFMU-QSS zip into .fmu failed: ' + str( err ) )
            sys.exit( 1 )

if __name__ == '__main__':
    fmu_qss_gen()
