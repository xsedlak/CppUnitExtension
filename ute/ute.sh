#!/bin/bash

################################################################################
# UTE variables
################################################################################

declare -i +x COLOURS=1
declare -i +x INTERACTIVE=0
declare -i +x REGRESSION=0
declare -i +x SILENT=0
declare -i +x VERBOSE=0
declare -i +x GENERATE_SCRIPT=0
declare -i +x INPUT_LIST_TC=0
declare -i +x STORE_SUCCESSFUL_TC=0
declare -i +x COUNTER_SUCCESSFUL_TC=0
declare -i +x STORE_FAILED_TC=0
declare -i +x COUNTER_FAILED_TC=0
declare -i +x DETAILED_DIFF=0
declare -i +x CREATE_EXP=0
declare -i +x NO_UNIFICATION=0
declare -i +x WITH_PUR=0
declare -i +x WITH_QUAN=0
declare -i +x WITH_TCOV=0
declare -i +x GLOBAL_RESULT=0
declare -i +x DO_DOWNGRADE=0
declare -a LIST_TC_STABLE=0 LIST_TC_ACT=0
declare -i COUNT_TC_STABLE=0 COUNT_TC_ACT=0
declare -a LIST_CONFIG_FILE=0
declare -a UTE_BINARY_FILE_PREVIOUS=""
TEST_PATH="$PWD"
DATE=$(date '+%y%m%d')
TIME=$(date '+%H:%M:%S')

# the comamnd line options (CLO_) sets variables for functions
declare -i +x CLO_DEBUG=0
declare -i +x CLO_IGNORE_RESULT=0
declare -i +x CLO_DIFF_OUT=0
declare -a CLO_STC_LIST=""
declare -i +x CLO_TRACE=0
declare -i +x CLO_VARIABLES_PRINT=0
declare CLO_SUFFIX="in"   #suffix of input files


#Change of $TZ shell variable for different Time Zone than MET.
if [ "$TZ" != "MET" ]; then
  B_TZ=$TZ
  export TZ=MET
fi

################################################################################
# UTE basic/common functions
################################################################################

# colourize function shows the line in various colours.
# Accepts pairs of parameters, first of pair is colour name and second printed
# string
# E.g.: colourize bold "Error: " red "Missing executable $BINARY"
function ute_colourize()#{{{
{
#Attribute codes:
#00=none 01=bold 04=underscore 05=blink 07=reverse 08=concealed
#Text color codes:
#30=black 31=red 32=green 33=yellow 34=blue 35=magenta 36=cyan 37=white
            #blue)   echo -en "\033[0;34m" ;;
            #bold)   echo -en "\033[1m" ;;
            #brown)   echo -en "\033[0;33m" ;;
            #cyan)   echo -en "\033[0;36m" ;;
            #d_gray)   echo -en "\033[1;30m" ;;
            #green)   echo -en "\033[0;32m" ;;
            #l_blue)   echo -en "\033[1;34m" ;;
            #l_cyan)   echo -en "\033[1;36m" ;;
            #l_gray)   echo -en "\033[0;37m" ;;
            #l_green)   echo -en "\033[1;32m" ;;
            #l_purple)   echo -en "\033[1;35m" ;;
            #l_red)   echo -en "\033[1;31m" ;;
            #purple)   echo -en "\033[0;35m" ;;
            #red)   echo -en "\033[0;31m" ;;
            #yellow)   echo -en "\033[1;33m" ;;
            #*)      echo -en "\033[m" ;;

    while [ $# -gt 1 ]; do  # first from pair is a colour
        [ $COLOURS -ne 0 ] &&
        case "$1" in        # skip if colours are suppressed
            blue)   echo -en "\033[1;34m" ;;
            bold)   echo -en "\033[1m" ;;
            cyan)   echo -en "\033[2;36m" ;;
            gray)   echo -en "\033[1;37m" ;;
            green)  echo -en "\033[1;32m" ;;
            purple) echo -en "\033[1;35m" ;;
            red)    echo -en "\033[1;31m" ;;
            redbg)  echo -en "\033[1;41m" ;;
            yellow) echo -en "\033[2;33m" ;;
            *)      echo -en "\033[m" ;;
        esac
        echo -en "$2"        # second from pair is the text
        shift 2
    done
    [ $COLOURS -ne 0 ] &&
    echo -e "\033[m" ||      # colour-less newline after last pair
    echo                     # empty newline if colours are suppressed
} #}}}

function ute_debug()#{{{
{
    [ $VERBOSE -ne 0 ] && [ $SILENT -eq 0 ] && ute_colourize blue "* " white "$@"
} #}}}

function ute_info()#{{{
{
    [ $SILENT -eq 0 ] && ute_colourize green "* " white "$@"
} #}}}

function ute_warning()#{{{
{
    [ $SILENT -eq 0 ] && ute_colourize yellow "* " white "Warning: $@"
} #}}}

function ute_silent_ok()#{{{
{
#    ute_colourize green "* " bold "$@"
    if [ $COLOURS -ne 0 ]; then 
    {
        printf "\033[1;32m* %-80s%-10s\n\033[m" "$@" "OK";
    } else {
        printf "* %-80s%-10s\n" "$@" "OK";
    }
    fi
} #}}}

function ute_silent_start()#{{{
{
#    ute_colourize red "* " bold "$@"
#    printf "\033[1;31m* %-80s%-10s\n\033[m" "$@" "ERROR";
    ute_colourize blue "* " white "Test started:\t\t\t$(date)"
    if [ $COLOURS -ne 0 ]; then 
    {
        printf "\033[1;34m* \033[m%-80s%-10s\n" "Test case name" "Status";
    } else {
        printf "* %-80s%-10s\n" "Test case name" "Status";
    }    
    fi;
    ute_silent_seperator   
} #}}}

function ute_end()#{{{
{
#    ute_colourize red "* " bold "$@"
#    printf "\033[1;31m* %-80s%-10s\n\033[m" "$@" "ERROR";
  ute_silent_seperator
  ute_colourize blue "* " white "Test finished:\t\t$(date)"
  if [ $INTERACTIVE -ne 1 ] && [ $COUNT_TC_ACT -gt 1 ]; then
    ute_silent_seperator
    ute_colourize blue "* " white "TOTAL of executed test cases:\t$COUNT_TC_ACT"
    if [ $STORE_FAILED_TC -eq 1 ]; then 
      if [ $COUNTER_FAILED_TC -gt 0 ]; then 
        ute_colourize red "* FAILED test cases:\t\t$COUNTER_FAILED_TC" white "\tFILE with tc's list: " red "$UTE_TC_FAILED"
      else
        ute_colourize blue "* " white"FAILED test cases:\t\t$COUNTER_FAILED_TC" white "\tFILE with tc's list: " red "$UTE_TC_FAILED"
      fi
    else
      if [ $COUNTER_FAILED_TC -gt 0 ]; then 
        ute_colourize red "* FAILED test cases:\t\t$COUNTER_FAILED_TC"
      else
        ute_colourize blue "* " white "FAILED test cases:\t\t$COUNTER_FAILED_TC"
      fi
    fi
    if [ $STORE_SUCCESSFUL_TC -eq 1 ]; then
      ute_colourize green "* SUCCESSFUL test cases:\t$COUNTER_SUCCESSFUL_TC" white "\tFILE with tc's list: " green "$UTE_TC_SUCCESSFUL"
    else
      ute_colourize green "* SUCCESSFUL test cases:\t$COUNTER_SUCCESSFUL_TC"
    fi
    ute_silent_seperator
  fi
} #}}}

function ute_silent_error()#{{{
{
#    ute_colourize red "* " bold "$@"
    if [ $COLOURS -ne 0 ]; then 
    {
        printf "\033[1;31m* %-80s%-10s\n\033[m" "$@" "ERROR";
    } else {
        printf "* %-80s%-10s\n" "$@" "ERROR";
    }
    fi;
} #}}}

function ute_silent_seperator()#{{{
{
    ute_colourize blue "* " white "==========================================================================================" 
} #}}}

function ute_error()#{{{
{
    ute_colourize red "* " white "Error: $@"
} #}}}

function ute_seperator()#{{{
{
    [ $SILENT -eq 0 ] && ute_colourize blue "* " bold "==========================================================================================" 
} #}}}

#brief      Reseting $RESULT value if this value is contained in the list of numbers (ignored return values)
#1st param  The result value of tested target binary
function ute_ignore_results()#{{{
{
  if [ $1 -ne 0 ]; then
    ute_info "The result values are ignored for $BINARY:\n" red "\tUTE_BINARY_FILE_IGNORE_RESULTS: $UTE_BINARY_FILE_IGNORE_RESULTS"
  fi  
  for ir in $UTE_BINARY_FILE_IGNORE_RESULTS; do
    if [ $1 -eq $ir ]; then
      RESULT=0;
    fi
  done
} #}}}

#prints and formating variables: [main|function name] [variable_name:variable_value]+
function ute_print_variables()#{{{
{
  ute_info "UTE_TRACE: " green "$1"
  shift
  for variable in $@; do 
    printf "\t%+10s : %s\n" ${variable%:*} ${variable#*:}
  done 
} #}}}

function ute_print_configuration()#{{{
{
  [ $# -eq 0 ] && diff $SET_UTE_CONFIG_BEFORE $SET_UTE_CONFIG_AFTER | grep "> " 
} #}}}

# Splits common path to dir and file names
# 1st par: input path
# 2nd par: output dir - variable's name
# 3rd par(optional): output name - variable's name
function ute_split_path()#{{{
{
    local -i I
    I=${#1}
    while [ $I -gt 0 ]; do
        I=I-1
        [ ${1:I:1} == "/" ] && break;
    done
    if [ $I -eq 0 ]; then
        eval $2="."
        [ -n "$3" ] && eval $3=\""$1"\"
    else
        eval $2=\""${1:0:I}"\"
        [ -n "$3" ] && eval $3=\""${1:I+1}"\"
    fi
} #}}}

# similar to original basename, fills the 2nd parameter
function ute_basename()#{{{
{
    local dir name
    ute_split_path "$1" dir name
    eval $2=\""$name"\"
} #}}}

function ute_binary_usage()#{{{
{
    :
} #}}}

function ute_tc_usage()#{{{
{
    :
} #}}}

function ute_usage()#{{{
{
    cat << !!

NAME
ute.sh - the main script for running Test Cases of Unit Test Environment (UTE). It is also known as text/graphics UI for UTF which can be used for automation running tests. The UTE is a powerful environment allowing automated execution of TCs for CppUnit with using other program. This envinronment is configurable by variables and function in the <*.config> files. The each example/project should contains developer's ute.dev.config file because then the default ute.config file is used. ute.dev.config is replica of ute.config and both are loaded automatically. The developers modify the variables and functions in the ute.dev.config, which they need to change. 

SYNOPSIS
ute.sh -dhins [-ce] [-dd] [-debugger] [-diffout] [-ir] [-lf] [-ls] [-nodsub] [-rc file.config]+ [-sif suffix] [-tl file] [-tmpc] [UTE_STC_ALL] [UTE_STC_<name>|dirname|file.in]*

DESCRIPTION

    -ce       creates(if don't exists) or rewrites(if exists and have allow write) expected <*.exp> file 
    -d        turn on debug messages - more verbose
    -dd       detailed error output with diff tool
    -debugger runs binary file by the debugger (configured by variable UTE_DEBUGGER: "$UTE_DEBUGGER")
    -diffout  diffs only substituted <*.out> and <*.exp> files
    -h        help (other switches for help [ --h | -help | --help ])
    -i        interactive mode - select test case from list of test cases
    -ir       ignore results - the list of numbers defining the return values to be ignored when tested target binary ended (configured by variable UTE_BINARY_FILE_IGNORE_RESULTS: "$UTE_BINARY_FILE_IGNORE_RESULTS").
    -lf       stores list of failed test cases (configured by variable UTE_TC_FAILED: "$UTE_TC_FAILED")
    -ls       stores list of successful test cases (configured by variable UTE_TC_SUCCESSFUL: "$UTE_TC_SUCCESSFUL")
    -n        don't use ANSI codes for colored output
    -nodsub   switch of default substitutions
    -rc #     ute configuration file <*.config>
    -s        minimal output to screen; default for regression test
    -sif      suffix of input files (default: "in")
    -tl #     test list of test cases with full path 
    -tmpc     cleans temporary directory (configured by variable UTE_TMP_DIR: "$UTE_TMP_DIR")
    UTE_STC_<#>   name of set of test cases defined in default developer's UTE configuration file as UTE_STC_<name> variable
    UTE_STC_ALL   all names of set of test cases defined in default developer's UTE configuration file as UTE_STC_<name> variables
    dirname   name of directory where input files will be search for
    file.in   name of any input test case file to run

AUTHOR
    Written by Miroslav Sedlak.

UTE BINARY USAGE
!!
    ute_binary_usage
echo -e "\nUTE TEST CASE USAGE"
    ute_tc_usage
    exit 1
} #}}}

function ute_arg()#{{{
{
    :
} #}}}

function ute_tc_arg()#{{{
{
    :
} #}}}

function ute_tmp_clean()#{{{
{
  ute_info "Clean temporary data: " green "find $UTE_TMP_DIR -user $USER -exec rm -f {} \+"
    find $UTE_TMP_DIR -user $USER -exec rm -f {} \+
  } #}}}

  function ute_check_argument()#{{{
  {
    if [ -z "$1" ]; then 
      ute_error "The file name argument is required after command line option " red "$2" white " !" 
      exit 1
    elif [ ! -e "$1" ]; then
      ute_error "The file name argument " red "$1" white " does not exist!"
      exit 1
    elif [ ! -r "$1" ]; then
      ute_error "The file name argument " red "$1" white " does not have permission for reading !\n" red "$(ls -la $1)"
      exit 1
    fi
  } #}}}

  function ute_check_file()#{{{
  {
    if [ -z "$1" ]; then 
      ute_error "The file name argument is required after command line option " red "$2" white " !" 
      exit 1
    elif [ ! -e "$1" ]; then
      ute_error "The file name argument " red "$1" white " does not exist!"
      exit 1
    elif [ ! -r "$1" ]; then
      ute_error "The file name argument " red "$1" white " does not have permission for reading !\n" red "$(ls -la $1)"
      exit 1
    fi
  } #}}}

  function ute_parse_arg()#{{{
  {
#debug echo Processing args $1 $2
      case "$1" in
          -tmpc)
              ute_tmp_clean;;
          -ce)
              CREATE_EXP=1;;
          -d|-n|-s|-rc|-trc|-varp) ;; #Needed to be parsed in the begining of ute.sh
          -dd)
              DETAILED_DIFF=1;;
          -debugger)
              CLO_DEBUG=1;;
          -diffout)
              CLO_DIFF_OUT=1;;
        -h|--h|-help|--help)
            ute_usage;;
        -i)
            INTERACTIVE=1;;
        -ir)
            CLO_IGNORE_RESULT=1;;
        -tl)
            ute_check_argument "$2" "-tl"
            LIST_TC_FILE="$2"
            INPUT_LIST_TC=1 
            return 1;; 
        -l)
            COMMAND="LIST";;
        -la|-ld)
            COMMAND="DETAIL";;
        -ll|-lb)
            COMMAND="BRIEF";;
        -ls)
            STORE_SUCCESSFUL_TC=1 
            :> $UTE_TC_SUCCESSFUL;;
        -lf)
            STORE_FAILED_TC=1 
            :> $UTE_TC_FAILED;;
        -nodsub)
            NO_UNIFICATION=1;;
        -pur)
            WITH_PUR=1;;
        -quan)
            WITH_QUAN=1;;
        -tcov)
            WITH_TCOV=1;;
#        -rc)
#            ute_check_argument "$2" "-rc"
#            LIST_CONFIG_FILE=(${LIST_CONFIG_FILE[@]} "$2")
#            return 1;;
        -sif) CLO_SUFFIX="$2"
              return 1;;
        -*)
            TC_PAR="${TC_PAR:+$TC_PAR }$1"
            my_module_arg "$@"
            return $?;;
        *)
            # go through list of test cases defined
            # if command line contains UTE_STC_ALL
            if [ "$1" == "UTE_STC_ALL" ]; then 
                  CLO_STC_LIST="$CLO_STC_LIST $1"
            # if command line contains UTE_STC_* (set of test cases)
            elif [ "${1:0:8}" == "UTE_STC_" ]; then
              if [ "${!1}" == "" ]; then 
                ute_print_configuration UTE_STC_
                ute_error "The varible " red "$1 " white "isn't defined in the UTE's configuration files: " red "${LIST_CONFIG_FILE[@]}"
                exit 1
              else
                  CLO_STC_LIST="$CLO_STC_LIST $1"
              fi
            else 
            # if command line contains name of directory or set of test cases 
              if [ -z "$UTE_TC_DIR" ]; then
                TMP="$1"
              else
                TMP="${UTE_TC_DIR}/$1"
              fi
              [ -r "$TMP" ] || ute_warning "Cannot read '$TMP' !"
              TC_PAR="${TC_PAR:+$TC_PAR }${TMP}"
            fi
    esac
    return 0
} #}}}

# From filename.in it derives .exp, .out and .trc filename
function ute_get_derived_files()#{{{
{
	local DIR BASE
	ute_split_path "$1" DIR BASE
	[ $CLO_TRACE -eq 1 ] &&  ute_print_variables "ute_get_derived_files" "TESTCASE_STRUCTURE:$TESTCASE_STRUCTURE" "1:$1" "DIR:$DIR" "BASE:$BASE"
	BASE=${BASE%$CLO_SUFFIX}
	[ -r "$DIR/${BASE}sim" ] && eval $2="$DIR/${BASE}sim" || eval $2=""
	#developer use 3 type of directory for <*.exp> files: 
	#1) for one set of TCs: $DIR/$UTE_TC_EXP_DIR/
	#2) for more than one set of TCs: $DIR/$UTE_TC_EXP_DIR/
	#3) for all TCs: $UTE_TC_EXP_DIR/
	[ $(echo $(expr match "$UTE_TC_EXP_DIR" "$DIR")) -gt 1 ] && eval $3="$UTE_TC_EXP_DIR/${BASE}exp" || eval $3="$DIR/$UTE_TC_EXP_DIR/${BASE}exp"
	[ -n "$4" ] && eval $4="$UTE_TC_OUT_DIR/${BASE}out"
	[ -n "$5" ] && eval $5="$UTE_TC_TRC_DIR/${BASE}trc"
	[ $CLO_TRACE -eq 1 ] && ute_print_variables "ute_get_derived_files" "!2:${!2}" "!3:${!3}" "!4:${!4}" "!5:${!5}"
} #}}}

function ute_read_till_separator()#{{{
{
# needed for read() not to ignore initial spaces
    IFS="";
    local line
    local -i missed="$1"
    while true; do
        read -r line || break
        [ "${line:0:SEP_LEN}" == "$SEPARATOR" ] && missed=missed-1
        [ $missed -gt 0 ] && echo "$line" || break
    done
    IFS=$' \t\n';        
} #}}}

function ute_read_from_separator()#{{{
{
# needed for read() not to ignore initial spaces
    IFS="";
    local line
    local -i skip="$1"
    while true; do
        read -r line || break
        [ "${line:0:SEP_LEN}" == "$SEPARATOR" ] && skip=skip-1
        [ $skip -le 0 ] && echo "$line"
    done
    IFS=$' \t\n';        
} #}}}

function ute_get_testcase_list()#{{{
{
	IS_STC_ALL_REQUIRED=0
	IS_STC_ALL_DEFINED=0    
	for i in ${!UTE_STC_ALL*}; do
		if [ "$i" == "UTE_STC_ALL" ]; then
			IS_STC_ALL_DEFINED=1
			break    
		fi
	done

	for i in $CLO_STC_LIST; do
		if [ "$i" == "UTE_STC_ALL" ]; then
			if [ $IS_STC_ALL_DEFINED == 1 ]; then
				#developer have defined UTE_STC_ALL -> use it
				[ -z "$TC_PAR" ] && TC_PAR="${!i}" || TC_PAR="$TC_PAR ${!i}"
			else
				#UTE_STC_ALL not defined -> generate content                
				for i in ${!UTE_STC_*}; do
					[ -z "$TC_PAR" ] && TC_PAR="${!i}" || TC_PAR="$TC_PAR ${!i}"
				done
			fi
			IS_STC_ALL_REQUIRED=1
			break
		fi
	done
	if [ $IS_STC_ALL_REQUIRED == 0 ]; then
		for i in $CLO_STC_LIST; do
			[ -z "$TC_PAR" ] && TC_PAR="${!i}" || TC_PAR="$TC_PAR ${!i}"
		done
	fi
	[ -z "$TC_PAR" ] && [ "$IS_STC_ALL_REQUIRED" == 0 ] && 
	{ 
		[ -z "$UTE_TC_DIR" ] && TC_PAR="." || TC_PAR="$UTE_TC_DIR" 
	}
	if [ "$TC_PAR" != "" ]; then 
		find $TC_PAR -name "*.$CLO_SUFFIX" -print | sort -u
	fi
} #}}}

function ute_compare_exp_and_out()#{{{
{
    OPTS="$exp $out $CNT $COLORS"
    [ $VERBOSE -ne 0 ] &&  OPTS="$OPTS -d"
    [ $DETAILED_DIFF -ne 0 ] && OPTS="$OPTS -dd"
    [ $COLOURS -ne 1 ] && OPTS="$OPTS -n"
    [ $CREATE_EXP -ne 0 ] && OPTS="$OPTS -ce"
    [ $NO_UNIFICATION -ne 0 ] && OPTS="$OPTS -nouni"
    [ $SILENT -ne 0 ] && OPTS="$OPTS -s >/dev/null"
    eval perl $UTE_DIR/ute.pl $OPTS
    [ $? != 0 ] && RESULT=1
    return $RESULT
} #}}}

function ute_run_act_bin()#{{{
{
#@    check_cfg_file "$i_in" "$i_dir"
    export LD_LIBRARY_PATH=${SAVED_LDLP}
    OPTS=""
    [ "$UTE_CPPUNIT_CLO" ] && { 
      [ "$OPTS" ] && OPTS="$OPTS $UTE_CPPUNIT_CLO" || OPTS="$UTE_CPPUNIT_CLO"
    }
#@    check_td_opt_file "$i_in" "$i_dir"
    [ "$TC_UTE_CPPUNIT_CLO" ] && OPTS="$OPTS $TC_UTE_CPPUNIT_CLO"
    [ $SILENT -ne 0 ] && OPTS="$OPTS &>/dev/null"
    BINARY="$UTE_BINARY_FILE_PATH/$UTE_BINARY_FILE"
    [ ! -x "$BINARY" ] && ute_error "Binary '$BINARY' missing!" && exit 1
    if [ $i_sim ]; then 
      if [ $CLO_DEBUG -eq 1 ]; then
	      export UTE_RUNARGS="runargs -i $i_in -s $i_sim -o $i_out $OPTS"
	      ute_debug "The arguments are exported as UTE_RUNARGS for $UTE_DEBUGGER: " blue "\"$UTE_RUNARGS\""
	      eval $UTE_DEBUGGER $BINARY 
      else
	      export UTE_RUNARGS="-i $i_in -s $i_sim -o $i_out $OPTS"
        ute_debug "" green "$BINARY " white "$UTE_RUNARGS"
        eval $BINARY $UTE_RUNARGS 2> test.err
      fi
    else
      if [ $CLO_DEBUG -eq 1 ]; then
	      export UTE_RUNARGS="runargs -i $i_in -o $i_out $OPTS"
	      ute_debug "The arguments are exported as UTE_RUNARGS for $UTE_DEBUGGER: " blue "\"$UTE_RUNARGS\""
	      eval $UTE_DEBUGGER $BINARY
      else
	      export UTE_RUNARGS="-i $i_in -o $i_out $OPTS"
        ute_debug "" green "$BINARY " white "$UTE_RUNARGS"
        eval $BINARY $UTE_RUNARGS 2> test.err
      fi
    fi 

    RESULT="$?"
    # Reseting $RESULT on value 0 if $RESULT value is defined in the $UTE_BINARY_FILE_IGNORE_RESULTS and set command line option "-ir"
    [ $CLO_IGNORE_RESULT -eq 1 ] && ute_ignore_results $RESULT;
    [ $RESULT -ne 0 ] && {
      cat test.err
      if [ $RESULT -gt 137 ]; then 
        ute_warning "Tested binary ended with " redbg "result=$RESULT" white "  probably CORE DUMPED"
      else
        ute_warning "Tested binary ended with " redbg "result=$RESULT"
      fi
    }
    return $RESULT
} #}}}

################################################################################
# UTE main
################################################################################
# tracks changes bash envinronment variables
if [ $CLO_VARIABLES_PRINT -eq 1 ]; then
  #storing set of default, ute_basic, ute_config, mte variables for comparison
  SET_DEFAULT="set_default.txt"
  SET_UTE_BASIC="set_ute_basic.txt"
  SET_UTE_CONFIG_BEFORE="set_ute_config_before.txt"
  SET_UTE_CONFIG_AFTER="set_ute_config_after.txt"
  SET_UTE="set_mte.txt"
  set > $SET_DEFAULT
fi

#storing set of ute_basic variables for comparison
[ $CLO_VARIABLES_PRINT -eq 1 ] && set > $SET_UTE_BASIC

################################################################################
# sets BRANCH, DIR, FILE
test -L "$0"
if [ $? -eq 0 ]; then
  file=$(ls -l $0 | awk -F\> '{print $2}' | sed 's/[* ]//g')
  ute_split_path "$file" UTE_DIR UTE_FILE
else
  ute_split_path "$0" UTE_DIR UTE_FILE
fi
ute_split_path "$TEST_PATH" TEST_VOB TEST_DIR
ute_split_path "$TEST_VOB" TEST_VOB TEST_UT

#prints variables: [main|function name] [variable_name:variable_value]+
[ $CLO_TRACE -eq 1 ] && ute_print_variables "main" "TEST_VOB:$TEST_VOB" "TEST_PATH:$TEST_PATH" "TEST_DIR:$TEST_DIR" "TEST_UT:$TEST_UT"

LIST_CONFIG_FILE=("$UTE_DIR/ute.config" "./ute.dev.config")
################################################################################
# some ute variables are set as dependences on command line options
if [ $# -ge 1 ]; then
  for command in ${@}; do
    case $command in
      -d)
      VERBOSE=1
      SILENT=0
      ;;
      -n)
      COLOURS=0 ;;
      -s)
      SILENT=1
      VERBOSE=0
      ;;
      -trc)
      CLO_TRACE=1               #prints all important variables during this script
      ;;
      -varp)
      CLO_VARIABLES_PRINT=1    #prints variables on the output
       ;;
      -rc)
      shift;
      LIST_CONFIG_FILE=(${LIST_CONFIG_FILE[@]} "$2")
      ;;
      *) ;;
    esac   
  done
fi

if [ $VERBOSE -eq 1 ]; then 
  ute_info "LOGNAME=$LOGNAME"
fi

ute_info "The directory for testing (current directory): '$TEST_DIR'"

#storing set of ute_config variables for comparison
[ $CLO_VARIABLES_PRINT -eq 1 ] && set > $SET_UTE_CONFIG_BEFORE

################################################################################
# loads <*.config> files

for icfg in  "${LIST_CONFIG_FILE[@]}"; do 
  if [ -r "$icfg" ]; then
    ute_debug "Loading UTE configuration file: " blue "$icfg"
    . "$icfg"
  elif [[ "$icfg" == "*ute\/ute.config*" ]]; then
    ute_error "Missing default UTE configuration file: " red "$icfg" 
    exit 1
  else 
    ute_warning "Missing developer's UTE configuration file: " yellow "$icfg"
  fi
done

# export unification file name for perl script (ute.pl)
#CHI as CLO to Perl
export UTE_TC_SUB
export UTE_DIFF

#storing set of <*.config> variables for comparison
[ $CLO_VARIABLES_PRINT -eq 1 ] && set > $SET_UTE_CONFIG_AFTER
#verify setting and prints configurations after reading <*.config> files
[ $CLO_VARIABLES_PRINT  -eq 1 ] && ute_print_configuration

################################################################################
# sets Command Line Option (CLO) for running

#UTE_BINARY_FILE_CLO is defined in the <*.config>
CLO=""
if [ $# -eq 0 ]; then
  if [ ${#UTE_BINARY_FILE_CLO} -eq 0 ]; then
    set -- "-s"
  else
    set -- $UTE_BINARY_FILE_CLO
  fi
  CLO="${@}"
else
  CLO="$UTE_BINARY_FILE_CLO ${@}" 
  set -- $CLO 
fi
ute_info "UTE is run with options: " green "$0$CLO"

################################################################################
# load command line arguments/options

while [ -n "$1" ]; do
  ute_parse_arg $1 $2 || shift
  shift
done

################################################################################
# anounces the configuration setting on the dependent CLO
[ $SILENT -eq 1 ] && ute_silent_start;

#developers function before testing
ute_testing_before

#CHI? [ -z $COMMAND ] && COMMAND="RUN"
#CHI? [ -z "$TC_PAR" ] && REGRESSION=1

if [ $VERBOSE -ne 0 ]; then
  #CHI?   ute_debug "Arguments given:"
  #CHI?   ute_debug " Command to process is '$COMMAND'"
  #CHI?    [ $REGRESSION -eq 0 ] && ute_debug " Manual module testing." || ute_debug " Regression test called."
  #CHI? [ $INTERACTIVE -eq 0 ] || ute_debug " Selection is interactive."
  [ -n "$TC_PAR" ] && [ $SILENT -eq 0 ] && printf "   Input file/dir = %s\n" $TC_PAR
fi

[ -n "$TC_PAR" ] && ute_debug "Got from command line: $TC_PAR"
ute_debug "Gathering list of testcases"

################################################################################
# set input files
if [ $INPUT_LIST_TC -eq 1 ]; then
  LIST_TC_ACT=($(cat $LIST_TC_FILE))
else
  LIST_TC_ACT=($(ute_get_testcase_list))
fi

COUNT_TC_ACT=${#LIST_TC_ACT[*]}
[ $COUNT_TC_ACT -lt 1 ] && ute_error "No testcase selected!" && exit 1

################################################################################
# wtites the list of Test Cases (TC)
if [ $INTERACTIVE -ne 0 ] && [ $COUNT_TC_ACT -gt 1 ]; then
  i=0
  ute_info "List of $COUNT_TC_ACT testcases to select:"
  while [ $i -lt $COUNT_TC_ACT ]; do
    i=$((i+1))
    printf "%-3d '%s'\n" $i "${LIST_TC_ACT[$i-1]}"
  done
  echo -e "\nEnter 'q' for quit\n"
  selected=0
  while [ $selected -lt 1 ] || [ $selected -gt $COUNT_TC_ACT ]; do
    echo -n "Select testcase number: "
    read selected
    if [ "$selected" == "" ]; then
      selected=0
    else 
      selected_tmp=`echo $selected | sed s/[0-9]//g`
      if [ "$selected_tmp" != "" ]; then
        if [ $selected == "q" ]; then
          echo "Testing exited by the developer"
          exit 1
        fi
        selected=0
      fi
    fi
  done
  # regenerate the list of testcases
  #@    TC_PAR="${LIST_TC_ACT[$selected-1]}"
  #@    LIST_TC_ACT=($(ute_get_testcase_list))
  #@    COUNT_TC_ACT=${#LIST_TC_ACT[*]}
  LIST_TC_ACT="${LIST_TC_ACT[$selected-1]}"
  COUNT_TC_ACT=1
fi

################################################################################
# mkdir directories, which are required
SAVED_LDLP="$LD_LIBRARY_PATH" #backups the LD_LIBRARY_PATH for ute_run_act_bin function

for ute_dir in "$UTE_TC_OUT_DIR" "$UTE_TC_TRC_DIR" "$UTE_TC_EXP_DIR"; do
  if [ ! -d $ute_dir ]; then
    mkdir -p $ute_dir
    ute_debug "The directory is created: " green "$ute_dir"
  fi
done

################################################################################
# runs TC/TCs
ute_i=0
while [ $ute_i -lt $COUNT_TC_ACT ]; do
  ute_debug "TEST_START_TIME = $(date '+%H:%M:%S')"
  i_in=${LIST_TC_ACT[ute_i]}
  [ $SILENT -eq 0 ] && echo
  ute_info "Testcase  " yellow "$i_in" cyan "  $((ute_i+1))/$COUNT_TC_ACT"
  ute_split_path "$i_in" i_dir i_name
  [ $CLO_TRACE -eq 1 ] && ute_print_variables "main" "i_in:$i_in" "i_dir:$i_dir" "i_name:$i_name"
  #loops component need to know name of TC, which is defined by name of directory
  export UTE_TC_NAME=$(echo ${i_name%.$CLO_SUFFIX})
  ute_get_derived_files "$i_in" i_sim i_exp i_out i_trc

  if [ $CLO_DIFF_OUT -eq 0 ]; then
    ute_tc_before "$i_in" "$i_out" "$i_exp" "$i_trc" "$CONFIG_FILE"

    ute_run_act_bin
    exp="$i_exp"
    out="$i_out"
    ute_tc_after "$i_in" "$out" "$exp" "$i_trc"
    [ $RESULT ] && ute_compare_exp_and_out
  elif [ $CLO_DIFF_OUT -eq 1 ]; then
    ute_tc_before "$i_in" "$i_out" "$i_exp" "$i_trc" "$CONFIG_FILE"
    #ute_compare_exp_and_out need to know $exp, $out variables
    exp="$i_exp"
    out="$i_out"
    ute_tc_after "$i_in" "$out" "$exp" "$i_trc"
    BINARY="$UTE_BINARY_FILE_PATH/$UTE_BINARY_FILE"
    #        BINARY="$UTE_BINARY_FILE_PATH/$UTE_BINARY_FILE_VERSION/$UTE_BINARY_FILE"
    ute_compare_exp_and_out
  fi

  TEST_END_TIME=`perl -e 'print int(time)'`
  ute_debug "TEST_END_TIME = $(date '+%H:%M:%S')"

  TEST_DURATION=$((TEST_END_TIME - TEST_START_TIME))
  ute_debug "TEST_DURATION = $TEST_DURATION [s]"

  if [ "$RESULT" -eq 0 ]; then 
    COUNTER_SUCCESSFUL_TC=$((COUNTER_SUCCESSFUL_TC+1));
    if [ $SILENT -eq 0 ]; then 
      ute_seperator
      ute_info "output   file: " yellow "$i_out"
      ute_info "expected file: " yellow "$i_exp" 
      ute_info "$i_in" green "  Successful"
      #        ute_info "" blue "Complete test status                                    " red "unsuccessful"
      ute_seperator
    else
      ute_silent_ok "$i_in"
    fi
    [ $STORE_SUCCESSFUL_TC -eq 1 ] && 
    { 
      echo $i_in >> $UTE_TC_SUCCESSFUL 
    }
  else
    COUNTER_FAILED_TC=$((COUNTER_FAILED_TC+1));
    if [ $SILENT -eq 0 ]; then 
      ute_seperator
      ute_info "output   file: " yellow "$i_out"
      ute_info "expected file: " yellow "$i_exp" 
      ute_error "$i_in" red "  Failed!"
      #        ute_info "" blue "Complete test status                                    " red "unsuccessful"
      ute_seperator
    else
      #        ute_silent_error "$i_in" red "         ERROR"
      ute_silent_error "$i_in"
    fi
    [ $STORE_FAILED_TC -eq 1 ] && 
    { 
      echo $i_in >> $UTE_TC_FAILED
    }
  fi
  GLOBAL_RESULT=$(($GLOBAL_RESULT + $RESULT))
  ute_i=$((ute_i+1))
done

################################################################################
# developer function after testing + clean + end function
ute_testing_after

if [ -e test.err ];then
  rm test.err
fi

# ute_info "Finish ($$)"
ute_end;

################################################################################
# resetting Time Zone
if [ $B_TZ ]; then
  export TZ=$B_TZ
fi

################################################################################
#storing set of mte variables for comparison
[ $CLO_VARIABLES_PRINT -eq 1 ] && set > $SET_UTE &&
{
  ute_info "UTE generates files with variables of environment:"
  for  file in ${!SET_*};do
    if [ -r ${!file} ]; then 
      printf "%+25s: \033[1;32m$(ls -la ${!file})\n\033[m" $file 
      #        rm ${!file}
    fi
  done 
}

[ $GLOBAL_RESULT -ne 0 ] && GLOBAL_RESULT=1
exit $GLOBAL_RESULT

# vim: ts=2 sw=2 et fdm=marker
