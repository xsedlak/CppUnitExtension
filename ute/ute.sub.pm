package ute_unify;
#The unifications are aplied always, even if used command line option "-nodsub" switchs of default substitutions defined by list @unifications = (); in the file ute.pl

#These unifications are aplied to output file only (e.g. <*.out>), not are aplied to expected file(e.g. <*.exp>)
@own_permanent_unifications = (
    );

#Example:
#@own_permanent_unifications = (
#    's/^(Time:).*$/\1/',
#    's/_/#/'
#    );

#These one line unifications are aplied to output and expected file (e.g. <*.out>, <*.exp>).
@own_unifications = (
    );

#Example:
#@own_unifications = (
#    's/^[0-9a-f]{9}$/ptr/'
#    );

#These multi line unifications are aplied to output and expected file (e.g. <*.out>, <*.exp>).
@own_multiline_unifications = (
    );

#Example:
#@own_multiline_unifications = (
#   's/ \n//'    
#    );

#This list defines sections which should be skipped to output and expected file (e.g. <*.out>, <*.exp>).
@own_skip_sections = (
    );

#This number define return value of including this part of perl script, it must be here!
1

# vim: ts=2 sw=2 et fdm=marker
