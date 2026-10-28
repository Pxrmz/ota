# !/bin/bash

function pr_info() {
    echo "### $@ ###"
}

function pr_err() {
    echo "### $@ ###"
}

function normalize_path() {
    if [[ -d "$1" ]]; then
        pushd "$1" > /dev/null 2>&1
        echo "$(pwd -P)"
    else
        pushd "$(dirname "$1")" > /dev/null 2>&1
        echo "$(pwd -P)/$(basename "$1")"
    fi

    popd > /dev/null 2>&1
}

function strreplace() { #replace param in string, return 1 on failure
    strreplace_orig="${1}"
    [ -n "${3}" ] || return 1 #nothing to search for: error
    strreplace_srch="${2}"  #pattern to replace
    strreplace_rep="${3}"   #replacement string
    case "${strreplace_orig}" in
        *"${strreplace_srch}"*) #if pattern exists in the string
            strreplace_sr1="${strreplace_orig%%$strreplace_srch*}" #take the string preceding the first match
            strreplace_sr2="${strreplace_orig#*$strreplace_srch}"  #and the string following the first match
            strreplace_orig="${strreplace_sr1}${strreplace_rep}${strreplace_sr2}" #and sandwich the replacement string between them
            ;;
    esac
    printf "%s" "${strreplace_orig}"
}

