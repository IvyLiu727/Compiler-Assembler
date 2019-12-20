#!/bin/bash

echo cs241.binasm '<' $1.asm '>' $1.mips
cs241.binasm < $1.asm > $1.mips
if [ $? -eq 0 ]
  then
    echo mips.array $1.mips
    mips.array $1.mips
fi
