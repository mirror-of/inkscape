# inkscape(1) completion 
# put this file in /etc/bash_completion.d/ 
# allali@univ-mlv.fr

have inkscape &&
_inkscape()
{
        local cur

        COMPREPLY=()
        cur=${COMP_WORDS[COMP_CWORD]}

        if [[ "$cur" == -* ]]; then
                COMPREPLY=( $( compgen -W '-? --help -V --version \
			-g --with-gui -p --pipe \
			-o --export-filename= --export-overwrite --export-type=\
			--export-png= -d --export-dpi= -a --export-area= \
			-w --export-width= -h --export-height= -i --export-id= \
			-j --export-id-only  -t --export-use-hints -b --export-background= \
			--action-list --select= --actions= --batch-process --pdf-page= \
			-y --export-background-opacity= ' -- $cur ) )
        else
                _filedir '@(ai|ani|bmp|cur|dia|eps|gif|ggr|ico|jpe|jpeg|jpg|pbm|pcx|pdf|pgm|png|ppm|pnm|ps|ras|sk|svg|svgz|targa|tga|tif|tiff|txt|wbmp|wmf|xbm|xpm)'
        fi

}
[ "${have:-}" ] && complete -F _inkscape $filenames inkscape
