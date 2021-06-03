#!/usr/bin/env python
#SBATCH -J EICSTUDY    # job name
#SBATCH -o logs/simple-%A_%a.out
#SBATCH -e logs/simple-%A_%a.out
#SBATCH -n 1
#SBATCH -c 1
#SBATCH --mem=128G
#SBATCH -p htc             # queue (partition) -- batch, parallel, etc.  parallel-medium
#SBATCH -t 08:00:00        # run time (hh:mm:ss)
#SBATCH -D .               # Directory where executable will be run
#SBATCH --mail-user=ssekula@smu.edu
#SBATCH --mail-type=fail    # email me when the job finishes


# This script was written originally for use on a SLURM-based batch system,
# like the one available at SMU ("ManeFrame II"). It can be run on the command-line
# alone; if the script doesn't detect the requested SLURM environment variables,
# it will ask you to specify them. For instance, to run the first variation in a
# study,
#
# SLURM_ARRAY_TASK_ID=0 ./simpleanalysis.py --input <DIRECTORY CONTAINING FILES> --name <OUTPUT DIRECTORY>
#
#

import subprocess
import math
import os
import sys
import shutil
import glob
import re
import ast

import argparse


parser = argparse.ArgumentParser()

parser.add_argument("-s", "--script", type=str,
                    help="ROOT Plotting Script to execute")
parser.add_argument("-a", "--arguments", type=str,
                    help="string of arguments as they should appear when passes to the script")


global args
args = parser.parse_args()

# Execute the study
subprocess.call("root -q -l -b {0[script]}'+({0[arguments]})'".format(
    {'script': args.script, 'arguments': args.arguments}), shell=True)
