#!/bin/bash

make "$@" 2>&1 | tee m.log
exit ${PIPESTATUS[0]}
