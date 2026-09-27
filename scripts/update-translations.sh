#!/bin/bash
# Regenerate the translation catalogues from the source with lupdate.
set -e
cd "/d/Users/Joseph/Downloads/CPMTG20230511/CPMTG(English)/src - vibe" || exit 1
export PATH=/usr/bin:/ucrt64/bin:$PATH

mkdir -p i18n

lupdate qt tools/uishot.cpp \
    -ts i18n/cpmtoolsgui_en.ts \
        i18n/cpmtoolsgui_ja.ts \
        i18n/cpmtoolsgui_zh_CN.ts \
        i18n/cpmtoolsgui_zh_TW.ts \
        i18n/cpmtoolsgui_it.ts \
        i18n/cpmtoolsgui_fr.ts \
        i18n/cpmtoolsgui_de.ts \
        i18n/cpmtoolsgui_es.ts \
    -no-obsolete

echo "--- generated ---"
ls -la i18n/
