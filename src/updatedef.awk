#!/usr/bin/awk -f

function load_def(filename, DEF,     A) {
  while ((getline < filename)>0) {
    gsub(/;.*/, ""); gsub(/\s*/, ""); gsub(/NONAME/, "")
    if (split($0, A, "@") == 2) {
      DEF[A[1]] = A[2] + 0
    }
  }
  close(filename)
}

function lookup_empty(DEF,     max, symbol) {
  for (symbol in DEF) {
    if (DEF[symbol] > max) max = DEF[symbol]
  }
  return max + 1
}

# merge NEW into ORIG keeping backwards compatibility
function merge_def(ORIG, NEW,     symbol) {
  for (symbol in NEW) {
    if (!(symbol in ORIG)) {
      ORIG[symbol] = lookup_empty(ORIG)
    }
  }
}

function output_def(filename, DEF,     symbol) {
  print "EXPORTS" >filename
  for (symbol in DEF) {
    print "\t" symbol " @ " DEF[symbol] " NONAME" >filename
  }
  close(filename)
}

BEGIN {
  load_def("lua53.def", ORIG)
  system("arm-epoc-pe-dlltool -z lua53-new.def " ARGV[1])
  load_def("lua53-new.def", NEW)
  merge_def(ORIG, NEW)
  output_def("lua53.def", ORIG)
  system("rm lua53-new.def")
}
