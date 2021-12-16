#!/bin/bash

set -e
set -o pipefail

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd ${DIR}

# rbggamemanager
echo "*** preparing gamemanager ***"
./rbggamemanager/scripts/make.sh

# rbgParser
echo "*** preparing rbgParser ***"
cd rbg2cpp/rbgParser
make rbgParser
cd ${DIR}

# rbg2cpp
echo "*** preparing rbg2cpp compiler ***"
cd rbg2cpp
make rbg2cpp
cd ${DIR}

# generate configuration files
python3 rbgPlayer/generate_agents.py

# create directory for logs
mkdir -p logs

echo "Configuration succeeded"
