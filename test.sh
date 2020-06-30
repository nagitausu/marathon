#!/bin/bash

path="samples"
files=`find $path -type f`

for file in $files;
do
  echo $file
  cat $file | ./heuristic 500
done