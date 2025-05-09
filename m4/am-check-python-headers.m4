dnl a macro to check for ability to create python extensions
dnl  AM_CHECK_PYTHON_HEADERS([ACTION-IF-POSSIBLE], [ACTION-IF-NOT-POSSIBLE])
dnl function also defines PYTHON_INCLUDES
AC_DEFUN([AM_CHECK_PYTHON_HEADERS],
[AC_REQUIRE([AM_PATH_PYTHON])
dnl deduce PYTHON_INCLUDES
AC_MSG_CHECKING([for python headers using $PYTHON-config --includes])
PYTHON_INCLUDES=`$PYTHON-config --includes`
if test $? = 0; then
  AC_MSG_RESULT([$PYTHON_INCLUDES])
else
  AC_MSG_RESULT([failed, will try another way])
  py_prefix=`$PYTHON -c "import sys; print(sys.prefix)"`
  py_exec_prefix=`$PYTHON -c "import sys; print(sys.exec_prefix)"`
  PYTHON_INCLUDES="-I${py_prefix}/include/python${PYTHON_VERSION}"
  if test "$py_prefix" != "$py_exec_prefix"; then
    PYTHON_INCLUDES="$PYTHON_INCLUDES -I${py_exec_prefix}/include/python${PYTHON_VERSION}"
  fi
  AC_MSG_NOTICE([setting python headers to $PYTHON_INCLUDES])
fi
dnl check if the headers exist:
AC_MSG_CHECKING([whether python headers are sufficient])
AC_SUBST(PYTHON_INCLUDES)
save_CPPFLAGS="$CPPFLAGS"
CPPFLAGS="$CPPFLAGS $PYTHON_INCLUDES"
AC_PREPROC_IFELSE([AC_LANG_SOURCE([[#include <Python.h>]])],dnl
[AC_MSG_RESULT(yes)
$1],dnl
[AC_MSG_RESULT(no)
$2])
CPPFLAGS="$save_CPPFLAGS"
])
