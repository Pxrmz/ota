#!/bin/bash
# dependencies libfoundation.so
# dependencies command xsltproc

ADM_CORE_UNITTEST_REPORT_PATH='./adm_core_unittest_report'
./adm_core_unittest --gtest_output=xml:${ADM_CORE_UNITTEST_REPORT_PATH}.xml
ADM_CORE_UNITTEST_RESULT=$?
if [ -f ${ADM_CORE_UNITTEST_REPORT_PATH}.xml ]; then
  xsltproc ./adm_gtest2html.xslt ${ADM_CORE_UNITTEST_REPORT_PATH}.xml > ${ADM_CORE_UNITTEST_REPORT_PATH}.html
fi
exit ${ADM_CORE_UNITTEST_RESULT}
