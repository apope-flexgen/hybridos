fout=""
cat run2.out | \
    while read ff ; do  if [ "$ff" != "" ]; then fout=$fout$ff; else echo "fout = $fout gg=gg" ;$fout; fout="";fi; done


