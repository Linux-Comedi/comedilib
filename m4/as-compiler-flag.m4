dnl as-compiler-flag.m4 0.0.1
dnl autostars m4 macro for detection of compiler flags
dnl
dnl ds@schleef.org

AC_DEFUN([AS_COMPILER_FLAG],
[
  AC_MSG_CHECKING(to see if compiler understands $1)

  save_CFLAGS="$CFLAGS"
  CFLAGS="$CFLAGS $1"

  AC_COMPILE_IFELSE([AC_LANG_PROGRAM([], [])], [flag_ok=yes], [flag_ok=no])
  CFLAGS="$save_CFLAGS"

  if test "X$flag_ok" = Xyes; then
    $2
  else
    $3
  fi
  AC_MSG_RESULT([$flag_ok])
])

