#!/bin/bash -e
### 
### my-script — does one thing well
### @描述：为vusproj的所有源码仓库打上对应的TAG信息
### 
### Usage:
###   my-script <input_1> <input_2>
###
### Options:
###   <input_1> 
###     @描述: 代表需要提交的库标识
###     @取值：tag，代表为所有的源码仓库，打上版本TAG
### 
###   <input_2>
###     @描述: 代表提交的tag名称
###     @取值：此次发布的版本号，例如：v1.0.3/...
###   <input_3>
###     @描述: 代表提交的分支名
###     @取值：此次发布的分支名，例如：develop/master/...
###   -h        Show this message.

function help() {
    awk -F'### ' '/^###/ { print $2 }' "$0"
}

if [[ $# == 0 ]] || [[ "$1" == "-h" ]]; then
	help
	exit 1
fi

if [ $# -ne 3 ]; then
    echo "$0 version platform "
    exit 1
fi

TOPDIR=$(pwd)
BRANCH=$3


# 对code仓库提交对应的TAG
function git_tag() {
    git checkout $BRANCH
    git tag -a $1 -m "release $1"
    git push origin $1
}

# 调用所有的code仓库，提交TAG
function make_tag() {
    cd $TOPDIR/../foundation
    git_tag $1
    echo "git_tag $1 to $TOPDIR/../foundation"
    cd $TOPDIR/../engine
    git_tag $1
    echo "git_tag $1 to $TOPDIR/../engine"
    cd $TOPDIR/../arpc
    git_tag $1
    echo "git_tag $1 to $TOPDIR/../arpc"
    cd $TOPDIR/../ewebserver
    git_tag $1
    echo "git_tag $1 to $TOPDIR/../ewebserver"
    cd $TOPDIR/../tools
    git_tag $1
    echo "git_tag $1 to $TOPDIR/../tools"
    cd $TOPDIR/../cmake
    git_tag $1
    echo "git_tag $1 to $TOPDIR/../cmake"
    # cd $TOPDIR/../vrpc
    # git_tag $1
    # echo "git_tag $1 to $TOPDIR/../vrpc"
}

if [ $1 == "tag" ]; then
    make_tag $2
else
    help
    exit 1
fi


